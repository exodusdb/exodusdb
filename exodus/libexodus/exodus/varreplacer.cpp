io   var::r(int fieldno, int valueno, int subvalueno, in replacement) {

	THISIS("io   var::r(int fieldno, int valueno, int subvalueno, in replacement)")
	assertStringMutator(function_sig);
	ISSTRING(replacement)

	// return whole thing if replace 0,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0) {
		UNLIKELY
		// functionmode return var(replacement);
		// proceduremode


		var_str = replacement.var_str;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::size_t pos = 0;
	std::size_t field_end_pos;



	// negative means append
	if (fieldno < 0) {

		// append a FM_ only if there is existing data
		if (not var_str.empty())
			//var_str += FM_;
			var_str.push_back(FM_);



		pos = var_str.size();
		field_end_pos = pos;
	} else {

		// find the starting position of the field or append enough FM_ to satisfy
		int fieldn2 = 1;
		while (fieldn2 < fieldno) {
			pos = var_str.find(FM_, pos);
			// past of of string?
			if (pos == std::string::npos) {



				var_str.append(fieldno - fieldn2, FM_);
				// pos=var_str.size();
				// fieldn2=fieldno
				break;
			}
			pos++;
			fieldn2++;
		}

		// find the end of the field (or string)
		if (pos == std::string::npos) {
			pos = var_str.size();
			field_end_pos = pos;
		} else {
			field_end_pos = var_str.find(FM_, pos);
			if (field_end_pos == std::string::npos)
				field_end_pos = var_str.size();
		}
	}

	////////////// FIND VALUE ///////////////////
	std::size_t value_end_pos;

	// zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno < 0) {
		if (field_end_pos - pos > 0) {
			var_str.insert(field_end_pos, _VM);
			field_end_pos++;
		}
		pos = field_end_pos;
		value_end_pos = field_end_pos;

	} else if (valueno == 0 && subvalueno == 0) {






		var_str.replace(pos, field_end_pos - pos, replacement.var_str);
		return *this;

	} else {

		// find the starting position of the value or insert enough VM_ to satisfy
		int valuen2 = 1;
		auto sv = std::string_view(var_str.data(), field_end_pos);
		while (valuen2 < valueno) {
			pos = sv.find(VM_, pos);
			// past end of string or field?
//			if (pos >= field_end_pos || pos == std::string::npos) {
			if (pos >= field_end_pos) {
				// pos = field_end_pos;
				// var_str.insert(field_end_pos,std::string(valueno-valuen2,VM_));

				var_str.insert(field_end_pos, valueno - valuen2, VM_);
				field_end_pos += valueno - valuen2;
				sv = std::string_view(var_str.data(), field_end_pos);
				pos = field_end_pos;
				break;
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

	// zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno < 0) {
		if (value_end_pos - pos > 0) {
			var_str.insert(value_end_pos, _SM);
			value_end_pos++;
		}
		pos = value_end_pos;
		subvalue_end_pos = value_end_pos;



	} else if (subvalueno == 0) {






		var_str.replace(pos, value_end_pos - pos, replacement.var_str);
		return *this;

	} else {
		// find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2 = 1;
		auto sv = std::string_view(var_str.data(), value_end_pos);
		while (subvaluen2 < subvalueno) {
			pos = sv.find(SM_, pos);
			// past end of string or value
//			if (pos >= value_end_pos || pos == std::string::npos) {
			if (pos >= value_end_pos) {



				// pos = value_end_pos;
				// var_str.insert(value_end_pos,std::string(subvalueno-subvaluen2,SM_));
				var_str.insert(value_end_pos, subvalueno - subvaluen2, SM_);
				value_end_pos += subvalueno - subvaluen2;
				sv = std::string_view(var_str.data(), value_end_pos);
				pos = value_end_pos;
				break;
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






	var_str.replace(pos, subvalue_end_pos - pos, replacement.var_str);

	return *this;
}
