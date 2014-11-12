#!/usr/bin/python
from __future__ import print_function

import os
from fileinput import FileInput

headers=[]
oldheaders={}

for dname, dirs, files in os.walk('../../include/sma/core'):
  headers += files

for header in headers:
  old = header.replace('_', '').replace('.hpp', '.hh')
  oldheaders[old] = header

for dname, dirs, files in os.walk('.'):
  files = [os.path.join(dname, fname) for fname in files]
  for line in FileInput(files, inplace=True):
    match = 'xxxxxx&&&&xxxxxxx'
    for header in oldheaders:
      if header in line:
        match = header
        break
    print(line.replace('"'+match+'"', '<sma/core/'+oldheaders[header]+'>'), end='')

