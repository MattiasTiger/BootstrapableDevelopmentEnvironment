#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "Type.h"

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

class Function
{
public:
	std::string name;
	char * returnValue;
	Type * returnType;
	std::vector<char*> arguments;
	std::vector<Member_header> argumentTypes;
public:
	Function(std::string name, Type * returnType) : name(name), returnType(returnType) {}
};

#endif
