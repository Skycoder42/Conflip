#!/bin/sh
# run with git root as pwd
# $1 out-dir
# $2 version
set -e

currdir=$(pwd)
pkgname=conflip
pkgver=${1:?Invalid commands. Syntax: arch.sh <version> <outdir>}
outdir=${2:?Invalid commands. Syntax: arch.sh <version> <outdir>}
pkgdir=$(mktemp -d)

pushd conflip
qpm install
popd

pushd $(mktemp -d)

qmake -r "$currdir"
make
make lrelease
make INSTALL_ROOT="$pkgdir" install

popd

install -D -m 644 -p conflip/${pkgname}.desktop $pkgdir/usr/share/applications/${pkgname}.desktop
install -D -m 644 -p conflip/icons/${pkgname}.svg $pkgdir/usr/share/icons/hicolor/scalable/apps/${pkgname}.svg

cd "$outdir"
export XZ_OPT=-9
tar cJf ${pkgname}-${pkgver}.tar.xz "$pkgdir"
