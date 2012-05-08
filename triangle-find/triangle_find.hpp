// 
// (c) Copyright 2012, Hewlett-Packard Development Company, LP
// 
//  See the file named COPYING for license details
//
// Header file for different triangle-find implementations.

#include <valgrind/callgrind.h>

#include <boost/thread/tss.hpp>

#include <Lintel/HashUnique.hpp>

static const int32_t default_print_after = 1000 * 1000;

int32_t findTrianglesNestedLoops(const vector< vector<int32_t> > &edge_list,
                                 int32_t start, int end, 
                                 int32_t print_after = default_print_after) {
    int32_t total = 0;
    int32_t last_total = 0;

    for (int32_t a = start; a < end; ++a) {
        if ((total - last_total) > print_after) {
            cout << format("%d %d found\n") % a % total;
            last_total = total;
        }
        const vector<int32_t> &b_nodes(edge_list[a]);
        BOOST_FOREACH(int32_t b, b_nodes) {
            if (b > a) {
                const vector<int32_t> &c_nodes(edge_list[b]);
                BOOST_FOREACH(int32_t c, c_nodes) {
                    if (c > b) {
                        const vector<int32_t> &d_nodes(edge_list[c]);
                        BOOST_FOREACH(int32_t d, d_nodes) {
                            if (a == d) {
                                ++total;
                            }
                        }
                    }
                }
            }
        }
    }
    if (start == 0 && end == edge_list.size()) {
        cout << format("total of %d triangles\n") % total;
    }
    return total;
}

int32_t findTrianglesNestedLowerBound(const vector< vector<int32_t> > &edge_list,
                                      int start, int end,
                                      int32_t print_after = default_print_after) {
    typedef vector<int32_t>::const_iterator vec_iter;

    int32_t total = 0;
    int32_t last_total = 0;

    for (int32_t a = start; a < end; ++a) {
        const vector<int32_t> &b_nodes(edge_list[a]);
        for(vec_iter b_a = lower_bound(b_nodes.begin(), b_nodes.end(), a+1);
            b_a != b_nodes.end(); ++b_a) {
            int32_t b = *b_a;

            const vector<int32_t> &c_nodes(edge_list[b]);
            for(vec_iter c_b = lower_bound(c_nodes.begin(), c_nodes.end(), b+1);
                c_b != c_nodes.end(); ++c_b) {
                int32_t c = *c_b;
                
                const vector<int32_t> &d_nodes(edge_list[c]);
                vec_iter d_a = lower_bound(d_nodes.begin(), d_nodes.end(), a);
                if (d_a != d_nodes.end() && *d_a == a) {
                    ++total;
                }
            }
        }
        if ((total - last_total) > print_after && (a % 1024) == 0) {
            cout << format("%d %d found\n") % a % total;
            last_total = total;
        }
    }
    if (start == 0 && end == edge_list.size()) {
        cout << format("total of %d triangles\n") % total;
    }
    return total;
}        

int32_t findTrianglesNestedLowerBoundDupPaths(const vector< vector<int32_t> > &edge_list,
                                              int start, int end,
                                              int32_t print_after = default_print_after) {
    
    typedef vector<int32_t>::const_iterator vec_iter;

    int32_t total = 0;
    int32_t last_total = 0;

    HashMap<int32_t, int32_t> c_to_count;

    for (int32_t a = start; a < end; ++a) {
        if ((total - last_total) > print_after) {
            cout << format("%d %d found\n") % a % total;
            last_total = total;
        }
        c_to_count.clear();

        const vector<int32_t> &b_nodes(edge_list[a]);
        for(vec_iter b_a = lower_bound(b_nodes.begin(), b_nodes.end(), a+1);
            b_a != b_nodes.end(); ++b_a) {
            int32_t b = *b_a;

            const vector<int32_t> &c_nodes(edge_list[b]);
            for(vec_iter c_b = lower_bound(c_nodes.begin(), c_nodes.end(), b+1);
                c_b != c_nodes.end(); ++c_b) {
                int32_t c = *c_b;
                
                ++c_to_count[c];
            }
        }

        typedef HashMap<int32_t, int32_t>::value_type cc_vt;
        BOOST_FOREACH(cc_vt &v, c_to_count) {
            const vector<int32_t> &d_nodes(edge_list[v.first]);
            vec_iter d_a = lower_bound(d_nodes.begin(), d_nodes.end(), a);
            if (d_a != d_nodes.end() && *d_a == a) {
                total += v.second;
            }
        }
    }
    if (start == 0 && end == edge_list.size()) {
        cout << format("total of %d triangles\n") % total;
    }
    return total;
}        

// NLB + lookup for a in c's edge list replaced by lookup for c in a's edge list
int32_t findTrianglesNestedLowerBoundReverse(const vector< vector<int32_t> > &edge_list,
                                             int start, int end,
                                             int32_t print_after = default_print_after) {
    typedef vector<int32_t>::const_iterator vec_iter;

    int32_t total = 0;
    int32_t last_total = 0;

    for (int32_t a = start; a < end; ++a) {
        const vector<int32_t> &b_nodes(edge_list[a]);
        for(vec_iter b_a = lower_bound(b_nodes.begin(), b_nodes.end(), a+1);
            b_a != b_nodes.end(); ++b_a) {
            int32_t b = *b_a;

            const vector<int32_t> &c_nodes(edge_list[b]);
            for(vec_iter c_b = lower_bound(c_nodes.begin(), c_nodes.end(), b+1);
                c_b != c_nodes.end(); ++c_b) {
                int32_t c = *c_b;
                
                // vec_iter c_a = lower_bound(b_nodes.begin(), b_nodes.end(), c);
                vec_iter c_a = lower_bound(b_a, b_nodes.end(), c);
                if (c_a != b_nodes.end() && *c_a == c) {
                    ++total;
                }
            }
        }
        if ((total - last_total) > print_after && (a % 1024) == 0) {
            cout << format("%d %d found\n") % a % total;
            last_total = total;
        }
    }
    if (start == 0 && end == edge_list.size()) {
        cout << format("total of %d triangles\n") % total;
    }
    return total;
}        

// NLB2 replacing lower_bound for closing lookup with hash-table lookup
int32_t findTrianglesNestedLowerBoundHash(const vector< vector<int32_t> > &edge_list,
                                          int start, int end,
                                          int32_t print_after = default_print_after) {
    typedef vector<int32_t>::const_iterator vec_iter;

    int32_t total = 0;
    int32_t last_total = 0;

    for (int32_t a = start; a < end; ++a) {
        const vector<int32_t> &b_nodes(edge_list[a]);
        
        if (b_nodes.empty()) {
            continue;
        }
        HashUnique<int32_t> a_edges;
        {
            // all nodes we can get to that can loop back are at least a+2
            vec_iter b_a = lower_bound(b_nodes.begin(), b_nodes.end(), a+2);
            size_t num = b_nodes.end() - b_a;
            a_edges.reserve(num);

//            if (a_edges.capacity() < num / 2) {
//                a_edges = HashUnique<int32_t>();
//                a_edges.reserve(num);
//            } else {
//                a_edges.clear();
//            }
            for(; b_a != b_nodes.end(); ++b_a) {
                a_edges.add(*b_a);
            }
        }

        for(vec_iter b_a = lower_bound(b_nodes.begin(), b_nodes.end(), a+1);
            b_a != b_nodes.end(); ++b_a) {
            int32_t b = *b_a;

            const vector<int32_t> &c_nodes(edge_list[b]);
            for(vec_iter c_b = lower_bound(c_nodes.begin(), c_nodes.end(), b+1);
                c_b != c_nodes.end(); ++c_b) {
                int32_t c = *c_b;
                
                if (a_edges.exists(c)) {
                    ++total;
                }
            }
        }
        if ((total - last_total) > print_after && (a % 1024) == 0) {
            cout << format("%d %d found\n") % a % total;
            last_total = total;
        }
    }
    if (start == 0 && end == edge_list.size()) {
        cout << format("total of %d triangles\n") % total;
    }
    return total;
}        

// Terence's algorithm -- set intersection
/* count triangles:  for every node pair a,b where b is a lower
   neighbor of a, does the set of a's lower neighbors intersect
   with the set of b's lower neighbors?  every node at intersection
   creates a triangle */
struct CheckIntersect {
    CheckIntersect(int32_t maxn) : bitmap()
    {
        bitmap.resize(maxn);
    }

    void set(int32_t v) {
        bitmap[v] = 1;
    }
    bool contains(int32_t v) {
        return bitmap[v];
    }
    bool clear(int32_t v) {
        bitmap[v] = 0;
    }

    vector<bool> bitmap;

};


int32_t findTrianglesSetIntersection(const vector< vector<int32_t> > &edge_list,
                                     int start, int end,
                                     int32_t print_after = default_print_after) {
    static boost::thread_specific_ptr<CheckIntersect> thread_intersect_set;
    
    if (thread_intersect_set.get() == NULL) {
        // cout << "reset set intersection tsp\n";
        thread_intersect_set.reset(new CheckIntersect(edge_list.size()));
    }

    CheckIntersect &intersect_set(*thread_intersect_set.get());

    typedef vector<int32_t>::const_iterator vec_iter;

    int32_t total = 0;
    int32_t last_total = 0;

    CALLGRIND_ZERO_STATS;
    CALLGRIND_START_INSTRUMENTATION;
    uint64_t ncheck = 0, outer = 0;
    for (int32_t a = start; a < end; ++a) {
        const vector<int32_t> &b_nodes(edge_list[a]);
        
        if (b_nodes.size() < 2) { // must have at least 2 edges to have a hope
            continue;
        }

        for(vec_iter b_a = b_nodes.begin(); b_a != b_nodes.end(); ++b_a) {
            ++outer;
            int32_t b = *b_a;
            const vector<int32_t> &c_nodes(edge_list[b]);
            // Mark all the c nodes
            for (vec_iter c_a = c_nodes.begin(); c_a != c_nodes.end(); ++c_a) {
                intersect_set.set(*c_a);
            }
            // Check for closure (a->d)
            for (vec_iter d_a = b_nodes.begin(); d_a != b_nodes.end(); ++d_a) {
                ++ncheck;
                if (intersect_set.contains(*d_a)) {
                    ++total;
                }
            }
            // Reset vector
            for (vec_iter c_a = c_nodes.begin(); c_a != c_nodes.end(); ++c_a) {
                intersect_set.clear(*c_a);
            }
        }

        if ((total - last_total) > print_after && (a % 1024) == 0) {
            cout << format("%d %d found\n") % a % total;
            last_total = total;
        }
    }
    CALLGRIND_STOP_INSTRUMENTATION;
    if (start == 0 && end == edge_list.size()) {
        cout << format("checks: %d; outer %d\n") % ncheck % outer;
        cout << format("total of %d triangles\n") % total;
    }
    return total;
}        

typedef int32_t (*CountFunction)(const vector< vector<int32_t> > &edge_list,
                                 int32_t start, int32_t end, int32_t print_after);

class Worker : public PThread {
public:
    typedef shared_ptr<Worker> Ptr;

    Worker(const vector< vector<int32_t> > &edge_list,
           AtomicCounter &next_start, AtomicCounter &sum, AtomicCounter &last_sum,
           uint32_t batch_size, CountFunction count_fn)
        : edge_list(edge_list), next_start(next_start), sum(sum), last_sum(last_sum),
          batch_size(batch_size), count_fn(count_fn)
    { }

    virtual void * run() {
        uint32_t adaptive_batch_size = 1;
        while (1) {
            // int32_t my_end = next_start.addThenFetch(batch_size);
            // int32_t my_start = my_end - batch_size;

            int32_t my_end = next_start.addThenFetch(adaptive_batch_size);
            int32_t my_start = my_end - adaptive_batch_size;
            if (my_start > edge_list.size()) {
                return NULL;
            }

            if (my_end > edge_list.size()) {
                my_end = edge_list.size();
            }

            int32_t amt = count_fn(edge_list, my_start, my_end, 10000000);
            if (amt < 1000) {
                adaptive_batch_size *= 2;
                if (adaptive_batch_size > 4096) adaptive_batch_size = 4096;
                // cout << format("%d abs++ %d\n") % my_start % adaptive_batch_size;
            } else if (amt > 10000) {
                adaptive_batch_size /= 2;
                if (adaptive_batch_size < 1) adaptive_batch_size = 1;
                // cout << format("%d abs-- %d\n") % my_start % adaptive_batch_size;
            }
                
            // cout << format("%d -> %d\n") % my_start % amt;
            int32_t current_sum = sum.addThenFetch(amt);
            int32_t last = last_sum.addThenFetch(0);
            if ((current_sum - last) > default_print_after) {
                last_sum.addThenFetch(current_sum - last);
                cout << format("%d ~= %d\n") % my_start % current_sum;
            }
        }
    }

    const vector< vector<int32_t> > &edge_list;
    AtomicCounter &next_start, &sum, &last_sum;
    uint32_t batch_size;
    CountFunction count_fn;
};

void parallelFind(const vector< vector<int32_t> > &edge_list, CountFunction count_fn) {
    int ncpus = PThreadMisc::getNCpus();
    SINVARIANT(ncpus > 0);
    vector<Worker::Ptr> workers;
    AtomicCounter next_start, sum, last_sum;
    for(int i=0; i < ncpus; ++i) {
        workers.push_back(Worker::Ptr(new Worker(edge_list, next_start, sum, last_sum,
                                                 4, count_fn)));
        workers.back()->start();
    }

    for(int i=0; i < ncpus; ++i) {
        workers[i]->join();
    }

    cout << format("total of %d triangles\n") % sum.addThenFetch(0);
}

