summary: Frequency Excursion Calculator
name: fex
version: 2
release: 1
license: GPL3
group: Applications/Science
source: https://github.com/BehaviorEnterprises/Fex.git
url: https://wiki.BehaviorEnterprises.com
vendor: Behavior Enterprises
packager: Jesse McClure <jesse@mccluresk9.com>
requires: desktop-file-utils, libasound2, libcairo2, libfftw3-3, libsndfile1, libxpm4, python, python-gtk2
buildrequires: libasound2-dev, libcairo2-dev, libfftw3-dev, gcc, libsndfile1-dev, libx11-dev, libxpm-dev, pkg-config
prefix: /usr

%description
Frequency Excursion Calculator

%prep
%setup -c

%build
make

%install
make "DESTDIR=${RPM_BUILD_ROOT}" install

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

