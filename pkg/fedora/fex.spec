summary: Frequency Excursion Calculator
name: fex
version: 1.0
release: 1
copyright: GPL3
group: Applications/Science
source: https://github.com/BehaviorEnterprises/fex.git
nosource: 0
url:
vendor: Behavior Enterprises
packager: Jesse McClure jesse [at] mccluresk9 [dot] com
requires: cairo, desktop-file-utils, fftw, libxpm, libsndfile, python2, git, texlive-core
prefix: /usr

%description
This spec file is a work in progress.  It is not yet complete, and
completely untested.  Do not use as-is.  Input from rpm-based distro
users would be appreciated.
TODO
check dependency names on rpm system
test whether git source for prep can even work
determine suitable version numbering (git tags perhaps)

%prep
rm -rf "${RPM_BUILD_DIR}/fex-${version}-${release}"
git clone https://github.com/BehaviorEnterprises/fex.git fex-${version}-${release}

%build
make

%install
make "DESTDIR=${RPM_BUILD_ROOT}" "PREFIX=${prefix}" install

%clean
rm -rf "${RPM_BUILD_ROOT}"

%post
update-desktop-database -q

%postun
update-desktop-database -q

%files
/usr/bin/fex
/usr/bin/fex-gtk
/usr/share/applications/
/usr/share/applications/fex.desktop
/usr/share/fex/
/usr/share/fex/config
/usr/share/man/man1/fex-help.1.gz
/usr/share/man/man1/fex.1.gz
/usr/share/pixmaps/fex.png

