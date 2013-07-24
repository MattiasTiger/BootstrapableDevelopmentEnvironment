#include "Parser.h"

StatementTree * StatementTree::newGroup()
{
	children.push_back(new StatementTree(this));
	return children.back();
}
StatementTree * StatementTree::removeGroup()
{
	return parent;
}

Parser::Parser(void)
{

}


Parser::~Parser(void)
{
}


void Parser::parse(std::string str)
{
	int n = 0;	// token counter
	std::stack<ParentContainer> parentPatterns;
	Trie * pattern = &patternTrie;
	StatementTree * group = &statementTree;
	group = group->newGroup();
		
	while(n <= str.length())
	{
		std::cout << "> '" << str[n] << "'\n";

		if(pattern->isConsistent(str[n]))	// Atleast one pattern is consistent with the current token
		{
			pattern = pattern->branch[str[n]];
			n++;
		}
		else
		if(pattern->isFinished())			// Atleast one pattern is finished
		{
			pattern->executeHandler(group);
			group = group->removeGroup();
			group = group->newGroup();
			pattern = &patternTrie;
		}
		else
		if(pattern->hasSubPattern())		// Atleast one pattern is consistent with a sub pattern
		{
			parentPatterns.push(pattern->getReturnPoint(group));
			group = group->newGroup();
			pattern = &patternTrie;
		}
		else								// A pattern has finished with a termination symbol or 
		{
			if(!parentPatterns.empty())		// A parent pattern exists
			{
				group->parent->children.remove(group);
				pattern = parentPatterns.top().pattern;
				group	= parentPatterns.top().group;
				parentPatterns.pop();
			}else
			if(n == str.length())
				n++;
			else
			{
				std::cout << "! token #" << n << " ('" << str[n] << "') does not match anything!\n";
			}
		}
	}
}

/*
void Parser::parse(std::string str)
{
	int n = 0;	// token counter
	std::stack<Trie*> parentPattern;
	Trie * node = &patternTrie;
	StatementTree * statements = &statementTree;
	
	while(n <= str.length())
	{
		std::cout << "> '" << str[n] << "'\n";
		if(node->branch[str[n]] != 0)	// is there a pattern that continue to match the string?
		{
			node = node->branch[str[n]];
			n++;
		}
		else if(node->p != 0)			// pattern matched?
		{
			node->p->handler(*statements, *node->p);	// apply handler

			if(!statements->isList && statements != &statementTree)
				statements = statements->parent;
			
			node = &patternTrie;
		}
		else if(node->requireChild)		// child branching?
		{
			parentPattern.push(node->branch['$']);
			node = &patternTrie;

			statements->children.push_back(new StatementTree(statements));
			statements = statements->children.back();
		}
		else
		{
			if(!parentPattern.empty()) {
				node = parentPattern.top();
				parentPattern.pop();
			}
			else if(n == str.length())
				n++;
			else
			{
				std::cout << "! token #" << n << " ('" << str[n] << "') does not match anything!\n";
				node = &patternTrie;
			}
		}
	}
}
*/
void Parser::addPattern(Pattern p)
{
	patternTrie.add(p.pattern, new Pattern(p));
}

bool Trie::add(std::string & pattern, Pattern * p, int index)
{

	if(pattern.length() < index)
		return false;

	if(pattern[index] == '$')
	{
		this->requireChild = true;
	}

	if(pattern.length() == index)
		if(this->p == 0)
		{
			this->p = p;
			return true;
		}
	if(branch[pattern[index]] == 0)
		branch[pattern[index]] = new Trie();
	return branch[pattern[index]]->add(pattern, p, index+1);
}


