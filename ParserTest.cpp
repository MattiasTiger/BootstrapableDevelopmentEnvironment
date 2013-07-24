#include "ParserTest.h"


ParserTest::ParserTest(void)
{
}


ParserTest::~ParserTest(void)
{
}

bool handler_digit(StatementTree & st, Pattern & p)
{
	st.children.push_back(new StatementTree(Statement("["+p.pattern+"]")));
	return true;
}
bool handler_character(StatementTree & st, Pattern & p)
{
	st.children.push_back(new StatementTree(Statement("["+p.pattern+"]")));
	return true;
}
bool handler_add(StatementTree & st, Pattern & p)
{
	st.statement = Statement("["+(*++st.parent->children.rbegin())->statement.str + " + " + st.children.front()->statement.str+"]");
	st.children.clear();
	st.isList = false;
	st.parent->children.remove(*++st.parent->children.rbegin());
	return true;
}
bool handler_parentesis(StatementTree & st, Pattern & p)
{
	st.statement = Statement("[("+st.children.front()->statement.str+")]");
	st.children.clear();
	st.isList = false;
	return true;
}

void ParserTest::run()
{
	Parser p;
	p.addPattern(Pattern("1", handler_digit));
	p.addPattern(Pattern("2", handler_digit));
	p.addPattern(Pattern("xyz", handler_character));
	p.addPattern(Pattern("+$", handler_add));
	p.addPattern(Pattern("($)", handler_parentesis));

	std::string in;
	in = "(1+xyz)+(2+1)";

	p.parse(in);
	std::cout << "In: " << in << "\n" << "Out: " << p.statementTree.children.front()->statement.str << "\n";
}