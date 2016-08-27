#Script to extract the byte frequencies of a given file
# Run on an individual file, or use loopDir.sh to process a directory
import sys
import math

if len(sys.argv) != 3:
    print "Usage: getFileFeatures.py isJPG [path]filename"
    print "	isJPG:	Input file is a jpg yes (1) or no (-1)"
    sys.exit()

isJPG = sys.argv[1]

# read the whole file into a byte array
f = open(sys.argv[2], "rb")
byteArr = map(ord, f.read())
f.close()
fileSize = len(byteArr)

blockArr = [0]*512
while (len(byteArr) > 0):
  # Get the first 512 bytes in blockArr
  for num in range(0,511):
	if len(byteArr) > 0:
		blockArr[num] = byteArr.pop(0)


  # calculate the frequency of each byte value in the file
  freqList = []
  for b in range(256):
      ctr = 0
      for byte in blockArr:
          if byte == b:
              ctr += 1
      freqList.append(float(ctr) / fileSize)


  print isJPG,
  k = 1
  for freq in freqList:
    print "%d:%f" % (k, freq),
    k += 1

  print
