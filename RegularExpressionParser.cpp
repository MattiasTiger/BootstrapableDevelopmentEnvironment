
#include "RegularExpressionParser.h"

///////////////
#include <sstream>
std::string getExtendedAsciiCharacterPattern(std::string except)
{
    std::stringstream ss;
    for(int n = 1; n < 128; n++)
        if(std::string::npos == except.find_first_of(char(n)))
            ss << char(n) << "|";
    std::string s = ss.str();
    return s.substr(0,s.size()-1);
}


//////////////////
