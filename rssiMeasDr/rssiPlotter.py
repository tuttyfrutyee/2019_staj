"""
=============================================
Generate polygons to fill under 3D line graph
=============================================

Demonstrate how to create polygons which fill the space under a line
graph. In this example polygons are semi-transparent, creating a sort
of 'jagged stained glass' effect.
"""

from mpl_toolkits.mplot3d import Axes3D
from matplotlib.collections import PolyCollection
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from matplotlib import colors as mcolors
import numpy as np




def cc(arg):
    return mcolors.to_rgba(arg, alpha=0.6)


with open('rssiData2.txt') as f:
    content = f.readlines()
# you may also want to remove whitespace characters like `\n` at the end of each line
content = [x.strip() for x in content] 

rssiValues = []
appendedRssiValues = []

for line in content:
    if "-" in line:
        rssiList = [-float(rssi) for rssi in line.split("-")[1:]  ]
        rssiList.sort()
        rssiValues.append( rssiList )


for i in range(len(rssiValues) / 2):
    appendedRssiValues.append( rssiValues[i * 2] + rssiValues[i * 2 + 1] )


print("\n\n\n")

for i in range(len(rssiValues) / 2):
    print(str(i) + ": mean = "+str(np.mean(appendedRssiValues[i])) + " std = " + str(np.std(appendedRssiValues[i])) )

print("\n\n\n")

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
nbins = 50

zs = np.arange(len(appendedRssiValues))

for  c, z in zip(['r', 'g', 'b', 'y', 'c', 'k'], zs):
    ys = appendedRssiValues[z]

    hist, bins = np.histogram(ys, bins=nbins)
    xs = (bins[:-1] + bins[1:])/2

    ax.bar(xs, hist, zs=z, zdir='y', color=c, ec=c, alpha=0.8)

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

plt.show()