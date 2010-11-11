/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/

#include <exodus/mvenvironment.h>
#include "mvwindow.h"

namespace exodus
{

class Definition : public MvLib
{

public:

    var operator() (const var& mode);

//	Definition() : env(*global_environments[*tss_environmentns.get()]), win(*tss_wins.get()){};
	Definition() :
	env(*global_environments[getenvironmentn()]), win(*global_wins[getenvironmentn()])
{};

private:

	MvEnvironment& env;
	MvWindow& win;

    void gosub_postreadfix();

};

} // of namespace exodus
