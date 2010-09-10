<%@ Language=JavaScript %>
<SCRIPT RUNAT=Server SRC=server0.js LANGUAGE=javascript></SCRIPT>
<SCRIPT RUNAT=Server SRC=server.js LANGUAGE=javascript></SCRIPT>
<%

 Response.Expires=0

 //extract the request parameters
 var currenttimeout=unescape(Request.QueryString('timeout'))
 var request=unescape(Request.QueryString('request'))
 var data=unescape(Request.QueryString('data'))
 if (data=='undefined') data=''
 
 Session('username')=unescape(Request.QueryString('username'))
 Session('password')=unescape(Request.QueryString('password'))
 Session('dataset')=unescape(Request.QueryString('dataset'))

 var db=new neosysdblink()

 //use default timeout if necessary
 if (currenttimeout!="") timeout=currenttimeout
 
 //send the request and data to the database
 var dbresult
 db.request=request
 try
 {
  dbresult=db.send(data)
 }
 catch(e)
 {
  dbresult=0
  db.response=e.description
  db.data=""
 }

 Response.Write('x')
 Response.Write(db.response)
 Response.Write('<br>')
 if (db.data) Response.Write(db.data?db.data:'')
%>