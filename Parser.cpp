#include "Parser.h"

Parser::Parser(void)
{

}


Parser::~Parser(void)
{
}


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

/*

bool Trie::add(std::string & pattern, Operator * op, int index)
{
	if(pattern.length() == index-1)
		if(this->op == 0)
		{
			this->op = op;
			return true;
		}
		else
			return false;
	if(branch[pattern[index]] == 0)
		branch[pattern[index]] = new Trie();
	return branch[pattern[index]]->add(pattern, op, index+1);
}

bool Trie::exist(std::string & pattern, int index = 0)
{
	if(pattern.length() == index-1)
		return op != 0;
	if(branch[pattern[index]] == 0)
		return false;
	return branch[pattern[index]]->exist(pattern, index+1);
}

Parser::Parser(void)
{
	Operator * constant = new Operator("constant", 1);
	Operator * add		= new Operator("add", 2);
	std::string s1("1"),s2("2"),s3("$+$");

	patterns.add(s1, constant);
	patterns.add(s2, constant);
	patterns.add(s3, add);
}


Parser::~Parser(void)
{
}


void Parser::parse(std::string str)
{
	int n = 0;
	Trie * node = &patterns;
	Trie * newNode = 0;
	while(n < str.length())
	{
		if(node->branch[str[n]] != 0)
		{
			newNode = node->branch[str[n]];
			if(newNode->op != 0)
			{
				if(newNode->op->name == "constant")
				{
					this->programTree.back().push_back(Statement("constant: " + str[n]));
					newNode = node->branch['$'];
				}
				else if(node->op->name == "add")
				{

				}
			}
			node = newNode;
		}
		else
		{

		}
	}
}
*/
