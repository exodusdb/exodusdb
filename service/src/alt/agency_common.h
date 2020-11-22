//equ agp to agency.params
	#define companyname agp.a(1)
	#define companywebsite agp.a(16)

//always taken from company file
	#define basecurrencycode agp.a(2)
	#define basecurrencyfmt agp.a(3)

//obsolete was for dos text printouts
	#define shortdates agp.a(4)

	#define splitextras agp.a(5)

//obsolete what was this?
	#define copyplans agp.a(6)

	#define wordingonsch agp.a(7)

//y=yes,n=no,a or ay=ask (yes),null or an=ask (no)
//equ schedule version increment mode to agp<17>

//[dateperiod,1,12]
//[weekly2,7,1,4] year starts on the first monday of july
//[weekly2,1,6] year starts january, week starts saturday
//equ dateperiodconv to agp<18>

//equ production.def.company.code to agp<19>
//equ media.inv.vch.type agp<46> see also 146 147
//equ prod.inv.vch.type agp<47>
//equ invnobycompany to agp<48> 1=separate for each company
//equ media.inv.pattern to agp<49> see also 149 150
//equ production.inv.pattern to agp<50>
//flag for required
//equ production.docs.are signed agp<51> (optional name is below in 61/62)
//equ notes.on.inv to agp<52>
//equ jobnopattern to agp<53>
//equ imagelocation to agp<54> zzz should be in system
//equ budgetnames to agp<55>
//equ forecastnames to agp<56>
//equ vehiclecol to agp<57>
//equ bookingmark to agp<58>
//equ standardbookingtext to agp<59>
//equ bookingordercompanycode to agp<60> and cost invoice company code
//text for signatory
//equ pordersignatoryname to agp<61>
//equ estimatesignatoryname to agp<62>
//equ schedulenopattern to agp<63>

//equ invoice tax code to agp<64>
//equ mediainvoice_useaccountname to agp<65>
//equ productioninvoice_useaccountname to agp<66>
//equ agplastreadtime to agp<67>
//equ materialoninvoice to agp<68> not 0 or '' means will show on all invoices
//equ productionordernopattern to agp<69>
//equ productionestimatenopattern to agp<70>
//equ plannopattern to agp<71>
//equ defaultplantitle to agp<72>
//equ defaultscheduletitle to agp<73>
//equ defaultestimatetitle to agp<74>
//equ sundry accnos agp<75>;*brand is client

	#define schtopmargin agp.a(8)
	#define botopmargin agp.a(9)
	#define invtopmargin agp.a(10)
//equ plan.footer to agp<34>
//equ sch.footer to agp<11>
	#define invfooter agp.a(15)
//bottom of both media and production invoices
//unless overridden on chart of accounts, currency, client or company file
//equ standard.payment.instructions to agp<38>
//equ standard.terms.of.payment to agp<76>
//equ show.account.terms to agp<151>
//equ purchase invoice tax code to agp<152>

//media only (appears to be obsolete only in plan.subs2,val.brand)
	#define standardtax agp.a(12)

	#define lastdayofweek agp.a(13)

	#define prodnaccrualsaccno agp.a(14)
	#define mediaincomeaccno agp.a(20)
//*not used equ media.downpayment.accno to agp<21>
	#define mediacostsaccno agp.a(22)
//equ media.wip.accno to agp<23>
	#define mediaaccrualsaccno agp.a(24)

	#define medialedger agp.a(25)
	#define prodnledger agp.a(26)
//equ media.wip.ledger to agp<27>
//equ prodn.wip.ledger to agp<28>

//in parallel with 25-28
//equ media.accrual.control.accno to agp<135>
//equ prodn.accrual.control.accno to agp<136>
//equ media.wip.control.accno to agp<137>
//equ prodn.wip.control.accno to agp<138>

//no longer configurable or used in user interface
//equ reqcertify to agp<30>
	#define reqcertify 0

//no longer used - see agp<84>
//equ chkcoincide to agp<31>

//equ unitbillrounding to agp<32,*>
//equ companynameforprodinvoicing to agp<33>

//equ def.market.code to agp<37>
	#define currcompanycode agp.a(39)
	#define prodnincomeaccno agp.a(40)
//no ui for downpayment equ downpayment.accno to agp<41>
	#define prodncostsaccno agp.a(42)
	#define prodnwipaccno agp.a(43)
//applied to media booking and production orders
//equ suppress.client.on.orders to agp<44>
	#define cpmr agp.a(45)

//say cancel/book instead of change from/change to

//*equ rebookmeanscancelbook to agp<77>
//*equ donthideunchangeddates to agp<78>
//*equ bookingsummary to agp<79>
//new method all options together
//equ bookingoptions to agp<79> 1=summary 2=suppfree 3=cancelrebook 4=showalldate

//bookingordernostyle
//1=sequential no
//2=sequential plus version
//3=scheduleno plus version
//equ bookingordernostyle to agp<80>

//y/n and on client<18>
//equ showexchrates to agp<81>

//equ joborderfooter to agp<82>
//equ estimatefooter to agp<83>

// 1=all clients, blank or 2=client group, 3=client only, 4=brand only
//broadcast media are always 2 at the moment
//equ clientcoincidencemode to agp<84>

//equ prod.purchinv.vch.type agp<85>
//show prior proforma invoice numbers agp<86>
//equ analonpostingsyear to agp<87> default 2008
//equ conv2billcurr to agp<88>
//equ commissiondaystopay=agp<89>
//equ commissionpercent=agp<90>
//equ media.purchinv.vch.type agp<91>
//equ campaignrequired to agp<92>
//equ analysisonpostingsyear to agp<93> default 2008 for reporting
//equ feeshareaccount to agp<94>
//equ deadlineweekenddays to agp<95>
//equ net_gross to agp<96>
//equ materialorderfooter to agp<97>
//equ mediashowexecutive to agp<98>
//equ mediabillpostmode to agp<100>
//equ jobbillpostmode to agp<101>
//equ jobcostpostmode to agp<102>
//equ taxcurrcode to agp<103> (show tax in this) also acc.params<40>
	#define technicalversion 2
//new version 3! supports one ad with many spots from 12/06/2010
//equ technicalversion to 3
//on schedule date grid (some people want to highlight last dow and some first)
//equ highlightdows to agp<104>
//equ version in heading to  agp<105> 1=version, 2=revision
//equ default/initial job task parent user/requestor to agp<106>
//equ checkmediabudget to agp<107> blank=ytd 1=month
//equ showtermsonestimates to agp<108> 1=yes
//equ invdelnotefootleft to agp<109>
//equ invdelnotefootright to agp<110>
//equ invdelnoterequest to agp<111>
//equ loadingcolumn to agp<112> 0=no else yes
//equ authbufferusercode to agp<113>
//equ authbuffersupplieraccess to agp<114,1>
//equ authbuffervehicleaccess to agp<114,2>
//equ defaultamendbookingstyle to agp<115> 0 changes 1 replace 2 reissue 3 combo (1+0)
//equ mediafontpercs to agp<116> ,1>=plan ,2>=schedule ,3>=booking ,4>=invoice
//equ ist2nd3rd to agp<117>
//equ reqsuppinvno to agp<118>;null optional 0=disallow entry 1=require entry
//equ copyordernotes to agp<119> 1/blank=yes, 0=no
//equ defaultextras to agp<120> 1-6 = load/disc/comm/fee/tax/oth (7 would be net)
//equ showextramts to agp<121> 1=% 2=amounts
//equ showcoltots to agp<122> 0=no total in gup column
//equ letterheadoninv to agp<123> 1=no (preprinted) 2=yes else yes
//equ maxschedulekeysize to agp<124> blank/0=no limit
//equ createads to agp<125> 1 for 5 years back or negative to indicate nyears back
//equ exportformat to agp<126> blank for ul/magna, 1 for exodus generic
//equ default.client.accno to agp<127>
//equ createadsnotifyuser to agp<128>
//equ voucherrefpattern to agp<129> not used
//equ voucherdetailspattern to agp<130> not used
//equ allowexportunposted to agp<131>
//agp<135-138> see above
//equ pordermaxamount to agp<139>

//equ media.inv.vch.type agp<146> inv <46> adjustment <146> credit note <147>
//equ media.inv.vch.type agp<147>
//equ media.inv.pattern to agp<149> inv <49> adjustment <149> credit note <150>
//equ media.inv.pattern to agp<150>
//equ datelist_show_x1 to agp<152> 0 no, 1 if any x2 or more, 2 always
//equ paymentinstructionspriority to agp<153>
//free 154

//heading
	#define ndecsfn 1
	#define completelybookeddatefn 8
	#define periodfn 12
	#define currencycodefn 13
	#define approvaldatefn 9

//list of invoices
	#define invdatefn 3
	#define invnofn 4
	#define invtypefn 5
	#define totinvoicedfn 6
	#define asinvoicedfn 7

//lines

//details
	#define vehiclecodefn 20
	#define vehiclecurrcodefn 45
	#define specfn 21
	#define datesfn 22
	#define freegridfn 71
	#define costfreegridfn 72
	#define detailsfn 23

//billing
	#define numberfn 39
	#define gunitbillfn 37
	#define extrasfn 140
	#define unitbillfn 38
	#define exchratefn 40

	#define billexchratefn 78

//costing
	#define costnumberfn 43
	#define gunitcostfn 41
	#define costextrasfn 139
	#define unitcostfn 42
	#define costexchratefn 44

//bookings
	#define bookingnofn 51

//originals are stored here if changes are made to a line after being booked
	#define vehiclecodeasbookedfn 52
	#define unusedasbookedfn 53
	#define specasbookedfn 54
	#define datesasbookedfn 55
	#define detailsasbookedfn 56
	#define freegridasbookedfn 73

//deletions are stored here if lines are deleted after being booked
	#define vehiclecodecancfn 57
	#define bookingnocancfn 58
	#define speccancfn 59
	#define datescancfn 60
	#define detailscancfn 61
	#define freegridcancfn 74

//invoice
	#define invbillfn 46
	#define invbillbasefn 47
	#define invcostfn 48
	#define invcostbasefn 49
	#define invlnfn 50

//ratings
	#define timefn 111
	#define weightfn 138

	#define lineidfn 160

	#define ordnosfn 162
	#define certnofn 159
	#define suppinvnosfn 67
	#define invnosfn 161

	#define domsfn 70

	#define survey1fn 110
	#define reach1fn 105
	#define rating1fn 106
	#define freq1fn 107

	#define notesfn 113
	#define invnotesfn 225