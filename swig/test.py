#!/usr/bin/python
from exodus import *
xo=var()

print "--- Testing ---"

filename="tempdbfile564"

if xo.createfile(filename,""): 
	print filename, " file created\n"
else:
	print filename, "file not created. maybe already exists. try deletefile filename\n"

#transactions
xo.begintrans()

#write some records
record=var("X").str(1000)

for ii in range(1,100):
	if not record.write(filename,ii):
		break                
	print ii,
print

xo.committrans()

#enumerate db files
dbfilenames=xo.listfiles()
print dbfilenames

#extract fields
for ii in range(1,dbfilenames.dcount(FM)):
	print ii,dbfilenames(ii)

#delete the db file
if var(filename).deletefile():
	print filename," file deleted"


print "Pick date:", xo.date(), xo.date().oconv("D")
print "Pick time:", xo.time(), xo.time().oconv("MT")

print "Finished"
