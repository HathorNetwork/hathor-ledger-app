#!/bin/bash

set -x
set -e

cmake -Bbuild -H.
make -C build
# This envvar enables output (stdout from tests) to be shown in failures, great for debugging
CTEST_OUTPUT_ON_FAILURE=1 make -C build test
