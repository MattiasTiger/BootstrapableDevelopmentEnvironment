#include <iostream>
#include <string>
#include "parser.h"
#include "ParserTest.h"
#include "RegularExpressionParser.h"
#include "Logic/SimpleLanguage.h"

int main()
{	
	//test_RegularExpressionParser();
	simple_language_test();

	std::cout << "end\n";
	std::cin.get();

	return 0;
}

