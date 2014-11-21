#!/usr/bin/env python

import sqlite3

conn=sqlite3.connect('/home/pi/heatpi/db/test.db')

curs=conn.cursor()

print "\nEntire database contents:\n"
for row in curs.execute("SELECT * FROM temps"):
    print row

print "\nDatabase entries for the Lounge:\n"
for row in curs.execute("SELECT * FROM temps WHERE zone='Lounge'"):
    print row

conn.close()
