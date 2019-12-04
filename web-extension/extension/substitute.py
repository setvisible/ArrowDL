#substitute.py
#
# Copyright 2019 Sebastien Vavassori. All rights reserved.
# Use of this source code is governed by a LGPL license that can be
# found in the LICENSE file.
#
import sys
import os

# print(sys.argv)

oldvalue = sys.argv[1]
newvaluefile = sys.argv[2]
inputfile = sys.argv[3]
outputfile = sys.argv[3] + "_temp"

with open(newvaluefile) as f:
    first_line = f.readline()

newvalue = first_line.split('\n')

fin = open(inputfile, "rt")
fout = open(outputfile, "wt")

for line in fin:
	fout.write(line.replace(oldvalue, newvalue[0]))
	
fin.close()
fout.close()

os.remove(inputfile)
os.rename(outputfile, inputfile)
