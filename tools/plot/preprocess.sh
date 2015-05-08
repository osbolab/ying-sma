#!/bin/bash

cat $1 | grep delay | grep "n(0)" | cut -d":" -f3 > ./delay_output
cat $1 | grep overall | grep "n(0)" | cut -d":" -f3 > ./utility_output
cat $1 | grep utilization | grep "n(0)" | cut -d"," -f 2-3 > ./utilization_output
cat $1 | grep "Complete download" | grep "n(57)" | cut -d"," -f 4 > ./util_57.txt
cat $1 | grep "Complete download" | grep "n(11)" | cut -d"," -f 4 > ./util_11.txt
cat $1 | grep "Complete download" | grep "n(24)" | cut -d"," -f 4 > ./util_24.txt
cat $1 | grep "Complete download" | grep "n(36)" | cut -d"," -f 4 > ./util_36.txt
cat $1 | grep "Complete download" | grep "n(29)" | cut -d"," -f 4 > ./util_29.txt
cat $1 | grep "Complete download" | grep "n(14)" | cut -d"," -f 4 > ./util_14.txt
cat $1 | grep "Complete download" | grep "n(13)" | cut -d"," -f 4 > ./util_13.txt
cat $1 | grep "Complete download" | grep "n(59)" | cut -d"," -f 4 > ./util_59.txt
cat $1 | grep "Complete download" | grep "n(39)" | cut -d"," -f 4 > ./util_39.txt
cat $1 | grep "Complete download" | cut -d"," -f  5 > ./before_deadline.txt


