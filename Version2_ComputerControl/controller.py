# how this code works
# 1. ensure you have the correct com port
# 	- you can find this in device manager
# 2. this program can be passed arguments (either 1 or 2)
# 	- 1 is for sequence and 2 for is for camera to shoot a picture
# 	- so if you run C:\Python27\python.exe controller.py 1
# 		the camera will shoot a sequence of photos, 1 flash at 
# 		a time, and then all flashes and then just ambient.
# 	# - if you run at 
# 3. the program also runs in interactive mode, so you can 
#  	trigger one mode or the other and it exits
# 4. The program will only exit after it reads in 2 lines
#	from the serial port. This is so that you can run the
# 	script within another script in a single thread, and have
#	the next step execute in a serial fashion. 

import serial
import time
import sys
# mac port name - /dev/cu.usbmodem1411
# pc port name - COM5
# ser = serial.Serial('/dev/cu.usbmodem1411', 9600)
ser = serial.Serial('COM4', 9600)
# time.sleep(2)
print "Serial port open?"
print ser.isOpen()
# time.sleep(2)
argIter = iter(sys.argv)
next(argIter)
for cmd in argIter: 
	print cmd
	b = cmd.encode()
	ser.write(bytearray(cmd + "\n"))
	time.sleep(0.05) #sleep for 50 ms between commands
	# ser.write(b'1')
# ser.open()
#  change this to run only if no args listed
if(len(sys.argv) == 1):
	print "Select action:\n"
	print "1 - shoot sequence"
	print "2 - take picture"
	action = input(":")
	ser.write(bytearray(str(action) + "\n"))
# print ser.write(b'1\n')
# time.sleep(1)
ser.readline()
ser.readline()

# while True:
	# ln = ser.readline()
	# if ln is "End Shooting Seq":
		# print "DONEZO"
		# break
# >>> x = ser.read()          # read one byte
# >>> s = ser.read(10)        # read up to ten bytes (timeout)
# >>> line = ser.readline()   # read a '\n' terminated line
# >>> ser.close()
# mac device port