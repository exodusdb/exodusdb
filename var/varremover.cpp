
// Similar code in three files. Use editor in parallel to keep in sync.
// nano varreplacer.cpp varinserter.cpp varremover.cpp
// alt+, and alt+. to switch file in nano

IO   var_stg::remover(int fieldno, int valueno, int subvalueno) REF {

	THISIS("void var_stg::remover(int fieldno, int valueno, int subvalueno) &")
	assertStringMutator(function_sig);


	// return "" if replacing 0,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0) {
		UNLIKELY
		// functionmode return "";//var(var1);
		// proceduremode
		var_str.clear();
		var_typ = VARTYP_STR;

		return THIS;
	}

	/////////////   FIND FIELD  /////////////////
	std::size_t pos = 0;
	std::size_t field_end_pos;



	// negative means remove nothing
	if (fieldno < 0) {
		return THIS;









	} else {

		// find the starting position of the field or quit
		int fieldn2 = 1;
		while (fieldn2 < fieldno) {
			pos = var_str.find(FM_, pos);
			// past of of string?
			if (pos == std::string::npos) {






				return THIS;
			}
			pos++;
			fieldn2++;
		}

		// find the end of the field (or string)




		field_end_pos = var_str.find(FM_, pos);
		if (field_end_pos == std::string::npos)
			field_end_pos = var_str.size();

	}

	////////////// FIND VALUE ///////////////////
	std::size_t value_end_pos;

	// zero means remove all values, negative means remove nothing
	if (valueno < 0) {
		return THIS;






	} else if (valueno == 0 && subvalueno == 0) {

		if (fieldno > 1)
			pos--;
		else if (field_end_pos < var_str.size())
			field_end_pos++;

		var_str.erase(pos, field_end_pos - pos);
		return THIS;

	} else {

		// find the starting position of the value or quit
		int valuen2 = 1;
		auto sv = std::string_view(var_str.data(), field_end_pos);
		while (valuen2 < valueno) {
			pos = sv.find(VM_, pos);
			// past end of string or field?
//			if (pos >= field_end_pos || pos == std::string::npos) {
			if (pos >= field_end_pos) {






				return THIS;
			}
			pos++;
			valuen2++;
		}

		// find the end of the value (or string)
		value_end_pos = sv.find(VM_, pos);
//		if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
		if (value_end_pos > field_end_pos)
			value_end_pos = field_end_pos;
	}

	////////// FIND SUBVALUE  //////////////////////
	std::size_t subvalue_end_pos;

	// zero means remove all subvalues, negative means remove nothing
	if (subvalueno < 0) {







		return THIS;

	} else if (subvalueno == 0) {

		if (valueno > 1)
			pos--;
		else if (value_end_pos < field_end_pos)
			value_end_pos++;

		var_str.erase(pos, value_end_pos - pos);
		return THIS;

	} else {
		// find the starting position of the subvalue or quit
		int subvaluen2 = 1;
		auto sv = std::string_view(var_str.data(), value_end_pos);
		while (subvaluen2 < subvalueno) {
			pos = sv.find(SM_, pos);
			// past end of string or value
//			if (pos >= value_end_pos || pos == std::string::npos) {
			if (pos >= value_end_pos) {

				return THIS;







			}
			pos++;
			subvaluen2++;
		}

		// find the end of the subvalue (or string)
		subvalue_end_pos = sv.find(SM_, pos);
//		if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
		if (subvalue_end_pos > value_end_pos)
			subvalue_end_pos = value_end_pos;
	}
	// wcout<<pos<<" "<<subvalue_end_pos<<" "<<subvalue_end_pos-pos<<endl;

	if (subvalueno > 1)
		pos--;
	else if (subvalue_end_pos < value_end_pos)
		subvalue_end_pos++;

	var_str.erase(pos, subvalue_end_pos - pos);

	return THIS;
}
