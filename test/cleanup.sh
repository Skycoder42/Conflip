#!/bin/sh
set -e

cd $(dirname $(realpath $0))
rm -rf dirs
rm -rf files
rm -rf repo
cp -r backup/* ./
