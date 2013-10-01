#include "SimpleLanguage.h"

DataManager memoryManager;

void simple_language_test()
{
	memoryManager.addType(new Type("void", 0));
	memoryManager.addType(new Type("char", 1));
	memoryManager.addType(new Type("bool", 1));
	memoryManager.addType(new Type("int", 4));

    
    ParserExtended<ParserStatement<LogicBlock*> > regExpParser;
    regExpParser.addPattern("\\$\\$",  new h_regexp_concat<LogicBlock*>);
    regExpParser.addPattern("(\\$)",  new h_regexp_parentheses<LogicBlock*>);
    regExpParser.addPattern("\\$|\\$",  new h_regexp_divider<LogicBlock*>);
    regExpParser.addPattern("\\$*",   new h_regexp_kleinClosure<LogicBlock*>);
    regExpParser.addPattern("\\\\", new h_regexp_character<LogicBlock*>);
	regExpParser.addPattern("\\(",  new h_regexp_character<LogicBlock*>);
	regExpParser.addPattern("\\)",  new h_regexp_character<LogicBlock*>);
	regExpParser.addPattern("\\|",  new h_regexp_character<LogicBlock*>);
	regExpParser.addPattern("\\*",  new h_regexp_character<LogicBlock*>);
	regExpParser.addPattern("\\\\$",  new h_regexp_character<LogicBlock*>);
    std::string c;
	for(int n = 0; n < 128; n++)
	{
        if(n != '$' && n != '(' && n != ')' && n != '*' && n != '|')
		{
		    c = char(n);
			regExpParser.addPattern(c, new h_regexp_character<LogicBlock*>);
		}
	}
    ParserExtended<LogicBlock*> p;
    p.assignPreParser(&regExpParser);
    p.addPattern("\\$", "\\$");  // Allow sub patterns..
    
    //p.addPattern("(0|1|2|3|4|5|6|7|8|9)",handler_constant);
    p.addPattern("(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*",handler_integer);
    p.addPattern("\\word","(a|b|c)(a|b|c|0|1|2|3|4|5|6|7|8|9)*");
	p.addPattern("( |\t|\n)( |\t|\n)*", handler_NOP);
    p.addPattern("\\ ", "( )");
    p.addPattern(" ", "( |\t|\n)*");
    
    p.addPattern("{ (\\$ )*} ", handler_multipleStatements);
	p.addPattern("int\\ \\word", handler_variableDeclaration);
	p.addPattern("\\word", handler_variable);
	p.addPattern(" ; ", handler_NOP);
	p.addPattern("if \\( \\$ \\) \\$ else \\$", handler_if);
	p.addPattern("while\\(\\$\\)\\$", handler_while);
	p.addPattern("\\$ == \\$", handler_equal);
	p.addPattern("\\$ + \\$", handler_add);
	p.addPattern("\\$ = \\$", handler_store);
	p.addPattern("print\\( \\$ \\)", handler_print);
	p.addPattern("\\(\\$\\)", handler_paranthesis);
    
    //p.setErrorHandler(handler_error);
    
    std::vector<std::string> input;
        
    input.push_back("1+2");
    input.push_back("1+5+1");
    input.push_back("print(1+5)");
	input.push_back("if(1==1)print(1)elseprint(2)");
	input.push_back("if(1 == 0)\n\tprint(1)\nelse\n\tprint(2)");
	input.push_back("{print(5) ; print(6)}");
    
	input.push_back("{\n\tint a;\n\ta = 9 + 2;\n\tprint( a );\n}");
    input.push_back("{\n\tint cba = 1;\n\tint b2;\n\tif(cba == 1)\n\t{\n\t\tb2 = cba + 45;\n\t}\n\telse\n\t{\n\t\tb2 = 9;\n\t}\n\tprint(b2);\n}");
    
    LogicBlock * program;
    for(int n = 0; n < input.size(); n++)
    {
        std::cout << "\nProgram #" << n+1 << "\n-----------------\n";
	    std::cout << "INPUT:\n" << input[n];
	    p.parse(input[n]);
	    program = p.statementTree.children.front()->statement;
        if(p.statementTree.children.size() > 1)
        {
            std::cout << "\nINTERMEDIATE:\n";
	        program->eval();
            std::cout << "\nRETURN:\n";
            if(program->type != 0)
	            std::cout << *(int*)program->value << "\n";
            else
                std::cout << "void" << "\n";
            delete program;
        }
        else
        {
            std::cout << "PARSING FAILED!\n";
        }
    }
    std::cout << "\n";
    
}
/*
bool handler_error(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
    if(str.back() == ';')
    {
        st.removeThis();
	    return true;
    }
    else
    {
        st.removeThis();
        return false;
    }
}*/

bool handler_NOP(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
    st.removeThis();
	return true;
}

bool handler_constant(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	trim(str, " \t\n");
	LogicBlock_data * data = new LogicBlock_data();
	data->value = memoryManager.allocate(memoryManager.getType("int"));
	*(int*)data->value = int(str[0] - '0');
	data->type = memoryManager.getType("int");
	st.statement = data;
	st.isList = false;
	return true;
}
#include <sstream>
bool handler_integer(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	trim(str, " \t\n");
	LogicBlock_data * data = new LogicBlock_data();
	data->value = memoryManager.allocate(memoryManager.getType("int"));
    std::stringstream ss;
    ss << str;
    ss >> *(int*)data->value;
	//*(int*)data->value = int(str[0] - '0');
	data->type = memoryManager.getType("int");
	st.statement = data;
	st.isList = false;
	return true;
}
bool handler_variable(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	trim(str, " \t\n");
	char * variable = memoryManager.getVariable(str);
	if(!variable)
		std::cout << "Error: No variable named \"" << str << "\" declared!\n";
	else
	{
		LogicBlock_data * data = new LogicBlock_data;
		data->value = variable;
		data->type = memoryManager.getVariableType(variable);
		data->name = "Var [" + str + "]";
		st.statement = data;
		st.isList = false;
	}
	return true;
}
bool handler_variableDeclaration(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	trim(str, " \t\n");
	std::list<std::string> parts;
	split(str, ' ', parts);
	std::string typeName = parts.front();
	std::string variableName = parts.back();
	trim(typeName, " \t\n");
	trim(variableName, " \t\n");
	Type * type = memoryManager.getType(typeName);
	if(!type)
		std::cout << "Error: No type named \"" << typeName << "\" declared!\n";
	else
	{
		// TODO: add a "declare variable" LogicBlock so that it is done at runtime... (in the right context)
		char * variable = memoryManager.allocate(type);
		memoryManager.addVariable(variableName, variable);
		LogicBlock_data * data = new LogicBlock_data;
		data->value = variable;
		data->type = type;
		data->name = "VarDecl. (" + parts.front() + ") [" + parts.back() + "]";
		st.statement = data;
		st.isList = false;
	}
	return true;
}
bool handler_if(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	if(st.firstPost()->statement->type->name != "bool")
		std::cout << "Error: The condition of the if-statement should be of \"bool\" type, but it is of \"" << st.firstPost()->statement->type->name << "\" type\n";
	else if(st.children.size() < 3)
		std::cout << "Error: To few arguments to if-else, it requires 3 (only got " << st.children.size() << ")\n";
	else
	{
		LogicBlock_if * if_statement = new LogicBlock_if;
		std::list<StatementTree_logic*>::iterator child = st.children.begin();
		if_statement->children.push_back((*child++)->statement);
		if_statement->children.push_back((*child++)->statement);
		if_statement->children.push_back((*child++)->statement);
		st.statement = if_statement;
		st.isList = false;
	}
	return true;
}
bool handler_while(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	if(st.firstPost()->statement->type->name != "bool")
		std::cout << "Error: The condition of the while-statement should be of \"bool\" type, but it is of \"" << st.firstPost()->statement->type->name << "\" type\n";
	else if(st.children.size() < 2)
		std::cout << "Error: To few arguments to while, it requires 2 (only got " << st.children.size() << ")\n";
	else
	{
		LogicBlock_while * while_statement = new LogicBlock_while();
		std::list<StatementTree_logic*>::iterator child = st.children.begin();
		while_statement->children.push_back((*child++)->statement);
		while_statement->children.push_back((*child++)->statement);
		st.statement = while_statement;
		st.isList = false;
	}
	return true;
}
bool handler_equal(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	if(st.children.size() < 1 || st.parent->children.size() < 2)
		std::cout << "Error: To few arguments to ==, it requires 2 \n";
	else if(st.firstPre()->statement->type != (*st.children.begin())->statement->type)
		std::cout << "Error: The arguments of the equal-statement (==) should be of \"int\" type, but it is of \"" << st.firstPost()->statement->type->name << "\" type\n";
	else
	{
		LogicBlock_equal * equal_statement = new LogicBlock_equal();
		std::list<StatementTree_logic*>::iterator child = st.children.begin();
		equal_statement->children.push_back(st.firstPre()->statement);
		equal_statement->children.push_back(st.firstPost()->statement);
		equal_statement->type = memoryManager.getType("bool");
		equal_statement->value = new char[1];
		st.statement = equal_statement;
		st.removeFirstPre();
		st.removeChildren();
		st.isList = false;
	}
	return true;
}
bool handler_add(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	if(st.children.size() < 1 || st.parent->children.size() < 2)
		std::cout << "Error: To few arguments to +, it requires 2 \n";
	else if(st.firstPre()->statement->type != (*st.children.begin())->statement->type)
		std::cout << "Error: The arguments of the add-statement (+) should be of \"int\" type, but it is of \"" << st.firstPost()->statement->type->name << "\" type\n";
	else
	{
		LogicBlock_add_int *add_statement = new LogicBlock_add_int();
		add_statement->children.push_back(st.firstPre()->statement);
		add_statement->children.push_back(st.firstPost()->statement);
		add_statement->type = memoryManager.getType("int");
		add_statement->value = new char[4];
		st.statement = add_statement;
		st.removeFirstPre();
		st.removeChildren();
		st.isList = false;
	}
	return true;
}
bool handler_store(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	if(st.children.size() < 1 || st.parent->children.size() < 1)
		std::cout << "Error: To few arguments to =, it requires 2 \n";
	else if(st.firstPre()->statement->type != (*st.children.begin())->statement->type)
		std::cout << "Error: The arguments of the store-statement (=) should be of \"int\" type, but it is of \"" << (*st.children.begin())->statement->type->name << "\" type\n";
	else
	{
		LogicBlock_store * store_statement = new LogicBlock_store();
		std::list<StatementTree_logic*>::iterator child = st.children.begin();
		store_statement->children.push_back(st.firstPre()->statement);
		store_statement->children.push_back(st.firstPost()->statement);
		store_statement->type = memoryManager.getType("int");
		store_statement->value = new char[4];
		st.statement = store_statement;
		st.removeFirstPre();
		st.removeChildren();
		st.isList = false;
	}
	return true;	
}
bool handler_print(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	LogicBlock_print * print_statement = new LogicBlock_print();
	print_statement->children.push_back(st.firstPost()->statement);
	st.statement = print_statement;
	st.isList = false;
	return true;
}
bool handler_statementSeparation(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	/*
	if(st.parent != 0 && st.firstPre() != 0)
	{
		LogicBlock_multipleStatement * multipleStatement_statement = new LogicBlock_multipleStatement();
		for(std::list<StatementTree_logic*>::iterator child = st.parent->children.begin(); *child != &st; child++)
			multipleStatement_statement->children.push_back((*child)->statement);
		st.parent->children.clear();
		st.parent->children.push_back(&st);
		st.statement = multipleStatement_statement;
		st.isList = false;
	}*/
	return true;
}

bool handler_multipleStatements(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	LogicBlock_multipleStatement * multipleStatement_statement = new LogicBlock_multipleStatement();
	for(std::list<StatementTree_logic*>::iterator child = st.children.begin(); child != st.children.end(); child++)
		multipleStatement_statement->children.push_back((*child)->statement);
	st.statement = multipleStatement_statement;
	st.removeChildren();
	st.isList = false;
	return true;
}
bool handler_paranthesis(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	
	return true;
}
bool handler_functionCall(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	
	return true;
}
bool handler_functionDeclaration(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	
	return true;
}



void trim(std::string & str, std::string discard)
{
	if(str.empty())
		return;
	unsigned int start = 0;
	unsigned int end = str.size();
	while(start < str.size() && std::find(discard.begin(),discard.end(),str[start]) != discard.end())
		start++;	
	while(end > 0 && std::find(discard.begin(),discard.end(),str[start]) != discard.end())
		end--;
	if(end > 0)
		str = str.substr(start, end-start);
}

void split(std::string & str, char splitter, std::list<std::string> & result)
{
	if(str.empty())
		return;
	bool word = str[0] != splitter;
	unsigned int n = 0;
	unsigned int start = 0;
	n++;
	while(n < str.size())
	{
		if(str[n-1] == splitter && str[n] != splitter)
			start = n;
		if(str[n-1] != splitter && str[n] == splitter)
			result.push_back(str.substr(start, n-start));
		n++;
	}
	if(str[n-1] != splitter)
		result.push_back(str.substr(start, n-start));
}
