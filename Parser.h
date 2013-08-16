/* Parser.h
 * 
 * Description:
 *	Contains the parser core and auxilirary functionality.
 * 
 * 
 * Author: Mattias Tiger, 2013
 */

#include <string>
#include <list>
#include <vector>
#include <stack>
#include <iostream>

#include "DFA_Trie.h"

#ifndef _PARSER_H_
#define _PARSER_H_

///////////////////////


template <class S> class Pattern;
template <class S> class StatementTree;

template <class S>
class StatementTree
{
public:
	bool isList;
	StatementTree<S> * parent;
	std::list<StatementTree<S>*> children;
	S statement;

public:
	StatementTree() { parent = this; isList = true; }
	StatementTree(StatementTree * parent) : parent(parent) { isList = true; }
	StatementTree(S statement) : statement(statement) { isList = false; }

	StatementTree * newGroup() {
		children.push_back(new StatementTree(this));
		return children.back();
	}
	StatementTree *  removeGroup() {
		return parent;
	}

	int preAmount() { return parent->children.size()-1; }
	int argAmount() { return children.size(); }
	StatementTree * firstPre()  { return *++parent->children.rbegin(); }
	StatementTree * firstPost() { return *children.begin(); }
	void add(StatementTree * child)
						   { child->parent = this; children.push_back(child); }
	void removeChildren()  { children.clear(); }	// Memory leak
	void removeFirstPre()  { parent->children.erase((++++parent->children.rbegin()).base()); }
	void removeFirstPost() { children.erase(children.begin()); }
	void removeRestPost()  { std::list<StatementTree*>::iterator i = ++children.begin(); while(i != children.end()) children.erase(i++); }

	void mergeAll()	{
		if(isList)
		{
			if(children.empty())
				std::cout << "Error: StatementTree::mergeAll - is a list but have no children\n";
			else if(children.size() == 1)
			{
				children.front()->mergeAll();
				statement = children.front()->statement;
			}
			else
			{
				children.front()->mergeAll();
				for(std::list<StatementTree*>::iterator it = ++children.begin(); it != children.end(); it++)
				{
					(*it)->mergeAll();
					children.front()->statement.str += (*it)->statement.str;
					merge(&children.front()->statement, &(*it)->statement);
				}
				statement = children.front()->statement;
			}
			children.clear();
			isList = false;
		}
	}
	void mergeAllPre() {
		if(parent)
		{
			if(parent->children.size() > 1)
			{
				parent->children.front()->mergeAll();
				std::list<StatementTree*>::iterator it = ++parent->children.begin();
				while(it != parent->children.end() && *it != this)
				{
					(*it)->mergeAll();
					parent->children.front()->statement.str += (*it)->statement.str;
					merge(&parent->children.front()->statement, &(*it)->statement);
					parent->children.erase(it++);
				}
			}
		}
	}
};

template <class S>
class ParentContainer
{
public:
	DFA_Trie<Pattern<S>,StatementTree<S> > * pattern;
	StatementTree<S> * group;
	std::string string;
	ParentContainer(DFA_Trie<Pattern<S>,StatementTree<S> > * p, StatementTree<S> * g, std::string str) : pattern(p),group(g),string(str) {}
};

///////////////////////

template <class S>
class Pattern
{
public:
	std::string pattern;
	std::string separators;		// e.g. ' ', '\n', '\t'
	bool (*handler) (StatementTree<S> & st, Pattern & p, std::string & str);
	StatementTree<S> * arguments;
	bool terminate;

public:
	Pattern(std::string pattern, bool (*handler) (StatementTree<S> & st, Pattern & p, std::string & str), bool terminate = false): pattern(pattern), handler(handler) { arguments = 0; this->terminate = terminate; }
};

////////////////////////

// S - Parsed statement
template <class S>
class Parser
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA_Trie_S;
public:
	StatementTree<S> statementTree;
	DFA_Trie_S patternTrie;

public:
	Parser() {}
	~Parser() {}
	void parse(std::string str)	{
		unsigned int n = 0;	// token counter
		std::stack<ParentContainer<S> > parentPatterns;
		DFA_Trie_S * pattern = &patternTrie;
		StatementTree<S> * group = &statementTree;
		group = group->newGroup();
		std::string string("");
		unsigned char next;
		
		while(n <= str.length())
		{
			next = str[n];
			std::cout << "> '" << next << "'\n";
			if(pattern->isConsistent(next))	// Atleast one pattern is consistent with the current token
			{
				string += next;
				pattern = pattern->branch[next];
				n++;
			}
			else
			if(pattern->isFinished())			// Atleast one pattern is finished
			{
				pattern->executeHandler(group, string);
				group = group->removeGroup();
				group = group->newGroup();
				string = "";
				pattern = &patternTrie;
			}
			else
			if(pattern->hasSubPattern())		// Atleast one pattern is consistent with a sub pattern
			{
				parentPatterns.push(ParentContainer<S>(pattern->getReturnPoint(), group, string));
				group = group->newGroup();
				string = "";
				pattern = &patternTrie;
			}
			else								// A pattern has finished with a termination symbol or 
			{
				if(!parentPatterns.empty())		// A parent pattern exists
				{
					group->parent->children.remove(group);
					pattern = parentPatterns.top().pattern;
					group	= parentPatterns.top().group;
					string	= parentPatterns.top().string;
					parentPatterns.pop();
				}else
				if(n == str.length())
					n++;
				else
				{
					std::cout << "! token #" << n << " ('" << next << "') does not match anything!\n";
					n++;
				}
			}
		}
	}

	void addPattern(Pattern<S> p) {
		patternTrie.add(p.pattern, new Pattern<S>(p), p.handler);
	}
	void addPattern(DFA_Trie_S * p) {
		patternTrie.add(p);
	}
};

// merge s1 and s2 into s1
template <class S>
void merge(S * s1, S * s2)
{
	for(std::list<DFA_Trie<Pattern<S>,StatementTree<S> > *>::iterator it = s1->goals.begin(); it != s1->goals.end(); it++) 
		(*it)->data = 0;// Clear debug info..
	for(std::list<DFA_Trie<Pattern<S>,StatementTree<S> > *>::iterator it = s1->goals.begin(); it != s1->goals.end(); it++) 
		merge(*it, s2->start, s2->goals);
	s1->goals.clear();
	/*
	for(std::list<Trie*>::iterator it = s2->goals.begin(); it != s2->goals.end(); it++)
		if(!(*it)->parents.empty())
			s1->goals.push_back(*it);
	*/
	s2->goals.unique();
	s1->goals.assign(s2->goals.begin(), s2->goals.end());
	//s2->start = s1->start;
	//s->goals.assign(s2->goals.begin(),s2->goals.end());
}



#endif // _PARSER_H_
