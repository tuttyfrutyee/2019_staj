#measure time elapsed between received images
import time

#image processing
from PIL import Image
from PIL import ImageFilter
import numpy as np

#displaying video
import cv2
from threading import Thread #opencv requires a thread to while(true) showImage stuff
#color conversion :(
import colorsys
#iot stuff to get the raw image data from another mqtt client
import paho.mqtt.client as mqtt

#to parse c float32 to python float
from struct import *


#For image processing
lastReading = time.time()
hotHueValue = 0.
coldHueValue = 255.
w, h = 32,24
scaleFactor = 20
paddingWidth = 8
imageCounter = 0.0
currentImage = []
currentFilteredImage = None
firstStartFlag = True
encounteredLoggingImageCount = 0
minTemperature = 25 #used if colorImgR is false
maxTemperature = 35 #used if colorImgR is false 
realMinTemperature = None
realMaxTemperature = None
centerTemperature = None
floatTuples = None
termalGridRectCount = 10

#For MMQT
connectIp = "192.168.1.143"
targetTopic = "kuartis/davlumbaz/termalImage"
lastReceivedMessageTime = time.time()

#FLAGS
printTEBetweenImR = True #printTimeElapsedBetweenImageReceives
printTEBetweenImP = False #printTimeElapsedBetweenImagesProcessed
showTVideo = True #showTermalVideo
colorImgR = False #colorImageRelative


def hsv2rgb(h,s,v):
    h /= 360.
    s /= 100.
    v /= 100.
    return tuple(round(i * 255) for i in colorsys.hsv_to_rgb(h,s,v))




#IMAGE PROCESSING FUNCTIONS
def scaler(scaleFactor):
    return scaleFactor * w , scaleFactor * h

def reverseFitBetweenNumbers(maxOArray,minOArray,targetMin,targetMax,value):    
    fitValue = (value - minOArray) / (maxOArray - minOArray) * (targetMax-targetMin) + targetMin
    return fitValue 
    return (targetMax - fitValue) + targetMin



def colorImage(imageArray):
    global realMaxTemperature
    global realMinTemperature
    global minTemperature
    global maxTemperature
    global centerTemperature

    imageArray = np.array(imageArray)
    paddingArray = np.zeros((paddingWidth, h))
    realMaxTemperature = np.amax(imageArray)
    realMinTemperature = np.amin(imageArray)    
    minTemperature = realMinTemperature if colorImgR else minTemperature
    maxTemperature = realMaxTemperature + 2 if colorImgR else maxTemperature
    centerTemperature = imageArray[imageArray.shape[0] / 2, imageArray.shape[1] / 2]

    def getHue(temperature):
        return int(reverseFitBetweenNumbers(maxTemperature,minTemperature,hotHueValue,coldHueValue,temperature))

    def getRgb(temperature, index):
        return hsv2rgb(getHue(temperature), 100, 100)[index]

    #imageArray[row,col]
    hueAdjustedImageArray = [[[getRgb(imageArray[row,col], 0),getRgb(imageArray[row,col], 1),getRgb(imageArray[row,col], 2)] for col in range(imageArray.shape[1])] for row in range(imageArray.shape[0])]
    return np.array(hueAdjustedImageArray)


def termalVideoThread(args):
    global currentFilteredImage

    while(True):

        
        if currentFilteredImage is not None:

            opencvImage = np.array(currentFilteredImage)#cv2.cvtColor(np.array(currentFilteredImage), cv2.COLOR_HSV2RGB)
            font = cv2.FONT_HERSHEY_SIMPLEX
            
            #add padding for info spaces
            BLACK = [50,50,50]

            topPadding = 35
            bottomPadding = 100
            rightPadding = 80
            leftPadding = 10

            opencvImage = cv2.copyMakeBorder(opencvImage,topPadding,bottomPadding,leftPadding,rightPadding,cv2.BORDER_CONSTANT,value=BLACK)
            imageHeight, imageWidth, _ = opencvImage.shape
            termalHeight = h * scaleFactor
            termalWidth = w * scaleFactor

            #add central circle
            radius = 5
            circleCenterX = leftPadding + termalWidth / 2  
            circleCenterY = topPadding + termalHeight / 2 
            cv2.circle(opencvImage,(circleCenterX,circleCenterY), radius, (255,255,255), 2)

            #add termal color grid
            rectHeight = int(termalHeight / termalGridRectCount * 0.8)
            hueIncrement = coldHueValue / termalGridRectCount
            for index in range(termalGridRectCount):
                
                gridX1 = leftPadding + termalWidth + 5
                gridY1 = topPadding + 20 + rectHeight * index
                gridX2 = gridX1 + rightPadding - 10
                gridY2 = gridY1 + rectHeight
                rectColor = hsv2rgb(coldHueValue - hueIncrement * (index+1),101,101)
                cv2.rectangle(opencvImage,(gridX1,gridY1),(gridX2,gridY2),(rectColor[0],rectColor[1],rectColor[2]),-1)

            cv2.putText(opencvImage,"%.2f"% maxTemperature + ' C',(leftPadding + termalWidth + 5,topPadding + 10),font,0.5,(255,255,255),2,cv2.LINE_AA)
            cv2.putText(opencvImage,"%.2f"% minTemperature + ' C',(leftPadding + termalWidth + 5,topPadding + 20 + rectHeight * termalGridRectCount + 20),font,0.5,(255,255,255),2,cv2.LINE_AA)

            #add termal infos
            cv2.putText(opencvImage,"Center Temp : %.2f"% centerTemperature + ' C',(imageWidth-300,imageHeight - 30),font,0.7,(255,255,255),2,cv2.LINE_AA)
            cv2.putText(opencvImage,'Min Temp : %.2f'% realMinTemperature +' C' , (20,imageHeight - 30), font, 0.6,(255,255,255),2,cv2.LINE_AA)
            cv2.putText(opencvImage,'Max Temp : %.2f'% realMaxTemperature +' C' , (20,imageHeight - 65), font, 0.6,(255,255,255),2,cv2.LINE_AA)
            
            
            #Control informations
            cv2.putText(opencvImage,'Press q to quit',(20,20), font, 0.5,(255,255,255),2,cv2.LINE_AA)
            cv2.putText(opencvImage,"Press c capture",(imageWidth-250,20), font, 0.5, (255,255,255),2,cv2.LINE_AA)

            cv2.imshow('termal',opencvImage)  

            keys = cv2.waitKey(1)                      
            if keys & 0xFF == ord('q'):
                cv2.destroyWindow('termal')
                break  
            elif keys & 0xFF == ord('c'):
                cv2.imwrite('termalImage.png',opencvImage) 
                f = open("termalArray.txt", "w")
                for x in range(w):
                    for y in range(h):
                        f.write("%.2f "%floatTuples[y*w + x])
                    f.write("\n") 
                f.close()

def processRawImageData():
    global currentFilteredImage
    global currentImage 
    
    print("processing image\n")

    if(printTEBetweenImP): #FLAG# osmanserdargedik@gmail.com

        global lastReading
        global imageCounter

        print("\n ---- \n")
        print("imagecounter : "+str(imageCounter))
        imageCounter = imageCounter + 1
        currentReading = time.time()
        print("time elapsed since last image reading "+str(currentReading - lastReading)+ "\n")
        lastReading = currentReading      
        print("\n ---- \n")   

    imageArray = np.array(currentImage)
    #maxPixel = np.max(imageArray)
    #imageArray = imageArray  #normalazing imageArray
    imageArray = imageArray.reshape((h,w))
    #print(colorImage(imageArray).shape)
    #data[256, 256] = [255, 0, 0]

    #Use pillow to process image
    img = Image.fromarray(colorImage(imageArray).astype('uint8'),"HSV") 
    img = img.resize(scaler(5))
    img = img.filter(ImageFilter.MinFilter(3))
    #img = img.filter(ImageFilter.MaxFilter(3))
    #img = img.filter(ImageFilter.SMOOTH)
    img = img.filter(ImageFilter.BLUR)
    img = img.resize(scaler(scaleFactor))

    #The ready image to be shown as a video frame with opencv
    print("currentFilteredImage = img")
    currentFilteredImage = img             


#MQTT FUNCTIONS

    #CALLBACKS
def on_connect(mqttc, obj, flags, rc):

    print("Connected to %s:%s" % (mqttc._host, mqttc._port))

def on_message(mqttc, obj, msg): # topic name = msg.topic, QoS = msg.qos, payload(rawImage) = msg.payload [Note : max of 256Mb is supported, so we are safe i guess]   
    
    global lastReceivedMessageTime
    global currentImage
    global floatTuples
    
    # print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
    if(printTEBetweenImR):
        print("\n\n")
        print("time between messages "+ str(time.time() - lastReceivedMessageTime))
        print("\n\n")
    lastReceivedMessageTime = time.time()   

    #process image
    if(msg.topic == targetTopic):
        try:
            formatString = "<768f" #to parse c float32 (esp32 is little endian)
            floatTuples = unpack(formatString, msg.payload)
            currentImage = [floatTuples[len(floatTuples) - x - 1] for x in range(len(floatTuples))]

        except Exception as e:
            print("Upper exception occured",e)

        try:
            processRawImageData()
        except Exception as e:
            print("An exception occurred",e)    
        

def on_publish(mqttc, obj, mid):
    print("mid: "+str(mid))

def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_log(mqttc, obj, level, string):
    print(string)

    #GO&WORK
def initAndStartMqttStreamSink(): #it will run forever as a thread, since mqttClient.loop_start() is async func 
   
    global connectIp
    global targetTopic

    mqttc = mqtt.Client()

    #setting callbacks
    mqttc.on_message = on_message
    mqttc.on_connect = on_connect
    mqttc.on_publish = on_publish
    mqttc.on_subscribe = on_subscribe

    # Uncomment to enable debug messages
    #mqttc.on_log = on_log

    mqttc.connect(connectIp) #default mqtt(without ssl) is 1883
    mqttc.subscribe(targetTopic, 0) # QoS level is 0

    #starting the loop
    mqttc.loop_start() #async loop start, can be stopped by calling mqttc.loop_stop()

    return mqttc #created mqtt client



#MAIN
mqttc = initAndStartMqttStreamSink() #start getting termal image stream from another mqtt client(mlx90640 connected esp32)

if(showTVideo):
    vidoeThread = Thread(target = termalVideoThread, args = (10, ))
    vidoeThread.start()

while(True):
    time.sleep(2)
