/* Parser.h
 * 
 * Description:
 *    Contains the parser core and auxilirary functionality.
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

///////////////////////
template <class S>
class ParserHandler : public DFA_handler<Pattern<S>, StatementTree<S> > {};
//////////////////////


template <class S>
class StatementTree
{
public:
    bool isList;
    StatementTree * parent;
    std::list<StatementTree*> children;
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
    void removeChildren()  { children.clear(); }    // Memory leak
    void removeFirstPre()  { parent->children.erase((++++parent->children.rbegin()).base()); }
    void removeFirstPost() { children.erase(children.begin()); }
    void removeRestPost()  { std::list<StatementTree*>::iterator i = ++children.begin(); while(i != children.end()) children.erase(i++); }
    void removeThis() { parent->children.erase((++parent->children.rbegin()).base()); }

    void clear() { isList = true; parent = this; children.clear(); statement = S(); }    // Memory leak
};

template <class S>
class ParentContainer
{
public:
    ParentContainer(DFA_Trie<Pattern<S>,StatementTree<S> > * p, StatementTree<S> * g, std::string str) : pattern(p),group(g),string(str) {}
public:
    DFA_Trie<Pattern<S>,StatementTree<S> > * pattern;
    StatementTree<S> * group;
    std::string string;
};

template <class S>
class Pattern
{
public:
    Pattern(std::string pattern, ParserHandler<S> * handler): pattern(pattern), handler(handler) { arguments = 0; }
public:
    std::string pattern;
    ParserHandler<S> * handler;
    StatementTree<S> * arguments;
};

template <class S>
class HandlerFunctionContainer : public ParserHandler<S>
{
public:
    HandlerFunctionContainer(bool (*handler)(StatementTree<S> & st, Pattern<S> & p, std::string & str)) : handler(handler) {}
    bool operator()(StatementTree<S> & st, Pattern<S> & p, std::string & str)
    {
        return handler(st,p,str);
    }
public:
    bool (*handler)(StatementTree<S> & st, Pattern<S> & p, std::string & str);
};


/* Parser DECLARATION **********************************************************/
/*******************************************************************************/
// S - Parsed statement
template <class S>
class Parser
{
public:
    typedef DFA_Trie<Pattern<S>,StatementTree<S> >  DFA_Trie_S;
    typedef DFA<Pattern<S>,StatementTree<S> >       DFA_S;
public:
    Parser() { handler_enterErrorState = 0; errorPattern = new Pattern<S>("",0); }
    ~Parser() {}
    void setErrorHandler(ParserHandler<S> * errorHandler);
    void setErrorHandler(bool (*handlerFunction)(StatementTree<S> & st, Pattern<S> & p, std::string & str));
    void addPattern(std::string pattern, ParserHandler<S> * handler);
    void addPattern(DFA_Trie_S * p);
    void addPattern(DFA_S * p);
    void addReplacePattern(DFA_S * p);
    void addReplacePattern(std::string pattern, ParserHandler<S> * handler);
    S * parse(std::string str);
public:
    StatementTree<S>    statementTree;  // The statement "stack"
    DFA_Trie_S          patternTrie;    // The pattern Graph

    // Error management
    DFA_Trie_S errorState;
    ParserHandler<S> * handler_enterErrorState;
    Pattern<S> * errorPattern;

};

/* Parser DEFINITION ***********************************************************/
/*******************************************************************************/

/* Name:            Parser::parse(std::string str)
 *  
 *  Description:    Parses a string, producing a final statement or returning 0.
 *  Return:         The pointer to the statement <S> of the first statementTree
 *                  child (The final product) or 0 if failure.
 **********************************/
template <class S>
S * Parser<S>::parse(std::string str)
{
    unsigned int n = 0; // token counter
    unsigned char next; // next token
    statementTree.clear();
    std::stack<ParentContainer<S> > parentPatterns;
    DFA_Trie_S * pattern = &patternTrie;
    StatementTree<S> * group = &statementTree;
    group = group->newGroup();
    std::string string("");
    
    while(n <= str.length())
    {
        next = str[n];
        //std::cout << "> '" << next << "'\n";

        if(pattern->isConsistent(next) && n < str.length()) // Atleast one pattern is consistent with the current token
        {
            string += next;
            pattern = pattern->branch[next];
            n++;
        }
        else
        if(pattern->isFinished() && !(string == "" && pattern == &patternTrie))           // Atleast one pattern is finished
        {
            pattern->executeHandler(group, string);

            if(n < str.length())
            {
                if(patternTrie.branch[SUBPATTERN_SYMBOL] && (patternTrie.branch[SUBPATTERN_SYMBOL]->isConsistent(next) || patternTrie.branch[SUBPATTERN_SYMBOL]->branch[SUBPATTERN_SYMBOL]))  // Can the pattern be continued?
                {
                    group = group->removeGroup();
                    group = group->newGroup();
                    string = "";
                    pattern = patternTrie.branch[SUBPATTERN_SYMBOL];
                }
                else if(!parentPatterns.empty() && (parentPatterns.top().pattern->isConsistent(next) || parentPatterns.top().pattern->isFinished() || parentPatterns.top().pattern->branch[SUBPATTERN_SYMBOL]))    // Should a parent continue?
                {
                    //group->parent->children.remove(group);
                    pattern = parentPatterns.top().pattern;
                    group   = parentPatterns.top().group;
                    string  = parentPatterns.top().string;
                    parentPatterns.pop();
                }
                else
                {
                    // Should enter the error state...
                    std::cout << "Parser::Parse) Possible parsing problem, token #" << n << " ('" << next << "') should probably produce an error\n";
                    group = group->removeGroup();
                    group = group->newGroup();
                    string = "";
                    pattern = &patternTrie;
                }
            }
            else
            {
                group = group->removeGroup();
                group = group->newGroup();
                string = "";
                pattern = &patternTrie;
            }

        }
        else
        if(pattern->hasSubPattern() && pattern != &patternTrie && patternTrie.isConsistent(next))        // Atleast one pattern is consistent with a sub pattern
        {
            parentPatterns.push(ParentContainer<S>(pattern->getReturnPoint(), group, string));
            group = group->newGroup();
            string = "";
            pattern = &patternTrie;
        }
        else
        if(!parentPatterns.empty() && (parentPatterns.top().pattern->isConsistent(next) || parentPatterns.top().pattern->isFinished() || parentPatterns.top().pattern->branch[SUBPATTERN_SYMBOL]))
        {
            group->parent->children.remove(group);
            pattern = parentPatterns.top().pattern;
            group   = parentPatterns.top().group;
            string  = parentPatterns.top().string;
            parentPatterns.pop();
        }
        else
        {
            if(n == str.length())
                n++;    // Terminate
            else
            {
                // Error handling
                if(handler_enterErrorState)
                {
                    string += next;
                    if((*handler_enterErrorState)(*group, *errorPattern, string))
                    {
                        group = group->removeGroup();
                        group = group->newGroup();
                        pattern = &patternTrie;
                    }
                    else
                    {
                        std::cout << "! token #" << n << " ('" << next << "') does not match anything!\n";
                        pattern = &errorState;  // Is freed when the top parent pattern is consistent with next token or if any alternative route is added (the later is not yet implemented)
                    }
                    string = "";
                }
                else
                {
                    std::cout << "! token #" << n << " ('" << next << "') does not match anything!\n";
                    pattern = &errorState;  // Is freed when the top parent pattern is consistent with next token or if any alternative route is added (the later is not yet implemented)
                }
                n++;
            }
        }
    }
    if(!string.empty())
        std::cout << "!! Failed to parse \"" << string << "\"!!\n"; 
    if(!parentPatterns.empty())
        std::cout << "!! Failed to finish " << parentPatterns.size() << " stacked patterns !!\n";
    if(statementTree.children.empty())
        return 0;
    return &statementTree.children.front()->statement;
}

template <class S>
void Parser<S>::setErrorHandler(ParserHandler<S> * errorHandler)
{
    handler_enterErrorState = errorHandler;
}

template <class S>
void Parser<S>::setErrorHandler(bool (*handlerFunction)(StatementTree<S> & st, Pattern<S> & p, std::string & str))
{
    setErrorHandler(new HandlerFunctionContainer<S>(handlerFunction));
}

template <class S>
void Parser<S>::addPattern(std::string pattern, ParserHandler<S> * handler) 
{
    patternTrie.add(pattern, new Pattern<S>(pattern, handler), handler);
}

template <class S>
void Parser<S>::addReplacePattern(std::string pattern, ParserHandler<S> * handler) 
{
    patternTrie.addReplace(pattern, new Pattern<S>(pattern, handler), handler);
}

template <class S>
void Parser<S>::addPattern(DFA_Trie_S * p) 
{
    patternTrie.add(p);
}

template <class S>
void Parser<S>::addPattern(DFA_S * p) 
{
    patternTrie.add(p->start);
}

template <class S>
void Parser<S>::addReplacePattern(DFA_S * p) 
{
    patternTrie.addReplace(p->start);
}


#endif // _PARSER_H_
