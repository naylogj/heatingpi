#!/usr/bin/env python

# V2 For Multiple Lines with irregular time sequence
# G. Naylor November 3rd 2014

# this is what the html data format should look like
# series: [{
#            name: 'Hall',
#            data: [
#		[Date.UTC(1970, 9, 27, 8, 15), 7.0],
#		[Date.UTC(1970, 9, 27, 9, 15), 10.1]
#	       	] 
#        }, {
#            name: 'Lounge',
#            data: [
#		[Date.UTC(1970, 9, 27, 8, 15), 7.0],
#		[Date.UTC(1970, 9, 27, 9, 15), 10.1]
#	       	] 
#        }, {
#            name: 'Study',
#            data: [
#		[Date.UTC(1970, 9, 27, 8, 15), 7.0],
#		[Date.UTC(1970, 9, 27, 9, 15), 10.1]
#	       	] 
#        }, {
#            name: 'Kitchen',
#            data: [
#		[Date.UTC(1970, 9, 27, 8, 15), 7.0],
#		[Date.UTC(1970, 9, 27, 9, 15), 10.1]
#	       	] 
#	}
#        }]
#
# include libs as necessary
import sqlite3

# setup variables
DB="/home/pi/heatpi/db/test.db"		# alter as necessary

# open output file
html_file = open("/home/pi/heatpi/html/data.html", "w")

# write start of data sequence

html_file.write("series: [\n")


conn=sqlite3.connect(DB)
curs=conn.cursor()

for zone in ('Hall','Lounge','Study','Kitchen'):
	html_file.write("{\n")
	html_file.write("\t\tname: '")
	html_file.write(zone)
	html_file.write("', \n")
	html_file.write("\t\tdata: [\n")

	for row in curs.execute("SELECT tdate, ttime, temperature FROM temps WHERE tdate=date('now') AND zone=(?)",(zone,)):

		# date format is 1970-11-01 need 1970, 11, 01
		# need to subtract 1 from the month because of HighCharts
		# time format is 13:11:01 need 1970, 11, 01
		# ddate=str(row[0]).replace("-",", ")
		ddate=str(row[0]).split('-')
		m = int(ddate[1])-1
		ddates = ddate[0] + ", " + str(m) + ", " + ddate[2]
		dtime=str(row[1]).replace(":",", ")
		html_file.write("\t\t[")
		html_file.write("Date.UTC(")
		html_file.write(ddates)
		html_file.write(", ")
		html_file.write(dtime)
		html_file.write("), ")
		html_file.write(str(row[2]))
		html_file.write("], \n")	

	html_file.write("]\n}, ")
	

html_file.write("\n]\n")

conn.close()
html_file.close()
