from PIL import Image
from PIL import ImageFilter
import numpy as np
import time

someGlobalVariable = "hello brother"
hotHueValue = 0.
coldHueValue = 205.

w, h = 32,24

def scaler(scaleFactor):
    return scaleFactor * w , scaleFactor * h


def rotateNTimes90(array,times):
    npArray = np.array(array)
    print(someGlobalVariable)
    for i in range(times):
        npArray = np.rot90(npArray)
    return npArray

def reverseFitBetweenNumbers(maxOArray,minOArray,targetMin,targetMax,value):
    fitValue = (value - minOArray) / (maxOArray - minOArray) * (targetMax-targetMin) + targetMin
    return (targetMax - fitValue) + targetMin

def colorImage(imageArray):
    imageArray = np.array(imageArray)
    minTemperature = np.amin(imageArray)
    maxTemperature = np.amax(imageArray)
    hueAdjustedImageArray = [[[int(reverseFitBetweenNumbers(maxTemperature,minTemperature,hotHueValue,coldHueValue,imageArray[row,col])),180,200] for col in range(imageArray.shape[1])] for row in range(imageArray.shape[0])]
    return np.array(hueAdjustedImageArray)



scaleFactor = 15
scaledSize = scaleFactor * w , scaleFactor * h

f = open("rawData2.txt", "r")


data = map(float,f.read().split()) #split string into a list



imageArray = np.array(data)
maxPixel = np.max(imageArray)
imageArray = imageArray  #normalazing imageArray
imageArray = imageArray.reshape((h,w))
print(colorImage(imageArray).shape)
#data[256, 256] = [255, 0, 0]

img = Image.fromarray(colorImage(rotateNTimes90(imageArray,1)).astype('uint8'),"HSV")
img = img.resize(scaler(5))
img = img.filter(ImageFilter.MinFilter(5))
#img = img.filter(ImageFilter.SMOOTH)
img = img.filter(ImageFilter.BLUR)


img = img.resize(scaler(20))


#img.save('oguz.bmp')
img.show()
print(img.getbands())


