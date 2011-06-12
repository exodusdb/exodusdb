#!/usr/bin/python
from exodus import *
print var().date().oconv(var("D"))
print var().time().oconv(var("MT"))
print var().createfile(var("tempfilepython"),var(""))
