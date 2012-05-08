#!/bin/sh
#
# (c) Copyright 2012, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Test program for triangle-find

SRCDIR=`dirname $0`
./triangle-find-ds $SRCDIR/few-edges.ds | grep -v reading >triangle-find-few-edges.out
cat >triangle-find-few-edges.ref <<EOF
18 edges
total of 6 triangles
EOF

if cmp triangle-find-few-edges.ref triangle-find-few-edges.out; then
    echo "Test passed."
    rm triangle-find-few-edges.ref triangle-find-few-edges.out
else
    echo "Error in triangle finding:"
    diff -c triangle-find-few-edges.ref triangle-find-few-edges.out
    exit 1
fi
