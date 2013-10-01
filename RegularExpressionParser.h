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
    DFA<Pattern<Statement_regexp>,StatementTree<Statement_regexp> > dfa;
	//DFA_Trie_regexp * start;
	//std::list<DFA_Trie_regexp *> goals;
public:
	std::string str;
	Statement_regexp() { dfa.start = 0; }
	Statement_regexp(std::string str) : str(str) {}
};

// For S other than Statement_regexp
template <class S>
class Statement_regexp_
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > dfaTrie;
public:
    DFA<Pattern<S>,StatementTree<S> > dfa;
	//dfaTrie * start;
	//std::list<dfaTrie *> goals;
public:
	std::string str;
	Statement_regexp_() { dfa.start = 0; }
	Statement_regexp_(std::string str) : str(str) {}
};

template <class S, class S_wrapper>
class RegularExpressionParser : public Parser<S_wrapper>
{
	typedef Pattern<S_wrapper> Pattern_regexp;
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > dfaTrie;
public:
	RegularExpressionParser(void) {}
	~RegularExpressionParser(void) {}

	
	S_wrapper * parse(std::string str) {
		return Parser::parse(str);
	}
	DFA<Pattern<S>,StatementTree<S> > * parse(std::string str, Pattern<S> * p) {
		parse(str);
		StatementTree<S_wrapper> st = statementTree.children.front()->statement;
		statementTree.parent->children.erase((++statementTree.parent->children.rbegin()).base());

		for(std::list<StatementTree<S_wrapper>*>::iterator i = ++statementTree.children.begin(); i != statementTree.children.end(); i++)
		{
			st.statement.str += (*i)->statement.str;
			merge<S,S_wrapper>(&st.statement, &(*i)->statement);
		}
		for(std::list<dfaTrie *>::iterator it = st.statement.dfa.goals.begin(); it != st.statement.dfa.goals.end(); it++) 
		{
			(*it)->data = p;
			if(p)
				(*it)->handler = p->handler;
		}
		statementTree.clear();
		return new DFA<Pattern<S>,StatementTree<S> >(st.statement.dfa);
	}

	void init()
	{
        ParserHandler<S_wrapper> * h_character = new handler_character<S,S_wrapper>;
        // Regular expression base rules
        addPattern("($)", new handler_parentheses<S,S_wrapper>);
        addPattern("$|$", new handler_divider<S,S_wrapper>);
        addPattern("$*", new handler_kleinClosure<S,S_wrapper>);

        // Allow for the usage of the special characters
		addPattern("\\\\", h_character);
		addPattern("\\(", h_character);
		addPattern("\\)", h_character);
		addPattern("\\|", h_character);
		addPattern("\\*", h_character);
		addPattern("\\$", h_character);
		
        //setErrorHandler(h_character); // Does not work as intended. Cannot easily replace the code below.. ("a|b": captures "a" but "|" finishes before capturing "b"...)
        
		std::string c;
		for(int n = 0; n < 256; n++)
		{
			if(n != '$' && n != '(' && n != '*' && n != '|')
			{
				c = char(n);
				addPattern(c, h_character);
			}
		}
	}
};


// merge s1 and s2 into s1
template <class S, class S_wrapper>
void merge(S_wrapper * s1, S_wrapper * s2)
{
	for(std::list<DFA_Trie<Pattern<S>,StatementTree<S> > *>::iterator it = s1->dfa.goals.begin(); it != s1->dfa.goals.end(); it++) 
		(*it)->data = 0;// Clear debug info..
	for(std::list<DFA_Trie<Pattern<S>,StatementTree<S> > *>::iterator it = s1->dfa.goals.begin(); it != s1->dfa.goals.end(); it++) 
		merge(*it, s2->dfa.start, s2->dfa.goals);
	s1->dfa.goals.clear();
	s2->dfa.goals.unique();
	s1->dfa.goals.assign(s2->dfa.goals.begin(), s2->dfa.goals.end());
}
template <class S, class S_wrapper>
void mergeAll(StatementTree<S_wrapper> & st)
{
		if(st.isList)
		{
			//if(st.children.empty())
			//	std::cout << "Error: StatementTree::mergeAll - is a list but have no children\n";
			if(st.children.size() == 1)
			{
				mergeAll<S,S_wrapper>(*st.children.front());
				st.statement = st.children.front()->statement;
			}
			else if(st.children.size() > 1)
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
class handler_character : public ParserHandler<S_wrapper>
{
bool operator()(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > dfaTrie;
	st.statement.str = str.back();
	st.statement.dfa.start = new dfaTrie();
	st.statement.dfa.goals.push_back(new dfaTrie());
	st.statement.dfa.start->add_branch(str.back(), st.statement.dfa.goals.front());
	st.statement.dfa.goals.front()->data = (Pattern<S>*)1;	// Debug... fix this
	st.isList = false;
	return true;
}
};
template <class S, class S_wrapper>
class handler_parentheses : public ParserHandler<S_wrapper>
{
bool operator()(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	mergeAll<S,S_wrapper>(st);
	st.statement.str = "(" + st.statement.str + ")";
	return true;
}
};
template <class S, class S_wrapper>
class handler_divider : public ParserHandler<S_wrapper>
{
bool operator()(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > dfaTrie;
	mergeAllPre<S,S_wrapper>(st);
	mergeAll<S,S_wrapper>(st);
	st.statement.str = st.firstPre()->statement.str + "|" + st.statement.str;

	merge(st.statement.dfa.start, st.firstPre()->statement.dfa.start, st.statement.dfa.goals);
	for(std::list<dfaTrie *>::iterator it = st.firstPre()->statement.dfa.goals.begin(); it != st.firstPre()->statement.dfa.goals.end(); it++)
		st.statement.dfa.goals.push_back(*it);
	st.statement.dfa.goals.unique();

	st.removeFirstPre();
	return true;
}
};
template <class S, class S_wrapper>
class handler_kleinClosure : public ParserHandler<S_wrapper>
{
bool operator()(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > dfaTrie;
	mergeAll<S,S_wrapper>(*st.firstPre());
	st.statement = st.firstPre()->statement;
	st.statement.str += "*";
	std::list<dfaTrie *> goals(st.statement.dfa.goals);
	//for(std::list<Trie*>::iterator it = st.statement.dfa.goals.begin(); it != st.statement.dfa.goals.end(); it++)
	//	merge(st.statement.dfa.start,*it, goals);
	for(std::list<dfaTrie*>::iterator goal = st.statement.dfa.goals.begin(); goal != st.statement.dfa.goals.end(); goal++)
	{
		if(st.statement.dfa.start->isFinished() && st.statement.dfa.start->data != (*goal)->data)
			std::cout << "kleinClosure: inconsistent pattern; different goal handlers, cannot merge\n";
		st.statement.dfa.start->data = (*goal)->data;

		for(std::list<dfaTrie*>::iterator parent = (*goal)->parents.begin(); parent != (*goal)->parents.end(); parent++)
			for(int b = 0; b < 256; b++)
					if((*parent)->branch[b] == *goal)
					{
						//(*parent)->add_branch(b, st.statement.dfa.start);
						(*parent)->branch[b] = st.statement.dfa.start;
					}
	}
	st.statement.dfa.goals.clear();
	st.statement.dfa.goals.push_back(st.statement.dfa.start);
	st.removeFirstPre();
	st.isList = false;
	return true;
}
};


///////////////////////////



// dfaTrie injection test
template <class S, class S_wrapper>
class handler_injectDFA : public ParserHandler<S_wrapper>
{
private:
    DFA<Pattern<S>,StatementTree<S> > * dfa;
public:
    handler_injectDFA(DFA<Pattern<S>,StatementTree<S> > * dfa) : dfa(dfa) {}
    bool operator()(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
    {
        //std::cout << "handler_injectDFA\n";

        st.statement.dfa.start = new DFA_Trie<Pattern<S>,StatementTree<S> >;
        deepCopy(dfa->start, st.statement.dfa.start, st.statement.dfa.goals);
        st.isList = false;
        return true;
    }
};
template <class S>
class handler_injectStatement : public ParserHandler<S>
{
private:
    S * statement;
public:
    handler_injectStatement(S * statement) : statement(statement) {}
    bool operator()(StatementTree<S> & st, Pattern<S> & p, std::string & str)
    {
        std::cout << "handler_injectStatement\n";
        st.statement = *statement;
        st.isList = false;
        return true;
    }
};





////////////////////////

#include "ParserExtended.h"

// Regexp
template <class S>
class h_regexp_parentheses : public ParserHandler<ParserStatement<S> >
{
bool operator()(StatementTree<ParserStatement<S> > & st, Pattern<ParserStatement<S> > & p, std::string & str)
{
	mergeAll<S,ParserStatement<S> >(st);
	st.statement.str = "(" + st.statement.str + ")";
	return true;
}
};
template <class S>
class h_regexp_divider : public ParserHandler<ParserStatement<S> >
{
bool operator()(StatementTree<ParserStatement<S> > & st, Pattern<ParserStatement<S> > & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA_Trie_S;
	mergeAllPre<S,ParserStatement<S> >(st);
	mergeAll<S,ParserStatement<S> >(st);
	st.statement.str = st.firstPre()->statement.str + "|" + st.statement.str;

	merge(st.statement.dfa.start, st.firstPre()->statement.dfa.start, st.statement.dfa.goals);
	for(std::list<DFA_Trie_S *>::iterator it = st.firstPre()->statement.dfa.goals.begin(); it != st.firstPre()->statement.dfa.goals.end(); it++)
		st.statement.dfa.goals.push_back(*it);
	st.statement.dfa.goals.unique();

	st.removeFirstPre();
	return true;
}
};
template <class S>
class h_regexp_kleinClosure : public ParserHandler<ParserStatement<S> >
{
bool operator()(StatementTree<ParserStatement<S> > & st, Pattern<ParserStatement<S> > & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA_Trie_S;
	mergeAll<S,ParserStatement<S> >(*st.firstPre());
	st.statement = st.firstPre()->statement;
	st.statement.str += "*";
	std::list<DFA_Trie_S *> goals(st.statement.dfa.goals);
	//for(std::list<Trie*>::iterator it = st.statement.dfa.goals.begin(); it != st.statement.dfa.goals.end(); it++)
	//	merge(st.statement.dfa.start,*it, goals);
	for(std::list<DFA_Trie_S*>::iterator goal = st.statement.dfa.goals.begin(); goal != st.statement.dfa.goals.end(); goal++)
	{
		if(st.statement.dfa.start->isFinished() && st.statement.dfa.start->data != (*goal)->data)
			std::cout << "kleinClosure: inconsistent pattern; different goal handlers, cannot merge\n";
		st.statement.dfa.start->data = (*goal)->data;

		for(std::list<DFA_Trie_S*>::iterator parent = (*goal)->parents.begin(); parent != (*goal)->parents.end(); parent++)
			for(int b = 0; b < 256; b++)
					if((*parent)->branch[b] == *goal)
					{
						//(*parent)->add_branch(b, st.statement.dfa.start);
						(*parent)->branch[b] = st.statement.dfa.start;
					}
	}
	st.statement.dfa.goals.clear();
	st.statement.dfa.goals.push_back(st.statement.dfa.start);
	st.removeFirstPre();
	st.isList = false;
	return true;
}
};


template<typename S>
class h_regexp_concat : public ParserHandler<ParserStatement<S> >
{
bool operator()(StatementTree<ParserStatement<S> > & st, Pattern<ParserStatement<S> > & p, std::string & str)
{
    st.children.push_front(st.firstPre());
    st.removeFirstPre();
	mergeAll<S,ParserStatement<S> >(st);
	return true;
}
};


//////////////////
std::string getExtendedAsciiCharacterPattern(std::string except = "");

#endif