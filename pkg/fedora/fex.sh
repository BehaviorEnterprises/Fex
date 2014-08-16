#!/bin/bash

## CHECK BUILD DEPENDENCIES:
makedeps=('cairo-devel' 'fftw-devel' 'git' 
	'libsndfile-devel' 'libXpm-devel' 'rpm-build')
to_install=
for dep in "${makedeps[@]}"; do
	rpm -q $dep >/dev/null 2>&1 || to_install+=" $dep"
done
if [[ -n $to_install ]]; then
	echo "installing missing build dependencies"
	sudo yum install $to_install
fi

## CHECK BUILD ENVIRONMENT:
TOPDIR=$(rpmbuild --eval '%_topdir')
mkdir -p ${TOPDIR}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

## GET CODE AND MAKE SOURCE TARBALL:
git clone https://github.com/BehaviorEnterprises/fex.git
cd fex
FEX_VER="2.$(git rev-list --count HEAD)"
make dist
mv ./fex*.tar.gz ${TOPDIR}/SOURCES/fex-${FEX_VER}.tar.gz

## PREPARE SPEC FILE:
cp fex/fedora/fex.spec ${TOPDIR}/SPECS/
sed -i 's/^version: .*$/version: '$FEX_VER'/' ${TOPDIR}/SPECS/fex.spec
sed -i 's/[0-9]*\.[0-9]*\.tar\.gz$/'$FEX_VER'.tar.gz/' ${TOPDIR}/SPECS/fex.spec

## CLEAN UP:
cd ..
rm -rf fex

## RUN RPMBUILD:
cd ${TOPDIR}/SPECS
rpmbuild -ba fex.spec

