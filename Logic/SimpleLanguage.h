
#include "LogicBlock.h"
#include "../Parser.h"
#include "../RegularExpressionParser.h"

#ifndef _SIMPLE_LANGUAGE_H
#define _SIMPLE_LANGUAGE_H_

void simple_language_test();


typedef StatementTree<LogicBlock*> StatementTree_logic;
typedef Pattern<LogicBlock*> Pattern_logic;

bool handler_constant(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_variable(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_variableDeclaration(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_if(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_while(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_equal(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_add(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_store(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_print(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_functionCall(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_functionDeclaration(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_multipleStatements(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_paranthesis(StatementTree_logic & st, Pattern_logic & p, std::string & str);
bool handler_statementSeparation(StatementTree_logic & st, Pattern_logic & p, std::string & str);


void trim(std::string & str, std::string discard);
void split(std::string & str, char splitter, std::list<std::string> & result);


#endif