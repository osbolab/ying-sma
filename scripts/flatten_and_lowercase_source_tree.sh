#!/bin/bash


# Flatten file tree
find . -mindepth 2 -type f -exec mv {} ./ \;
find . -type d -exec rm -r {} \;
# Make names lowercase
find . -type f -exec sh -c 'mv $1 `echo $1 | tr "[A-Z]" "[a-z]"`' _ {} \;

mkdir ./new/

# Replace include entries with new paths
for filename in $(find . -type f); do
  if [ $filename != $0 ]; then
   cat $filename | perl -ne 's/(?<=#include ")([a-zA-Z]+\/)*(.+\.hh")/\L\2/g; print' > "./new/$filename"
  fi
done

# Show pending changes
find . -maxdepth 1 -type f -exec diff {} new/{} \;

# Move modified files to finish
# mv new/* ./
# rm -r new
