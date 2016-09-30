#!/usr/bin/python

# A STL file has the following format
# solid COMMENTS
# facet normal 0.00 0.00 1.00
#   outer loop
#      vertex  2.00  2.00  0.00
#      vertex -1.00  1.00  0.00
#      vertex  0.00 -1.00  0.00
#   endloop 
# endfacet
# ...
# endsolid

# This script dumbly read facets and the 3 lines of coordinates following a "facet" identifier
#  normals are ignored
# ASCII file only
import sys;

f = open(sys.argv[1],"r")

l = f.readline().strip()
while l != "":
  w = l.split();
  if(len(w)):
    if(w[0] == "solid"):
      print "";
    if(w[0] == "facet"):
      f.readline()
      n1 = f.readline().strip().split()
      n2 = f.readline().strip().split()
      n3 = f.readline().strip().split()
      print "triangle " + n1[1] + " " + n1[2] + " " + n1[3] + " " + n2[1] + " " + n2[2] + " " + n2[3] + " " + n3[1] + " " + n3[2] + " " + n3[3]
      f.readline()
  l = f.readline().strip()

f.close();

