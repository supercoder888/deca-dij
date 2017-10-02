# DECA - decision theoretic (jpeg) file carver  #

This is a proof-of-concept project intended to demonstrate / evaluate decision-theoretic approach to digital forensics. 
It is a command line utility with a variety of command line options (run deca without parameters to see them).
It has been compiled and tested on Ubuntu Linux or Mac. It shouldn't be difficult to port it to Windows environment.
The code is written in Pure C for portability, but it depends on libtsk (the Thleuthkit library) being available on the target platform.

### How do I get set up? ###

* Use Ubuntu or Mac OS X
* Install libtsk, and optionally libmagic-dev, liblinear-dev, and libsvm-dev packages (if you would like to play with older code that is commented out in the source files). Use apt-get install on Ubuntu or Homebrew on Mac.
* Check that you have working gcc and make 
* Clone this repository
* Edit Makefile to specify correct paths to include folders (-Ixxxxxx in CFLAGS), as well as the appropriate LIBS=... and LIB_DIR=... values. Please note that Makefile is configured for Mac OS X by default. If you are using Ubuntu you will need to comment out the currently enabled  LIBS=... line  and uncomment one for Linux.
* Open terminal, navigate to the root folder of the cloned repo, then run make.  

If you get any problems compiling it, do send us a message.

The project was initially developed with Eclipse, so if you like Eclipse, please go ahead and use it:

* Download & Unzip Eclipse-CDT from eclipse.org
* Install eGit Eclipse plugin from eclipse.org if you like (integrates Git & Eclipse)

### Who do I talk to? ###

* Repo owner: Pavel Gladyshev <pavel@digitalfire.cc>
* Joshua James <joshua@cybercrimetech.org>