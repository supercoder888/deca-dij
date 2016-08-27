from mpl_toolkits import mplot3d
import matplotlib.pyplot as plt
import numpy as np
from test.test_math import acc_check
from matplotlib._cntr import Cntr

size=0
acc=[]
inst=[]

def init(s):
  global size
  global min0len
  global max0len
  global min1len
  global max1len
  global display
  global testloc
  global acc
  global cnt
  global inst
  size = s
  min0len = 1
  max0len = size
  min1len = 1
  max1len = size
  display = 0
  testloc = []
  acc = []
  for u in range(0,size+1,1):
    accline = np.arange(0,size,1.0)
    accline[:] = 0
    acc = acc + [ accline ]
  acc=np.array(acc)
  cnt=np.arange(0,size+1,1.0)
  cnt[:]=0
  inst = np.arange(0,size,1.0)
  inst[:]=0

def matches(testloc,inst):
  for (pos,val) in testloc:
    if inst[pos] != val:
      return False
  return True

def gen(pref,plen,elem,sum):
  global acc
  global inst
  global cnt
  global size
  global min0len
  global max0len
  global min1len
  global max1len
  global display
  if plen == size:
      j=0
      for (e,l) in pref:
         for i in range(l):
            inst[j]=e
            j=j+1
      if len(testloc)==0 or matches(testloc,inst):      
         if display != 0:
            print pref,sum
         cnt[int(sum)]=cnt[int(sum)]+1
         for j in range(len(inst)):
           acc[sum,j]=acc[sum,j]+inst[j]
  if plen < size:
     if elem==1: 
        for x in range(min1len,max1len+1,1):
           if (x+plen <= size):
              gen(pref+[(elem,x)],plen+x,1-elem,sum+elem*x);
     else:
        for x in range(min0len,max0len+1,1):
           if (x+plen <= size):
              gen(pref+[(elem,x)],plen+x,1-elem,sum+elem*x);

def calcprob():
  global acc
  global inst
  global cnt
  global size
  acc = []
  for u in range(0,size+1,1):
    accline = np.arange(0,size,1.0)
    accline[:] = 0
    acc = acc + [ accline ]
  acc=np.array(acc)
  cnt=np.arange(0,size+1,1.0)
  cnt[:]=0
  inst = np.arange(0,size,1.0)
  inst[:]=0
  gen([],0,1,0)
  gen([],0,0,0)
  for i in range(1,len(acc)):
    for j in range(len(acc[i])):
      if cnt[i] != 0:
        acc[i,j]=acc[i,j]/cnt[i]

def average(minsum,maxsum):
  global acc
  avg = np.arange(len(acc[0]))
  avg[:] = 0.0
  cnt2 = 0
  for i in range(minsum,maxsum+1,1):
     if cnt[i] != 0:
        cnt2 = cnt2+1
        avg = avg + acc[i]
  avg = avg / cnt2
  return avg  

def graphs(minsum,maxsum,filename='',alpha=0.12,beta=1.152241e+01):
  global acc
  global size
  y=range(len(acc))
  x=range(len(acc[0]))
  X,Y=np.meshgrid(x,y)
  fig1 = plt.figure(1)
  ax = fig1.add_subplot(111, projection='3d')
  ax.plot_wireframe(X,Y,acc)
  ax.set_xlabel('block number')
  ax.set_ylabel('No. blocks with relevant data')
  ax.set_zlabel('probability of finding relevant data')
  if filename != '':
    plt.savefig(filename+'3d.png')
  fig2 = plt.figure(2)
  ax = fig2.add_subplot(111)
  avg=average(minsum,maxsum)
  ax.plot(range(len(acc[0])),avg)
  ax.set_xlabel('block number')
  ax.set_ylabel('probability of finding relevant data')
  if filename != '':
    plt.savefig(filename+'2d.png')
  fig3 = plt.figure(3)
  ax1 = fig3.add_subplot(111)
  ax2 = ax1.twinx()
  avg=average(minsum,maxsum)
  blocks = np.arange(len(acc[0]))
  tproc = alpha*blocks+beta
  invtproc = 1 / tproc
  ax1.set_ylim([0.04,0.087])
  ax1.set_xlabel('block number')
  ax1.set_xlim([3.0,16.0])
  ax1.set_ylabel('utility value')
  ax1.plot(range(len(acc[0])),avg*invtproc,'b-')
  ax2.set_ylabel('inverse $T_{proc}$')
  ax2.set_ylim([0.04,0.087])
  ax2.plot(range(len(acc[0])),invtproc,'r--')
  if filename != '':
    plt.savefig(filename+'utility.png')
  plt.show()

  