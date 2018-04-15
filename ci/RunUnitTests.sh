#! /usr/bin/env sh
##
## This script runs the unit tests in travis.
## envs:
##     - EFD_BUILD_HOME
##

cd $EFD_BUILD_HOME
make test
