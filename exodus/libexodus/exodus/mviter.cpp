#include <mv.h>

namespace exodus {

    //CONSTRUCTOR from a var
    var_iter::var_iter(var& v) : data(&v) {};

    //check iter != iter (i.e. iter != end()
    bool var_iter::operator != (var_iter& vi) {
        return index != vi.index;
    }

    //CONVERSION - conversion to var
    var_iter::operator var*() {

        //find the end of the field if not already known
        if (index2 == std::string::npos)
            index2 = data->var_str.find(FM_, index);

        //extract the field
        field = data->var_str.substr(index, index2);

        return &field;
    }

    //INCREMENT
    var_iter var_iter::operator ++ () {

        //find the end of the field if not already found from a call to above CONVERSION
        if (index2 == std::string::npos)
            index2 = data->var_str.find(FM_, index);

        //move up to the next field
        index = index2;

        //skip over any FM character
        if (index !=std::string::npos && data->var_str[index] == FM_)
            index++;

        //indicate that the end of the next field is not yet known
        index2 = std::string::npos;

        return *this;

    }

//BEGIN - free function to create an iterator -> begin
DLL_PUBLIC var_iter begin(var& v) {
    return var_iter(v);
}

//END - free function to create an interator -> end
DLL_PUBLIC var_iter end(var& v) {
    var_iter vi{v};
    vi.index = std::string::npos;
    return vi;
}

} //namespace exodus
