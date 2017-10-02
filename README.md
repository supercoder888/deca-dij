# DECA - decision theoretic (jpeg) file carver  #

This is a proof-of-concept project intended to demonstrate / evaluate decision-theoretic approach to digital forensics. 
It is a command line utility with a variety of command line options (run deca without parameters to see them).
Can be compiled/used on Ubuntu or Mac. It shouldn't be difficult to port it to Windows environment.
The code is written in Pure C for portability, but the code depends on libtsk (the Thleuthkit library) being present.

### How do I get set up? ###

* Use Ubuntu or Mac OS X (
* Install libtsk3-3, (and optionally libmagic-dev, liblinear-dev, and libsvm-dev packages) using apt-get install 
* Check that you have gcc and make installed
* Clone this repository
* Edit Makefile to uncomment the appropriate LIB=... definition (Mac is enabled by default, so if you are using Ubuntu you will need to comment it and uncomment one for Linux
* Open terminal and navigate to the root folder of the cloned repository, then run make.  If you get any problems compiling it, do send us a message.

### Who do I talk to? ###

* Repo owner: Pavel Gladyshev <pavel@digitalfire.cc>
* Joshua James <joshua@cybercrimetech.org>