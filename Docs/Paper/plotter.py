#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt
import random
import sys
import math

#if len(sys.argv)<2:

print("This script reads file lineartest.txt and plots various graphs")

try:
   d=np.loadtxt("lineartest.txt", delimiter=',')
except:
   print("Could not process file %s!" % (sys.argv[1]))
   sys.exit(-1)

# number of relevant clusters recovered: WC(t) 
x=np.arange(len(d)*2+1)
y=np.arange(len(d)*2+1)
w=0
x[0]=0
y[0]=0

#total number of sectors examined: WT(t)
xtot=np.arange(len(d)+1)
ytot=np.arange(len(d)+1)
xtot[0]=0
ytot[0]=0

# Carving power (Mb/sec)
dt=(d[:,3]-d[:,0])/142685.0  # time in seconds required to carve individual files
ds=d[:,5]/2048.0             # size of individual files in Mb

dt[len(dt)-1]=1              # the last element was added artificially, 
                             # so let's set dt to 1 to avoid division by 0

pc=ds/dt                     # carving power for individual files
                             # note that the carving power will be 
                             # zero everywhere else - where file is not being carved.

#build WC(t), WT(t) graphs
for i in np.arange(0,len(d),1):
   x[(i+1)*2-1]=d[i][0]
   y[(i+1)*2-1]=w
   x[(i+1)*2]=d[i][3]
   y[(i+1)*2]=d[i][6]
   w=d[i][6]
   xtot[i+1]=d[i][3]
   ytot[i+1]=d[i][7]
   


#built pc(t) graph
xpc=[]
ypc=[]
i=0
for t in np.arange(0.0,d[len(d)-1,3],143.0):  #time step
   xpc.append(t/142685.0)
   if t<d[i,0]:
      ypc.append(0.0)
   else:
      while d[i,0]<t:
         i=i+1
      if t>d[i,3]:
         ypc.append(0.0)
      else:
         ypc.append(pc[i])

xpc=np.array(xpc)
ypc=np.array(ypc)   

#  -===now simulate idealized decision-theoretic graphs of WC(t) and pc(t) ===-

# The idea is that the precision of decision theoretic guessing should
# decrease proportionally to the remaining amount of relevant uncarved data 
# in the disk space

# First let's generate simulated data
rem_data=(d[len(d)-2,6]/2048.0)+1.0 # Total amount of relevant data in mb
rec_data=0.0                  # Data recovered so far
rem_space=d[len(d)-2,7]/2048.0 # Total space
dd=d
tim=0                         # curent time
for i in np.arange(len(dd)-2):
  # get random delay before next relevant file is found. This is inversely proportional 
  # to the amount of remaining data
  #
  # ***************************************************************
  # *
  delta_t=random.uniform(0,rec_data/rem_data)*5000
  print(delta_t)
  # *
  # ***************************************************************
  #
  tim = tim+delta_t
  dd[i][3]=dd[i][3]-dd[i][0]
  dd[i][0]=tim
  dd[i][3]=d[i][3]+tim
  tim = dd[i][3]
  rec_data=dd[i][6]/2048.0   

w=0
  
# Now let's produce simulated WC(t)

xx=np.arange(len(dd)*2+1)
yy=np.arange(len(dd)*2+1)
w=0
xx[0]=0
yy[0]=0
for i in np.arange(0,len(dd),1):
   xx[(i+1)*2-1]=dd[i][0]
   yy[(i+1)*2-1]=w
   xx[(i+1)*2]=dd[i][3]
   yy[(i+1)*2]=dd[i][6]
   w=dd[i][6]

# Now let's produce simulated pc(t)   
# Simulated carving power (Mb/sec)
ddt=(dd[:,3]-dd[:,0])/142685.0  # time in seconds required to carve individual files
dds=dd[:,5]/2048.0             # size of individual files in Mb

ddt[len(ddt)-1]=1              # the last element was added artificially, 
                             # so let's set dt to 1 to avoid division by 0

ppc=dds/ddt                     # carving power for individual files
                             # note that the carving power will be 
                             # zero everywhere else - where file is not being carved.

xxpc=[]
yypc=[]
i=0
for t in np.arange(0.0,d[len(dd)-1,3],143.0):  #time step
   xxpc.append(t/142685.0)
   if t<dd[i,0]:
      yypc.append(0.0)
   else:
      while dd[i,0]<t:
         i=i+1
      if t>dd[i,3]:
         yypc.append(0.0)
      else:
         yypc.append(ppc[i])

xxpc=np.array(xxpc)
yypc=np.array(yypc)   

#Now let's plot the graphs

# figure 1 - WT(t) vs WC(t) - amount of data processed vs amount of data recovered 
plt.figure(1,dpi=300)
ax=plt.subplot(111)
ax.set_ylabel("Processed data, Mb")
ax.set_xlabel("Time, seconds")
wc,=plt.plot(x/142685,y/2048,c='black',lw=2.0, label='WC(t)')
wt,=plt.plot(xtot/142685,ytot/2048,c='black',lw=2.0,ls='--', label='WT(t)')
plt.legend([wt,wc],['WT(t)','WC(t)'])
xmin,xmax,ymin,ymax = plt.axis()
plt.axis((xmin,xmax,ymin,ymax+2000))
plt.savefig('wt_wc_fig.png',type='png')

# figure 2 - WC(t) - amount of data recovered
plt.figure(2,dpi=300)
ay=plt.subplot(111)
ay.set_ylabel("Recovered data, Mb")
ay.set_xlabel("Time, seconds")
wc,=plt.plot(x/142685,y/2048,c='black',lw=2.0)
xmin,xmax,ymin,ymax = plt.axis()
plt.axis((xmin,xmax,ymin-1,ymax+20))
plt.savefig('wc_fig.png',type='png')

# figure 3 - carving power pc(t) - amout of data recovered per second
plt.figure(3,dpi=300)
ay=plt.subplot(111)
ay.set_ylabel("Carving speed, Mb/sec")
ay.set_xlabel("Time, seconds")
pc,=plt.plot(xpc,ypc,c='black',lw=2.0, label='pc(t)')
plt.legend([pc],['pc(t)'])
xmin,xmax,ymin,ymax = plt.axis()
plt.axis((xmin,xmax,ymin-1,ymax+3))
plt.savefig('pc_fig.png',type='png')

# figure 4 - WC(t) vs pc(t) - total amount of data carved vs data revoered per second
plt.figure(4,dpi=300)
ay=plt.subplot(211)
ay.set_ylabel("Recovered data, Mb")
#ay.set_xlabel("Time, seconds")
wc,=plt.plot(x/142685,y/2048,c='black',lw=2.0, label='WC(t)')
plt.legend([wc],['WC(t)'])
xmin,xmax,ymin,ymax = plt.axis()
plt.axis((xmin,xmax,ymin-1,ymax+2))

ax=plt.subplot(212)
ax.set_ylabel("Carving speed, Mb/sec")
ax.set_xlabel("Time, seconds")
pc,=plt.plot(xpc,ypc,c='black',lw=2.0, label='WC(t)')
plt.legend([pc],['pc(t)'])
plt.axis((xmin,xmax,ymin-1,ymax+2))

plt.savefig('wc_pc_fig.png',type='png')

# figure 5 - Simulated WC(t) vs pc(t) for DECA - total amount of data carved vs data revoered per second
plt.figure(5,dpi=300)
ay=plt.subplot(211)
ay.set_ylabel("Recovered data, Mb")
#ay.set_xlabel("Time, seconds")
wc,=plt.plot(xx/142685,yy/2048,c='black',lw=2.0, label='WC(t)')
plt.legend([wc],['WC(t)'])
xmin,xmax,ymin,ymax = plt.axis()
plt.axis((xmin,xmax,ymin-1,ymax+2))

ax=plt.subplot(212)
ax.set_ylabel("Carving speed, Mb/sec")
ax.set_xlabel("Time, seconds")
pc,=plt.plot(xxpc,yypc,c='black',lw=2.0, label='WC(t)')
plt.legend([pc],['pc(t)'])
plt.axis((xmin,xmax,ymin-1,ymax+2))

plt.savefig('deca_wc_pc_fig.png',type='png')

# figure 6 (Fig1 for the paper) WC(t), pc(t) for llinear vs WC(t), pc(t) for DECA
f,axarr = plt.subplots(2, 2)
axarr[0, 0].set_title('Linear carver')
axarr[0, 1].set_title('Decision-theoretic carver')
axarr[1, 0].set_title('')
axarr[1, 1].set_title('')
f.subplots_adjust(wspace=0.3)

ay=axarr[0,0]
ay.set_ylabel("Recovered data, Mb")
wc,=ay.plot(x/142685,y/2048,c='black',lw=2.0, label='WC(t)')
ay.legend([wc],['WC(t)'])
xmin,xmax,ymin,ymax = ay.axis()
ay.axis((xmin,xmax,ymin-1,ymax+2))

ax=axarr[1,0]
ax.set_ylabel("Carving speed, Mb/sec")
ax.set_xlabel("Time, seconds")
pc,=ax.plot(xpc,ypc,c='black',lw=2.0, label='WC(t)')
ax.legend([pc],['pc(t)'])
ax.axis((xmin,xmax,ymin-1,ymax+2))

ay=axarr[0,1]
ay.set_ylabel("Recovered data, Mb")
wc,=ay.plot(xx/142685,yy/2048,c='black',lw=2.0, label='WC(t)')
ay.legend([wc],['WC(t)'])
xmin,xmax,ymin,ymax = ay.axis()
ay.axis((xmin,xmax,ymin-1,ymax+2))

ax=axarr[1,1]
ax.set_ylabel("Carving speed, Mb/sec")
ax.set_xlabel("Time, seconds")
pc,=ax.plot(xxpc,yypc,c='black',lw=2.0, label='WC(t)')
ax.legend([pc],['pc(t)'])
ax.axis((xmin,xmax,ymin-1,ymax+2))

plt.savefig('fig1.png',type='png',dpi=200)

plt.show()
