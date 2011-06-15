#!/usr/bin/python
print "Started\n"
from exodus import *
print var().date().oconv(var("D"))
print var().time().oconv(var("MT"))
print var().createfile(var("tempfilepython"),var(""))
print "Finished"
