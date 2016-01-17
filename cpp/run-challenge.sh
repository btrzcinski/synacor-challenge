#!/bin/bash

pushd `dirname $0` > /dev/null
make
echo -- fe output --
fe/fe -f ../materials/challenge.bin 2>fe.log
echo -- log tail --
tail fe.log
popd > /dev/null

