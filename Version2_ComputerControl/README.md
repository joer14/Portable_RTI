Python script is used to control the camera.

To make the relight 2 (one w/o white box) work in this mode
we disabled the "test shot" mode, so when the shutter button is
depressed it won't trigger the flash. The camera will only 
trigger the flash when it is doing a full sequence. 


The arduino software and python script are found in this same directory. 


How to use RELIGHT version 2 with computer control:

1. connected the usb cable to the computer and to the relight
2. connect the wall power adapter or the battery to the relight. 
	- you shouldn't need both. 
	- flip the power switch if it isn't working (but it should be
	already tapped in the correct position)
3. if need be upload this firmware again using the arduino IDE,
	you may need to unplug the arduino from the external power source,
	then unplug and replug in the usb cable.
	- to program it make sure you select the correct com port (probably 4)
	and the correct device (arduino/ genuino micro) 
	- once it is programmed, you can open the serial port monitor 
	in the arduino IDE - top right of the IDE there is a magnifying glass icon.
	Click that and then enter the number 1 and hit return. It should shoot a 
	sequence of photos. Number 2 should just take a picture.

4. The relight should still be triggerable from the external remote, 
	so you can use that for testing also. 
