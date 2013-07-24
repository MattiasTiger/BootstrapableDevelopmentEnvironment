#include "ParserTest.h"


ParserTest::ParserTest(void)
{
}


ParserTest::~ParserTest(void)
{
}

bool handler_digit(StatementTree & st, Pattern & p)
{
	st.statement = Statement("["+p.pattern+"]");
	st.isList = false;
	return true;
}
bool handler_character(StatementTree & st, Pattern & p)
{
	st.statement = Statement("["+p.pattern+"]");
	st.isList = false;
	return true;
}
bool handler_add(StatementTree & st, Pattern & p)
{
	st.statement = Statement("["+(*++st.parent->children.rbegin())->statement.str + " + " + st.children.back()->statement.str+"]");
	st.isList = false;
	st.children.clear();
	*(*++st.parent->children.rbegin()) = st;
	st.parent->children.erase((++st.parent->children.rbegin()).base());
	return true;
}
bool handler_parentesis(StatementTree & st, Pattern & p)
{
	st.statement = Statement("("+st.children.front()->statement.str+")");
	st.children.clear();
	st.isList = false;
	return true;
}
bool handler_tester(StatementTree & st, Pattern & p)
{
	st.statement = Statement("[a "+st.children.front()->statement.str+" "+st.children.back()->statement.str+"]");
	st.children.clear();
	st.isList = false;
	return true;
}

void ParserTest::run()
{
	Parser p;
	p.addPattern(Pattern("1", handler_digit));
	p.addPattern(Pattern("2", handler_digit));
	//p.addPattern(Pattern("xyz", handler_character));
	p.addPattern(Pattern("+$", handler_add));
	p.addPattern(Pattern("($)", handler_parentesis));
	p.addPattern(Pattern("a $ $", handler_tester));

	std::string in;
	in = "(1+xyz)+(2+1)";
	in = "a 1 2";
	in = "a (1+2)+2 2+1";

	p.parse(in);
	std::cout << "In: " << in << "\n" << "Out: " << p.statementTree.children.front()->statement.str << "\n";
}

