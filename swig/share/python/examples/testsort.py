#!/usr/bin/python

from exodus import *
x=var()

def sortselect(filename, sortselectcmd) :

	print "\nSSELECT the data - ", sortselectcmd

	if not x.select(sortselectcmd):
		print "Cannot sselect"
		return

	print "Read the data"

	#could also use the readnextrecord() function here
	# if we had used the new selectrecord() function above
	record=var()
	key=var()
	value=var()
	while x.readnext(key,value) :

		if not record.read(filename, key) :
			print key, " missing from file"
			continue

		print key, ": ", record.convert(FM, "|")


filename="myclients"
dictfilename="dict_"+ filename

if not x.connect(""):
	abort("Cannot connect to database. Please check configuration or run configexodus.")

#leave the test data files around for playing with
cleanup=False
if cleanup :
	x.deletefile(filename)
	x.deletefile(dictfilename)
		
print "\nOpen or create test file ", filename

if not x.open(filename,""):
	x.createfile(filename)
	if not x.open(filename,""):
		x.abort("Cannot open ".filename)

print "\nOpen or create the test files dictionary ", dictfilename

if not x.open(dictfilename,""):
	x.createfile(dictfilename)
	if not x.open(dictfilename,""):
		x.abort("Cannot open dictionary " + dictfilename)

print "\nPrepare some dictionary records"

fm=FM.__str__()
dictrecs = ""
dictrecs  =      "CLIENT_CODE |F|0|Code     ||||  ||L|8"
dictrecs += fm + "CLIENT_NAME |F|1|Name     ||||  ||T|15"
dictrecs += fm + "CLIENT_TYPE |F|2|Type     ||||  ||L|5"
dictrecs += fm + "DATE_CREATED|F|3|Date     ||||D4||L|12"
dictrecs += fm + "TIME_CREATED|F|4|Time     ||||MTH       ||L|12"
dictrecs += fm + "BALANCE     |F|5|Balance  ||||MD20P     ||R|10"
dictrecs += fm + "TIMESTAMP   |F|6|Timestamp||||[DATETIME]||L|12"
dictrecs += fm + "@CRT|G| |CLIENT_CODE CLIENT_NAME CLIENT_TYPE BALANCE DATE_CREATED TIME_CREATED TIMESTAMP"

print "\nWrite the dictionary records to the dictionary"

dictrecs=var(dictrecs)
nrecs=dictrecs.dcount(FM)
for recn in range(1,nrecs) :

	dictrec=dictrecs(recn)
	key=dictrec.field("|", 1)
	rec=dictrec.field("|", 2, 9999)

	print key, ": ", rec
	key.trimmer("")
	rec.trimmer("")
	rec.swapper(" |", "|","")
	rec.converter("|", FM)

	rec.write(dictfilename, key)

	#check we can read the record back
	print "dictfile ", dictfilename, "key", key,
	rec2=var()
	if rec2.read(dictfilename, key):
		if rec2 != rec: 
			print "record differs?!", rec2
	else:
		print "Cant read ", key, " back"

rec=var()
if not rec.read(dictfilename,"BALANCE"):
	print "Cant read BALANCE record from dictionary"

print "\nClear the client file"
x.clearfile(filename)

print "\nPrepare some data records in a readable format"

recs = ""
recs += fm + "SB001|Client AAA |A |15070|76539|1000.00|15070.76539"
recs += fm + "JB002|Client BBB |B |15000|50539|200.00|15000.50539"
recs += fm + "JB001|Client CCC |B |15010|60539|2000.00|15010.60539"
recs += fm + "SB1  |Client SB1 |1 |     |     |       |   "
recs += fm + "JB2  |Client JB2 |2 |14000|10539|0      |14000.10539"
recs += fm + "JB10 |Client JB10|10|14010|10539|2000.00|14010.10539"
recs=var(recs).splice(1, 1, "")

print "\nWrite the data records to the data file"

nrecs=recs.dcount(FM)
for recn in range(1,nrecs):

	rec=recs(recn)
	key=rec.field("|", 1)
	rec=rec.field("|", 2, 9999)

	print key, ": ", rec
	key.trimmer("")
	rec.trimmer("")
	rec.swapper(" |", "|","")
	rec.converter("|", FM)

	rec.write(filename, key)

prefix="SELECT "+ filename

sortselect(filename, prefix+ " BY CLIENT_CODE")

sortselect(filename, prefix+ " BY BALANCE BY CLIENT_CODE")

sortselect(filename, prefix+ " BY TIMESTAMP")

sortselect(filename, prefix+ " WITH CLIENT_TYPE 'B' BY BALANCE")

cmd=var("list "+ filename+ " id-supp")
print "\nList the file using ", cmd.quote()
cmd.osshell()

cmd=var("list "+ dictfilename)
print "\nList the dict using ", cmd.quote()
cmd.osshell()

if cleanup :
	print "\nCleaning up. Delete the files"
	x.deletefile(filename)
	x.deletefile(dictfilename)

print "\nJust type 'list' to see the syntax of list"
print "or list dict_"+ filename+ " to see the dictionary"
print "Type edic cli/src/testsort to see or edit/recompile this program."


