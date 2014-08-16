#!/bin/bash

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
cp pkg/fedora/fex.spec ${TOPDIR}/SPECS/
sed -i 's/^version: .*$/version: '$FEX_VER'/' ${TOPDIR}/SPECS/fex.spec
sed -i 's/[0-9]*\.[0-9]*\.tar\.gz$/'$FEX_VER'.tar.gz/' ${TOPDIR}/SPECS/fex.spec

## INSTALL BUILD DEPS:
## Uncomment the next line for the initial build
#sudo yum-builddep ${TOPDIR}/SPECS/fex.spec

## CLEAN UP:
cd ..
rm -rf fex

## RUN RPMBUILD:
cd ${TOPDIR}/SPECS
rpmbuild -ba fex.spec

