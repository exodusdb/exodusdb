
// Similar code in three files. Use editor in parallel to keep in sync.
// nano varreplacer.cpp varinserter.cpp varremover.cpp
// alt+, and alt+. to switch file in nano

IO   var::inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) REF {

	THISIS("void var::inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) &")
	assertStringMutator(function_sig);
	ISSTRING(insertion)

	// 0,0,0 is like 1,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0) {
		UNLIKELY
		//if (var_str.size())
		if (not var_str.empty())
			var_str.insert(0, insertion.var_str + FM_);
		else
			var_str = insertion.var_str;
		return THIS;
	}

	/////////////   FIND FIELD  /////////////////
	std::size_t pos = 0;
	std::size_t field_end_pos;

	int pad = false;

	// negative means append
	if (fieldno < 0) {

		// append a FM_ only if there is existing data
		if (not var_str.empty())
			//var_str += FM_;
			var_str.push_back(FM_);

		pad = true;

		pos = var_str.size();
		field_end_pos = pos;
	} else {

		// find the starting position of the field or append enough FM_ to satisfy
		int fieldn2 = 1;
		while (fieldn2 < fieldno) {
			pos = var_str.find(FM_, pos);
			// past of of string?
			if (pos == std::string::npos) {

				pad = true;

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
		pad = true;
	} else if (valueno == 0 && subvalueno == 0) {



		if (!pad && !var_str.empty())
			var_str.insert(pos, _FM);

		var_str.insert(pos, insertion.var_str);
		return THIS;

	} else {

		// find the starting position of the value or insert enough VM_ to satisfy
		int valuen2 = 1;
		auto sv = std::string_view(var_str.data(), field_end_pos);
		while (valuen2 < valueno) {
			pos = sv.find(VM_, pos);
			// past end of string or field?
//			if (pos >= field_end_pos || pos == std::string::npos) {
			if (pos >= field_end_pos) {
				pad = true;
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
	//std::size_t subvalue_end_pos;

	// zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno < 0) {
		if (value_end_pos - pos > 0) {
			var_str.insert(value_end_pos, _SM);
			value_end_pos++;
		}
		pos = value_end_pos;
		//subvalue_end_pos = value_end_pos;

		pad = true;

	} else if (subvalueno == 0) {

		if (!pad && (pos < field_end_pos || valueno > 1))
			var_str.insert(pos, _VM);



		var_str.insert(pos, insertion.var_str);
		return THIS;

	} else {
		// find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2 = 1;
		auto sv = std::string_view(var_str.data(), value_end_pos);
		while (subvaluen2 < subvalueno) {
			pos = var_str.find(SM_, pos);
			// past end of string or value
//			if (pos >= value_end_pos || pos == std::string::npos) {
			if (pos >= value_end_pos) {

				pad = true;

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

		//not required
		// find the end of the subvalue (or string)
		//subvalue_end_pos = var_str.find(SM_, pos);
		//if (subvalue_end_pos > value_end_pos)
		//	subvalue_end_pos = value_end_pos;
	}


	if (!pad && (pos < value_end_pos || subvalueno > 1))
		var_str.insert(pos, _SM);



	var_str.insert(pos, insertion.var_str);

	return THIS;
}
