#!/bin/bash

pushd `dirname $0` > /dev/null
cargo run -- -f ../materials/challenge.bin
popd > /dev/null

