#include "parser.h"
#include "RegularExpressionParser.h"

#ifndef _PARSER_EXTENDED_H_
#define _PARSER_EXTENDED_H_

template <class S>
class ParserStatement
{
    typedef DFA<Pattern<S>,StatementTree<S> > DFA_S;
public:
    DFA_S dfa;
public:
	std::string str;
	ParserStatement() { dfa.start = 0; }
	ParserStatement(std::string str) : str(str) {}
};

template <class S>
class h_regexp_character : public ParserHandler<ParserStatement<S> >
{
bool operator()(StatementTree<ParserStatement<S> > & st, Pattern<ParserStatement<S> > & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA_Trie_S;
    //if(str.length() > 0)
	st.statement.str = str.back();
	st.statement.dfa.start = new DFA_Trie_S();
	st.statement.dfa.goals.push_back(new DFA_Trie_S());
	st.statement.dfa.start->add_branch(str.back(), st.statement.dfa.goals.front());
	st.statement.dfa.goals.front()->data = (Pattern<S>*)1;	// Debug... fix this
	st.isList = false;
	return true;
}
};

template <class S>
class ParserExtended : public Parser<S>
{
public:
    ParserExtended();
    ~ParserExtended();
    void addPattern(std::string pattern, bool (*handlerFunction)(StatementTree<S> & st, Pattern<S> & p, std::string & str));
    void addPattern(std::string pattern, ParserHandler<S> * handler);
    void addReplacePattern(std::string pattern, bool (*handlerFunction)(StatementTree<S> & st, Pattern<S> & p, std::string & str));
    void addReplacePattern(std::string pattern, ParserHandler<S> * handler);
    void addPattern(std::string pattern, std::string preParserPattern);
    void setSubPatternSymbol(char symbol);
    void assignPreParser(Parser<ParserStatement<S> > * preParser);

    void addPattern_(std::string aliasPattern, std::string pattern);
public:
    //RegularExpressionParser<S, ParserStatement<S> > * defaultPatternParser;
    //RegularExpressionParser<S, ParserStatement<S> > * activePatternParser;
    Parser<ParserStatement<S> > * defaultPatternParser;
    Parser<ParserStatement<S> > * activePatternParser;
};

template <class S>
ParserExtended<S>::ParserExtended()
{
    Parser::Parser();

    defaultPatternParser = new Parser<ParserStatement<S> >();  // Default pre-parser
    // Initiate to allow all characters
    std::string c;
	for(int n = 0; n < 256; n++)
	{
        if(n != '$')
        {
		    c = char(n);
            defaultPatternParser->addPattern(c, new h_regexp_character<S>);
        }
	}
    defaultPatternParser->addPattern("\\$", new h_regexp_character<S>);
    defaultPatternParser->addPattern("$$", new h_regexp_concat<S>);

    activePatternParser = defaultPatternParser;
}

template <class S>
ParserExtended<S>::~ParserExtended()
{
}



template <class S>
void ParserExtended<S>::addPattern(std::string pattern, bool (*handlerFunction)(StatementTree<S> & st, Pattern<S> & p, std::string & str))
{
    addPattern(pattern, new HandlerFunctionContainer<S>(handlerFunction));
}
template <class S>
void ParserExtended<S>::addReplacePattern(std::string pattern, bool (*handlerFunction)(StatementTree<S> & st, Pattern<S> & p, std::string & str))
{
    addReplacePattern(pattern, new HandlerFunctionContainer<S>(handlerFunction));
}

template <class S>
void ParserExtended<S>::addPattern(std::string pattern, ParserHandler<S> * handler)
{
    activePatternParser->parse(pattern);
    StatementTree<ParserStatement<S> > * sTree = &activePatternParser->statementTree;
    Pattern<S> * p = new Pattern<S>(pattern, handler);
    StatementTree<ParserStatement<S> > st = sTree->children.front()->statement;
    sTree->parent->children.erase((++sTree->parent->children.rbegin()).base());

    for(std::list<StatementTree<ParserStatement<S> >*>::iterator i = ++sTree->children.begin(); i != sTree->children.end(); i++)
        merge<S,ParserStatement<S> >(&st.statement, &(*i)->statement);
    for(std::list<DFA_Trie_S *>::iterator goal = st.statement.dfa.goals.begin(); goal != st.statement.dfa.goals.end(); goal++) 
    {
        (*goal)->data = p;
        if(p)
            (*goal)->handler = p->handler;
    }
    sTree->clear();
    Parser::addPattern(new DFA<Pattern<S>,StatementTree<S> >(st.statement.dfa));
}

template <class S>
void ParserExtended<S>::addReplacePattern(std::string pattern, ParserHandler<S> * handler)
{
    activePatternParser->parse(pattern);
    StatementTree<ParserStatement<S> > * sTree = &activePatternParser->statementTree;
    Pattern<S> * p = new Pattern<S>(pattern, handler);
    StatementTree<ParserStatement<S> > st = sTree->children.front()->statement;
    sTree->parent->children.erase((++sTree->parent->children.rbegin()).base());

    for(std::list<StatementTree<ParserStatement<S> >*>::iterator i = ++sTree->children.begin(); i != sTree->children.end(); i++)
        merge<S,ParserStatement<S> >(&st.statement, &(*i)->statement);
    for(std::list<DFA_Trie_S *>::iterator goal = st.statement.dfa.goals.begin(); goal != st.statement.dfa.goals.end(); goal++) 
    {
        (*goal)->data = p;
        if(p)
            (*goal)->handler = p->handler;
    }
    sTree->clear();
    Parser::addReplacePattern(new DFA<Pattern<S>,StatementTree<S> >(st.statement.dfa));;
}

template <class S>
void ParserExtended<S>::addPattern_(std::string aliasPattern, std::string pattern)
{
    parse(pattern);
    S * st = &statementTree.children.front()->statement;
    addPattern(aliasPattern, new handler_injectStatement<S>(st));
}

template <class S>
void ParserExtended<S>::addPattern(std::string pattern, std::string preParserPattern)
{
    typedef handler_injectDFA<S,ParserStatement<S> > h_injectDFA;

    activePatternParser->parse(preParserPattern);
    StatementTree<ParserStatement<S> > * sTree = &activePatternParser->statementTree;
    StatementTree<ParserStatement<S> > st = sTree->children.front()->statement;
    sTree->parent->children.erase((++sTree->parent->children.rbegin()).base());

    for(std::list<StatementTree<ParserStatement<S> >*>::iterator i = ++sTree->children.begin(); i != sTree->children.end(); i++)
        merge<S,ParserStatement<S> >(&st.statement, &(*i)->statement);
    for(std::list<DFA_Trie_S *>::iterator goal = st.statement.dfa.goals.begin(); goal != st.statement.dfa.goals.end(); goal++) 
        (*goal)->data = (Pattern<S>*)1;
    h_injectDFA * h =  new handler_injectDFA<S,ParserStatement<S> >(new DFA_S(st.statement.dfa));
    activePatternParser->statementTree.clear();
    activePatternParser->addReplacePattern(pattern, h);
}

template <class S>
void ParserExtended<S>::setSubPatternSymbol(char symbol)
{
    // TODO
}

template <class S>
void ParserExtended<S>::assignPreParser(Parser<ParserStatement<S> > * preParser)
{
    activePatternParser = preParser;
}

#endif

