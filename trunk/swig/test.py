#!/usr/bin/python
print "Started\n"
from exodus import *
print "Pick date:", var().date(), var().date().oconv(var("D"))
print "Pick time:", var().time(), var().time().oconv(var("MT"))
print var().createfile(var("tempfilepython"),var(""))
print "Finished"
