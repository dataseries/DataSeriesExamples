#
# (c) Copyright 2012, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# make [PREFIX=/path/to/root/of/dataseries/install]
# PREFIX un-needed if installed from binary packages
# PREFIX=$BUILD_OPT if installed from source
PREFIX=/usr

CFLAGS := -I$(PREFIX)/include `pkg-config --cflags libxml-2.0` -g
LDFLAGS := -L$(PREFIX)/lib -lDataSeries

PROGRAMS := ds-simple-write ds-simple-read merge-example processing \
  running_multiple_modules writing_a_file

all: $(PROGRAMS)

%: %.cpp
	g++ $(CFLAGS) -o $@ $< $(LDFLAGS)

test:
	./test-ds-simple-read-write.sh $(PREFIX)
	./test-writing_a_file.sh $(PREFIX)

clean:
	-rm -f $(PROGRAMS) *\~

