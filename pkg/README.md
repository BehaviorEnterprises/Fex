###FEX Distibion and packaging

For distributions equipped to build from git sources (e.g. Archlinux),
all packaging materials will be provided in subdirectories here.

For other distrobutions, I will work on making suitable packaging materials
available and/or will gladly accept pull requests from those who are more
familiar with other distros.  The directories for such distros contain the
materials I currently use to build packages for those targets.

Binary and/or source packages will be available at the site below as they
become available:

http://behaviorenterprises.com/repo/

####Dependencies

The following packages and all their dependencies are required.

+ cairo
+ desktop-file-utils
+ fftw
+ libxpm
+ libsndfile
+ python2

Dependencies for building fex include the above packages, along with any
associated -dev or -devel packages for distros that use split packages, and,
of course, git to pull the source code.

####Building

There is no need to configure:

```bash
git clone http://github.com/TrilbyWhite/fex.git
cd fex
make
```

####Installation

make install accepts DESTDIR and PREFIX variables:

```bash
make DESTDIR=/path/to/pkgdir PREFIX=/usr install
```

