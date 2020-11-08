//all labelled common, no plain common
//to keep it simpler for c++ conversion, dont mix the two in one program

	#define isordertypenewchanges (ordertype == newchanges_type_0)
	#define isordertypereplacement (ordertype == replacement_type_1)
	#define isordertypereissue (ordertype == reissue_type_2)
	#define isordertypecombo (ordertype == combo_type_3)
	#define isordertypecancelall (ordertype == cancelall_type_4)
	#define isordertypecombochanges (ordertype == newchanges_incombo_type_m1)

//netgross: 1=both 2=gross only 3=net only
	#define showgrossonly netgross eq 2
	#define showgrossunitcost netgross ne 3
	#define showdisccharges netgross eq 1
	#define shownetunitcost netgross ne 2
	#define shownetcost netgross ne 2

	#define draft win.registerx(1)
	highlighton = "<B><I><FONT SIZE=5>";
	highlightoff = "</FONT></I></B>";
	bo = "<b>";
	bx = "</b>";