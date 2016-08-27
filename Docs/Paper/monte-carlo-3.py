# This simulation calculates cumulative probability of carving 
# a number of blocks sequentially from the beginning of the disk block array

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

#size of the disk (in blocks)
disksize=30000

# do debug printing
debugprn=2

#parameters of filesize distribution
mufil=300.0
sigfil=20.0

#parameters of the number of files distribution
mutot=20.0
sigtot=5.0

# ---------
print "total number of blocks on the disk = %d" % (disksize)
print "expected value of the file size = %s" % (mufil)
print "file size sigma = %s" % (sigfil)
print "expected value of the number of files = %s" % (mutot)
print "number of files sigma = %s" % (sigtot)

print "disk space after writing combinations of files satisfying above criteria:"
print "--------------"

# zero accumulator array 
b=np.empty(disksize)
b.fill(0.0)

# accumulator for finding cumulative probability of finding a relevant file by 
# carving i consecutive blocks from thestart
cumul=np.empty(disksize)
cumul.fill(0.0)

# generate all possible file writing sequences satisfying the above criteria  
# update block accumulators accordingly
def simulate(nsim):
    global b,cumul,disksize,mufil,sigfil,mutot,sigtot
    db=np.empty(disksize)
    if debugprn>=2:
        print "number of simulations: ", nsim
    for n in range(nsim):
        db.fill(0.0)
        tot=int(np.random.normal(mutot,sigtot))
        for i in range(0,tot,1):
            fsize=int(np.random.normal(mufil,sigfil))
            fpos=int(np.random.uniform(0.0,disksize-fsize+1.0))
            db[fpos:(fpos+fsize)]=[1.0]*fsize
        b=b+db
        s=0.0
        for m in range(4800):
           if db[m]==1: 
               s=1.0
           cumul[m]+= s    
        if debugprn>=2 and ((n % 5000) == 0):
            print "simulations count = ",n
    b=b/nsim
    cumul=cumul/nsim    
    return

simulate(10000)

#print "block prob:", b

plt.figure(1)
f,axarr = plt.subplots(1, 1)
ay=axarr
ay.set_ylabel("$P(b_i)$")
ay.set_xlabel("$i$")
x = np.arange(0,disksize,1)
ay.plot(x,b,c='black',lw=2.0)
ay.plot(x,cumul,c='blue',lw=2.0)
xmin,xmax,ymin,ymax = ay.axis()
ay.axis((0,4800,ymin,ymax))

#plt.legend([pc],["mintot=%s maxtot=%s minfil=%s maxfil=%s" % (mintot,maxtot,minfile,maxfile)])
#plt.axis((0,disksize,-0.1,1.1))

plt.savefig('linear-integral-prob.png',type='png',dpi=200)

plt.show()