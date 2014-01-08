###FEX Distibion and packaging

Currently only a PKGBUILD for archlinux is provided.

I will work on making suitable packaging materials for other
distributions and/or will gladly accept pull requests from those who are
more familiar with other distros.

####Dependencies

The following packages, all their dependencies are required.

+ cairo
+ desktop-file-utils
+ fftw
+ libxpm
+ libsndfile
+ python2

Dependencies for building fex include the above packages, along with any
associated -dev packages for distros that use split packages (e.g.,
ubuntoo), and the following:

+ git
+ latex2man

####Building

There is no need to configure:

```bash
git clone http://github.com/TrilbyWhite/fex.git
cd fex
make
make man
```

####Installation

make install accepts DESTDIR and PREFIX variables:

```bash
make DESTDIR=/path/to/pkgdir PREFIX=/usr install
```

