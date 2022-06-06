#!/bin/sh

cd vendor/libDaisy
make

cd ../DaisySP
make

cd ../..
make
