#!/usr/bin/python

print "--- Testing ---\n"

from exodus import *

options=var("")
if not var().connect(options):
        print "no default database connection\n"
else:
        filename=var("tempdbfile564")
        options=var("")

        if var().createfile(filename,options): 
                print filename, " file created\n"
        else:
                print filename, "file not created. maybe already exists. try deletefile filename\n"
        

        #write 10 records
	record=var("X").str(100)
        for ii in range(1,100):
                id=var(ii)
                if not record.write(filename,id):
                        break
                
                print id," ",
        
        print "\n"

        if filename.deletefile():
               print "filename file deleted\n"
        

print "Pick date:", var().date(), var().date().oconv(var("D"))
print "Pick time:", var().time(), var().time().oconv(var("MT"))

print "Finished"
