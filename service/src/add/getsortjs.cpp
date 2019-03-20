
#include <exodus/library.h>
libraryinit()

var t2;

function main(io tx) {
        //c xxx io
        //TODO consider merging with readcss() perhaps with mode

	//in revelation get from GBP file
        //tx = var("$SORT.JS").xlate("GBP", "", "X").substr(1,0x1A) ^ "\r\n";

	//in c++ get from raw text string below
	tx=getvogonpoetry();

        tx.swapper(FM, "\r\n");

        if (DATEFMT.index("E", 1)) {
                t2 = "d/M/yyyy";
        } else if (DATEFMT.index("J", 1)) {
                t2 = "yyyy/M/d";
        } else {
                t2 = "M/d/yyyy";
        }

        tx.swapper("//gdateformat=\'d/M/yyy\'", "gdateformat=\'" ^ t2 ^ "\'");

        return 0;

}

function getvogonpoetry() {

return R"V0G0N(
<script type="text/javascript">
//gdateformat='d/M/yyyy'
function sorttable(event)
{
 if (document.body.getAttribute('contenteditable'))
  return true
 //locate the current element
 event=event||window.event
 event.target=event.target||event.srcElement
 var th=event.target
 if (th.tagName!="TH") return(0)
 //var rows=th.parentElement.parentElement.parentElement.rows
 var rows=th.parentElement.parentElement.parentElement.tBodies[0].getElementsByTagName('tr')
 var coln=th.cellIndex
 var rown=th.parentElement.rowIndex
 var nrows=rows.length
 fromrown=0
 uptorown=nrows-=1
 /*
 var rown=th.parentElement.rowIndex
 //locate the first td row
 var fromrown=rown

 //search forwards (incl current row) for the first row with a td element
 //if not found then rown=nrows+1
 while (fromrown<nrows && ( rows[fromrown].cells.length<(coln+1) || rows[fromrown].cells[coln].tagName=="TH" ) )
  ++fromrownn

 //include prior td rows
 while (fromrown>0 && rows[fromrown-1].cells.length>coln && rows[fromrown-1].cells[coln].tagName=="TD")
  --fromrown

 //include following td rows
 var uptorown=(rown>fromrown)?rown:fromrown
 */
 while ((uptorown<nrows) && rows[uptorown+1] && (rowchildNodes=rows[uptorown+1].cells) && (rowchildNodes.length>coln) && (rowchildNodes[coln].tagName=="TD"))
  uptorown++

 if (gdateformat == 'M/d/yyyy')
  var dateformat=[2,0,1]
 else if (gdateformat == 'yyyy/M/d')
  var dateformat=[0,1,2]
 else //gdateformat = 'd/M/yyyy'
  var dateformat=[2,1,0]
 var yy=dateformat[0]+1
 var mm=dateformat[1]+1
 var dd=dateformat[2]+1

 var dateregex=/ ?(\d{1,2})\/ ?(\d{1,2})\/(\d{4}|\d{2})/
 var periodregex=/ ?(\d{1,2})\/(\d{4})/g
 for (var ii=fromrown;ii<=uptorown;++ii){
  var cell=rows[ii].cells[coln]
  if (cell.getAttribute('sortvalue'))
   break
  var value=(cell.textContent||cell.innerText||"").toUpperCase()
  var match
  while(match=value.match(dateregex)) {
   //convert dates like n/n/yy or n/n/yyyy to sortable yyyy|mm|dd format
   value=value.replace(dateregex,('0000'+match[yy]).slice(-4)+'|'+('00'+match[mm]).slice(-2)+'|'+('00'+match[dd]).slice(-2))
   //console.log(value)
  }
  value=value.replace(periodregex,'$2|$1')
  value=value.replace(/([-+]?[1234567890.,]+)([A-Z]{2,3})/g,'$2$1')//prefix currency
  //natural sort
  //value=value.replace(/[-+]?[1234567890.,]+/g,function(x){if (x.slice(0,1)=='-') {y='';x=(100000000000000000000+Number(x.replace(/,/g,''))).toString();alert(x)} else y='';return y+('00000000000000000000000'+x).slice(-20)})
  value=value.replace(
   /[-+]?[1234567890.,]+/g,
   function(x){
     /*decimal*/ x=x.replace(/,/g,'');
     /*comma*/   //x=x.replace(/./g,'').replace(/,/,'.');
     if (x.slice(0,1)=='-') {
       y='-';
       x=(999999999999.999+Number(x.replace(/,/g,''))).toString();
     } else {
       y='';
     }
     x=x.split('.');
     y+=('00000000000000000000'+x[0]).slice(-20);
     if (x[1])
      y+='.'+(x[1]+'0000000000').slice(0,10);
     return y;
   }
  )
  value+=('000000000000'+ii).slice(-10)//stable sort
  cell.setAttribute('sortvalue',value)
  //cell.innerHTML=value
  //console.log(ii+' '+value)
 }
 QuickSort(rows,coln,fromrown,uptorown);
}

function QuickSort(rows,coln,min,max){

 if (max<=min) return(true)

 var low = min;
 var high = max;
 var mid=rows[Math.floor((low+high)/2)].cells[coln].getAttribute('sortvalue');

 do {
  while (rows[low].cells[coln].getAttribute('sortvalue') < mid) low++;
  while (rows[high].cells[coln].getAttribute('sortvalue') > mid) high--;

  if (low <= high) {

   //if (stopped) return;

   rows[low].swapNode(rows[high]);

   low++;
   high--;
  }

  //pauseSort();

 } while (low <= high);

 if (high > min) QuickSort(rows, coln, min, high);
 if (low < max) QuickSort(rows, coln, low, max);
}
if (!document.swapNode) {
 Node.prototype.swapNode = function (node) {
  var p = node.parentNode;
  var s = node.nextSibling;
  this.parentNode.replaceChild(node, this);
  p.insertBefore(this, s);
  return this;
 }
}
</script>
)V0G0N";

}

libraryexit()

