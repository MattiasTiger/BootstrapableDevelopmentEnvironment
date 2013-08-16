#include "RegularExpressionParser.h"


RegularExpressionParser::RegularExpressionParser(void)
{
	init();
}

RegularExpressionParser::~RegularExpressionParser(void)
{
}

void RegularExpressionParser::init()
{
	std::string c;
	for(int n = 0; n < 256; n++)
	{
		if(n != '(' && n != ')' && n != '|' && n != '$' && n != '\\')
		{
			c = char(n);
			addPattern(Pattern_regexp(c, handler_character));
		}
	}
	/*
	addPattern(Pattern_regexp("a", handler_character));
	addPattern(Pattern_regexp("b", handler_character));
	addPattern(Pattern_regexp("c", handler_character));
	addPattern(Pattern_regexp("[", handler_character));
	addPattern(Pattern_regexp("]", handler_character));
	addPattern(Pattern_regexp(",", handler_character));
	addPattern(Pattern_regexp(" ", handler_character));
	*/
	// Regular expression base rules
	addPattern(Pattern_regexp("($)", handler_parentheses));
	addPattern(Pattern_regexp("|$", handler_divider));
	addPattern(Pattern_regexp("*", handler_kleinClosure));

	// Allow for the usage of the special characters
	addPattern(Pattern_regexp("\\\\", handler_character));
	addPattern(Pattern_regexp("\\(", handler_character));
	addPattern(Pattern_regexp("\\)", handler_character));
	addPattern(Pattern_regexp("\\|", handler_character));
	addPattern(Pattern_regexp("\\*", handler_character));

}

void test_RegularExpressionParser()
{
	RegularExpressionParser p;
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
	for(std::list<StatementTree_regexp*>::iterator i = p.statementTree.children.begin(); i != p.statementTree.children.end(); i++)
	{
		std::cout << c++ << ") " << (*i)->statement.str << "\n" << "\n" << (*i)->statement.start->toString() << "---\n";
	}
	for(std::list<StatementTree_regexp*>::iterator i = ++p.statementTree.children.begin(); i != p.statementTree.children.end(); i++)
	{
		p.statementTree.statement.str += (*i)->statement.str;
		merge(&p.statementTree.statement, &(*i)->statement);
	}
	//for(std::list<Trie*>::iterator i = p.statementTree.statement.goals.begin(); i != p.statementTree.statement.goals.end(); i++)
	//{
	//	(*i)->p = (Pattern*)1;
	//}
	p.statementTree.children.clear();
	p.statementTree.isList = false;
	std::cout << "In: " << in << "\n" << "Out: " << p.statementTree.statement.str << ":\n" << p.statementTree.statement.start->toString() << "\n";
}



// Handlers
// ----------------------

bool handler_character(StatementTree_regexp & st, Pattern_regexp & p, std::string & str)
{
	st.statement.str = str.back();
	st.statement.start = new DFA_Trie_regexp();
	st.statement.goals.push_back(new DFA_Trie_regexp());
	st.statement.start->add_branch(str.back(), st.statement.goals.front());
	st.statement.goals.front()->data = (Pattern_regexp*)1;	// Debug... fix this
	st.isList = false;
	return true;
}
bool handler_parentheses(StatementTree_regexp & st, Pattern_regexp & p, std::string & str)
{
	st.mergeAll();
	st.statement.str = "(" + st.statement.str + ")";
	return true;
}
bool handler_divider(StatementTree_regexp & st, Pattern_regexp & p, std::string & str)
{
	st.mergeAllPre();
	st.mergeAll();
	st.statement.str = st.firstPre()->statement.str + "|" + st.statement.str;

	merge(st.statement.start, st.firstPre()->statement.start, st.statement.goals);
	for(std::list<DFA_Trie_regexp *>::iterator it = st.firstPre()->statement.goals.begin(); it != st.firstPre()->statement.goals.end(); it++)
		st.statement.goals.push_back(*it);
	st.statement.goals.unique();

	st.removeFirstPre();
	return true;
}

bool handler_kleinClosure(StatementTree_regexp & st, Pattern_regexp & p, std::string & str)
{
	st.firstPre()->mergeAll();
	st.statement = st.firstPre()->statement;
	st.statement.str += "*";
	std::list<DFA_Trie_regexp *> goals(st.statement.goals);
	//for(std::list<Trie*>::iterator it = st.statement.goals.begin(); it != st.statement.goals.end(); it++)
	//	merge(st.statement.start,*it, goals);
	for(std::list<DFA_Trie_regexp*>::iterator goal = st.statement.goals.begin(); goal != st.statement.goals.end(); goal++)
	{
		if(st.statement.start->isFinished() && st.statement.start->data != (*goal)->data)
			std::cout << "kleinClosure: inconsistent pattern; different goal handlers, cannot merge\n";
		st.statement.start->data = (*goal)->data;

		for(std::list<DFA_Trie_regexp*>::iterator parent = (*goal)->parents.begin(); parent != (*goal)->parents.end(); parent++)
			for(int b = 0; b < 256; b++)
					if((*parent)->branch[b] == *goal)
					{
						//(*parent)->add_branch(b, st.statement.start);
						(*parent)->branch[b] = st.statement.start;
					}
	}
	st.statement.goals.clear();
	st.statement.goals.push_back(st.statement.start);
	st.removeFirstPre();
	st.isList = false;
	return true;
}