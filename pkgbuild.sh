#!/bin/sh
# run with git root as pwd
# $1 version
# $2 out-dir
set -e

currdir=$(pwd)
pkgname=conflip
pkgver=${1:?Invalid commands. Syntax: arch.sh <version> <outdir>}
outdir=$(readlink -f ${2:?Invalid commands. Syntax: arch.sh <version> <outdir>})
pkgdir=$(mktemp -d)

pushd conflip
echo qpm install
popd

pushd $(mktemp -d)

qmake -r "$currdir"
make
make lrelease
make INSTALL_ROOT="$pkgdir" install

popd

install -D -m 644 -p conflip/${pkgname}.desktop $pkgdir/usr/share/applications/${pkgname}.desktop
install -D -m 644 -p conflip/icons/${pkgname}.svg $pkgdir/usr/share/icons/hicolor/scalable/apps/${pkgname}.svg

mkdir -p "$outdir"
cd "$pkgdir"
export XZ_OPT=-9
tar cJf "$outdir/${pkgname}-${pkgver}.tar.xz" ./*
