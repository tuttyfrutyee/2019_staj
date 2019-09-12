import serial
import numpy as np

#to parse c float32 to python float
from struct import *


# Configuring and starting Serial communication
ser = serial.Serial() # Creating serial port object
ser.baudrate = 115200
ser.port = "/dev/ttyUSB3" # Port being used
ser.timeout = 0.5

ser.open()




trialCount = 2

measDetails = [
    {"distance" : 1},
    {"distance" : 2},
    {"distance" : 3},
    {"distance" : 4},
    {"distance" : 5},
    {"distance" : 0}, #distance 0 means 5+ meters
]

measIndex = 0
internalIndex = 0

logFileName = "rssiMeas.txt"


open(logFileName, 'w').close() #clear file before measurement


while (True):
    # Monitoring the esp logs
    if(ser.isOpen() !=False):
        if (ser.inWaiting() > 0):
            reading = (ser.readline().decode("ISO-8859-1")).strip() # Strip to eliminate the newline at the end of the line, since logger already introduces one
            if  len(reading)>1: # To eliminate null char appearing at the beginning (It messes the encoding)

                
                if "#" in reading:


                    content = reading.split(" ")[1]

                    numberOfRssiInString = int(content.split(":")[0]) #rssi values are integers(32 bit)

                    rssiBufferAsString = content.split(":")[1].encode("ISO-8859-1")  


                    formatString = "<" + str(numberOfRssiInString) + "i"
                    floatTuples = unpack(formatString, rssiBufferAsString)

                    rssiBuffer = [floatTuples[x] for x in range(len(floatTuples))]   

                    rssiBuffer_string = ""
                    for i in range(len(rssiBuffer)):
                        rssiBuffer_string += str(rssiBuffer[i])

                    f = open(logFileName, "a")
                    f.write("$\n")
                    f.write("distance : "+str(measDetails[measIndex]["distance"]) + " meters, trialCount : " + str(internalIndex + 1) + "\n" )
                    f.write(rssiBuffer_string)    
                    f.write("\n")
                    f.close()   

                    internalIndex += 1         
                    
                    if(internalIndex > (trialCount-1)):
                        measIndex += 1
                        internalIndex = 0

                    if(measIndex >= len(measDetails)):
                        break

                    

print "exiting program"
                




                                    
                  