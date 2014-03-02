use URI::Escape;
use strict;
use warnings;
#use CGI qw(:standard);
use CGI qw(:all);
use CGI::Carp qw(fatalsToBrowser carpout);
use XML::DOM

print "Content-type: text/html\n\n"; 

my $query=new CGI;
my $string1=$query->query_string;
#decode the escaped characters
$string1 =~ s/%([0-9A-Fa-f]{2})/chr(hex($1))/eg;
#remove the keywords= prefix
$string1=substr($string1,9);

#$string1=~ s/</"x"/eg;
#print $string1;

my $parser = new XML::DOM::Parser;
my $doc = $parser->parse($string1);
#print $doc->toString();

my @nodes=$doc->getFirstChild()->getChildNodes();
#print @nodes[2]->getFirstChild()->getData();
my $token=getdata(@nodes[0]);
my $currenttimeout=getdata(@nodes[1]);
my $request=getdata(@nodes[2]);
my $data=getdata(@nodes[3]);
print "/$token/$currenttimeout/$request/$data/";

#my $request=@nodes[2];
#my $requesttext=$request->getFirstChild();
#print $requesttext->getData();

$doc->dispose;

sub getdata()
{
 my $node=shift;
 if (!$node) {return ''};
 #return $node->getNodeName();
 my $child=$node->getFirstChild();
 if (!$child) {return ''};
 
 return $child->getData();
 
}

__END__

<%

	//load the posted XML document
	var requestx = Server.CreateObject("Microsoft.XMLDOM");
	requestx.load(Request)
	
	//extract the request parameters
	var token=unescape(requestx.firstChild.childNodes[0].text)
	var currenttimeout=unescape(requestx.firstChild.childNodes[1].text)
	var request=unescape(requestx.firstChild.childNodes[2].text)
	var data=unescape(requestx.firstChild.childNodes[3].text)
	
	//use default timeout if necessary
	if (currenttimeout!="") timeout=currenttimeout
	
	//use this for testing or keeping session alive
	if (this.request.slice(0,9)=='KEEPALIVE')
	{
		db=new Object
		db.data='KEEPALIVE'
		db.response='OK'
		dbresult=true
	}
	else
	{  
		//send the request and data to the database
		var db=new neosysdblink()
		db.request=request
		var error=null 
		try
		{
			var dbresult=db.send(data)
		}
		catch(e)
		{
			db.response=e.description
			db.data=""
		}
	}
		
	//package the response in xml text format
	//zzz maybe faster to send as xml object?
	xmltext="<root>"
	xmltext+="<data>" + escape(db.data) + "</data>"
	xmltext+="<response>" + escape(db.response) + "</response>"
	xmltext+="<result>" + escape(dbresult) + "</result>"
	xmltext+="</root>"
	
	Response.ContentType="text/xml"
	Response.Expires=-1000
	Response.Buffer=0
	//Response.Write "<?xml version='1.0' encoding='ISO-8859-1'?>"
	Response.Write(xmltext)
	
%>