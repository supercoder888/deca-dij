import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

l=0.0      # size (in blocks) of the files being carved (let's assume that all files have the same length
lmin=300.0
lmax=3000.0
ds=60000   # total number of blocks on the disk 

alpha=10**-1
beta=10

p=np.arange(0.0,ds,1.0)
pacc=np.arange(0.0,ds,1.0)
pacc[:]=0.0
b=np.arange(0,ds,1)

for l in np.arange(lmin,lmax+1,1.0):
    ramp=np.arange(1,l+1,1)
    ramp2=np.arange(l,0,-1)
    p[0:l]=(ramp/l)*0.001
    p[l:ds-l]=1.0*0.001
    p[ds-l:ds]=(ramp2/l)*0.001
    pacc=pacc+p

p = pacc / (lmax-lmin+1)

tp=alpha*b+beta

u=(1.0/tp)*p

#alpha < beta is required for u to have a maximum not at 0 for trapezoidal u()


plt.figure(1,dpi=75)
ax=plt.subplot(111)
ax.set_ylabel("Probability of occurrence; Utility")
ax.set_xlabel("disk block")
wc,=plt.plot(b,u,c='green',lw=2.0)
#wc,=plt.plot(b,p,c='blue',lw=2.0)

plt.show()

#plt.savefig('utility-1.png',type='png',dpi=200)
#plt.show()