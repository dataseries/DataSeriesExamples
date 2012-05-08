#!/bin/sh
set -e -x
case `pwd` in
   */DataSeriesExamples | */DataSeriesExamples.0.20[0-9][0-9].[0-9][0-9].[0-9][0-9])
      : 
      ;;
   *) echo "Being executed in the wrong directory, should be in .../DataSeriesExamples"
      exit 1
      ;;
esac

. $HOME/tmp/make-dist/version

cwd=`pwd`
dir=`basename $cwd`
cd ..
[ "$dir" = "DataSeriesExamples-$RELEASE_VERSION" ] || ln -snf $dir DataSeriesExamples-$RELEASE_VERSION
tar cvvfhz DataSeriesExamples-$RELEASE_VERSION.tar.gz --exclude=DataSeriesExamples-$RELEASE_VERSION/.git --exclude=\*\~ DataSeriesExamples-$RELEASE_VERSION/
[ "$dir" = "DataSeriesExamples-$RELEASE_VERSION" ] || rm DataSeriesExamples-$RELEASE_VERSION

mv DataSeriesExamples-$RELEASE_VERSION.tar.gz /var/www/pb-sources
