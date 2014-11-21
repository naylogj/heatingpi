#!/usr/bin/env python

# this is what the html data format should look like
# series: [{
#            name: 'Hall',
#            data: [7.0, 6.9, 9.5, 14.5, 18.2, 21.5, 25.2, 26.5, 23.3, 18.3, 13.9, 9.6]
#        }, {
#            name: 'Lounge',
#            data: [-0.2, 0.8, 5.7, 11.3, 17.0, 22.0, 24.8, 24.1, 20.1, 14.1, 8.6, 2.5]
#        }, {
#            name: 'Study',
#            data: [-0.9, 0.6, 3.5, 8.4, 13.5, 17.0, 18.6, 17.9, 14.3, 9.0, 3.9, 1.0]
#        }, {
#            name: 'Kitchen',
#            data: [3.9, 4.2, 5.7, 8.5, 11.9, 15.2, 17.0, 16.6, 14.2, 10.3, 6.6, 4.8]
#        }]
#
# include libs as necessary
import sqlite3

# open output file
html_file = open("/home/pi/heatpi/html/data.html", "w")

# write start of data sequence

html_file.write("series: [\n")


conn=sqlite3.connect('/home/pi/heatpi/db/test.db')
curs=conn.cursor()

for zone in ('Hall','Lounge','Study','Kitchen'):
	html_file.write("{\n")
	html_file.write("\t\tname: '")
	html_file.write(zone)
	html_file.write("', \n")
	html_file.write("\t\tdata: [")

	for row in curs.execute("SELECT temperature FROM temps WHERE tdate=date('now') AND zone=(?)",(zone,)):
		html_file.write(str(row[0]))
		html_file.write(", ")	

	html_file.write("]\n}, ")
	

html_file.write("\n]\n")

conn.close()
html_file.close()
