# Maintainer: Jesse McClure AKA "Trilby" <jmcclure [at] cns [dot] umass [dot] edu>
_gitname="fex"
pkgname="${_gitname}-git"
pkgver=0
pkgrel=1
pkgdesc="Frequency Excursion Calculator"
url="https://github.com/TrilbyWhite/fex"
arch=('any')
license=('GPLv3')
depends=('libx11' 'cairo' 'fftw' 'libsndfile')
makedepends=('git')
source=("${_gitname}::git://github.com/TrilbyWhite/fex.git")
sha256sums=('SKIP')

pkgver() {
	cd "${_gitname}";
	echo "2.$(git rev-list --count HEAD).$(git describe --always )"
}

build() {
	cd "${_gitname}"
	make linux
}

package() {
	cd "${_gitname}"
	make PREFIX=/usr DESTDIR="${pkgdir}" install.linux
}
