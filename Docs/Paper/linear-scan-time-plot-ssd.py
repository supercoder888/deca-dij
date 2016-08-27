import numpy as np
import matplotlib.pyplot as plt

jx=np.array([1.000000e+02,1.000010e+07,2.000010e+07,3.000010e+07])
jy=np.array([4.384000e-03,1.085499e+03,2.044755e+03,3.015771e+03])
jxx=np.array([0,240000000])
jyy=jxx*(5.857526e-08)+(1.388750e+01)

plt.figure(1)
plt.ylabel("Time, ms")
plt.xlabel("4k blocks read / 4k blocks jumped")
gp,=plt.plot(jx/8,jy,'ro',c='black')
#grp,=plt.plot(jxx/8,jyy,c='black',lw=2.0)
xmin,xmax,ymin,ymax = plt.axis()
#plt.axis((-1,40000,ymin-1,ymax))
#plt.legend([gp,grp],['$T_{proc}$ time (empirical)','$T_{proc}$ time (linear regression)'],bbox_to_anchor=(0.63, 1),prop={'size':13})
plt.savefig('linear-scan-time-ssd.png',type='png',dpi=200)

plt.show()