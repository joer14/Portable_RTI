Version 2 Overview


Materials:
- 6V LEDs Cree LEDs (MKRAWT-00-0000-0D0HH240H) [Datasheet](http://www.cree.com/led-components-and-modules/products/xlamp/arrays-directional/~/media/Files/Cree/LED%20Components%20and%20Modules/XLamp/Data%20and%20Binning/XLampMKR.pdf)
- arduino micro
	- no shift registers, we just use all of the IO pins. 

Power:
- it must be powered by 2 sources at least. USB + wall adapter or wall adapter + battery. Power should be around 12v, 2 or 3A if using external wall adapter. 
- battery is Sony NP-F770 Battery 


Connecting everything:
- the cords should be color coded with electrical tape, but if they are not:
	- the top 3.5mm hole is for hotshoe, the side 2.5mm is for the remote, the side 3.5mm for is for a 3.5mm->2.5mm cable that connects to the camera's shutter release. 

Source Code:

- Version2_1
	- original code for relight 2. Allows full sequence shot when triggered via remote. 
	- also has one-shot mode for testing lighting conditions. So if the shutter releases on the camera it will always trigger the flash. 
- Version2_ComputerControl
	- same as original, except no one-shot mode and allows for usb control. 
	- usb control code and documentation is available in the respective folder. 