#include "Parser.h"
#include "testing\Testing.h"

#ifndef _PARSER_TEST_H_
#define _PARSER_TEST_H_

#include <iostream>

class h_test_test1 : public ParserHandler<int>
{
    bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
    {
	    st.statement = 1337;
	    return true;
    }
};
class h_test_test2 : public ParserHandler<int>
{
    bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
    {
	    st.statement = 777;
	    return true;
    }
};

// Aritmetic test
class h_test_digit : public ParserHandler<int>
{
bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
{
	st.statement = str.back()-'0';
	return true;
}
};
class h_test_add : public ParserHandler<int>
{
bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
{
	st.statement = st.firstPre()->statement + st.firstPost()->statement;
	st.removeFirstPre();
	st.removeChildren();
	return true;
}
};
class h_test_mult : public ParserHandler<int>
{
bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
{
	st.statement = st.firstPre()->statement * st.firstPost()->statement;
	st.removeFirstPre();
	st.removeChildren();
	return true;
}
};
class h_test_paranthesis : public ParserHandler<int>
{
bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
{
	st.statement = st.firstPost()->statement;
	st.removeChildren();
	return true;
}
};

// alias generation test
template <class S>
class Statement_DFA
{
	typedef DFA_Trie<Pattern<S>,StatementTree<S> > dfaTrie;
public:
    DFA<Pattern<S>,StatementTree<S> > dfa;
	//dfaTrie * start;
	//std::list<dfaTrie *> goals;
public:
	std::string str;
	Statement_DFA() { dfa.start = 0; }
	Statement_DFA(std::string str) : str(str) {}
};
template<typename S>
class h_test_charGen : public ParserHandler<Statement_DFA<S> >
{
bool operator()(StatementTree<Statement_DFA<S> > & st, Pattern<Statement_DFA<S> > & p, std::string & str)
{
	typedef DFA_Trie<Pattern<S>, StatementTree<S>> dfaTrie;
	st.statement.dfa.start = new dfaTrie();
	st.statement.dfa.goals.push_back(new dfaTrie());
	st.statement.dfa.start->add_branch(str.back(), st.statement.dfa.goals.front());
	st.statement.dfa.goals.front()->data = (Pattern<int>*)1;	// Debug... fix this
	return true;
}
};

template<typename S>
class h_test_concat : public ParserHandler<Statement_DFA<S> >
{
bool operator()(StatementTree<Statement_DFA<S> > & st, Pattern<Statement_DFA<S> > & p, std::string & str)
{
    st.children.push_front(st.firstPre());
    st.removeFirstPre();
	mergeAll<S,Statement_DFA<S> >(st);
	return true;
}
};
#include "../RegularExpressionParser.h"

// RegularExpression tests
class h_test_noMatchingCharacter : public ParserHandler<int>
{
bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
{
	std::cout << "No match: " << str << "\n";
	st.removeThis();
	return true;
}
};
class h_test_matchingCharacter : public ParserHandler<int>
{
bool operator()(StatementTree<int> & st, Pattern<int> & p, std::string & str)
{
	std::cout << "Match: " << str << "\n";
	st.removeThis();
	return true;
}
};




// special parser test
template <class S>
class SuperParser : public Parser<S>
{
public:
    RegularExpressionParser<S, Statement_DFA<S> > * defaultPatternParser;
    RegularExpressionParser<S, Statement_DFA<S> > * activePatternParser;
public:
    SuperParser()
    { 
        Parser::Parser();
        defaultPatternParser = new RegularExpressionParser<S, Statement_DFA<S> >;
        /*ParserHandler<Statement_regexp_<S> > * h_character = new handler_character<S,Statement_regexp_<S> >;
        std::string c;
		for(int n = 0; n < 256; n++)
		{
			if(n != '$')
			{
				c = char(n);
				defaultPatternParser->addPattern(c, h_character);
			}
		}*/
        activePatternParser = defaultPatternParser;
        activePatternParser->init();    // Activate reg-exp functionality
        activePatternParser->addPattern("$$", new h_test_concat<S>);    // Allow concatination of string patterns
    }
public:
    
	void addPattern(std::string pattern, ParserHandler<S> * handler) {
        typedef DFA<Pattern<S>,StatementTree<S> > DFA_;
        DFA_ * dfa = activePatternParser->parse(pattern, new Pattern<S>(pattern, handler));
        Parser::addPattern(dfa);
        std::cout << "SuperParser::addPattern\n";
	}
    void addReplacePattern(std::string pattern, ParserHandler<S> * handler) {
        typedef DFA<Pattern<S>,StatementTree<S> > DFA_;
        DFA_ * dfa = activePatternParser->parse(pattern, new Pattern<S>(pattern, handler));
        Parser::addReplacePattern(dfa);
        std::cout << "SuperParser::addPattern\n";
	}
	void addPattern(std::string pattern, std::string preParserPattern)
	{
        typedef DFA<Pattern<S>,StatementTree<S> > DFA_;
        typedef DFA_Trie<Pattern<S>,StatementTree<S> > DFA_Trie_;
        typedef handler_injectDFA<int,Statement_DFA<int> > h_injectDFA;

        DFA_ * dfa = activePatternParser->parse(preParserPattern, (Pattern<S>*)0);
        for(std::list<DFA_Trie_*>::iterator goal = dfa->goals.begin(); goal != dfa->goals.end(); goal++)
            (*goal)->data = (Pattern<S>*)1;
        h_injectDFA * h =  new handler_injectDFA<int,Statement_DFA<int> >(dfa);
        activePatternParser->addReplacePattern(pattern, h);

    }
};

// character Range (A-Z)
template <class S, class S_wrapper>
class handler_characterRange : public ParserHandler<S_wrapper>
{
public:
    bool operator()(StatementTree<S_wrapper> & st, Pattern<S_wrapper> & p, std::string & str)
    {
        std::cout << "handler_characterRange\n";
        if(str.front() > str.back())
            return false;
        
        st.statement.dfa.start = new DFA_Trie<Pattern<S>,StatementTree<S> >;
        for(int n = str.front(); n <= str.back(); n++)
        {
            st.statement.dfa->goals.push_back(new DFA_Trie<Pattern<S>,StatementTree<S> >);
            st.statement.dfa->start->add_branch(n, st.statement.dfa->goals.back());
        }
        st.isList = false;
        return true;
    }
};


void parser_test()
{
	INIT_TESTING("Parser_test");
    
	Parser<int> parser, parser_calc;
	parser.addPattern("ab", new h_test_test1);
	parser.addPattern("ab", new h_test_test2);
	ASSERT_TEST(*parser.parse("ab"),1337);
	parser.addReplacePattern("ab", new h_test_test2);
	ASSERT_TEST(*parser.parse("ab"),777);
    
    
	parser_calc.addPattern("1", new h_test_digit);
	parser_calc.addPattern("2", new h_test_digit);
	parser_calc.addPattern("3", new h_test_digit);
	parser_calc.addPattern("$+$", new h_test_add);
	parser_calc.addPattern("$*$", new h_test_mult);
	parser_calc.addPattern("($)", new h_test_paranthesis);
	ASSERT_TEST(*parser_calc.parse("1+2"),3)
	ASSERT_TEST(*parser_calc.parse("2*3"),6);
	ASSERT_TEST(*parser_calc.parse("1+2+3"),6);
	ASSERT_TEST(*parser_calc.parse("(1+1)*2"),4);
	ASSERT_TEST(*parser_calc.parse("(1 4)*2"),2);   // Error management test
	
	Parser<int> parser_alias;
	std::cout << "\n";
	Parser<Statement_DFA<int> > parser_alias_gen;
	parser_alias_gen.addPattern("1", new h_test_charGen<int>);
	parser_alias_gen.addPattern("2", new h_test_charGen<int>);
	parser_alias_gen.addPattern("$$", new h_test_concat<int>);
	Statement_DFA<int> * r = parser_alias_gen.parse("12");
	// chain merge (one statement at a time)
	parser_alias_gen.statementTree.parent->children.erase((++parser_alias_gen.statementTree.parent->children.rbegin()).base());
	for(std::list<StatementTree<Statement_DFA<int> >*>::iterator i = ++parser_alias_gen.statementTree.children.begin(); i != parser_alias_gen.statementTree.children.end(); i++)
		merge<int, Statement_DFA<int>>(r, &(*i)->statement);
	r->dfa.goals.front()->handler = new h_test_test1;
	parser_alias.addPattern(r->dfa.start);
	ASSERT_TEST(*parser_alias.parse("12"),1337);
    
	Parser<int> p_;
	RegularExpressionParser<int,Statement_regexp_<int> > p_regexp;
	p_regexp.init();
	p_.addPattern("aa", new h_test_matchingCharacter);
	p_.addPattern("aaaa", new h_test_matchingCharacter);
	std::cout << ">\n";
	p_.addPattern(p_regexp.parse("a*", new Pattern<int>("a*", new h_test_noMatchingCharacter)));
	std::cout << "a: "; p_.parse("a");
	std::cout << "aa: "; p_.parse("aa");
	std::cout << "aaa: "; p_.parse("aaa");
	std::cout << "aaaa: "; p_.parse("aaaa");
	std::cout << "aaaaa: "; p_.parse("aaaaa");
    
    typedef DFA<Pattern<int>,StatementTree<int> > DFA_;
    typedef handler_injectDFA<int,Statement_regexp_<int> > h_injectDFA;

	std::cout << "\nDFA injection test\n";
	Parser<int> p2_;
	RegularExpressionParser<int,Statement_regexp_<int> > p_regexp2;
	p_regexp2.init();

    DFA_ * dfa = p_regexp2.parse("a|b", new Pattern<int>("a|b", new h_test_matchingCharacter));
    h_injectDFA * h =  new handler_injectDFA<int,Statement_regexp_<int> >(dfa);
    p_regexp2.addPattern("\\any", h);
    DFA_ * dfa2 = p_regexp2.parse("\\any", new Pattern<int>("\\any", new h_test_matchingCharacter));
    p2_.addPattern(dfa2);
    p2_.parse("a");
    p2_.parse("b");

	std::cout << "\nSpecial Parser test\n";
    SuperParser<int> superP;
    superP.addPattern("ab", new h_test_matchingCharacter);
    superP.parse("ab");
    superP.addPattern("moho", "bc");
    superP.addPattern("moho", new h_test_matchingCharacter);
    //superP.addPattern("mo", new h_test_matchingCharacter);
    superP.parse("ab");
    superP.parse("bc");
    //superP.parse("mo");
    superP.addPattern("any", "c|d");
    superP.addPattern("any*", new h_test_matchingCharacter);
    superP.parse("c");
    superP.parse("d");
    superP.parse("ccd");
    superP.parse("dcd");

    std::cout << "\nASCII-test\n";
    superP.addPattern("ASCII", getExtendedAsciiCharacterPattern("$(*|\\"));
    superP.addPattern("eASCII", new h_test_matchingCharacter);
    superP.parse("e0");
    superP.parse("e?");

    std::cout << "\nCharacter range test\n";
    //superP.addPattern("m-o", new handler_characterRange<int,Statement_regexp_<int> >);
    

    std::cout << "Fill upp all empty with *\n";
    SuperParser<int> superP_1;
	superP_1.addPattern("aa", new h_test_matchingCharacter);
	superP_1.addPattern("aaaa", new h_test_matchingCharacter);
	std::cout << ">\n";
	superP_1.addPattern("a*", new h_test_noMatchingCharacter);
	std::cout << "a: "; superP_1.parse("a");
	std::cout << "aa: "; superP_1.parse("aa");
	std::cout << "aaa: "; superP_1.parse("aaa");
	std::cout << "aaaa: "; superP_1.parse("aaaa");
	std::cout << "aaaaa: "; superP_1.parse("aaaaa");
    
    std::cout << "Replace intermediate goals from * by replacement override:\n";
    SuperParser<int> superP_2;
	superP_2.addPattern("aa", new h_test_matchingCharacter);
	std::cout << ">\n";
	superP_2.addPattern("a*", new h_test_noMatchingCharacter);
	superP_2.addReplacePattern("aaaa", new h_test_matchingCharacter);
	std::cout << "a: "; superP_2.parse("a");
	std::cout << "aa: "; superP_2.parse("aa");
	std::cout << "aaa: "; superP_2.parse("aaa");
	std::cout << "aaaa: "; superP_2.parse("aaaa");
	std::cout << "aaaaa: "; superP_2.parse("aaaaa");

    SuperParser<int> superP_3;
	superP_3.addPattern("a", new h_test_matchingCharacter);
	std::cout << ">\n";
	superP_3.addPattern("a*", new h_test_noMatchingCharacter);
	std::cout << "a: "; superP_3.parse("a");
	std::cout << "aa: "; superP_3.parse("aa");
	std::cout << "aaa: "; superP_3.parse("aaa");

	END_TESTING;
}

#endif