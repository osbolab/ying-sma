#!/bin/bash

cat $1 | grep delay | grep "n(0)" | cut -d":" -f3 > ./delay_output
cat $1 | grep overall | grep "n(0)" | cut -d":" -f3 > ./utility_output
cat $1 | grep utilization | grep "n(0)" | cut -d"," -f 2-3 > ./utilization_output

