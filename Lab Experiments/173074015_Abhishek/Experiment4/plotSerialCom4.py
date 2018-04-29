#  Python Script
#  Course : CS-684
#  Lab 4 : XY Plot from UART data
#  Submitted By: Abhishek Pal: Roll# 173074015
# 			  Anil Garg : Roll# 173074018
# 

# import time
import serial
import numpy
import matplotlib.pyplot as plt
#import matplotlib.pyplot as plt

from drawnow import *

#ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2, xonxoff=False, rtscts=False, dsrdtr=False) 
ser = serial.Serial('COM4', 115200, timeout=2, xonxoff=False, rtscts=False, dsrdtr=False) 
ser.flushInput()
ser.flushOutput()

plt.ion()

def dataPlot():
	#plt.axis([-1, 410, -1, 410])
	plt.axis([-10, 4100, -10, 4100])
	#plt.axis([0, 2, 0, 2])
	plt.title("Joystick Position Data(XY)")
	plt.grid(True)
	plt.xlabel("Left/Right (Adc Count)")
	plt.ylabel("Up/Down (Adc Count)")
	#plt.scatter(leftrightA, updownA)
	plt.scatter(leftright, updown)
        		
	plt.draw()
	plt.pause(0.05)
			

#dataStream = []
#dataArray = []	
#leftright = []; #0;
#updown = []; #0;
#leftrightA = 0;
#updownA = 0;

#count = 0;	

while True:
	while (ser.inWaiting()==0):
		pass
	#dataStream = ser.readline();
	dataStream = ser.readline().decode('ascii');
	#print(dataStream);
	#if dataStream != '\n':
	dataArray = dataStream.split(',');
	print(dataArray);
	if len(dataArray) != 2:
		continue;
	if (dataArray[0] != '' and dataArray[1] != '' ) :			
		#print (dataArray[0][0:6])
		#print (dataArray[1][0:4])
		leftright = int(dataArray[0]) 
		updown = int(dataArray[1][0:4])
		#print (leftright,",",updown)

		#leftrightA = leftright
		#updownA = updown

		drawnow(dataPlot)

		#time.sleep(1) #delay

		#ser.flush() #flush the buffer
		ser.flushInput()
		#ser.flushOutput()
