#!/bin/bash


if [ $1 != $0 ]; then
  # Move files into flat directories
  #find . -mindepth 2 -type f -exec mv {} ./ \;

  # Make filenames lowercase
  #mv $1 `echo $1 | tr '[A-Z]' '[a-z]'`

  # Run this script on files
  #find . -depth 1 -type f -exec ./fix.sh {} \;

  # Replace header names in includes
  cat $1 | perl -ne 's/(?<=#include ")([a-zA-Z]+\/)*(.+\.hh")/\L\2/g; print' > "new/$1"

  # Diff the new versions
  #find . -depth 1 -type f -exec diff {} new/{} \;
fi
