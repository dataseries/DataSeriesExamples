#!/bin/sh
#
# (c) Copyright 2012, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Script for testing the writing_a_file program

if [ $# = 0 -o ! -d "$1" ]; then
    echo "Usage: $0 <install-prefix>"
    exit 1
fi

set -e -x

LD_LIBRARY_PATH=$1/lib:$1/lib64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
PATH=$1/bin:$PATH
[ -x $1/bin/ds2txt ]
./writing_a_file

cat >writing_a_file.ref <<EOF
# Extent Types ...
<ExtentType name="DataSeries: ExtentIndex">
  <field type="int64" name="offset" />
  <field type="variable32" name="extenttype" />
</ExtentType>

<ExtentType name="DataSeries: XmlType">
  <field type="variable32" name="xmltype" />
</ExtentType>

<ExtentType name="ExtentType1" version="1.0" namespace="test.example.org" >  <field name="timestamp" type="int64" />  <field name="requested_bytes" type="int64" />  <field name="actual_bytes" type="int64" /></ExtentType>

<ExtentType name="ExtentType2" version="1.0" namespace="test.example.org" >  <field name="timestamp" type="int64" />  <field name="bytes" type="int64" /></ExtentType>

# Extent, type='ExtentType2'
timestamp bytes
2 50
4 32768
5 523
7 50
8 50
# Extent, type='ExtentType1'
timestamp requested_bytes actual_bytes
1 100 100
3 12 12
6 70 59
EOF

ds2txt --skip-index writing_a_file.ds >writing_a_file.out
diff -c writing_a_file.out writing_a_file.ref

rm writing_a_file.ds writing_a_file.out writing_a_file.ref
