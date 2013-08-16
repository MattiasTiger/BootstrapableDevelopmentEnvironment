#include "SimpleLanguage.h"

DataManager memoryManager;

void simple_language_test()
{
	memoryManager.addType(new Type("void", 0));
	memoryManager.addType(new Type("char", 1));
	memoryManager.addType(new Type("bool", 1));
	memoryManager.addType(new Type("int", 4));

	Parser<LogicBlock*> p;
//	RegularExpressionParser<LogicBlock*> p_regexp;
//	p_regexp.init();
//	p_regexp.parse("( |\t|\n)*");
//	p.addPattern(p_regexp.statementTree.children.front()->statement.start);

	p.addPattern(Pattern_logic("0", handler_constant));
	p.addPattern(Pattern_logic("1", handler_constant));
	p.addPattern(Pattern_logic("5", handler_constant));
	p.addPattern(Pattern_logic("int a", handler_variableDeclaration));
	p.addPattern(Pattern_logic("a", handler_variable));
	p.addPattern(Pattern_logic("if($) $ else $", handler_if));
	p.addPattern(Pattern_logic("while($) $", handler_while));
	p.addPattern(Pattern_logic("==$", handler_equal));
	p.addPattern(Pattern_logic("+$", handler_add));
	p.addPattern(Pattern_logic("=$", handler_store));
	p.addPattern(Pattern_logic("=$;", handler_store));
	p.addPattern(Pattern_logic("print($)", handler_print));
	p.addPattern(Pattern_logic("{$}", handler_multipleStatements));
	p.addPattern(Pattern_logic("($)", handler_paranthesis));
	//p.addPattern(Pattern_logic(";", handler_statementSeparation));


	std::string in;
	in = "1+5";
	in = "1+5+1";
	in = "print(1+5)";
	in = "if(1==0) print(1) else print(0)";
	in = "{int aa=5;print(a)}";	// Will be solved using a termination property to patterns.
	//in = " 1 + 1 ";
	p.parse(in);
	LogicBlock * program = p.statementTree.children.front()->statement;
	program->eval();
	std::cout << "\n";
	//std::cout << *(int*)program->value << "\n";

}





bool handler_constant(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	trim(str);
	LogicBlock_data * data = new LogicBlock_data();
	data->value = memoryManager.allocate(memoryManager.getType("int"));
	*(int*)data->value = int(str[0] - '0');
	data->type = memoryManager.getType("int");
	st.statement = data;
	st.isList = false;
	return true;
}
bool handler_variable(StatementTree_logic & st, Pattern_logic & p, std::string & str)
{
	trim(str);
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
	
	std::list<std::string> parts;
	split(str, ' ', parts);
	std::string typeName = parts.front();
	std::string variableName = parts.back();
	Type * type = memoryManager.getType(typeName);
	if(!type)
		std::cout << "Error: No type named \"" << typeName << "\" declared!\n";
	else
	{
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



void trim(std::string & str)
{
	unsigned int start = 0;
	unsigned int end = str.size()-1;
	while(start < str.size()  && str[start] == ' ')
		start++;	
	while(end > 0 && str[end] == ' ')
		end--;
	if(end > 0)
		str = str.substr(start, end-start);
}

void split(std::string & str, char splitter, std::list<std::string> & result)
{
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
