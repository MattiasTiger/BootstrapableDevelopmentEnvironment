#include "ParserTest.h"


ParserTest::ParserTest(void)
{
}


ParserTest::~ParserTest(void)
{
}

bool handler_digit(StatementTree & st, Pattern & p)
{
<<<<<<< HEAD
	st.children.push_back(new StatementTree(Statement("["+p.pattern+"]")));
=======
	st.children.push_back(new StatementTree(Statement(p.pattern)));
>>>>>>> origin/master
	return true;
}
bool handler_character(StatementTree & st, Pattern & p)
{
<<<<<<< HEAD
	st.children.push_back(new StatementTree(Statement("["+p.pattern+"]")));
=======
	st.children.push_back(new StatementTree(Statement(p.pattern)));
>>>>>>> origin/master
	return true;
}
bool handler_add(StatementTree & st, Pattern & p)
{
<<<<<<< HEAD
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
=======
	st.statement = Statement((*++st.parent->children.rbegin())->statement.str + " + " + st.children.front()->statement.str);
	st.children.clear();
	st.isList = false;
	st.parent->children.remove(*++st.parent->children.rbegin());
	
	return true;
}

>>>>>>> origin/master

void ParserTest::run()
{
	Parser p;
	p.addPattern(Pattern("1", handler_digit));
	p.addPattern(Pattern("2", handler_digit));
<<<<<<< HEAD
	p.addPattern(Pattern("xyz", handler_character));
	p.addPattern(Pattern("+$", handler_add));
	p.addPattern(Pattern("($)", handler_parentesis));

	std::string in;
	in = "(1+xyz)+(2+1)";
=======
	p.addPattern(Pattern("x", handler_character));
	p.addPattern(Pattern("+$", handler_add));

	std::string in = "1+2+1+2";
>>>>>>> origin/master

	p.parse(in);
	std::cout << "In: " << in << "\n" << "Out: " << p.statementTree.children.front()->statement.str << "\n";
}