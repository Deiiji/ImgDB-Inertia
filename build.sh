#!/bin/sh

qmake
make

cd tests

qmake
make

cd ..
