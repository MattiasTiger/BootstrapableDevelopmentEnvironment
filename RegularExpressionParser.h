/* RegularExpressionParser.h
 * 
 * Description:
 *	Basic regular expression parser
 *  (Builds a DFA_Trie of the type specified of the regular expression provided)
 * 
 * 
 * Author: Mattias Tiger, 2013
 */

#include "Parser.h"

#ifndef _REGULAREXPRESSIONPARSER_H_
#define _REGULAREXPRESSIONPARSER_H_

class Statement_regexp;
typedef DFA_Trie<Pattern<Statement_regexp>,StatementTree<Statement_regexp> > DFA_Trie_regexp;

// For self improvements
class Statement_regexp
{
public:
	DFA_Trie_regexp * start;
	std::list<DFA_Trie_regexp *> goals;
public:
	std::string str;
	Statement_regexp() { start = 0; }
	Statement_regexp(std::string str) : str(str) {}
};

// For S other than Statement_regexp
template <class S>
class Statement_regexp_
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA;
public:
	DFA * start;
	std::list<DFA *> goals;
public:
	std::string str;
	Statement_regexp_() { start = 0; }
	Statement_regexp_(std::string str) : str(str) {}
};

//typedef StatementTree<Statement_regexp> StatementTree_regexp;
//typedef Pattern<Statement_regexp> Pattern_regexp;

template <class S, class S_wrapper>
class RegularExpressionParser : public Parser<S_wrapper>
{
	typedef Pattern<S_wrapper> Pattern_regexp;
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA;
public:
	RegularExpressionParser(void) {}
	~RegularExpressionParser(void) {}

	
	void parse(std::string str) {
		Parser::parse(str);
	}
	DFA * parse(std::string str, Pattern<S> * p) {
		parse(str);
		StatementTree<S_wrapper> st = statementTree.children.front()->statement;
		statementTree.parent->children.erase((++statementTree.parent->children.rbegin()).base());

		for(std::list<StatementTree<S_wrapper>*>::iterator i = ++statementTree.children.begin(); i != statementTree.children.end(); i++)
		{
			st.statement.str += (*i)->statement.str;
			merge<S,S_wrapper>(&st.statement, &(*i)->statement);
		}
		for(std::list<DFA *>::iterator it = st.statement.goals.begin(); it != st.statement.goals.end(); it++) 
		{
			(*it)->data = p;
			if(p)
				(*it)->handler = p->handler;
		}
		return st.statement.start;
	}

	void init()
	{
		std::string c;
		for(int n = 0; n < 256; n++)
		{
			if(n != '(' && n != ')' && n != '|' && n != '$' && n != '\\')
			{
				c = char(n);
				addPattern(Pattern_regexp(c, handler_character<S,S_wrapper>));
			}
		}

		// Regular expression base rules
		addPattern(Pattern_regexp("($)", handler_parentheses<S,S_wrapper>));
		addPattern(Pattern_regexp("|$", handler_divider<S,S_wrapper>));
		addPattern(Pattern_regexp("*", handler_kleinClosure<S,S_wrapper>));

		// Allow for the usage of the special characters
		addPattern(Pattern_regexp("\\\\", handler_character<S,S_wrapper>));
		addPattern(Pattern_regexp("\\(", handler_character<S,S_wrapper>));
		addPattern(Pattern_regexp("\\)", handler_character<S,S_wrapper>));
		addPattern(Pattern_regexp("\\|", handler_character<S,S_wrapper>));
		addPattern(Pattern_regexp("\\*", handler_character<S,S_wrapper>));
	}
};


// merge s1 and s2 into s1
template <class S, class S_wrapper>
void merge(S_wrapper * s1, S_wrapper * s2)
{
	for(std::list<DFA_Trie<Pattern<S>,StatementTree<S> > *>::iterator it = s1->goals.begin(); it != s1->goals.end(); it++) 
		(*it)->data = 0;// Clear debug info..
	for(std::list<DFA_Trie<Pattern<S>,StatementTree<S> > *>::iterator it = s1->goals.begin(); it != s1->goals.end(); it++) 
		merge(*it, s2->start, s2->goals);
	s1->goals.clear();
	s2->goals.unique();
	s1->goals.assign(s2->goals.begin(), s2->goals.end());
}
template <class S, class S_wrapper>
void mergeAll(StatementTree<S_wrapper> & st)
{
		if(st.isList)
		{
			if(st.children.empty())
				std::cout << "Error: StatementTree::mergeAll - is a list but have no children\n";
			else if(st.children.size() == 1)
			{
				mergeAll<S,S_wrapper>(*st.children.front());
				st.statement = st.children.front()->statement;
			}
			else
			{
				mergeAll<S,S_wrapper>(*st.children.front());
				for(std::list<StatementTree<S_wrapper>*>::iterator it = ++st.children.begin(); it != st.children.end(); it++)
				{
					mergeAll<S,S_wrapper>(**it);
					st.children.front()->statement.str += (*it)->statement.str;
					merge<S,S_wrapper>(&st.children.front()->statement, &(*it)->statement);
				}
				st.statement = st.children.front()->statement;
			}
			st.children.clear();
			st.isList = false;
		}
	}
template <class S, class S_wrapper>
void mergeAllPre(StatementTree<S_wrapper> & st)
{
	if(st.parent)
	{
		if(st.parent->children.size() > 1)
		{
			mergeAll<S,S_wrapper>(*st.parent->children.front());
			std::list<StatementTree<S_wrapper>*>::iterator it = ++st.parent->children.begin();
			while(it != st.parent->children.end() && *it != &st)
			{
				mergeAll<S,S_wrapper>(**it);
				st.parent->children.front()->statement.str += (*it)->statement.str;
				merge<S,S_wrapper>(&st.parent->children.front()->statement, &(*it)->statement);
				st.parent->children.erase(it++);
			}
		}
	}
}


// Handlers
// ----------------------
template <class S, class S_wrapper>
bool handler_character(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA;
	st.statement.str = str.back();
	st.statement.start = new DFA();
	st.statement.goals.push_back(new DFA());
	st.statement.start->add_branch(str.back(), st.statement.goals.front());
	st.statement.goals.front()->data = (Pattern<S>*)1;	// Debug... fix this
	st.isList = false;
	return true;
}
template <class S, class S_wrapper>
bool handler_parentheses(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	mergeAll<S,S_wrapper>(st);
	st.statement.str = "(" + st.statement.str + ")";
	return true;
}
template <class S, class S_wrapper>
bool handler_divider(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA;
	mergeAllPre<S,S_wrapper>(st);
	mergeAll<S,S_wrapper>(st);
	st.statement.str = st.firstPre()->statement.str + "|" + st.statement.str;

	merge(st.statement.start, st.firstPre()->statement.start, st.statement.goals);
	for(std::list<DFA *>::iterator it = st.firstPre()->statement.goals.begin(); it != st.firstPre()->statement.goals.end(); it++)
		st.statement.goals.push_back(*it);
	st.statement.goals.unique();

	st.removeFirstPre();
	return true;
}
template <class S, class S_wrapper>
bool handler_kleinClosure(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA;
	mergeAll<S,S_wrapper>(*st.firstPre());
	st.statement = st.firstPre()->statement;
	st.statement.str += "*";
	std::list<DFA *> goals(st.statement.goals);
	//for(std::list<Trie*>::iterator it = st.statement.goals.begin(); it != st.statement.goals.end(); it++)
	//	merge(st.statement.start,*it, goals);
	for(std::list<DFA*>::iterator goal = st.statement.goals.begin(); goal != st.statement.goals.end(); goal++)
	{
		if(st.statement.start->isFinished() && st.statement.start->data != (*goal)->data)
			std::cout << "kleinClosure: inconsistent pattern; different goal handlers, cannot merge\n";
		st.statement.start->data = (*goal)->data;

		for(std::list<DFA*>::iterator parent = (*goal)->parents.begin(); parent != (*goal)->parents.end(); parent++)
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

// Debug
void test_RegularExpressionParser();

#endif