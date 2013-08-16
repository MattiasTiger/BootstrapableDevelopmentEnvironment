
#include <list>
#include <string>
#include <iostream>

#include "Type.h"

#ifndef _LOGICBLOCK_H_
#define _LOGICBLOCK_H_

class LogicBlock
{
public:
	std::string str;	//Debugging
	std::string name;
	char * value;
	Type * type;

	std::vector<LogicBlock *> parents;
	std::vector<LogicBlock *> children;

public:
	LogicBlock() { name = "undefined"; type = 0; value = 0; }
	~LogicBlock() {}
	virtual void eval() = 0;
};


// Logic blocks
// -------------

/* if(child1) child2 else child3 */
class LogicBlock_if : public LogicBlock
{
public:
	LogicBlock_if() { name = "if"; type = 0; value = 0; }	// Should be of type VOID
	void eval() {
		children.front()->eval();
		if(*children.front()->value != 0)
			(*++children.begin())->eval();
		else
			children.back()->eval();
	}
};

/* while(child0) child1 */
class LogicBlock_while : public LogicBlock
{
public:
	void eval() {
		children[0]->eval();
		while(children[0]->value != 0)
		{
			children[1]->eval();
			children[0]->eval();
		}
	}
};

/* child0(child1, child2, ...) */
class LogicBlock_functionCall : public LogicBlock
{
public:
	std::vector<char*> arguments;
	std::vector<Member_header> argumentTypes;
public:
	void eval() {
		for(unsigned int n = 0; n < arguments.size(); n++)
		{
			// Allocate arguments?
			memcpy(arguments[n], children[n+1], argumentTypes[n].type->size);
		}
		children[0]->eval();
	}
};

/* const/varible etc */
class LogicBlock_data : public LogicBlock
{
public:
	LogicBlock_data() { name = "data"; type = 0; value = 0; }
	void eval() {}
};


/* child1 == child2 */
class LogicBlock_equal : public LogicBlock
{
public:
	LogicBlock_equal() { name = "equal"; type = 0; value = 0; }	// Should be of type VOID
	void eval() {
		children[0]->eval();
		children[1]->eval();
		*value = memcmp(children[0]->value, children[1]->value, children[0]->type->size) == 0;
		// Should be replaced by memcmp (or == function..)
	}
};

/* child0 = child1 */
class LogicBlock_store : public LogicBlock
{
public:
	LogicBlock_store() { name = "store"; type = 0; value = 0; }
	void eval() {
		children[1]->eval();
		//*children.front()->value = *children.back()->value;	 // Should be replaced by memcopy of the size specified by the TYPE (or = function..)
		memcpy(children[0]->value, children[1]->value, children[0]->type->size);
	}
};

/* child0 + child1 */
class LogicBlock_add_int : public LogicBlock
{
public:
	LogicBlock_add_int() { name = "add_int"; type = 0; value = 0; }	// Should be of type VOID
	void eval() {
		children[0]->eval();
		children[1]->eval();
		*(int*)value = *(int*)children[0]->value + *(int*)children[1]->value;
	}
};

/*{child1; child2; ...} */
class LogicBlock_multipleStatement : public LogicBlock
{
public:
	LogicBlock_multipleStatement() { name = "multipleStatements"; type = 0; value = 0; }	// Should be of type VOID
	void eval() {
		for(std::vector<LogicBlock*>::iterator child = children.begin(); child != children.end(); child++)
			(*child)->eval();
	}
};

/* cout << child0 */
class LogicBlock_print : public LogicBlock
{
public:
	LogicBlock_print() { name = "print"; type = 0; value = 0; }	// Should be of type VOID
	void eval() {
		children[0]->eval();
		if(children[0]->type->name == "int")
			std::cout << *(int *)children[0]->value;
		else
			std::cout << children[0]->value;
	}
};

#endif