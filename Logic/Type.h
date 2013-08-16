
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#ifndef _TYPE_H_
#define _TYPE_H_

class Type;
class Member_header
{
public:
	std::string name;
	Type * type;
	int start;
public:
	Member_header() {}
	Member_header(std::string name, Type * type, int start) : name(name), type(type), start(start) {}
};

class Type
{
public:
	std::string name;
	int size;
	std::vector<Member_header> members;
public:
	Type() {}
	Type(std::string name, int size) : name(name), size(size) {}

	Member_header * member(std::string name) {
		for(std::vector<Member_header>::iterator it = members.begin(); it != members.end(); it++)
		{
			if(it->name == name)
				return &*it;
		}
		return 0;
	}
};

class DataManager
{
public:
	std::map<std::string, Type*> types;		// Type by name
	std::map<char *, Type*> dataType;		// Type by data instance

	std::map<std::string, char *> variables; // Instanciated types by name

public:
	DataManager() {}
	~DataManager() {
		for(std::map<std::string, Type*>::iterator type = types.begin(); type != types.end(); type++)
			delete type->second;
		types.clear();
		for(std::map<char *, Type*>::iterator data = dataType.begin(); data != dataType.end(); data++)
			delete [] data->first;
		types.clear();
	}
	Type * getType(std::string name)  { return types[name]; }
	Type * getVariableType(char * variable)  { return dataType[variable]; }
	void addType(Type * type)		    { types[type->name] = type; }
	void removeType(std::string name) { types.erase(name); }

	char * allocate(Type * type) {
		char * data = new char[type->size];
		memset(data, 0, type->size);
		dataType[data] = type;
		return data;
	}	
	char * allocateArray(Type * type, int size) {
		char * data = new char[type->size*size];
		memset(data, 0, type->size*size);
		dataType[data] = type;
		return data;
	}
	void free(char * data) {
		dataType.erase(data);
		delete [] data;
	}

	void addVariable(std::string name, char * variable) { variables[name] = variable; }
	char * getVariable(std::string name) { return variables[name]; }
};

extern DataManager memoryManager;

#endif