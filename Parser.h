#include <string>
#include <list>
#include <vector>
#include <stack>
#include <iostream>

#ifndef _PARSER_H_
#define _PARSER_H_

///////////////////////

class Statement
{
public:
	std::string str;
	Statement() {}
	Statement(std::string str) : str(str) {}
};

class StatementTree
{
public:
	bool isList;
	StatementTree * parent;
	std::list<StatementTree*> children;
	Statement statement;

public:
	StatementTree() { parent = this; isList = true; }
	StatementTree(StatementTree * parent) : parent(parent) { isList = true; }
	StatementTree(Statement statement) : statement(statement) { isList = false; }

	StatementTree * newGroup();
	StatementTree *  removeGroup();
};

class Trie;
class ParentContainer
{
public:
	Trie * pattern;
	StatementTree * group;
	ParentContainer(Trie * p, StatementTree * g) : pattern(p),group(g) {}
};

///////////////////////

class Pattern
{
public:
	std::string pattern;
	std::string separators;		// e.g. ' ', '\n', '\t'
	bool (*handler) (StatementTree & st, Pattern & p);
	StatementTree * arguments;

public:
	Pattern(std::string pattern, bool (*handler) (StatementTree & st, Pattern & p)): pattern(pattern), handler(handler) { arguments = 0; }
};

class Trie
{
public:
	Trie * branch[256];
	Pattern * p;
	bool requireChild;

public:
	Trie() { memset(branch, 0, sizeof(branch)); p = 0; requireChild = false; }
	bool add(std::string & pattern, Pattern * p, int index = 0);

	bool isConsistent(char c) { return branch[c] != 0; }
	bool isFinished() { return p != 0; }
	bool hasSubPattern() { return branch['$'] != 0; }
	void executeHandler(StatementTree * group) { p->handler(*group, *p); } // dfggdfss fgdgsdfsg dsg df
	ParentContainer getReturnPoint(StatementTree * group) { return ParentContainer(branch['$'], group); }
};

////////////////////////

class Parser
{
public:
	StatementTree statementTree;
	Trie patternTrie;

public:
	Parser(void);
	~Parser(void);
	void parse(std::string str);

	void addPattern(Pattern p);
};


/*
class Statement
{
public:
	std::string name;
	std::list<Statement> children;
public:
	Statement(std::string name) : name(name) {}
};

class Operator
{
public:
	std::string name;
	int childAmount;
public:
	Operator(std::string name, int childAmount) : name(name),childAmount(childAmount) {}
};

class Trie
{
public:
	Trie * branch[256];
	Operator * op;

public:
	Trie() { memset(branch, 0, sizeof(branch)); op = 0; }
	bool add(std::string & pattern, Operator * op, int index = 0);
	bool exist(std::string & pattern, int index = 0);
};


class Parser
{
public:
	Trie patterns;
	std::list<std::list<Statement> > programTree;

public:
	Parser(void);
	~Parser(void);
	void parse(std::string str);
};

*/

#endif // _PARSER_H_

