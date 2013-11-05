#!/bin/bash

# functions
function out() {
	str=$2; [[ -n "$3" ]] && str="${2/$3/\033[36m$3\033[0m}"
	echo -e "\033[$1m==>\033[0m $str"
}
function sub() {
	str=$1; [[ -n "$2" ]] && str="${1/$2/\033[36m$2\033[0m}"
	echo -e "      $str"
}
function msg() { out 32 "$@"; }
function warn() { out 33 "$@"; }
function err() { out 31 "$@"; }
function die() { err "Installation cannot continue"; exit 1; }

# check for macports
if [[ ! $(which port 2>/dev/null) ]]; then
	err "Unable to find MacPorts" "MacPorts"
	sub "Follow guide at www.macports.org to install" "www.macports.org"
	die
fi

# check for dependencies
deps="
libsndfile
fftw-3
git-core
"
need=""
for pkg in $deps; do
	msg "Checking for $pkg" "$pkg"
	if [[ -z "$(port installed | grep $pkg)" ]]; then
		need="$need $pkg"
		warn "$pkg not found" "$pkg"
		sub "$pkg queued for installation" "$pkg"
	fi
done
if [[ -n "$need" ]]; then
	msg "Installing$need via macports" "$need"
	sudo port install $need
else
	msg "Found all dependencies"
fi

# get fex from git
msg "Cloning into fex at github.com" "fex"
git clone http://github.com/TrilbyWhite/fex.git
if [[ $? -ne 0 ]]; then
	err "Error cloning into fex"
	die
fi
msg "Sources retreived, building fex"
cd fex
./configure
make
if [[ $? -ne 0 ]]; then
	err "Error building fex"
	die
fi
sudo make install
if [[ $? -ne 0 ]]; then
	err "Error installing fex"
	die
fi





