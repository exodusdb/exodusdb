/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
#ifndef MARKET_H
#define MARKET_H

#include <string>
#include <map>

#include <exodus/mv.h>

#include <exodus/mvenvironment.h>

//TODO #include <validcode3.h>

namespace exodus
{

class Market : public MvLib
{

private:

    //holder for pseudo global variables to do with thread
    MvEnvironment& env;

    //define a type called MarketFunction that is a pointer to a member function
    //that takes no parameters and returns an var
    typedef var (Market::*MarketFunction)();

    //define a type called Dict which is a map of MarketFunctions
    typedef std::map<const std::string, MarketFunction> Dict;

    //define a member variable of type Dict
    Dict dict;

public:

var operator() (const var& dictid);

//constructor builds the dict
//Market::Market(MvEnvironment& env=*global_environments[*tss_environmentns.get()]) : env(env);
Market();

var code();
var name();
var population();
var seq();
var vehicle_code();
var currency_code();
var date_time();
var stopped();
var version();
var username_updated();
var datetime_updated();
var station_updated();
var version_updated();
var status_updated();
var authorised();
var currency_name();
var rawname();

};

}// of namespace exodus

#endif //MARKET_H
