#!/bin/bash

## comment any lines in main to skip that step ##
main() {
	startdir=$(pwd)
	get_deps
	get_source
	prepare
	build
	sudo apt-get uninstall $remove_deps
	sudo apt-get uninstall $remove_build_deps
	package
	mv "${pkgname}_${pkgver}-${pkgrel}_${arch}.deb" $startdir
}

## do not edit below this line ##

if [[ ! -f DEBBUILD ]]; then
	echo "ERROR: DEBBUILD does not exist."; exit 1;
fi
source DEBBUILD

remove_deps=""
remove_build_deps=""
deps=""

get_deps() {
	local to_install=""
	for dep in "${makedepends[@]}"; do
		# check if installed, continue if exists
		to_install="${to_install} $dep"
		remove_build_deps="${remove_build_deps} $dep"
	done
	for dep in "${depends[@]}"; do
		# check if installed, continue if exists
		to_install="${to_install} $dep"
		deps="${deps}${dep}, "
		remove_deps="${remove_deps} $dep"
	done
	sudo apt-get install $to_install
	deps="${deps%, }"
}

get_source() {
	for src in "${source[@]}"; do
		local protocol="$(echo "$src" | cut -d ':' -f 1)"
		case "$protocol" in
			git) git clone "$src" ;;
			*) error "unknown protocol \"$protocol\"" ;;
		esac
	done
}

package() {
	checkinstall \
		--default \
		--install=no \
		--type debian \
		--pkgname "${pkgname}" \
		--pkgversion "${pkgver}" \
		--pkgrelease "${pkgrel}" \
		--pkglicense "${license}" \
		--pkgsource "${url}" \
		--maintainer "${maintainer}" \
		--requires "${deps}" \
		--gzman \
		make install
}

main


