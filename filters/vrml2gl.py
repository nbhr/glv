#!/usr/bin/python
import sys;

# This filters only vrml files defining SIMPLE IndexedLineSet and IndexedFaceSet primitives.
# everything else is ignored
f = open(sys.argv[1],"r");

currentMode = "";
l = f.readline();
while l != "":
  l = l.strip();
  d = l.split();
  if("IndexedLineSet" in d):
    mode = "line";
  elif("IndexedFaceSet" in d):
    mode = "face";
  elif("point" in d  and  "[" in d):
    # Read points
    print "clear_vertex"
    print "raw_vertex"
    d = f.readline().strip().split()
    while not("]" in d):
      print d[0] + " " + d[1] + " " + d[2][:-1]
      d = f.readline().strip().split()
    print "raw_end"
  elif("coordIndex" in d):
    d = f.readline().strip().split(",")
    while not("]" in d):
      if(mode == "line"):
        print "line_v " + d[0] + " " + d[1]
      elif(mode == "face"):
        if(d[3].strip() == "-1"):
          print "triangle_v " + d[0] + " " + d[1] + " " + d[2];
        else:
          print "quad_v " + d[0] + " " + d[1] + " " + d[2] + " " + d[3];
      d = f.readline().strip().split(",")
  l = f.readline();

f.close();

