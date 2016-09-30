#!/usr/bin/python

# This imports the standart datafiles (.ply) used for the "standford bunny"
#  Only triangular faces are implemented, only ASCII format is supported
import sys

f = open(sys.argv[1]);

nnodes = 0
nfaces = 0

l = f.readline().split();
while(l[0] != "end_header"):
  if(l[0] == "element"):
    if(l[1] == "vertex"):
      nnodes = int(l[2]);
    elif(l[1] == "face"):
      nfaces = int(l[2]);
  l = f.readline().split();

print "object_begin"
print "raw_vertex"

for i in range(0,nnodes):
  l = f.readline().split();
  print l[0] + " " + l[1] + " " + l[2]

print "raw_end"

print "raw_triangle_v"

for i in range(0,nfaces):
  l = f.readline().split();
  print l[1] + " " + l[2] + " " + l[3]

print "raw_end"
print "object_end"

