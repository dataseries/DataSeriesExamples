// 
// (c) Copyright 2012, Hewlett-Packard Development Company, LP
// 
//  See the file named COPYING for license details
//
// Many implementations of the problem at:
// https://github.com/vertica/Graph-Analytics----Triangle-Counting

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

#include <Lintel/AtomicCounter.hpp>
#include <Lintel/Clock.hpp>
#include <Lintel/HashMap.hpp>
#include <Lintel/ProgramOptions.hpp>
#include <Lintel/PThread.hpp>
#include <Lintel/StatsQuantile.hpp>

/// analysis module/program for edge_list_graph (ns = ssd.hpl.hp.com, version = 1.0)
#include <DataSeries/RowAnalysisModule.hpp>
#include <DataSeries/SequenceModule.hpp>
#include <DataSeries/TypeIndexModule.hpp>

/// big data set copy at:
/// Internet: http://apotheca.hpl.hp.com/pub/datasets/graph/triangles/edges-sorted-gz.ds
/// HP Intranet: http://fileserver-2.u.hpl.hp.com/datasets/graph/triangles/edges-sorted-gz.ds
using namespace std;
using boost::format;
using boost::shared_ptr;
using lintel::AtomicCounter;
using lintel::ProgramOption;

ProgramOption<string> po_fn("fn", "function to use for finding triangles. options:"
                            " dumb, lower-bound, lb-dup-paths, lb-reverse, lb-hash, intersect", 
                            "intersect");
ProgramOption<bool> po_no_parallel("no-parallel", "run the triangle finding in series");

#include "triangle_find.hpp"

class TriangleAnalysis : public RowAnalysisModule {
public:
    TriangleAnalysis(DataSeriesModule &source)
        : RowAnalysisModule(source), a(series, "a"), b(series, "b"), total(0), edge_list(),
          tmp_a(-1), tmp_edge(), start_reading(), max_node_id(-1), edges(0)
    {
        // dsstatgroupby edge basic a from edges-sorted-lzf.ds
        edge_list.resize(5000000); // actual is around 4.85m 
    }

    virtual ~TriangleAnalysis() { }

    virtual void firstExtent(const Extent &e) {
        start_reading = Clock::todTfrac();
    }

    virtual void processRow() {
        SINVARIANT(edge_list.size() > a.val());
        if (a.val() >= b.val()) {
            return; // ignore self edges, and the reverse edges.
        }
        ++edges;
        if (true) {
            // faster to bulk calculate all the relevant edges in one vector and then copy into
            // the shared edge list; also more memory efficient by > 10%
            if (tmp_a != a.val()) { 
                if (tmp_a >= 0) {
                    edge_list[tmp_a] = tmp_edge;
                }
                tmp_edge.clear();
                tmp_a = a.val();
                if (max_node_id < a.val()) {
                    max_node_id = a.val();
                }
            }
            SINVARIANT(tmp_edge.empty() || tmp_edge.back() < b.val());
            tmp_edge.push_back(b.val());
        } else {
            SINVARIANT(edge_list[a.val()].empty() || edge_list[a.val()].back() < b.val());
            edge_list[a.val()].push_back(b.val());
        }
    }

    virtual void printResult() {
        cout << format("%d edges\n") % edges;
        // findNHopDistr();
        // exit(0);
        if (!tmp_edge.empty()) {
            edge_list[tmp_a] = tmp_edge;
            tmp_edge.clear();
        }
        edge_list.resize(max_node_id + 1);
        Clock::Tfrac end_reading = Clock::todTfrac();

        CountFunction count_fn = NULL;
        if (po_fn.get() == "dumb") {
            count_fn = findTrianglesNestedLoops;
        } else if (po_fn.get() == "lower-bound") {
            count_fn = findTrianglesNestedLowerBound;
        } else if (po_fn.get() == "lb-dup-paths") {
            count_fn = findTrianglesNestedLowerBoundDupPaths;
        } else if (po_fn.get() == "lb-reverse") {
            count_fn = findTrianglesNestedLowerBoundReverse;
        } else if (po_fn.get() == "lb-hash") {
            count_fn = findTrianglesNestedLowerBoundHash;
        } else if (po_fn.get() == "intersect") {
            count_fn = findTrianglesSetIntersection;
        } else {
            FATAL_ERROR(format("Unknown function '%s'; try --help for list") % po_fn.get());
        }

        Clock::Tfrac start_find = Clock::todTfrac();
        if (po_no_parallel.get()) {
            count_fn(edge_list, 0, max_node_id + 1, 1000000);
        } else {
            parallelFind(edge_list, count_fn);
        }
        Clock::Tfrac end_find = Clock::todTfrac();

        cout << format("%.2fs reading %.2fs finding\n")
            % Clock::TfracToDouble(end_reading - start_reading)
            % Clock::TfracToDouble(end_find - start_find);
        // findTrianglesNestedLoops(edge_list, 0, edge_list.size(), 10000);
        // findTrianglesNestedLowerBound(edge_list, 0, edge_list.size(), 10000);
        // findTrianglesNestedLowerBoundDupPaths(edge_list);
        // findDupPaths();
    }

    // total of 285730264 triangles
    // ./edges ~/projects/Misc/edges/edges-sorted-lzf.ds  487.81s user 1.07s system 101% cpu 8:04.02 total

    // a -> b -> c and a -> b' -> c
    void findDupPaths() {
        HashMap<int32_t, int32_t> c_to_count;
        c_to_count.reserve(16384);

        int32_t total_paths = 0;
        int32_t total_pairs = 0;
        int32_t total_pairs_gt_1 = 0;
        for (int a = 0; a < edge_list.size(); ++a) {
            if ((a % 1024) == 0) {
                cout << format("# %d %d paths %d pairs %d pairs >1 path, %.2f%%\n") % a 
                    % total_paths % total_pairs % total_pairs_gt_1
                    % (100.0 * (double)total_pairs_gt_1 / total_pairs);
            }
            c_to_count.clear();
            vector<int32_t> &b_nodes(edge_list[a]);
            BOOST_FOREACH(int32_t b, b_nodes) {
                vector<int32_t> &c_nodes(edge_list[b]);
                BOOST_FOREACH(int32_t c, c_nodes) {
                    ++c_to_count[c];
                }
            }
            
            typedef HashMap<int32_t, int32_t>::value_type cc_vt;
            BOOST_FOREACH(cc_vt &v, c_to_count) {
                total_paths += v.second;
                ++total_pairs;
                if (v.second > 1) {
                    ++total_pairs_gt_1;
                    // cout << format("found %d -> %d * %d\n") % a % v.first % v.second;
                }
            }
        }
        cout << format("total of %d paths %d pairs and %d pairs with > 1 path, %.2f%%\n")
            % total_paths % total_pairs % total_pairs_gt_1
            % (100.0 * (double)total_pairs_gt_1 / total_pairs);
    }

    void findNHopDistr() {
        StatsQuantile distr2,distr3;
        for (int32_t a = 0; a < edge_list.size(); ++a) {
            if ((a % 16384) == 0) {
                cout << a << "\n";
            }
            double two_hop_count = 0, three_hop_count = 0;
            const vector<int32_t> &b_nodes(edge_list[a]);
            BOOST_FOREACH(int32_t b, b_nodes) {
                const vector<int32_t> &c_nodes(edge_list[b]);
                two_hop_count += c_nodes.size();
                BOOST_FOREACH(int32_t c, c_nodes) {
                    three_hop_count += edge_list[c].size();
                }
            }
            distr2.add(two_hop_count);
            distr3.add(three_hop_count);
        }
        distr2.printText(cout);
        distr3.printText(cout);
    }

private:
    Int32Field a;
    Int32Field b;

    int32_t total;
    vector< vector<int32_t> > edge_list;

    int32_t tmp_a;
    vector<int32_t> tmp_edge;
    Clock::Tfrac start_reading;
    int32_t max_node_id;
    int32_t edges;
};

int main(int argc, char *argv[]) {
    vector<string> args = lintel::parseCommandLine(argc, argv, true);
    TypeIndexModule *source = new TypeIndexModule("edge_list_graph");

    INVARIANT(!args.empty(), boost::format("Usage: %s [options] <file...>\n") % argv[0]);

    BOOST_FOREACH(string file, args) {
        source->addSource(file);
    }

    SequenceModule seq(source);
    
    seq.addModule(new TriangleAnalysis(seq.tail()));
    
    seq.getAndDeleteShared();
    RowAnalysisModule::printAllResults(seq);
    return 0;
}
