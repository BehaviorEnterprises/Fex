#FEX
FEX is a task specific tool for specific research purposes.  The relevant usage details will be elaborated in an upcoming publication.

FEX is available for linux/bsd as XCairoFex or for Mac OSX (>=10.5) as MacFex. (MacFex has not been updated with recent changes to the core fex code and is currently non-functional).

A Windows version is in the early stages - development of this port will depend on user feedback.

All versions are currently experimental, and undoubtedly have bugs.  But they are also in active development.  Good software comes from detailed feedback from users.  Please report any bugs or issues you run into.  Feature requests from users are also appreciated.  I'm happy to receieve such requests: your input helps me make Fex better.

##Building Fex:

###Download the source:
1. install git and `git clone http://github.com/TrilbyWhite/fex.git`, or
1. download and uncompress zip file on the right side of the github web interface

###Build:
1. Arch Linux: use included PKGBUILD
1. Other Linux:
	1. install all dependencies: cairo, libsndfile, and fftw-3.  If your distro uses split packages (eg unbuntoo) you'll also need the -dev package(s)
	1. navigate to the fex directory then run the following:
```bash
./configure
make
sudo make install
```
