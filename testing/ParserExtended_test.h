#include "../ParserExtended.h"
#include "Testing.h"

#ifndef _PARSER_EXTENDED_TEST_H_
#define _PARSER_EXTENDED_TEST_H_

#include <iostream>

template <class S = int>
class h_testHandler : public ParserHandler<S>
{
public:
    h_testHandler(std::string str) : string(str) {}
    h_testHandler(std::string str, S value) : string(str), value(value) {}
    bool operator()(StatementTree<S> & st, Pattern<S> & p, std::string & str)
    {
        std::cout << ":testHandler) \"" << str << "\" => " << string << "\n";
        st.statement = value;
	    return true;
    }
public:
    std::string string;
    S value;
};



#define PARSE_DEBUG(str, parser) std::cout << "\"" << str << "\"\n>>\n" << parser.parse(str)->dfa.start->toString() << "<<\n\n"; 
void parser_extended_test()
{
    INIT_TESTING("ParserExtended_test");
    
    std::string c;
    std::cout << "_Test RegExp syntax structure_\n";
    ParserExtended<int> p1;
    p1.addPattern("(\\$)",  new h_testHandler<>("Regexp ($)"));
    p1.addPattern("\\$|\\$",  new h_testHandler<>("Regexp $|$"));
    p1.addPattern("\\$*",   new h_testHandler<>("Regexp $*"));
    p1.addPattern("a",    new h_testHandler<>("char a"));
    p1.addPattern("b",    new h_testHandler<>("char b"));
    p1.parse("(a)");
    p1.parse("a|b");
    p1.parse("b*");
    p1.parse("(a|b)*");

    std::cout << "\n_Test RegExp semantic implementation_\n"; 
    ParserExtended<ParserStatement<ParserStatement<int> > > p2;
    p2.addPattern("\\$\\$",  new h_regexp_concat<ParserStatement<int>>);
    p2.addPattern("(\\$)",  new h_regexp_parentheses<ParserStatement<int>>);
    p2.addPattern("\\$|\\$",  new h_regexp_divider<ParserStatement<int>>);
    p2.addPattern("\\$*",   new h_regexp_kleinClosure<ParserStatement<int>>);
    p2.addPattern("\\\\", new h_regexp_character<ParserStatement<int>>);
	p2.addPattern("\\(",  new h_regexp_character<ParserStatement<int>>);
	p2.addPattern("\\)",  new h_regexp_character<ParserStatement<int>>);
	p2.addPattern("\\|",  new h_regexp_character<ParserStatement<int>>);
	p2.addPattern("\\*",  new h_regexp_character<ParserStatement<int>>);
	p2.addPattern("\\\\$",  new h_regexp_character<ParserStatement<int>>);
	for(int n = 0; n < 128; n++)
	{
        if(n != '$' && n != '(' && n != ')' && n != '*' && n != '|')
		{
		    c = char(n);
			p2.addPattern(c, new h_regexp_character<ParserStatement<int>>);
		}
	}

    std::cout << "\"(\\(\\*)\"\n>>\n" << p2.parse("(\\(\\*)")->dfa.start->toString() << "<<\n\n";
    std::cout << "\"a|b\"\n>>\n" << p2.parse("a|b")->dfa.start->toString() << "<<\n\n";
    std::cout << "\"b*\"\n>>\n" << p2.parse("b*")->dfa.start->toString() << "<<\n\n";
    std::cout << "\"(a|b)*\"\n>>\n" << p2.parse("(a|b)*")->dfa.start->toString() << "<<\n\n";
    std::cout << "\"(ba*(a|b)b*)\"\n>>\n" << p2.parse("(ba*(a|b)b*)")->dfa.start->toString() << "<<\n\n";
    //std::cout << "\"(a(ab)*b)\"\n>>\n" << p2.parse("(a(ab)*b)")->dfa.start->toString() << "<<\n\n";   // Not working yet (must complete klein closure special cases..)
    
    return;
    // Very inefficient implementation...

    ParserExtended<ParserStatement<int> > p3;
    p3.assignPreParser(&p2);
    p3.addPattern("\\any", getExtendedAsciiCharacterPattern("\\$()|*"));
    p3.addPattern("\\lChar",  "(a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)");
    p3.addPattern("\\uChar",  "(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z)");
    p3.addPattern("\\digit",  "(0|1|2|3|4|5|6|7|8|9)");
    p3.addPattern("\\number", "((0|1|2|3|4|5|6|7|8|9)*)");
    p3.addPattern("\\spaces",       "( |\t|\r|\n|\v|\f)");
    p3.addPattern("\\char",         "(\\lChar|\\uChar)");
    p3.addPattern("\\alphaNumChar", "(\\char|\\digit)");
    p3.addPattern("\\alphaNum",     "(\\alphaNumChar*)");
    p3.addPattern("\\word",         "(\\char(\\alphaNumChar*))");
    p3.addPattern("$", "\\$");  // Allow sub patterns..
    
    /*
    p3.addPattern("\\alphaNum", new h_testHandler<ParserStatement<int> >("not a word"));
    p3.addReplacePattern("\\word", new h_testHandler<ParserStatement<int> >("word"));
    p3.parse("a1sadf23423ed324");
    p3.parse("1aa");
    p3.parse("1a");
    */
    
    p3.addPattern("\\word", new h_testHandler<ParserStatement<int> >("variable"));
    p3.addPattern("\\number", new h_testHandler<ParserStatement<int> >("integer"));
    p3.addReplacePattern("$=$", new h_testHandler<ParserStatement<int> >("store"));
    p3.addReplacePattern("if\\($\\)", new h_testHandler<ParserStatement<int> >("if"));
    p3.parse("abc");
    p3.parse("a312");
    p3.parse("1231");
    p3.parse("a=2");
    p3.parse("if(3)");

    END_TESTING;}

#endif