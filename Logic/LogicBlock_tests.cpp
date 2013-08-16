
#include "LogicBlock_tests.h"
#include "LogicBlock.h"
#include <iostream>

void LogicBlock_tests()
{
	DataManager dataManager;

	// Create a char type, 1 byte large
	Type * t_char = new Type("char", 1);
	dataManager.addType(t_char);

	// Create an integer type, 4 bytes large
	Type * t_int = new Type("int", 4);
	dataManager.addType(t_int);

	// Create an double type, 8 bytes large
	Type * t_double = new Type("double", 8);
	dataManager.addType(t_double);

	// Create a struct with a char[10] name, an int for age and an double for length
	Type * t_personStruct = new Type("Person", 0);
	t_personStruct->members.push_back(Member_header("name", dataManager.getType("char"), 0));
	t_personStruct->members.push_back(Member_header("age", dataManager.getType("int"), 0+10*1));
	t_personStruct->members.push_back(Member_header("length", dataManager.getType("double"), 0+10*1+1*4));
	t_personStruct->size = 10 + 4 + 8;
	dataManager.addType(t_personStruct);

	// Instanciate a Person, set its member variables and print it out.
	char name[] = "John";
	int age = 25;
	double length = 1.80;
	Type * Person = dataManager.getType("Person");
	char * person = dataManager.allocate(Person);
	memcpy(person + Person->member("name")->start  , name, 5);
	memcpy(person + Person->member("age")->start   , (char*)&age, 4);
	memcpy(person + Person->member("length")->start, (char*)&length, 8);

	std::cout << "Person:\n" << "  Name: " << &person[0] << "\n  Age: " << *(int*)&person[10] << "\n  Length: " << *(double*)&person[14] << "\n";
}
