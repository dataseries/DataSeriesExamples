// -*-C++-*-
/*
   (c) Copyright 2008 Harvey Mudd College
   (c) Copyright 2012 Hewlett Packard Development Company, LP

   See the file named COPYING for license details
*/

#include <Lintel/StringUtil.hpp>

#include <DataSeries/ExtentType.hpp>
#include <DataSeries/DataSeriesFile.hpp>
#include <DataSeries/DataSeriesModule.hpp>

using namespace std;
const char* original_records[] = {
    "read 1 100 100",
    "write 2 50",
    "read 3 12 12",
    "write 4 32768",
    "write 5 523",
    "read 6 70 59",
    "write 7 50",
    "write 8 50"
};

int main() {
    /*  First we need to create XML descriptions of the Extents that we mean to use.  For this
        example we will use two ExtentTypes--one for reads and one for writes.  DataSeries doesn't
        like to mix different types of records, so we need separate types for reads and writes if
        we want each field to be non-null. */

    const char* read_xml =
        "<ExtentType name=\"ExtentType1\" version=\"1.0\" namespace=\"test.example.org\" >"
        "  <field name=\"timestamp\" type=\"int64\" />"
        "  <field name=\"requested_bytes\" type=\"int64\" />"
        "  <field name=\"actual_bytes\" type=\"int64\" />"
        "</ExtentType>\n";

    const char* write_xml =
        "<ExtentType name=\"ExtentType2\" version=\"1.0\" namespace=\"test.example.org\" >"
        "  <field name=\"timestamp\" type=\"int64\" />"
        "  <field name=\"bytes\" type=\"int64\" />"
        "</ExtentType>\n";

    /*  Next we need to put both of these in an ExtentType library which
        will be the first thing written to the file. */

    ExtentTypeLibrary types_for_file;
    const ExtentType::Ptr read_type = types_for_file.registerTypePtr(read_xml);
    const ExtentType::Ptr write_type = types_for_file.registerTypePtr(write_xml);

    /*  Then we open a file to write the records to. This will overwrite an existing file.
        DataSeries files can not be updated once closed. */

    DataSeriesSink output_file("writing_a_file.ds");
    output_file.writeExtentLibrary(types_for_file);

    /*  Now, we create structures for writing the individual
        types.  Note that each type of extent is stored separately.
        The ExtentSeries will allow us to set the fields in each
        record that we create.  We'll split the records into
        approximately 1024 byte chunks uncompressed. */

    ExtentSeries read_series;
    OutputModule read_output(output_file, read_series, read_type, 1024);
    ExtentSeries write_series;
    OutputModule write_output(output_file, write_series, write_type, 1024);

    /*  These are handles to the fields in the "current record" of
        each ExtentSeries. */

    Int64Field read_timestamp(read_series, "timestamp");
    Int64Field read_requested_bytes(read_series, "requested_bytes");
    Int64Field read_actual_bytes(read_series, "actual_bytes");

    Int64Field write_timestamp(write_series, "timestamp");
    Int64Field write_bytes(write_series, "bytes");

    size_t nrecords = sizeof(original_records) / sizeof(original_records[0]);
    for (size_t i=0; i < nrecords; ++i) {
        const char *record = original_records[i];

        vector<string> parts = split(record, " ");
        SINVARIANT(!parts.empty());

        if (parts[0] == "read") {
            SINVARIANT(parts.size() == 4);
            /*  Create a new record and set its fields. */
            read_output.newRecord();
            read_timestamp.set(stringToInteger<int64_t>(parts[1]));
            read_requested_bytes.set(stringToInteger<int64_t>(parts[2]));
            read_actual_bytes.set(stringToInteger<int64_t>(parts[3]));
        } else if (parts[0] == "write") {
            /*  Create a new record and set its fields. */
            write_output.newRecord();
            write_timestamp.set(stringToInteger<int64_t>(parts[1]));
            write_bytes.set(stringToInteger<int64_t>(parts[2]));
        }
    }

    /*  At this point the destructors will kick in.
        - the OutputModules will flush the Extents that
          they are currently holding
        - the DataSeriesFile will be closed. */
}
