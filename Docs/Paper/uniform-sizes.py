#displays JPEG file size distribution in a personal photo collection
import numpy as np
import matplotlib.pyplot as plt

x=np.arange(0,25000,1.0)
b=np.arange(0,25000,1.0)
b[:700]=0.0
b[20000:]=0.0
b[700:20000]=1.0/(20000.0-700.0)
#x=np.arange(0,len(b),1.0)
#f=plt.figure(1,dpi=200)
f=plt.figure(1)
plt.ylim((0,0.00035))
plt.plot(x,b,c='black',lw=2.0)
plt.ylabel("Probability")
plt.xlabel("File size, Kb")
plt.savefig('fdist.png',type='png',dpi=200)
plt.show()
