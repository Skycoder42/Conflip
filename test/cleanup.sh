#!/bin/sh
set -e

cd $(dirname $(realpath $0))
rm -rf dirs
rm -rf files
rm -rf ini
rm -rf xml
rm -rf repo
cp -r backup/* ./
