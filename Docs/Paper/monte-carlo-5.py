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

db=np.empty(disksize)
cnt=0.0

# generate all possible file writing sequences satisfying the above criteria  
# update block accumulators accordingly
def simulate(nsim):
    global b,disksize,mufil,sigfil,mutot,sigtot,cnt
    middb=int(disksize/2)
    if debugprn>=2:
        print "number of simulations: ", nsim
    for n in range(nsim):
        db.fill(0.0)
        tot=int(np.random.normal(mutot,sigtot))
        for i in range(0,tot,1):
            fsize=int(np.random.normal(mufil,sigfil))
            fpos=int(np.random.uniform(0.0,disksize-fsize+1.0))
            db[fpos:(fpos+fsize)]=[1.0]*fsize
        # The following code carves out a file if it occupies the middle block
        #if (db[middb]==1):
        #    i=middb
        #    while db[i]==1:
        #        db[i]=0.0
        #        i+=1 
        #    i=middb-1
        #    while db[i]==1:
        #        db[i]=0.0
        #        i-=1
        if db[7000]==1.0 and db[6850]==1.0:
           b += db
           cnt += 1.0 
        if ((n % 5000) == 0):
            print "simulations count = ",n
    b=b/cnt    
    return

simulate(100000)

#print "block prob:", b

plt.figure(1)
f,axarr = plt.subplots(1, 2)
axarr[0].set_title('$p(a)$')
axarr[1].set_title('$p(a)$')

f.subplots_adjust(wspace=0.2, hspace=0.2)

ay=axarr[0]
ay.set_ylabel("Probability")
ay.set_xlabel("Block number")
x = np.arange(0,disksize,1)
wc,=ay.plot(x,b,c='black',lw=2.0)
#xmin,xmax,ymin,ymax = ay.axis()
#ay.axis((xmin,xmax,ymin-1,ymax+2))

ay=axarr[1]
ay.set_ylabel("")
ay.set_xlabel("Block number")
x = np.arange(0,disksize,1)
wc,=ay.plot(x,b,c='black',lw=2.0)
xmin,xmax,ymin,ymax = ay.axis()
ay.axis((6650,7350,ymin,ymax))

#plt.legend([pc],["mintot=%s maxtot=%s minfil=%s maxfil=%s" % (mintot,maxtot,minfile,maxfile)])
#plt.axis((0,disksize,-0.1,1.1))

#plt.savefig('montecarlo5.png',type='png',dpi=200)

plt.show()