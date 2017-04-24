/*
 * globalFunctions.cc

 *
 *  Created on: Apr 17, 2017
 *      Author: adamgeva
 */

#include "globalFunctions.hh"

//todo: remove from here to a macro!!!
std::string IntToString (int a)
{
    std::ostringstream temp;
    temp<<a;
    return temp.str();
}



