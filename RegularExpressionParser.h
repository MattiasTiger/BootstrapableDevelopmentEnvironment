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

class Statement_regexp;
typedef DFA_Trie<Pattern<Statement_regexp>,StatementTree<Statement_regexp> > DFA_Trie_regexp;

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

typedef StatementTree<Statement_regexp> StatementTree_regexp;
typedef Pattern<Statement_regexp> Pattern_regexp;

class RegularExpressionParser : public Parser<Statement_regexp>
{
public:
	RegularExpressionParser(void);
	~RegularExpressionParser(void);

	void init();
};

bool handler_character(StatementTree_regexp & st, Pattern_regexp & p, std::string & str);
bool handler_parentheses(StatementTree_regexp & st, Pattern_regexp & p, std::string & str);
bool handler_divider(StatementTree_regexp & st, Pattern_regexp & p, std::string & str);
bool handler_kleinClosure(StatementTree_regexp & st, Pattern_regexp & p, std::string & str);

// Debug
void test_RegularExpressionParser();
