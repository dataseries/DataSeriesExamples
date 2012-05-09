#!/bin/sh
#
# (c) Copyright 2012, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Script for testing the ds-simple-{write,read}.cpp programs

if [ $# = 0 -o ! -d "$1" ]; then
    echo "Usage: $0 <install-prefix>"
    exit 1
fi

set -e -x

LD_LIBRARY_PATH=$1/lib:$1/lib64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
PATH=$1/bin:$PATH
[ -x $1/bin/ds2txt ]
./ds-simple-write 10000 abc r 20000 30000 test-ds-simple.ds

cat >test-ds-simple.ref <<EOF
# Extent Types ...
<ExtentType name="DataSeries: ExtentIndex">
  <field type="int64" name="offset" />
  <field type="variable32" name="extenttype" />
</ExtentType>

<ExtentType name="DataSeries: XmlType">
  <field type="variable32" name="xmltype" />
</ExtentType>

<ExtentType namespace="http://www.fsl.cs.sunysb.edu/" name="Trace::Fictitious::Linux" version="0.1" comment="extent comment does NOT go to the ds file">
  <field type="int64" name="timestamp" comment="field comment does NOT go to the ds file"/>
  <field type="variable32" name="filename"/>
  <field type="byte" name="opcode" comment="0=read 1=write 2=other" />
  <field type="int64" name="offset"/>
  <field type="int64" name="iosize"/>
</ExtentType>

# Extent, type='Trace::Fictitious::Linux'
timestamp filename opcode offset iosize
10000 abc 0 20000 30000
EOF

ds2txt --skip-index test-ds-simple.ds >test-ds-simple.out
diff -c test-ds-simple.out test-ds-simple.ref

cat >test-ds-simple.ref <<EOF
row	tstmp	fname	opcode	offset	iosize
0	10000	abc	r	20000	30000
1 row(s) processed
EOF

./ds-simple-read test-ds-simple.ds >test-ds-simple.out
diff -c test-ds-simple.out test-ds-simple.ref

rm test-ds-simple.ds test-ds-simple.out test-ds-simple.ref
