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