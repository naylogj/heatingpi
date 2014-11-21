#!/bin/bash
# test script to place dummy data into the db
# called wiith
# $1 = file name of db
# $2 = number of days back from today
#
# V1 26.10.2014
# G. Naylor
#

BASE=~/heatpi
WWWDIR=/var/www
BINDIR=$BASE/bin
DBDIR=$BASE/db
DBFILE=$DBDIR/$1

echo
#echo "DBFILE = "$DBFILE

if [ ! -f "$DBFILE" ] 
then
	echo "ERROR Database file cannot be found."
	exit -1
fi

NUMDAYS=$2

if [ -z "$NUMDAYS" ]
then 
	echo "ERROR Number of days backwards not specified"
	exit -2
fi

# if we get here we have got correct variables.

SQLFILE=/tmp/sql.$$		# create a temp file to hold the sql we will execute at the end.

echo "BEGIN;" > $SQLFILE	# start of SQL statement.

# loop for the number of days
while [ $NUMDAYS -ge 0 ]
do
	# now add an entry for every hour	
	HR=00
	while [ $HR -lt 24 ]
	do
		# INSERT INTO temps values(date('now', '-1 day'), time('now'), "Hall", 19.6);
		# INSERT INTO temps values(date('now'), time('now', '-12 hours'), "Hall", 19.5);
		echo "INSERT INTO temps values(date('now', '"-$NUMDAYS" day'), time('00:01:34', '+"$HR" hours'), "\"Hall\"", 20.5);" >> $SQLFILE
		echo "INSERT INTO temps values(date('now', '"-$NUMDAYS" day'), time('00:01:34', '+"$HR" hours'), "\"Lounge\"", 19.5);" >> $SQLFILE
		echo "INSERT INTO temps values(date('now', '"-$NUMDAYS" day'), time('00:01:34', '+"$HR" hours'), "\"Study\"", 18.5);" >> $SQLFILE
		echo "INSERT INTO temps values(date('now', '"-$NUMDAYS" day'), time('00:01:34', '+"$HR" hours'), "\"Kitchen\"", 21.5);" >> $SQLFILE
		HR=`expr $HR + 1`
	done

NUMDAYS=`expr $NUMDAYS - 1`
done
echo "COMMIT;" >> $SQLFILE	# end of SQL statement.
echo
echo "SQL statements written to temporary file $SQLFILE "
echo 
echo "Use:"
echo "sqlite3 "$DBFILE" < "$SQLFILE
echo "to execute and insert data into db"
echo 
echo "CTRL-C to exit (does not delete "$SQLFILE") or press <enter> to insert data into database"
echo
read input
echo "Inserting data into database: "$DBFILE
echo
sqlite3 $DBFILE < $SQLFILE
rm $SQLFILE
