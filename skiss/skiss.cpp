
/* SETUP LANGUAGE */
/******************/

	/////// Basic Regular Expression Parser
	////////////////////////
	Parser basicRegularExpressionParser;
	basicRegularExpressionParser.setSubPatternSymbol("$");

	/* Setup Basic regular expression expressivity */
	regularExpressionParser.addPattern("($)", h_parentheses);
	regularExpressionParser.addPattern("$|$", h_divider);		// Handles "$|", "|$" and "|" aswell.
	regularExpressionParser.addPattern("$*", h_kleinClosure);
	regularExpressionParser.addPattern("\\", h_specialCharacter);
	regularExpressionParser.addPattern("\$", h_specialCharacter);
	regularExpressionParser.addPattern("\*", h_specialCharacter);
	regularExpressionParser.addPattern("\(", h_specialCharacter);
	regularExpressionParser.addPattern("\)", h_specialCharacter);
	regularExpressionParser.addPattern("\|", h_specialCharacter);
	regularExpressionParser.addPattern(EXTENDED_ASCII_CHARACTER_SET, h_character);


	/////// Standard Regular Expression Parser
	////////////////////////
	Parser regularExpressionParser;
	regularExpressionParser.setSubPatternSymbol("$");
	regularExpressionParser.assignPreParser(basicRegularExpressionParser);

	/* Setup standard regular expression expressivity */
	regularExpressionParser.addPattern("\any", EXTENDED_ASCII_CHARACTER_SET);
	regularExpressionParser.addPattern("$?", "$|");
	regularExpressionParser.addPattern("[\any*]", "\any?*");			// Hmmmm... will this work anyhow?
	regularExpressionParser.addPattern("\any-\any", h_characterRange);
	regularExpressionParser.addPattern("\aChar", "[a-zA-Z]");
	regularExpressionParser.addPattern("\digit", "[0-9]");
	regularExpressionParser.addPattern("\alphNumChar", "[a-zA-Z0-9]");
	regularExpressionParser.addPattern("\spaces", "[ \t\r\n\v\f]");
	regularExpressionParser.addPattern("\word", "\aChar*");
	regularExpressionParser.addPattern("\alphNum", "\alphNumChar*");
	regularExpressionParser.addPattern("^$", h_exclude);
	

	/////// Language Parser
	////////////////////////
	Parser<LogicBlock> simpleC_parser;
	simpleC_parser.assignPreParser(regularExpressionParser);		//Assigns non-defult pre-parser of patterns

	/* Setup language */
	simpleC_parser.addPattern(" ", "\spaces*");	// Make all single spaces aliases for whitespace character
	simpleC_parser.addPattern("\name", "\word\alphNum");
	simpleC_parser.addPattern("{ $* }", h_statementGroup);
	simpleC_parser.addPattern("( $ )", h_paranthesis);
	simpleC_parser.addPattern("$(, $)*", h_list);
	simpleC_parser.addPattern("$ \name $", h_binaryOperator)
	simpleC_parser.addPattern("\name \name", h_declaration);
	simpleC_parser.addPattern("if ($) $", h_if);
	simpleC_parser.addPattern("if ($) $ else $", h_ifelse);
	simpleC_parser.addPattern("while ($) $", h_while);
	simpleC_parser.addPattern("\name ( )", h_functionCall);
	simpleC_parser.addPattern("\name ( $ ( , $)*)", h_functionCall);


/* Interpret */
/******************/

	std::string programString = "...";
	ProgramTree programTree = simpleC_parser.parse(programString);	// Must handle errors of type: "No matching pattern found"
	Context programInternalContext = generateContext(programTree);
	Program program = createProgram(programTree, programInternalContext, programStack, codeLibrary);	// Must handle errors of type: "... not declared, wrong type, no type conversion found etc.."
	program.execute();



/********************************************************/

/* Interfaces*/
Parser:
	.addPattern(string, handler)	// addPattern is non-intrusive (e.g. do not write over other patterns), returns false if it is inconsistent with existing patterns
	.addPattern(string, string)		// The second string is parsed through an optional parser or itself.
	.addPattern(string, DFA)
	.removePattern(string)
	.replacePattern(string, handler)
	.replacePattern(string, string)
	.replacePattern(string, DFA)

CodeLibrary:
	.findFunction(string name, ...







		
	
	/*
	/////// Natural Regular Expression Parser (everything special require \ as prefix)
	////////////////////////
	Parser naturalRegularExpressionParser;
	naturalRegularExpressionParser.setSubPatternSymbol("\$");
	naturalRegularExpressionParser.assignPreParser(basicRegularExpressionParser);

	// Setup standard regular expression expressivity //
	naturalRegularExpressionParser.addPattern("\any", EXTENDED_ASCII_CHARACTER_SET);
	naturalRegularExpressionParser.addPattern("\$?", "$|");	// Implements the "?" operator
	naturalRegularExpressionParser.addPattern("(\$)", "$|");	// Implements the "?" operator
	naturalRegularExpressionParser.addPattern("\[\any\*]", "\any?*");			// Hmmmm... will this work anyhow?
	naturalRegularExpressionParser.addPattern("\any-\any", h_characterRange);
	naturalRegularExpressionParser.addPattern("\aChar", "\[a-zA-Z]");
	naturalRegularExpressionParser.addPattern("\digit", "\[0-9]");
	naturalRegularExpressionParser.addPattern("\alphNumChar", "\[a-zA-Z0-9]");
	naturalRegularExpressionParser.addPattern("\spaces", "\[ \t\r\n\v\f]");
	naturalRegularExpressionParser.addPattern("\word", "\aChar*");
	naturalRegularExpressionParser.addPattern("\alphNum", "\alphNumChar*");
	naturalRegularExpressionParser.addPattern("^\$", h_exclude);
	*/