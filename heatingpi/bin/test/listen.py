#!/usr/bin/env python3

# V2 Python program to listen for recvieved 
# temperature data via the USB serial port
# and insert into the database
#
# runs as a daemon process in the background
# started automatically at bootup
# 
# G. Naylor December 1st 2014
# V1 - first version
# V2 - added scheduled job to ask local gateway
#	moteino for its tempearature
#	migrated to Python V3
#
#

# include libs as necessary
import serial
import schedule
import time

DEBUG=2				# set to 1 for extra output to console

PERIOD=20			# time period in munutes to ask for a local temp

# setup variables
DB="/home/pi/heatpi/db/test.db"	# alter as necessary
BASE="/home/pi/heatpi"
SPORT="/dev/ttyUSB0"		# Serial port Moteino is attached to.

#  nodes holds the node id to zone location mapping
# this needs to reflect the zones table in the db

nodes = [ (0,'Gateway'),
          (1,'Hall'),
          (2,'Lounge'),
          (3,'Study'),
          (4,'Kitchen') ]

if DEBUG==2: print(nodes) ;


if DEBUG==1: 
	print(DB)
	print(BASE)
	print(SPORT)


# Funtion to ask the connected gateway moteino for 
# the local temperature by sending a "z" via USB Serial.

def get_local_temp():
	if DEBUG==2: print("\nZ: Asking for temperature\n");
	ser.write(('z\n').encode('utf-8'))	# send a z\n encoded as bytes


# open serial port to gateway moteino
# open the Serial port at 115200 baud, with a timeout of 0 seconds
# /dev/ttyUSB0 is normally the first (lower) USB connection on the Raspberry Pi
# Check ls -l /dev/ttyUS* after plugging in the USB cable and
# ammend accordingly
# ensure the sketch running on the moteino has the same BAUD rate!

try:
	ser = serial.Serial(port=SPORT,
	baudrate=115200,
	bytesize=serial.EIGHTBITS,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	timeout=3)
except:
	print("\nSerial Port cannot be opened.  Is Motenio connected to lower USB ?\n")
	exit()


if DEBUG: print("\nport, listening for data ....\n")


# setup schedular to ask for local temp every PERIOD minutes

schedule.every(PERIOD).seconds.do(get_local_temp)


while 1:
	try:
		schedule.run_pending()
		line = ser.readline()
		if DEBUG==2: print(line);
		# data will be format nodeid:temp*100
		# need to split out and divide temp by 100
		# but first need to convert to str data
		datastring=line.decode('utf-8')
		if DEBUG==2: print(datastring);
		data=str(datastring).split(':',2)
		try:
			node=data[0]
			temp=float(data[1])/100.00
			zone=str(nodes[int(node)][1])
			if DEBUG==2: print("Node is %s , Zone is %s, Temp is %s " % (str(node),zone,str(temp)));
		except:
			continue # if we get a corrupted data ignore it
	except:
		# close database connection
		ser.close()
		exit()
