#displays JPEG file size distribution in a personal photo collection
import numpy as np
import matplotlib.pyplot as plt

sizes=np.loadtxt("./jpeg-sizes.txt", int)
(b,v)=np.histogram(sizes,1000)
#x=np.arange(0,len(b),1.0)
f=plt.figure(1,dpi=200)
plt.plot(v[1:],b,c='black',lw=2.0)
plt.ylabel("Number of files")
plt.xlabel("File size, Kb")
plt.xlim((0,8000))
plt.savefig('jpeg-sizes.png',type='png',dpi=200)
plt.show()
