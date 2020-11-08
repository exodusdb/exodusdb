//

	#define maxages 20
//do not redim bals since c++ read will change size of bals causing real redim
	if (lg.html.unassigned()) {;
	 lg.html = "";
	 lg.vch.redim(70);
	 lg.origv.redim(70);
	 lg.bals.redim(30);
	 lg.sbals.redim(30);
	 lg.aging.redim(20 + 2);
	 lg.totaging.redim(2, 20 + 2);
	 lg.chart.redim(50);
	   };