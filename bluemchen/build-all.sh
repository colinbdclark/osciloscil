#!/bin/sh

cd vendor/libDaisy
make $1

cd ../DaisySP
make $1

cd ../..
make $1
