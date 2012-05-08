#
# (c) Copyright 2012, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# make [PREFIX=/path/to/root/of/dataseries/install]
# PREFIX un-needed if installed from binary packages
# PREFIX=$BUILD_OPT if installed from source
PREFIX=/usr

CFLAGS := -I$(PREFIX)/include `pkg-config --cflags libxml-2.0`
LDFLAGS := -L$(PREFIX)/lib -lDataSeries

PROGRAMS := ds-simple-write ds-simple-read merge-example processing \
  running_multiple_modules writing_a_file

all: $(PROGRAMS)

%: %.cpp
	g++ $(CFLAGS) -o $@ $< $(LDFLAGS)

writing_a_file: writing_a_file.cpp
	g++ $(CFLAGS) -o $@ $< $(LDFLAGS) -lboost_regex

clean:
	rm -f $(PROGRAMS)

