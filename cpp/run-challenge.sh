#!/bin/bash

pushd `dirname $0` > /dev/null
make
echo -- fe output --
fe/fe -f ../materials/challenge.bin
popd > /dev/null

