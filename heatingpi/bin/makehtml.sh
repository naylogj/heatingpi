#!/bin/bash
# script to re-make index.html and place in
# webserver directory
# called wiith no parameters
# expects the fragments to be in
# ~/html
# expects webserver root dir to be /var/www
#
#
# V1 26.10.2014
# G. Naylor
#

WWWDIR=/var/www
HTMLDIR=~/heatpi/html


# build html

cd $HTMLDIR
cat part1.html > index.html
cat data.html >> index.html
cat part2.html >> index.html
chgrp www-data index.html
cp -p index.html $WWWDIR/index.html

exit 0
