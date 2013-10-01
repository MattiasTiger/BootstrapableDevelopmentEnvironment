#include <iostream>
#include <string>
#include "testing/parser_test.h"
#include "testing/RegularExpressionParser_test.h"
#include "testing/ParserExtended_test.h"
#include "Logic/SimpleLanguage.h"

#include <fstream>

int main()
{	
	//parser_test();
	
    //regularExpressionParser_test();
    //parser_extended_test();
    simple_language_test();
	std::cout << "end\n";
	std::cin.get();

	return 0;
}

