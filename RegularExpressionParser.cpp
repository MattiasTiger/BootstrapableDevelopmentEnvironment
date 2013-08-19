#include "RegularExpressionParser.h"


void test_RegularExpressionParser()
{
	RegularExpressionParser<Statement_regexp,Statement_regexp> p;
	p.init();

	std::string in;
	in = "(a|b)*b";
	in = "ba*(a|b)b";
	//in = "a(ab)*b";
	//in = "(a|b)(a|b)*[(a|b)(a|b)*(, (a|b)(a|b)*)*]";
	//in = "aa*[aa*(,( )*aa*)*]";
	//in = "aa* *\\(aa* *(, *aa*)*\\)";	// A function (with name "a+" taking any number of arguments with names "a+"

	std::cout << "Parser:\n-----------------\n" << p.patternTrie.toString() << "\n\n";

	p.parse(in);

	p.statementTree.parent->children.erase((++p.statementTree.parent->children.rbegin()).base());
	p.statementTree.statement = p.statementTree.children.front()->statement;
	p.statementTree.statement.str = p.statementTree.children.front()->statement.str;
	int c = 0;
	std::cout << "\n>>Statments<<\n";
	for(std::list<StatementTree<Statement_regexp>*>::iterator i = p.statementTree.children.begin(); i != p.statementTree.children.end(); i++)
	{
		std::cout << c++ << ") " << (*i)->statement.str << "\n" << "\n" << (*i)->statement.start->toString() << "---\n";
	}
	for(std::list<StatementTree<Statement_regexp>*>::iterator i = ++p.statementTree.children.begin(); i != p.statementTree.children.end(); i++)
	{
		p.statementTree.statement.str += (*i)->statement.str;
		merge<Statement_regexp,Statement_regexp>(&p.statementTree.statement, &(*i)->statement);
	}
	//for(std::list<Trie*>::iterator i = p.statementTree.statement.goals.begin(); i != p.statementTree.statement.goals.end(); i++)
	//{
	//	(*i)->p = (Pattern*)1;
	//}
	p.statementTree.children.clear();
	p.statementTree.isList = false;
	std::cout << "In: " << in << "\n" << "Out: " << p.statementTree.statement.str << ":\n" << p.statementTree.statement.start->toString() << "\n";
}
