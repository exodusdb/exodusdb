/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/* exclude from documentation - unlikely to be implemented ever since qm's cli interface is primitive
#include <exodus/mv.h>
#include <exodus/mvimpl.h>
#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

namespace exodus {

//turn off warnings about strncpy (programmers ensure that all strncpy and strncat are followed by str[size-1]="\0";
#pragma warning (disable: 4996)

extern "C"
{
	#include "qmclilib.h"
}

//
//0 SV_OK		Action successful
//1 SV_ON_ERROR	Action took the ON ERROR clause to recover from a situation that would
//		otherwise cause the server process to abort.
//2 SV_ELSE	Action took the ELSE clause. In most cases the QMStatus() function can
//		be used to determine the error number.
//3 SV_ERROR	An error occurred for which extended error text can be retrieved using the
//		QMError() function.
//4 SV_LOCKED	The action was blocked by a lock held by another user. The QMStatus()
//		function can be used to determine the blocking user.
//5 SV_PROMPT	A command executed on the server is waiting for input. The only valid
//		client functions when this status is returned are QMRespond(),
//		QMEndCommand and QMDisconnect.
//
var _STATUS;

#define MV_MAX_KEY_LENGTH 8092-1
#define MV_MAX_KEY_LENGTH_EXCEEDED "MV_MAX_KEY_LENGTH_EXCEEDED"
#define MV_MAX_RECORD_LENGTH 1048576-1
#define MV_MAX_RECORD_LENGTH_EXCEEDED "MV_MAX_RECORD_LENGTH_EXCEEDED"
#define MV_MAX_FILENAME_LENGTH 8092-1
#define MV_MAX_FILENAME_LENGTH_EXCEEDED "MV_MAX_FILENAME_LENGTH_EXCEEDED"
#define MV_MAX_CONNECTPARAM_LENGTH 8092-1
#define MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED "MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED"
#define MV_BAD_FILENAME "MV_BAD_FILENAME"

bool var::connectlocal(const var& accountname)
{
	assert(accountname.assigned());

	//if (!QMConnect(SERVER_ADDRESS, SERVER_PORT, SERVER_USER, SERVER_PASSWORD,SERVER_ACCOUNT))
	if (accountname.toString().length()>=MV_MAX_CONNECTPARAM_LENGTH)
	{
		_STATUS=MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED;
		return false;
	};
	char caccountname[MV_MAX_CONNECTPARAM_LENGTH+1]="";
	strncpy(caccountname,accountname.toString().c_str(),MV_MAX_CONNECTPARAM_LENGTH);
	caccountname[MV_MAX_CONNECTPARAM_LENGTH-1]='\0';

	if (!QMConnectLocal(caccountname))
	{
		//TODO MAP QM statuses to some invented var standard
		_STATUS=QMStatus();
		return false;
	}

	var_mvint=QMGetSession();
	var_mvstr=intToString(var_mvint);
	var_mvtyp='3';

	return true;
}

bool var::connect(const var& address, const var& port, const var& user, const var& pass, const var& account)
{
	//if (!QMConnect(SERVER_ADDRESS, SERVER_PORT, SERVER_USER, SERVER_PASSWORD,SERVER_ACCOUNT))

	assert(address.assigned());
	assert(port.assigned());
	assert(user.assigned());
	assert(pass.assigned());
	assert(account.assigned());

	if (address.toString().length()>=MV_MAX_CONNECTPARAM_LENGTH)
	{
		_STATUS=MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED;
		return false;
	};
	char caddress[MV_MAX_CONNECTPARAM_LENGTH+1]="";
	strncpy(caddress,address.toString().c_str(),MV_MAX_CONNECTPARAM_LENGTH);
	caddress[MV_MAX_CONNECTPARAM_LENGTH-1]='\0';

	assert(port.isnum());

	if (user.toString().length()>=MV_MAX_CONNECTPARAM_LENGTH)
	{
		_STATUS=MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED;
		return false;
	};
	char cuser[MV_MAX_CONNECTPARAM_LENGTH+1]="";
	strncpy(cuser,user.toString().c_str(),MV_MAX_CONNECTPARAM_LENGTH);
	cuser[MV_MAX_CONNECTPARAM_LENGTH-1]='\0';

	if (pass.toString().length()>=MV_MAX_CONNECTPARAM_LENGTH)
	{
		_STATUS=MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED;
		return false;
	};
	char cpass[MV_MAX_CONNECTPARAM_LENGTH+1]="";
	strncpy(cpass,pass.toString().c_str(),MV_MAX_CONNECTPARAM_LENGTH);
	cpass[MV_MAX_CONNECTPARAM_LENGTH-1]='\0';

	if (account.toString().length()>=MV_MAX_CONNECTPARAM_LENGTH)
	{
		_STATUS=MV_MAX_CONNECTPARAM_LENGTH_EXCEEDED;
		return false;
	};
	char caccount[MV_MAX_CONNECTPARAM_LENGTH+1]="";
	strncpy(caccount,account.toString().c_str(),MV_MAX_CONNECTPARAM_LENGTH);
	caccount[MV_MAX_CONNECTPARAM_LENGTH-1]='\0';

	if (!QMConnect(caddress,port.toInt(),cuser,cpass,caccount))
	{
		//TODO MAP QM statuses to some invented var standard
		_STATUS=QMStatus();
		return false;
	}

	var_mvint=QMGetSession();
	var_mvstr=intToString(var_mvint);
	var_mvtyp='3';

	return true;
}

bool var::disconnect()
{
	var_mvstr=L"";
	var_mvint=0;
	var_mvtyp=L'3';
	QMDisconnect();

	return true;
}

bool var::open(const var& filename)
{
	assert(filename.assigned());

	 return open("",filename);
}

bool var::open(const var& dictcode,const var& filename)
{
	assert(dictcode.assigned());
	assert(filename.assigned());

	if (1+dictcode.toString().length()+filename.toString().length()>MV_MAX_FILENAME_LENGTH)
	{
		_STATUS=MV_MAX_FILENAME_LENGTH_EXCEEDED;
		return false;
	};

	char cfilename[MV_MAX_FILENAME_LENGTH+1]="";
	strncpy(cfilename,dictcode.toString().c_str(),MV_MAX_CONNECTPARAM_LENGTH);
	cfilename[MV_MAX_CONNECTPARAM_LENGTH-1]='\0';

	if (filename.length()>0)
	{
	 if (dictcode.length()>0) strcat(cfilename," ");
	 strncat(cfilename,filename.toString().c_str(),MV_MAX_CONNECTPARAM_LENGTH);
	}
	cfilename[MV_MAX_CONNECTPARAM_LENGTH-1]='\0';

	int fileno;
	if (0==(fileno=QMOpen(cfilename)))
	{
		_STATUS=QMStatus();
		return false;
	}

	var_mvint=fileno;
	var_mvstr=intToString(fileno);
	var_mvtyp='3';

	return true;

}

void var::close()
{
	if (var_mvtyp!='0')
		QMClose(var_mvint);
}

bool var::readv(const var& fileno,const var& key,const int fieldno)
{

	if (!read(fileno,key))
		return false;

	var_mvstr=extract(fieldno).var_mvstr;

	return true;
}

bool var::read(const var& fileno,const var& key)
{
	assert(fileno.assigned());
	assert(key.assigned());

	char* crecord;
	if (key.toString().length()>MV_MAX_KEY_LENGTH)
	{
		_STATUS=MV_MAX_KEY_LENGTH_EXCEEDED;
		return false;
	};
	char ckey[MV_MAX_KEY_LENGTH+1]="";
	strncpy(ckey,key.toString().c_str(),MV_MAX_KEY_LENGTH);
	ckey[MV_MAX_KEY_LENGTH-1]='\0';

	int errno;
	crecord=QMRead(fileno.var_mvint,ckey,&errno);
	if (errno!=0)
	{
		QMFree(crecord);
		_STATUS=QMStatus();
		return false;
	}

	var_mvstr=wstringfromUTF8((const UTF8*)crecord,int(strlen(crecord)));
	var_mvint=0;
	var_mvtyp='1';

	QMFree(crecord);

	return true;
}

bool var::readnext(var& key) const
{

	assert(assigned());
	assert(isnum());

	//TODO check listn is 0-10 according to QM docs Aug 2006
	char* ckey;
	ckey=QMReadNext(toInt());
	if (ckey==NULL)
	{
		QMFree(ckey);
		_STATUS=QMStatus();
		return false;
	}

	var_mvstr=wstringfromUTF8((const UTF8*)ckey,int(strlen(ckey)));

	QMFree(ckey);

	return true;

}

bool var::lock(const var& key) const
{
	assert(assigned());
	assert(isnum());
	assert(key.assigned());

	char* crecord;
	if (key.toString().length()>MV_MAX_KEY_LENGTH)
	{
		_STATUS=MV_MAX_KEY_LENGTH_EXCEEDED;
		return false;
	};
	char ckey[MV_MAX_KEY_LENGTH+1]="";
	strncpy(ckey,key.toString().c_str(),MV_MAX_KEY_LENGTH);
	ckey[MV_MAX_KEY_LENGTH-1]='\0';

	int errno;
	crecord=QMReadu(var_mvint, ckey, false, &errno);
	if (_STATUS==SV_LOCKED)
	{
		return false;
	}
	if (errno!=0)
	{
		QMFree(crecord);
		_STATUS=QMStatus();
		return false;
	}

	QMFree(crecord);

	return true;
}

void var::unlock(const var& key) const
{
	assert(assigned());
	assert(key.assigned());

	if (key.toString().length()>MV_MAX_KEY_LENGTH)
	{
		_STATUS=MV_MAX_KEY_LENGTH_EXCEEDED;
		return;
	};
	char ckey[MV_MAX_KEY_LENGTH+1]="";
	strncpy(ckey,key.toString().c_str(),MV_MAX_KEY_LENGTH);
	ckey[MV_MAX_KEY_LENGTH-1]='\0';

	//int errno;
	QMRelease(var_mvint,ckey);

	return;
}

void var::unlockall() const
{
	char ckey[1]="";

	QMRelease(0,ckey);

	return;
}

bool var::writev(const var& fileno,const var& key,const int fieldno) const
{
	assert(assigned());

	//get the old record
	var record;
	if (!record.read(fileno,key)) record="";

	//and update it
	record.replacer(fieldno,0,0,var_mvstr);

	//write it back
	record.write(fileno,key);

	return true;

}

bool var::write(const var& fileno,const var& key) const
{

	assert(assigned());
	assert(fileno.assigned());
	assert(key.assigned());

	if (toString().length()>MV_MAX_RECORD_LENGTH)
	{
		_STATUS=MV_MAX_RECORD_LENGTH_EXCEEDED;
		return false;
	};
	char crecord[MV_MAX_RECORD_LENGTH+1]="";
	strncpy(crecord,toString().c_str(),MV_MAX_RECORD_LENGTH);
	crecord[MV_MAX_RECORD_LENGTH-1]='\0';

	if (key.toString().length()>MV_MAX_KEY_LENGTH)
	{
		_STATUS=MV_MAX_KEY_LENGTH_EXCEEDED;
		return false;
	};
	char ckey[MV_MAX_KEY_LENGTH+1]="";
	strncpy(ckey,key.toString().c_str(),MV_MAX_KEY_LENGTH);
	ckey[MV_MAX_KEY_LENGTH-1]='\0';

	QMWrite(fileno.var_mvint,ckey,crecord);

	return true;
}

bool var::deleterecord(const var& key) const
{
	assert(assigned());
	assert(key.assigned());

	if (key.toString().length()>MV_MAX_KEY_LENGTH)
	{
		_STATUS=MV_MAX_KEY_LENGTH_EXCEEDED;
		return false;
	};
	char ckey[MV_MAX_KEY_LENGTH+1]="";
	strncpy(ckey,key.toString().c_str(),MV_MAX_KEY_LENGTH);
	ckey[MV_MAX_KEY_LENGTH-1]='\0';

	QMDelete(var_mvint,ckey);
	_STATUS=QMStatus();
	if (_STATUS)
	{
		return false;
	}

	return true;
}

var var::xlate(const var& filename,const var& fieldno, const var& mode) const
{
	assert(assigned());
	assert(filename.assigned());
	assert(fieldno.assigned());
	assert(mode.assigned());

	//get ready to return "" or if mode is "C" the unchanged key(s)
	var newmv="";
	if (mode=="C")
	{
		newmv.var_mvstr=var_mvstr;
		newmv.var_mvint=var_mvint;
		newmv.var_mvtyp=var_mvtyp;
	}

	//open the file
	var file;
	if (!file.open(filename))
	{
		_STATUS=MV_BAD_FILENAME^filename;
		return newmv;
	}

	//read the record
	if (newmv.read(file,var_mvstr))
		return newmv;

	//extract the field
	if (fieldno)
		newmv=newmv.extract(fieldno);

	//close the file
	file.close();

	return newmv;
}

} // namespace
*/