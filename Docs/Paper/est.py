import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

#size of the disk (in blocks)
disksize=10000

# do debug printing
debugprn=2

#parameters of filesize distribution
mufil=300.0
sigfil=50.0

#parameters of the number of files distribution
mutot=35.0
sigtot=10.0

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

#file size distribution
fdist=np.empty(mufil+6*sigfil)
fdist.fill(0.0)

#total number of files distribution
totdist=np.empty(mutot+4*sigtot*mutot)
totdist.fill(0.0)

db=np.empty(disksize)

# generate all possible file writing sequences satisfying the above criteria  
# update block accumulators accordingly
def simulate(nsim):
    global b,disksize,mufil,sigfil,mutot,sigtot,fdist,totdist
    if debugprn>=2:
        print "number of simulations: ", nsim
    for n in range(nsim):
        db.fill(0.0)
        tot=int(np.random.normal(mutot,sigtot))
        totdist[tot]+=1.0
        for i in range(0,tot,1):
            fsize=int(np.random.normal(mufil,sigfil))
            fdist[fsize]+=1.0
            fpos=int(np.random.uniform(0.0,disksize-fsize+1.0))
            db[fpos:(fpos+fsize)]=[1.0]*fsize
        b += db 
    b=b/nsim    
    fdist=fdist/nsim
    totdist=totdist/nsim
    return


simulate(100000)

print "block prob:", b

plt.figure(1)
x = np.arange(0,disksize,1)
plt.plot(x,b)
#plt.legend([pc],["mintot=%s maxtot=%s minfil=%s maxfil=%s" % (mintot,maxtot,minfile,maxfile)])
#plt.axis((0,disksize,-0.1,1.1))
plt.figure(2)
x=np.arange(0,len(fdist),1.0)
plt.plot(x,fdist)

plt.figure(3)
x=np.arange(0,len(totdist),1.0)
plt.plot(x,totdist)


plt.show()