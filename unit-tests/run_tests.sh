#!/bin/bash

set -x
set -e

cmake -Bbuild -H.
make -C build
make -C build test
