#include <exodus/library.h>

//#include <gen.h>

libraryinit(executive_name)

function main() {

	//ans={EXECUTIVE_CODE}
	//ans2=ans
	//convert @lower.case to @upper.case in ans2
	//@ans=xlate('USERS',ans2,1,'X')
	//if @ans else @ans=ans
	var ans = calculate("EXECUTIVE_CODE");
	var ans2 = ans;
	ans2.ucaser();
	if (ans2 ne ans) {
		ans.transfer(ANS);
	}else{
		ANS = var("%" ^ ans2 ^ "%").xlate("USERS", 1, "X");
		if (ANS) {
			ans2 = ANS;
			ans2.ucaser();
			if (ans2 == ANS) {
				ANS = mv.capitalise(ANS);
			}
		}else{
			ans.transfer(ANS);
			ANS = mv.capitalise(ANS);
		}
	}
	return ANS;

}
libraryexit(executive_name)

libraryinit(alert_subject)

function main()  {
	return "Job " ^ ID ^ calculate("VERSION") ^ " " ^ calculate("STATUS") ^ " " ^ calculate("EXECUTIVE_NAME") ^ " - " ^ calculate("DESCRIPTION1");
}
libraryexit(alert_subject)


libraryinit(analyse)
#include <jobanalysis.h>
function main()  {
	call jobanalysis("");
	return "";
}
libraryexit(analyse)


libraryinit(anal_costs)

function main()  {
	var ans = "";
	var ordernos = calculate("ORDER_NO");
	if (ordernos) {
		var prodorders;
		if (prodorders.open("", "PRODUCTION_ORDERS")) {
			var norders = ordernos.count(VM) + 1;
			for (var vn = 1; vn <= norders; ++vn) {
				var prodorder;
				if (prodorder.read(prodorders, ordernos.a(1, vn))) {
					// call msg(prodorder<3>:'/':prodorder<5>,'','','')
					ans.r(1, vn, (prodorder.a(3)).sum() / prodorder.a(5));
				}
			};//vn;
		}
	}
	return ans;
}
libraryexit(anal_costs)


libraryinit(authorised)
#include <validcode2.h>
#include <validcode3.h>
function main()  {
	var valid = 1;
	var msg;
	var xx;
	if (not(validcode2(RECORD.a(14), "", RECORD.a(2), xx, msg))) {
		valid = 0;
	}
	if (valid) {
		if (not(validcode3(RECORD.a(12), "", "", xx, msg))) {
			valid = 0;
		}
	}
	return valid;
}
libraryexit(authorised)


libraryinit(brand_name)

function main()  {
	return calculate("BRAND_CODE").xlate("BRANDS", 2, "C");
}
libraryexit(brand_name)


libraryinit(brand_name15)

function main()  {
	return calculate("BRAND_NAME");
}
libraryexit(brand_name15)


libraryinit(brief)

function main()  {
	return calculate("DESCRIPTION").field(VM, 2, 9999);
}
libraryexit(brief)


libraryinit(client_code)

function main()  {
	return RECORD.a(2).xlate("BRANDS", 1, "X").a(1, 1);
}
libraryexit(client_code)


libraryinit(client_group2_code)

function main()  {
	var ans = RECORD.a(2).xlate("BRANDS", 1, "X");
	ans = ans.a(1, 1);
	var principlecode = ans.xlate("CLIENTS", 55, "X");
	if (principlecode) {
		ANS = principlecode;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(client_group2_code)


libraryinit(client_group_code)

function main()  {
	var ans = RECORD.a(2).xlate("BRANDS", 1, "X");
	ans = ans.a(1, 1);
	var principlecode = ans.xlate("CLIENTS", 16, "X");
	if (principlecode) {
		ANS = principlecode;
	}else{
		ANS = ans;
	}
	return ANS;
}
libraryexit(client_group_code)


libraryinit(client_name)

function main()  {
	return calculate("BRAND_CODE").xlate("BRANDS", 3, "X");
}
libraryexit(client_name)


libraryinit(client_name15)

function main()  {
	return calculate("CLIENT_NAME");
}
libraryexit(client_name15)


libraryinit(commission_margin)

function main()  {
	var income = calculate("COMMISSION_BASE");
	var cost = calculate("REBILLABLE_COST_BASE");
	if (income and cost) {
		ANS = (income - cost) / income * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(commission_margin)


libraryinit(company_name)

function main()  {
	return RECORD.a(14).xlate("COMPANIES", 1, "C");
}
libraryexit(company_name)


libraryinit(date)

function main()  {
	return var("1/" ^ RECORD.a(1)).iconv("D/E");
}
libraryexit(date)


libraryinit(datetime_created)

function main()  {
	return field2(RECORD.a(31), VM, -1);
}
libraryexit(datetime_created)


libraryinit(date_created)

function main()  {
	ANS = RECORD.a(54);
	if (not ANS) {
		ANS = field2(RECORD.a(31), VM, -1);
	}
	return ANS;
}
libraryexit(date_created)


libraryinit(description1)

function main()  {
	ANS = (RECORD.a(9, 1, 1)).field(TM, 1);
	var tt = ANS.index("<br />", 1);
	if (tt) {
		ANS.splicer(tt, 999999, "");
	}
	return ANS;
}
libraryexit(description1)


libraryinit(description15)

function main()  {
	return calculate("DESCRIPTION");
}
libraryexit(description15)


libraryinit(description1_ucase)

function main()  {
	return calculate("DESCRIPTION1").ucase();
}
libraryexit(description1_ucase)


libraryinit(estimate_status)

function main()  {
	return mv.capitalise(calculate("QUOTE_NO").xlate("PRODUCTION_INVOICES", 11, "X"));
}
libraryexit(estimate_status)


libraryinit(full_brand_name)

function main()  {
	var brandcode = calculate("BRAND_CODE");
	var ans = brandcode.xlate("BRANDS", 2, "X");
	var brandcode1 = brandcode.xlate("BRANDS", 8, "X");
	brandcode1.converter(SVM, VM);
	brandcode1 = brandcode1.a(1, 1);
	if (brandcode1 and brandcode1 ne brandcode) {
		ans.splicer(1, 0, brandcode1.xlate("BRANDS", 2, "X") ^ " ");
	}
	return ans;
}
libraryexit(full_brand_name)


libraryinit(internal_netincome_base)

function main()  {
	return calculate("INTERNAL_INCOME_BASE") - calculate("INTERNAL_COST_BASE");
}
libraryexit(internal_netincome_base)


libraryinit(internal_netmargin)

function main()  {
	var cost = calculate("REBILLABLE_INCOME_BASE") + calculate("COMMISSION_BASE");
	var income = cost + calculate("INTERNAL_NETINCOME_BASE");
	if (income and cost) {
		ANS = (income - cost) / income * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(internal_netmargin)


libraryinit(invoice_key)
#include <agy.h>
function main()  {
	var ans = RECORD.a(5);
	if (ans) {
		//$insert bp,agency.common
		if (agy.agp.a(48)) {
			var nn = ans.count(VM) + 1;
			var compcode = calculate("COMPANY_CODE");
			for (var ii = 1; ii <= nn; ++ii) {
				ans.r(1, ii, (ans.a(1, ii)).fieldstore("*", 3, 1, compcode));
			};//ii;
		}
	}
	ans.transfer(ANS);
	return ANS;
}
libraryexit(invoice_key)


libraryinit(inv_amount)

function main()  {
	return calculate("INVOICE_KEY").xlate("INVOICES", 3, "X");
}
libraryexit(inv_amount)


libraryinit(inv_amount_and_curr)

function main()  {
	var ans = calculate("INVOICE_KEY").xlate("INVOICES", 3, "X");
	var ans2 = calculate("QUOTE_NO").xlate("PRODUCTION_INVOICES", 4, "X");
	for (var ii = 1; ii <= 999; ++ii) {
		var currcode = ans2.a(1, ii);
	///BREAK;
	if (not currcode) break;;
		if (ans.a(1, ii)) {
			ans.r(1, ii, 2, currcode);
		}
	};//ii;
	ans.converter(SVM, "");
	return ans;
}
libraryexit(inv_amount_and_curr)


libraryinit(inv_amount_base)

function main()  {
	return (calculate("INVOICE_KEY").xlate("INVOICES", 10, "X")).sum();
}
libraryexit(inv_amount_base)


libraryinit(inv_currency)

function main()  {
	return calculate("INVOICE_KEY").xlate("INVOICES", 20, "X");
}
libraryexit(inv_currency)


libraryinit(inv_date)

function main()  {
	return calculate("INVOICE_KEY").xlate("INVOICES", 2, "X");
}
libraryexit(inv_date)


libraryinit(inv_desc)

function main()  {
	return calculate("INVOICE_KEY").xlate("INVOICES", 9, "X");
}
libraryexit(inv_desc)


libraryinit(master_job_no)

function main()  {
	return ID.field("-", 1);
}
libraryexit(master_job_no)


libraryinit(open)

function main()  {
	if (RECORD.a(7) == "Y") {
		ANS = "";
	}else{
		ANS = "O";
	}
	return ANS;
}
libraryexit(open)


libraryinit(order_amount)

function main()  {
	//$insert gbp,general.common
	ANS = xlate("PRODUCTION_ORDERS", calculate("ORDER_NO"), "TOTAL_AMOUNT", "X");
	//@ANS = sum(XLATE( "PRODUCTION_ORDERS" , {ORDER_NO} , 3 , "X" ))
	//if @ans else @ans=''
	return ANS;
}
libraryexit(order_amount)


libraryinit(order_amount_and_curr)

function main()  {
	//$insert gbp,general.common
	var ans = xlate("PRODUCTION_ORDERS", calculate("ORDER_NO"), "TOTAL_AMOUNT", "X");
	//@ANS = sum(XLATE( "PRODUCTION_ORDERS" , {ORDER_NO} , 3 , "X" ))
	//if @ans else @ans=''
	var ans2 = xlate("PRODUCTION_ORDERS", calculate("ORDER_NO"), 4, "X");
	return ans^ans2;
}
libraryexit(order_amount_and_curr)


libraryinit(order_amount_base)
#include <fin.h>
function main()  {
	//$insert gbp,general.common
	fin.basefmt = "MD" ^ USER2[3] ^ "0P";
	return (xlate("PRODUCTION_ORDERS", calculate("ORDER_NO"), "TOTAL_BASE", "X")).oconv(fin.basefmt);
}
libraryexit(order_amount_base)


libraryinit(order_currency)

function main()  {
	return calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 4, "X");
}
libraryexit(order_currency)


libraryinit(order_date)

function main()  {
	return calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 1, "X");
}
libraryexit(order_date)


libraryinit(order_desc)

function main()  {
	return calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 6, "X");
}
libraryexit(order_desc)


libraryinit(order_estimate_no)

function main()  {
	return calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 35, "X");
}
libraryexit(order_estimate_no)


libraryinit(order_inv_date)

function main()  {
	return calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 9, "X");
}
libraryexit(order_inv_date)


libraryinit(order_inv_no)

function main()  {
	return calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 7, "X");
}
libraryexit(order_inv_no)


libraryinit(order_status)

function main()  {
	return mv.capitalise(calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 11, "X"));
}
libraryexit(order_status)


libraryinit(order_status2)

function main()  {
	var ordernos = calculate("ORDER_NO");
	if (ordernos) {
		ANS = ordernos.xlate("PRODUCTION_ORDERS", 11, "X");
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(order_status2)


libraryinit(ordinv_no)

function main()  {
	var orderno = calculate("ORDER_NO");
	ANS = orderno;
	var order = "";
	var orders;
	if (orders.open("", "PRODUCTION_ORDERS")) {
		if (order.read(orders, ANS)) {
			ANS = ANS ^ "/" ^ order.a(7);
		}
	}
	return ANS;
}
libraryexit(ordinv_no)


libraryinit(outstanding_base)
#include <fin.h>
function main()  {
	//$insert abp,common
	//$insert gbp,general.common
	var invnos = RECORD.a(5, MV);
	var ans = "";
	if (invnos) {
		var ninvs = invnos.count(VM) + 1;
		for (var invn = 1; invn <= ninvs; ++invn) {
			var invno = invnos.a(1, invn);
			if (invno) {
				var voucher;
				if (voucher.read(fin.vouchers, "INV*" ^ invno ^ "*" ^ fin.currcompany)) {
					var amount = voucher.a(11, 1);
					var allocs = ((voucher.a(20, 1)).sum()).sum();
					var outstanding = (amount - allocs).oconv(fin.basefmt);
					//garbagecollect;
					if (outstanding) {
						ans.r(1, invn, outstanding);
					}
				}
			}
		};//invn;
	}
	return ans;
}
libraryexit(outstanding_base)


libraryinit(outstanding_order_amount)

function main()  {
	ANS = calculate("PO_NO_INV").xlate("PRODUCTION_ORDERS", "TOTAL_AMOUNT_AND_CURR", "X");
	//if index(@ans,@svm,1) then @ans=sum(@ans)
	return ANS;
}
libraryexit(outstanding_order_amount)


libraryinit(outstanding_order_amount_base)

function main()  {
	//$insert gbp,general.common
	return xlate("PRODUCTION_ORDERS", calculate("PO_NO_INV"), "TOTAL_BASE", "X");
}
libraryexit(outstanding_order_amount_base)


libraryinit(outstanding_order_curr)

function main()  {
	return calculate("PO_NO_INV").xlate("PRODUCTION_ORDERS", 4, "X");
}
libraryexit(outstanding_order_curr)


libraryinit(outstanding_order_date)

function main()  {
	return calculate("PO_NO_INV").xlate("PRODUCTION_ORDERS", 1, "X");
}
libraryexit(outstanding_order_date)


libraryinit(percent_margin)

function main()  {
	var income = calculate("TOTAL_AMOUNT_BASE");
	var cost = calculate("TOTAL_COST_BASE");
	if (income and cost) {
		ANS = (income - cost) / income * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(percent_margin)


libraryinit(percent_margin2)

function main()  {
	var income = (calculate("QUOTE_AMOUNT_BASE")).sum();
	var cost = (calculate("ORDER_AMOUNT_BASE")).sum();
	if (income and cost) {
		ANS = (income - cost) / income * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(percent_margin2)


libraryinit(percent_margin_after_time)

function main()  {
	var income = calculate("TOTAL_AMOUNT_BASE");
	var cost = calculate("TOTAL_COST_BASE") + calculate("TOTAL_TIME_BASE");
	if (income and cost) {
		ANS = (income - cost) / income * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(percent_margin_after_time)


libraryinit(percent_markup)

function main()  {
	var income = calculate("TOTAL_AMOUNT_BASE");
	var cost = calculate("TOTAL_COST_BASE");
	if (cost and income) {
		ANS = (income - cost) / cost * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(percent_markup)


libraryinit(percent_markup2)

function main()  {
	var income = (calculate("QUOTE_AMOUNT_BASE")).sum();
	var cost = (calculate("ORDER_AMOUNT_BASE")).sum();
	if (cost and income) {
		ANS = (income - cost) / cost * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(percent_markup2)


libraryinit(percent_markup_after_time)

function main()  {
	var income = calculate("TOTAL_AMOUNT_BASE");
	var cost = calculate("TOTAL_COST_BASE") + calculate("TOTAL_TIME_BASE");
	if (cost and income) {
		ANS = (income - cost) / cost * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(percent_markup_after_time)


libraryinit(prodtype_desc)

function main()  {
	return calculate("PRODUCTION_TYPE").xlate("JOB_TYPES", 1, "C");
}
libraryexit(prodtype_desc)


libraryinit(profit_and_commission_base)

function main()  {
	return calculate("REBILLABLE_PROFIT_BASE") + calculate("COMMISSION_BASE");
}
libraryexit(profit_and_commission_base)


libraryinit(profit_and_commission_margin)

function main()  {
	var cost = calculate("REBILLABLE_COST_BASE");
	var income = calculate("REBILLABLE_INCOME_BASE") + calculate("COMMISSION_BASE");
	if (income and cost) {
		ANS = (income - cost) / income * 100;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(profit_and_commission_margin)


libraryinit(quote_amount)

function main()  {
	//$insert gbp,general.common
	return xlate("PRODUCTION_INVOICES", calculate("QUOTE_NO"), "TOTAL_AMOUNT", "X");
}
libraryexit(quote_amount)


libraryinit(quote_amount_base)
#include <fin.h>
function main()  {
	//$insert gbp,general.common
	fin.basefmt = "MD" ^ USER2[3] ^ "0P";
	return (xlate("PRODUCTION_INVOICES", calculate("QUOTE_NO"), "TOTAL_AMOUNT_BASE", "X")).oconv(fin.basefmt);
}
libraryexit(quote_amount_base)


libraryinit(quote_currency)

function main()  {
	return calculate("QUOTE_NO").xlate("PRODUCTION_INVOICES", 4, "X");
}
libraryexit(quote_currency)


libraryinit(quote_date)

function main()  {
	var quotenos = calculate("QUOTE_NO");
	if (quotenos) {
		ANS = quotenos.xlate("PRODUCTION_INVOICES", 1, "X");
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(quote_date)


libraryinit(quote_desc)

function main()  {
	return calculate("QUOTE_NO").xlate("PRODUCTION_INVOICES", 6, "X");
}
libraryexit(quote_desc)


libraryinit(quote_inv_date)

function main()  {
	var invno = calculate("QUOTE_NO").xlate("PRODUCTION_INVOICES", 10, "X");
	if (invno) {
		var ans = invno.xlate("INVOICES", 2, "X");
		if (ans == "" and RECORD.a(14) ne "") {
			ans = invno ^ "**" ^ RECORD.a(14).xlate("INVOICES", 2, "X");
		}
		ANS = ans;
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(quote_inv_date)


libraryinit(quote_inv_no)

function main()  {
	return calculate("QUOTE_NO").xlate("PRODUCTION_INVOICES", 10, "X");
}
libraryexit(quote_inv_no)


libraryinit(quote_proforma_no)

function main()  {
	var quotenos = calculate("QUOTE_NO");
	if (quotenos) {
		ANS = quotenos.xlate("PRODUCTION_INVOICES", "PROFORMA_INVOICE_NO2", "X");
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(quote_proforma_no)


libraryinit(quote_status2)

function main()  {
	var quotenos = calculate("QUOTE_NO");
	if (quotenos) {
		ANS = quotenos.xlate("PRODUCTION_INVOICES", 11, "X");
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(quote_status2)


libraryinit(quote_status2c)

function main()  {
	var quotenos = calculate("QUOTE_NO");
	if (quotenos) {
		ANS = mv.capitalise(quotenos.xlate("PRODUCTION_INVOICES", 11, "X"));
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(quote_status2c)


libraryinit(rebillable_profit_base)

function main()  {
	return calculate("REBILLABLE_INCOME_BASE") - calculate("REBILLABLE_COST_BASE");
}
libraryexit(rebillable_profit_base)


libraryinit(status)

function main()  {
	var closed = RECORD.a(7);
	if (closed == "Y" or closed == "1") {
		ANS = "Closed";
	} else if (closed == "" or closed == "0") {
		ANS = "Open";
	} else if (closed == " ") {
		ANS = "Open";
	} else if (closed == "N") {
		ANS = "Reopened";
	} else {
		ANS = closed.quote();
	}
	return ANS;
}
libraryexit(status)


libraryinit(status2)

function main()  {
	var closed = RECORD.a(7);
	if (closed == "Y") {
		ANS = "Closed";
	} else if (closed == "") {
		ANS = "Open";
	} else if (closed == " ") {
		ANS = "Open";
	} else if (closed == "N") {
		ANS = "Reopened";
	} else {
		ANS = closed.quote();
	}
	return ANS;
}
libraryexit(status2)


libraryinit(sundry)

function main()  {
	var clientcode = RECORD.a(2).xlate("BRANDS", 1, "X").a(1, 1);
	return clientcode.xlate("CLIENTS", 38, "X");
}
libraryexit(sundry)


libraryinit(supplier_code)

function main()  {
	return calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 10, "X").a(1, 1, 1);
}
libraryexit(supplier_code)


libraryinit(supplier_name)

function main()  {
	ANS = calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 10, "X");
	return ANS.xlate("SUPPLIERS", 1, "C");
}
libraryexit(supplier_name)


libraryinit(supplier_names)

function main()  {
	var suppliercodes = calculate("ORDER_NO").xlate("PRODUCTION_ORDERS", 10, "X");
	var nn = suppliercodes.count(VM) + (suppliercodes ne "");
	for (var ii = nn; ii >= 1; --ii) {
		var vn;
		if (suppliercodes.locate(suppliercodes.a(1, ii), vn, 1)) {
			if (vn < ii) {
				suppliercodes.eraser(1, ii);
			}
		}
	};//ii;
	ANS = suppliercodes.xlate("SUPPLIERS", 1, "X");
	ANS.swapper(VM, var().chr(13) ^ var().chr(10));
	return ANS;
}
libraryexit(supplier_names)


libraryinit(total_amount_base)

function main()  {
	var mvx = MV;
	MV = "";
	ANS = (calculate("AMOUNT_BASE")).sum();
	ANS = ANS.oconv("MD" ^ USER2[3] ^ "0PZ");
	MV = mvx;
	return ANS;
}
libraryexit(total_amount_base)


libraryinit(total_cost_base)

function main()  {
	var mvx = MV;
	MV = "";
	ANS = (calculate("COST_BASE")).sum();
	ANS = ANS.oconv("MD" ^ USER2[3] ^ "0PZ");
	MV = mvx;
	return ANS;
}
libraryexit(total_cost_base)


libraryinit(total_inv_amount_base)

function main()  {
	var mvx = MV;
	MV = "";
	ANS = (calculate("INV_AMOUNT_BASE")).sum();
	ANS = ANS.oconv("MD" ^ USER2[3] ^ "0PZ");
	MV = mvx;
	return ANS;
}
libraryexit(total_inv_amount_base)


libraryinit(total_pending_amount_base)

function main()  {
	var mvx = MV;
	MV = "";
	ANS = (calculate("PENDING_AMOUNT_BASE")).sum();
	ANS = ANS.oconv("MD" ^ USER2[3] ^ "0PZ");
	MV = mvx;
	return ANS;
}
libraryexit(total_pending_amount_base)


libraryinit(total_profit_after_time_base)

function main()  {
	ANS = calculate("TOTAL_AMOUNT_BASE") - calculate("TOTAL_COST_BASE") - calculate("TOTAL_TIME_BASE");
	return ANS.oconv("MD" ^ USER2[3] ^ "0PZ");
}
libraryexit(total_profit_after_time_base)


libraryinit(total_profit_base)

function main()  {
	ANS = calculate("TOTAL_AMOUNT_BASE") - calculate("TOTAL_COST_BASE");
	return ANS.oconv("MD" ^ USER2[3] ^ "0PZ");
}
libraryexit(total_profit_base)


libraryinit(total_profit_percent)

function main()  {
	var income = calculate("TOTAL_INV_AMOUNT_BASE");
	var cost = calculate("TOTAL_COST_BASE");
	return (income - cost) / cost;
}
libraryexit(total_profit_percent)


libraryinit(tot_accrued_amount_base)

function main()  {
	ANS = "";
	var ordernos = RECORD.a(6);
	if (ordernos) {
		var prodorders;
		if (prodorders.open("", "PRODUCTION_ORDERS")) {
			var norders = ordernos.count(VM) + 1;
			for (var vn = 1; vn <= norders; ++vn) {
				var prodorder;
				if (prodorder.read(prodorders, ordernos.a(1, vn))) {
					ANS += prodorder.a(3) / prodorder.a(5);
				}
			};//vn;
		}
	}
	return ANS;
}
libraryexit(tot_accrued_amount_base)


libraryinit(tot_suppinv_amount_base)

function main()  {
	ANS = "";
	var ordernos = RECORD.a(4);
	if (ordernos) {
		var prodorders;
		if (prodorders.open("", "PRODUCTION_ORDERS")) {
			var norders = ordernos.count(VM) + 1;
			for (var vn = 1; vn <= norders; ++vn) {
				var prodorder;
				if (prodorder.read(prodorders, ordernos.a(1, vn))) {
					if (prodorder.a(7)) {
						//supplier invoice received
						ANS += (prodorder.a(3)).sum() / (prodorder.a(5)).sum();
					}
				}
			};//vn;
		}
	}
	return ANS;
}
libraryexit(tot_suppinv_amount_base)


libraryinit(year_period)

function main()  {
	var period = RECORD.a(1);
	return period.field("/", 2) ^ "." ^ ("00" ^ period.field("/", 1)).substr(-2, 2);
}
libraryexit(year_period)


libraryinit(market_name)

function main()  {
	return calculate("MARKET_CODE").xlate("MARKETS", 1, "X");

}

libraryexit(market_name)

