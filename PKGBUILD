# Maintainer: Jesse AKA "Trilby" <jesse [at] mcclurek9 [dot] com>
_gitname="fex"
pkgname="${_gitname}-git"
pkgver=2.0
pkgrel=1
pkgdesc='Frequency Excursion Calculator'
url='http://github.com/TrilbyWhite/fex.git'
arch=('any')
license=('GPL3')
depends=('libx11' 'libxrandr' 'cairo' 'freetype2')
makedepends=('git' 'texlive-core')
source=("${_gitname}::git://github.com/TrilbyWhite/fex.git")
sha256sums=('SKIP')

pkgver() {
	cd "${_gitname}";
	echo "2.$(git rev-list --count HEAD).$(git describe --always )"
}

build() {
	cd "${_gitname}"
	make
	make man
}

package() {
	cd "${_gitname}"
	make DESTDIR="${pkgdir}" install
}
