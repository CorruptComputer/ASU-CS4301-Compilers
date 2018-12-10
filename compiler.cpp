// Nickolas Gupton & Zach Nadeau
// CS 4301
// Compiler Stage 2

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <cctype>
#include <stack>

using namespace std;


///////////////////////// Constants/Variables /////////////////////////
const int MAX_SYMBOL_TABLE_SIZE = 256; enum
storeType {INTEGER, BOOLEAN, PROG_NAME, UNKNOWN};
enum allocation {YES,NO};
enum modes {VARIABLE, CONSTANT};
struct entry {  //define symbol table entry format
	string internalName;
	string externalName;
	storeType dataType;
	modes mode;
	string value;
	allocation alloc;
	int units;
};
vector<entry> symbolTable;
ifstream sourceFile;
ofstream listingFile,objectFile;
string token;
char charac;
stack<string> operatorStk, operandStk, beginEndStk;
const char END_OF_FILE = '$'; // arbitrary choice

// Our Variables
uint currentLine = 0;
uint errorCount = 0;
bool hasReachedEnd = false;
string contentOfA = "";
int currentTempNo = -1;
int maxTempNo = -1;


///////////////////////// Prototypes /////////////////////////
void CreateListingHeader();
void Parser();
void CreateListingTrailer();
void PrintSymbolTable();

// Grammar Rules
void Prog();
void ProgStmt();
void Consts();
void Vars();
void BeginEndStmt();
void VarStmts();
void ConstStmts();
string Ids();
void ExecStmts();
void AssignStmt();
void ReadStmt();
void WriteStmt();
void Express();
void Expresses();
void Term();
void Terms();
void Factor();
void Factors();
void Part();
void IfStmt();
void ElsePt();
void WhileStmt();
void RepeatStmt();
void NullStmt();

// Action Routines
void Insert(string, storeType, modes, string, allocation, int);
storeType WhichType(string);
string WhichValue(string);
string GenInternalName(storeType);
void Code(string, string = "", string = "");
void PushOperator(string);
void PushOperand(string);
void PushBeginEnd(string);
string PopOperator();
string PopOperand();
string PopBeginEnd();
void FreeTemp();
string GetTemp();

void EmitAdditionCode(string, string);
void EmitSubtractionCode(string, string);
void EmitNegationCode(string);
void EmitNotCode(string);
void EmitMultiplicationCode(string, string);
void EmitDivisionCode(string, string);
void EmitModuloCode(string, string);
void EmitAndCode(string, string);
void EmitOrCode(string, string);
void EmitEqualsCode(string, string);
void EmitLessThanCode(string, string);
void EmitGreaterThanCode(string, string);
void EmitGreaterOrEqualsCode(string, string);
void EmitLessOrEqualsCode(string, string);
void EmitNotEqualsCode(string, string);
void EmitAssignCode(string, string);
void EmitThenCode(string);
void EmitElseCode(string);
void EmitPostIfCode(string);
void EmitWhileCode();
void EmitDoCode(string);
void EmitPostWhileCode(string, string);
void EmitRepeatCode();
void EmitUntilCode(string, string);

// Lexical Scanner
string NextToken();
char NextChar();

// "Glue" Functions
bool isKeyword(string);
bool isSpecToken(string);
bool isSpecSym(string);
bool isSpecSym(char);
bool isInteger(string);
bool isBoolean(string);
bool isNonKeyID(string);
string printDataType(int);
string printMode(int);
string printAlloc(int);
string WhichInternName(string);
string WhichExternName(string);
uint indexOfVariable(string);
bool isInSymbolTable(string);
bool isRelOp(string);                    // Roses are red, Violets are blue, ';' expected on line 132
bool isAddLevelOp(string);
bool isMultLevelOp(string);
void printAssembly(string, string, string, string, string);
string getNextLocation();


///////////////////////// Main Functions /////////////////////////
int main(int argc, char **argv) {
	if(argc != 4) {
		cout << "Expected 3 arguments, received " << (argc-1) << "\nUsage: ./stage0 <input> <listing file> <object file>" << endl;
		return -1;
	}

	try {
		//this program is the stage0 compiler for Pascallite. It will accept
		//input from argv[1], generating a listing to argv[2], and object code to
		//argv[3]
		sourceFile.open(argv[1]);
		listingFile.open(argv[2]);
		objectFile.open(argv[3]);
		
		CreateListingHeader();
		
		listingFile << "\n";
		Parser();
		// PrintSymbolTable();
		
	} catch (const char* e) {
		listingFile << endl << "Error: Line " << currentLine << ": " << e << endl;
		cerr << argv[1] << "  Error: Line " << currentLine << ": " << e << endl;
	  errorCount++;
	}
   
   CreateListingTrailer();
   
   sourceFile.close();
   listingFile.close();
   objectFile.close();

	return 0;
}

void CreateListingHeader() {
	time_t now = time (NULL);
	listingFile << "STAGE0:  " << "NICKOLAS GUPTON & ZACH NADEAU       " << ctime(&now) << endl;
	listingFile << "LINE NO.              SOURCE STATEMENT" << "\n";
	//listingFile << setw(5) << 1 << '|';
	
	//line numbers and source statements should be aligned under the headings
}

void Parser() {
	NextChar();
	//charac must be initialized to the first character of the source file
	if(NextToken() != "program") {
		// process error: keyword "program" expected;
		throw "keyword 'program' expected";
		
	}
	//a call to NextToken() has two effects
	// (1) the variable, token, is assigned the value of the next token
	// (2) the next token is read from the source file in order to make
	// the assignment. The value returned by NextToken() is also
	// the next token.
	Prog();
	//parser implements the grammar rules, calling first rule
}

void CreateListingTrailer() {
	// print "COMPILATION TERMINATED", "# ERRORS ENCOUNTERED";
	listingFile << "\nCOMPILATION TERMINATED      " << errorCount << " ERRORS ENCOUNTERED" << endl;
}

void PrintSymbolTable() {
	time_t now = time (NULL);
	objectFile << "STAGE0:  " << "NICKOLAS GUPTON & ZACH NADEAU       " << ctime(&now) << endl;
	objectFile << "Symbol Table" << endl << endl;
	for (uint i = 0; i < symbolTable.size(); i++) {
		objectFile << setw(15) << left  << symbolTable.at(i).externalName.substr(0, 15) << "  ";
		objectFile << setw(4)  << left  << symbolTable.at(i).internalName << "  ";
		objectFile << setw(9)  << right << printDataType(i) << "  ";
		objectFile << setw(8)  << right << printMode(i) << "  ";
		objectFile << setw(15) << right << symbolTable.at(i).value.substr(0, 15) << "  ";
		objectFile << setw(3)  << right << printAlloc(i) << "  ";
		objectFile << symbolTable.at(i).units << '\n';
	}
}


///////////////////////// Grammar Rules /////////////////////////
void Prog() {  //token should be "program"
	if (token != "program") {
		throw "keyword 'program' expected";
	}
   
	ProgStmt();
	NextToken();
	if (token == "const") {
		Consts();
	}
	if (token == "var") {
		Vars();
	}
	if (token != "begin") {
		throw "keyword 'begin' expected";
	}
	PushBeginEnd("begin");
	BeginEndStmt();
	NextToken();
	if (hasReachedEnd && token[0] != END_OF_FILE) {
		throw "no text may follow 'end.'";
	}
}

void ProgStmt() {  //token should be "program"
	string x;
	if (token != "program") {
		throw "keyword 'program' expected";
	}
	x = NextToken();
	if (!isNonKeyID(token)) {
		throw "program name expected";
	}
	if (NextToken() != ";") {
		throw "semicolon expected";
	}

	Insert(x,PROG_NAME,CONSTANT,x,NO,0);
	Code("program",x);
}

void Consts() {  //token should be "const"
	if (token != "const") {
		throw "keyword 'const' expected";
	}
	if (!isNonKeyID(NextToken())) {
		throw "non-keyword identifier must follow 'const'";
	}
	ConstStmts();
}

void Vars() {  //token should be "var"
	if (token != "var") {
		throw "keyword 'var' expected";
	}
	if (!isNonKeyID(NextToken())) {
		throw "non-keyword identifier must follow 'var'";
	}
	VarStmts();
}

void BeginEndStmt() {  //token should be "begin"
	if (token != "begin") {
		throw "keyword 'begin' expected";
	}
	ExecStmts();
	if (token != "end") {
		throw "non-keyword identifier, 'read', 'write', 'if', 'while', 'repeat', ';', or 'begin' expected";
	}
	if (NextToken() == ".") {
		PopBeginEnd();
		if (!beginEndStk.empty()) {
			throw "';' expected after non-final end";
		}
		hasReachedEnd = true;
		Code("end",".");
	} else if (token == ";") {
		PopBeginEnd();
		if (beginEndStk.empty()) {
			throw "'.' expected after final 'end'";
		}
	} else if (!beginEndStk.empty()) {
		throw "';' expected";
	} else {
		throw "'.' expected";
	}
	// NextToken();
}

void ConstStmts() {  //token should be NON_KEY_ID
	string x,y;
	if (!isNonKeyID(token)) {
		throw "non-keyword identifier expected";
	}
	x = token;
	if (NextToken() != "=") {
		// process error: "=" expected
		throw "'=' expected";
	}
	y = NextToken();
	if (y != "+"
		&& y != "-"
		&& y != "not"
		&& !isNonKeyID(y)
		&& !isInteger(y)
		&& !isBoolean(y)) {
		throw "token to right of '=' illegal";
	}
	if (y == "+" || y == "-") {
		if(!isInteger(NextToken())) {
			throw "integer expected after sign";
		}
		y = y + token;
	}
	if (y == "not") {
		if (NextToken() != "true" && token != "false" && !isNonKeyID(token)) {
			throw "boolean expected after not";
		}
		if (token == "true") {
			y = "false";
		} else if (token == "false") {
			y = "true";
		} else {
			if (WhichType(token) != BOOLEAN) {
				throw "expected BOOLEAN after 'not' statement";
			} else {
				y = (WhichValue(token) == "1") ? "false" : "true";  // if the value of token is true, y's value is false and vice-versa
			}
		}
	}
	// Tests if the value on the rhs of an equal sign is the program name
	if (isNonKeyID(y)) {
		for (uint i = 0; i < symbolTable.size(); i++) {
			if (symbolTable.at(i).externalName == y && symbolTable.at(i).dataType == PROG_NAME) {
				throw "data type of token to right of '=' illegal";
			}
		}
	}
	if (NextToken() != ";") {
		throw "semicolon expected";
	}
	Insert(x, WhichType(y), CONSTANT, WhichValue(y), YES, 1);
	if (NextToken() != "begin"
		&& token != "var"
		&& !isNonKeyID(token)) {
		throw "non-keyword identifier, 'begin', or 'var' expected";
	}
	if (isNonKeyID(token)) {
		ConstStmts();
	}
}

void VarStmts() {  //token should be NON_KEY_ID
	string x, y;
	if (!isNonKeyID(token)) {
		// process error: non-keyword identifier expected
		throw "non-keyword identifier expected";
	}
	x = Ids();
	if (token != ":") {
		// process error: ":" expected
		throw "':' expected";
	}
	if(NextToken() != "integer" && token != "boolean") {
		// process error: illegal type follows ":"
		throw "illegal type follows ':'";
	}
	y = token;
	if(NextToken() != ";") {
		// process error: semicolon expected
		throw "semicolon expected";
	}
	
	if (y == "integer") {
		Insert(x,INTEGER,VARIABLE,"",YES,1);
	} else if (y == "boolean") {
		Insert(x,BOOLEAN,VARIABLE,"",YES,1);
	}
	
	if (NextToken() != "begin" && !isNonKeyID(token)) {
		// process error: non-keyword identifier or "begin" expected
		throw "non-keyword identifier or 'begin' expected";
	}
	if (isNonKeyID(token)) {
		VarStmts();
	}
}

string Ids() {  //token should be NON_KEY_ID
	string temp,tempString;
	if (!isNonKeyID(token)) {
		// process error: non-keyword identifier expected
		throw "non-keyword identifier expected";
	}
	tempString = token;
	temp = token;
	if(NextToken() == ",") {
		if (!isNonKeyID(NextToken())) {
			// process error: non-keyword identifier expected
			throw "non-keyword identifier expected";
		}
		tempString = temp + "," + Ids();
	}
	return tempString;
}

void ExecStmt() {
	if (isNonKeyID(token)) {
		AssignStmt();
	} else if (token == "read") {
		ReadStmt();
	} else if (token == "write") {
		WriteStmt();
	} else if (token == "if") {
		IfStmt();
	} else if (token == "while") {
		WhileStmt();
	} else if (token == "repeat") {
		RepeatStmt();
	} else if (token == ";") {
		NullStmt();
	} else if (token == "begin") {
		PushBeginEnd("begin");
		BeginEndStmt();
	}
}

/* void ExecStmts() {
	while (isNonKeyID(NextToken())
			|| token == "read"
			|| token == "write"
			|| token == "if"
			|| token == "while"
			|| token == "repeat"
			|| token == ";"
			|| token == "begin") {
		if (isNonKeyID(token)) {
			AssignStmt();
		} else if (token == "read") {
			ReadStmt();
		} else if (token == "write") {
			WriteStmt();
		} else if (token == "if") {
			IfStmt();
		} else if (token == "while") {
			WhileStmt();
		} else if (token == "repeat") {
			RepeatStmt();
		} else if (token == ";") {
			NullStmt();
		} else if (token == "begin") {
			BeginEndStmt();
		}
	} 
	// if (token != "end") {
		// throw "non-keyword identifier, 'read', 'write', or 'end' expected";
	// }
} */

void ExecStmts() {
	NextToken();
	while (isNonKeyID(token)
			|| token == "read"
			|| token == "write"
			|| token == "if"
			|| token == "while"
			|| token == "repeat"
			|| token == ";"
			|| token == "begin") {
		ExecStmt();
		// This is done because when checking else in ElsePt, we can return here with the beginning
		// of the next ExecStmt, not the end of the current ExecStmt
		if (token == ";") {
			NextToken();
		}
	}
	// if (token != "end") {
		// throw "non-keyword identifier, 'read', 'write', or 'end' expected";
	// }
}

void AssignStmt() {
	if (!isNonKeyID(token)) {
		throw "non-keyword identifier expected";
	}
	PushOperand(token);
	if (NextToken() != ":=") {
		throw "':=' expected";
	}
	PushOperator(token);
	Express();
	if (token != ";") {
		throw "';' expected";
	}
	string rhs = PopOperand();
	string lhs = PopOperand();
	Code(PopOperator(),rhs,lhs);
}

void ReadStmt() {
	string x = "";
	if (NextToken() != "(") {
		throw "'(' expected";
	}
	
	NextToken();
	string idList = Ids();
	
	for(uint i = 0; i < idList.length(); i++) {
		if(idList[i] == ',') {
			Code("read",x);
			x = "";
		} else {
			x += idList[i];
		}
	}
	Code("read",x);
	
	if (token != ")") {
		throw "',' or ')' expected after non-keyword identifier";
	}
	if (NextToken() != ";") {
		throw "';' expected";
	}
}

void WriteStmt() {
	string x = "";
	if (NextToken() != "(") {
		throw "'(' expected";
	}
	
	NextToken();
	string idList = Ids();
	
	for(uint i = 0; i < idList.length(); i++) {
		if(idList[i] == ',') {
			Code("write",x);
			x = "";
		} else {
			x += idList[i];
		}
	}
	Code("write",x);
	
	if (token != ")") {
		throw "',' or ')' expected after non-keyword identifier";
	}
	if (NextToken() != ";") {
		throw "';' expected";
	}
}

void Express() {
	NextToken();
	if (!(token == "not" || isBoolean(token) || token == "(" || token == "+" || token == "-" || isInteger(token) || isNonKeyID(token))) {
		throw "'not', '(', '+', '-', boolean literal, integer literal, or non-keyword identifier expected";
	}
	Term();
	Expresses();
}

void Expresses() {
	if (isRelOp(token)) {
		PushOperator(token);
		NextToken();
		Term();
		string rhs = PopOperand();
		string lhs = PopOperand();
		Code(PopOperator(), rhs, lhs);
		Expresses();
	}/*  else if(!(token == ";" || token == ")" || token == "then" || token == "do")) {
		throw "';', ')', 'then', 'do', or relationship operator expected";
	} */
}

void Term() {
	if (!(token == "not" || isBoolean(token) || token == "(" || token == "+" || token == "-" || isInteger(token) || isNonKeyID(token))) {
		throw "'not', '(', '+', '-', boolean literal, integer literal, or non-keyword identifier expected";
	}
	Factor();
	Terms();
}

void Terms() {
	if (isAddLevelOp(token)) {
		PushOperator(token);
		NextToken();
		Factor();
		string rhs = PopOperand();
		string lhs = PopOperand();
		Code(PopOperator(), rhs, lhs);
		Terms();
	}/*  else if(!(isRelOp(token) 
				|| token == ";" 
				|| token == ")"
				|| token == "then"
				|| token == "do")) {
		throw "';', ')', 'then', 'do', relationship operator, or addition-level operator expected";
	} */
}

void Factor() {
	if (!(token == "not" || isBoolean(token) || token == "(" || token == "+" || token == "-" || isInteger(token) || isNonKeyID(token))) {
		throw "'not', '(', '+', '-', boolean literal, integer literal, or non-keyword identifier expected";
	}
	Part();
	Factors();
}

void Factors() {
	if (isMultLevelOp(token)) {
		PushOperator(token);
		NextToken();
		Part();
		string rhs = PopOperand();
		string lhs = PopOperand();
		Code(PopOperator(), rhs, lhs);
		Factors();
	}/*  else if(!(isRelOp(token) 
				|| isAddLevelOp(token) 
				|| token == ";" 
				|| token == ")"
				|| token == "then"
				|| token == "do")) {
		throw "';', ')', 'then', 'do', relationship operator, addition-level operator, or multiplication-level operator expected";
	} */
}

void Part() {
	if (token == "not") {
		NextToken();
		if (token == "(") {
			Express();
			if (token != ")") {
				throw "')' expected";
			}
			NextToken();
			string rhs = PopOperand();
			Code("not", rhs);
		} else if (isBoolean(token)) {
			if (WhichValue(token) == "1") {
				PushOperand("false");
			} else {
				PushOperand("true");
			}
			NextToken();
		} else if (isNonKeyID(token)) {
			Code("not", WhichInternName(token));
			NextToken();
		} else {
			throw "'(', boolean literal, or non-keyword identifier expected";
		}
	} else if (token == "+") {
		NextToken();
		if (token == "(") {
			Express();
			if (token != ")") {
				throw "')' expected";
			}
			NextToken();
		} else if (isInteger(token) || isNonKeyID(token)) {
			PushOperand(token);
			NextToken();
		} else {
			throw "'(', integer literal, or non-keyword identifier expected";
		}
	} else if (token == "-") {
		NextToken();
		if (token == "(") {
			Express();
			if (token != ")") {
				throw "')' expected";
			}
			NextToken();
			string rhs = PopOperand();
			Code("neg", rhs);
		} else if (isInteger(token)) {
			PushOperand("-" + token);
			NextToken();
		} else if (isNonKeyID(token)) {
			Code("neg", WhichInternName(token));
			NextToken();
		} else {
			throw "'(', integer literal, or non-keyword identifier expected";
		}
	} else if (isInteger(token) || isBoolean(token) || isNonKeyID(token)) {
		PushOperand(token);
		NextToken();
	} else if (token == "(") {
		Express();
		if (token != ")") {
			throw "')' expected";
		}
		NextToken();
	} else {
		throw "'not', '(', '+', '-', boolean literal, integer literal, or non-keyword identifier expected";
	}
}

void IfStmt() {
	if (token != "if") {
		throw "'if' expected";
	}
	Express();
	if (token != "then") {
		throw "'then' expected";
	}
	string opnd = PopOperand();
	Code("then", opnd);
	NextToken();
	ExecStmt();
	if (token == ";") {
		NextToken();
	}
	ElsePt();
}

void ElsePt() {
	if (token == "else") {
		string opnd = PopOperand();
		Code("else", opnd);
		
		NextToken();
		ExecStmt();
		
		opnd = PopOperand();
		Code("post_if", opnd);
	} else if (token == "end"
			|| token == ";"
			|| token == "until"
			|| token == "begin"
			|| token == "while"
			|| token == "if"
			|| token == "repeat"
			|| token == "read"
			|| token == "write"
			|| isNonKeyID(token)) {
		string opnd = PopOperand();
		Code("post_if", opnd);
	} else {
		throw "'else', 'end', ';', 'until', 'begin', 'while', 'if', 'repeat', 'read', 'write', or non-keyword identifier expected";
	}
}

void WhileStmt() {
	if (token != "while") {
		throw "'while' expected";
	}
	Code("while");
	
	Express();
	if (token != "do") {
		throw "'do' expected";
	}
	string poperand = PopOperand();
	Code("do", poperand);
	NextToken();
	ExecStmt();
	
	string rhs = PopOperand();
	string lhs = PopOperand();
	Code("post_while", rhs, lhs);
}

void RepeatStmt() {
		if (token != "repeat") {
		throw "'repeat' expected";
	}
	Code("repeat");
	ExecStmts();
	
	if (token != "until") {
		throw "'until' expected";
	}
	Express();
	
	string rhs = PopOperand();
	string lhs = PopOperand();
	Code("until", rhs, lhs);
	
	if (token != ";") {
		throw "';' expected";
	}
}

void NullStmt() {
	if (token != ";") {
		throw "';' expected";
	}
}


///////////////////////// Action Routines /////////////////////////
void Insert(string externalName, storeType inType, modes inMode, string inValue, allocation inAlloc, int inUnits) {
	//create symbol table entry for each identifier in list of external names
	//Multiply inserted names are illegal
	vector<string> names;
	names.push_back("");
	
	for(uint i = 0, word = 0; i < externalName.length(); i++) {
		if(externalName[i] == ',') {
			word++;
			names.push_back("");
			continue;
		}
		names.at(word) += externalName[i];
	}
	
	while (names.size() > 0) {  // name broken from list of external names and put into name != ""
		// symbolTable[name] is defined
		for (uint i = 0; i < symbolTable.size(); i++) {
			if (symbolTable.at(i).externalName == names.front().substr(0,15)) {
				// process error: multiple name definition
				PrintSymbolTable();
				throw "multiple name definition";
			}
		}
			
		if (isKeyword(names.front()) && !(names.front() == "true" || names.front() == "false")) { // name is a keyword
			// process error: illegal use of keyword
			throw "illegal use of keyword";
		}
		
		entry thing;
		thing.externalName = names.front().substr(0,15);
		thing.dataType = inType;
		thing.mode = inMode;
		thing.value = inValue;
		thing.alloc = inAlloc;
		thing.units = inUnits;
		
		if (symbolTable.size() < MAX_SYMBOL_TABLE_SIZE) {
			// create table entry
			if (isupper(names.front().at(0))) {
				thing.internalName = names.front();
			} else if (names.front() == "true"){
				thing.internalName = "TRUE";
			} else if (names.front() == "false"){
				thing.internalName = "FALS";
			} else {
				thing.internalName = GenInternalName(inType);
			}
		} else {
			throw "symbol table cannot store more than " + to_string(MAX_SYMBOL_TABLE_SIZE) + " entries";
		}
	  
		symbolTable.emplace_back(thing);
		names.erase(names.begin());
	}
}

storeType WhichType(string name) {  //tells which data type a name has
	if (isBoolean(name)) {
		return BOOLEAN;
	} else if (isInteger(name)) {
		return INTEGER;
	} 

	// name is an identifier and hopefully a constant
	// if symbolTable[name] is defined then data type = type of symbolTable[name]
	for (uint i = 0; i < symbolTable.size(); i++) {
		if (symbolTable.at(i).externalName == name || symbolTable.at(i).internalName == name) {
			return symbolTable.at(i).dataType;
		}
	}
	
	throw "reference to undefined variable";
}

string WhichValue(string name) {  //tells which value a name has
	if (isBoolean(name)) {
		return (name == "true") ? "1" : "0";
	} else if (isInteger(name)) {
		return name;
	} 

	// name is an identifier and hopefully a constant
	for (uint i = 0; i < symbolTable.size(); i++) {
		if (symbolTable.at(i).externalName == name || symbolTable.at(i).internalName == name) {
			if (symbolTable.at(i).mode == CONSTANT){
				return symbolTable.at(i).value;
			} else {
				return name;
			}
		}
	}
	
	throw "reference to undefined variable";
}

string GenInternalName(storeType type) {
	static int booleans = 0;
	static int integers = 0;
	
	string internName;
	
	switch(type) {
		case PROG_NAME: 
			internName = "P0";
			break;
		case INTEGER:
			internName =  "I" + to_string(integers);
			integers++;
			break;
		case BOOLEAN:
			internName = "B" + to_string(booleans);
			booleans++;
			break;
		case UNKNOWN:
			break;
	}
	
	return internName;
}

void Code(string optr, string rhs, string lhs) {
	// NOTE: rhs = operand1
	//       lhs = operand2
	
	// cout << lhs << ' ' << rhs << ' ' << optr << ' ' << contentOfA << endl;
	
	lhs = lhs.substr(0,15);
	rhs = rhs.substr(0,15);
	
	if (optr == "program") {
		printAssembly("STRT", "NOP", "", "", rhs + " - Nickolas Gupton & Zach Nadeau");
	} else if (optr == "end") {
		// EmitEndCode();
		printAssembly("", "HLT", "", "", "");
		for (entry& ent : symbolTable) {
			if (ent.alloc == YES) {
				if (ent.mode == CONSTANT) {
					printAssembly(ent.internalName, "DEC", ent.value, "", ent.externalName);
				}
				if (ent.mode == VARIABLE) {
					printAssembly(ent.internalName, "BSS", "0001", "", ent.externalName);
				}
			}
		}
		printAssembly("", "END", "STRT", "", "");
	} else if (optr == "read") {
		// EmitReadCode();
		if (!isInSymbolTable(rhs)) {
			throw "reference to undefined variable";
		} if (symbolTable.at(indexOfVariable(rhs)).mode != VARIABLE) {
			throw "reading in of read-only location";
		}
		printAssembly("", "RDI", WhichInternName(rhs), "", "read(" + rhs + ")");
	} else if (optr == "write") {
		// EmitWriteCode();
		printAssembly("", "PRI", WhichInternName(rhs), "", "write(" + rhs + ")");
	} else if (optr == "+") {
		EmitAdditionCode(rhs, lhs);
	} else if (optr == "-") {
		EmitSubtractionCode(rhs, lhs);
	} else if (optr == "neg") {
		EmitNegationCode(rhs);
	} else if (optr == "not") {
		EmitNotCode(rhs);
	} else if (optr == "*") {
		EmitMultiplicationCode(rhs, lhs);
	} else if (optr == "div") {
		EmitDivisionCode(rhs, lhs);
	} else if (optr == "mod") {
		EmitModuloCode(rhs, lhs);
	} else if (optr == "and") {
		EmitAndCode(rhs, lhs);
	} else if (optr == "or") {
		EmitOrCode(rhs, lhs);
	} else if (optr == "=") {
		EmitEqualsCode(rhs, lhs);
	} else if (optr == "<") {
		EmitLessThanCode(rhs, lhs);
	} else if (optr == ">") {
		EmitGreaterThanCode(rhs, lhs);
	} else if (optr == ">=") {
		EmitGreaterOrEqualsCode(rhs, lhs);
	} else if (optr == "<=") {
		EmitLessOrEqualsCode(rhs, lhs);
	} else if (optr == "<>") {
		EmitNotEqualsCode(rhs, lhs);
	} else if (optr == ":=") {
		EmitAssignCode(rhs, lhs);
	} else if (optr == "then") {
		EmitThenCode(rhs);
	} else if (optr == "else") {
		EmitElseCode(rhs);
	} else if (optr == "post_if") {
		EmitPostIfCode(rhs);
	} else if (optr == "while") {
		EmitWhileCode();
	} else if (optr == "do") {
		EmitDoCode(rhs);
	} else if (optr == "post_while") {
		EmitPostWhileCode(rhs, lhs);
	} else if (optr == "repeat") {
		EmitRepeatCode();
	} else if (optr == "until") {
		EmitUntilCode(rhs, lhs);
	} else {
		throw "undefined operation";
	}
}

void PushOperator(string optr) {
	operatorStk.push(optr);
}

void PushOperand(string operand) {
	if ((isBoolean(operand) || isInteger(operand)) && !isInSymbolTable(operand)) {
		Insert(operand,WhichType(operand),CONSTANT,WhichValue(operand),YES,1);
	}
	operandStk.push(WhichInternName(operand));
}

void PushBeginEnd(string str) {
	beginEndStk.push(str);
}

string PopOperator() {  // Poperator
	if (!operatorStk.empty()) {
		string temp = operatorStk.top();
		operatorStk.pop();
		return temp;
	}
	throw "operator stack underflow";
}

string PopOperand() {  // Poperand
	if (!operandStk.empty()) {
		string temp = operandStk.top();
		operandStk.pop();
		return temp;
	}
	throw "operand stack underflow";
}

string PopBeginEnd() {
	if (!beginEndStk.empty()) {
		string temp = beginEndStk.top();
		beginEndStk.pop();
		return temp;
	}
	throw "begin-end stack underflow; check for unmatched 'end'";
}

void FreeTemp() {
	currentTempNo--;
	if (currentTempNo < -1) {
		throw "compiler error, currentTempNo < -1";
	}
}

string GetTemp() {
	string temp;
	currentTempNo++;
	temp = "T" + to_string(currentTempNo);
	if (currentTempNo > maxTempNo) {
		Insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
		maxTempNo++;
	}
	return temp;
}

void EmitAdditionCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER || WhichType(rhs) != INTEGER) {
		throw "illegal type for addition operation";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) == 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) != 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs && contentOfA != rhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory addition
	printAssembly("", "IAD", ((contentOfA == lhs) ? rhs : lhs), "", WhichExternName(lhs) + " + " + WhichExternName(rhs));
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the addition
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = INTEGER;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitSubtractionCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER || WhichType(rhs) != INTEGER) {
		throw "illegal type for subraction operation";
	}
	
	// if A-reg holds a temp variable that is not lhs
	if (!contentOfA.empty() && contentOfA.at(0) == 'T' && contentOfA != lhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is not lhs
	if (!contentOfA.empty() && contentOfA.at(0) != 'T' && contentOfA != lhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if lhs is not in A-reg, load it into A-reg
	if (contentOfA != lhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory subraction
	printAssembly("", "ISB", rhs, "", WhichExternName(lhs) + " - " + WhichExternName(rhs));
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the subraction
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = INTEGER;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitNotCode(string rhs) {
	if (WhichType(rhs) != BOOLEAN) {
		throw "illegal type for 'not' operation";
	}
	
	// if A-reg holds a temp variable that is not rhs
	if (!contentOfA.empty() && contentOfA.at(0) == 'T' && contentOfA != "TRUE" && contentOfA != rhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is not rhs
	if (!contentOfA.empty() && contentOfA.at(0) != 'T' && contentOfA != "TRUE" && contentOfA != rhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if rhs is not in A-reg, load it into A-reg
	if (contentOfA != rhs) {
		contentOfA = rhs;
		printAssembly("", "LDA", rhs, "", "");
	}
	
	// print jumps to test if boolean stores true or false
	string loc = getNextLocation();
	printAssembly("", "AZJ", loc, "", "not " + WhichExternName(rhs));
	printAssembly("", "LDA", "FALS", "", "");
	if (!isInSymbolTable("false")) {
		Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}
	printAssembly("", "UNJ", loc, "+1", "");
	printAssembly(loc, "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the negation
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitNegationCode(string rhs) {
	if (WhichType(rhs) != INTEGER) {
		throw "illegal type for negation operation";
	}
	
	// if A-reg holds a temp variable that is not rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE")) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is not rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != rhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if rhs is not in A-reg, load it into A-reg
	printAssembly("", "LDA", "ZERO", "", "");
	contentOfA = "ZERO";
	if (!isInSymbolTable("ZERO")) {
		Insert("ZERO", INTEGER, CONSTANT, "0", YES, 1);
	}
	
	// perform register-memory multiplication

	printAssembly("", "ISB", rhs, "", "-" + WhichExternName(rhs));
	
	// deassign all temporary variables involved
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the negation
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = INTEGER;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitMultiplicationCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER || WhichType(rhs) != INTEGER) {
		throw "illegal type for multiplication operation";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && (contentOfA != lhs && contentOfA != rhs)) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && (contentOfA != lhs && contentOfA != rhs)) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs && contentOfA != rhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory multiplication
	printAssembly("", "IMU", ((contentOfA == lhs) ? rhs : lhs), "", WhichExternName(lhs) + " * " + WhichExternName(rhs));
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the multiplication
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = INTEGER;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitDivisionCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER || WhichType(rhs) != INTEGER) {
	  throw "illegal type for division operation";
	}
   
	// if A-reg holds a temp variable that is not lhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is not lhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if lhs is not in A-reg, load it into A-reg
	if (contentOfA != lhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory division
	printAssembly("", "IDV", rhs, "", WhichExternName(lhs) + " div " + WhichExternName(rhs));
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T') {
		FreeTemp();
	}
	if (rhs.at(0) == 'T') {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the division
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = INTEGER;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitModuloCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER || WhichType(rhs) != INTEGER) {
	  throw "illegal type for modulo operation";
	}
   
	// if A-reg holds a temp variable that is not lhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is not lhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if lhs is not in A-reg, load it into A-reg
	if (contentOfA != lhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory division
	printAssembly("", "IDV", rhs, "", WhichExternName(lhs) + " mod " + WhichExternName(rhs));
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// Q-reg contains the remainder, so store the remainder in a temporary variable and load that variable to A-reg
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
	symbolTable.at(indexOfVariable(contentOfA)).units = 1;
	printAssembly("", "STQ", contentOfA, "", "store remainder in memory");
	printAssembly("", "LDA", contentOfA, "", "load remainder from memory");
	symbolTable.at(indexOfVariable(contentOfA)).dataType = INTEGER;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitAndCode(string rhs, string lhs) {
	if (WhichType(lhs) != BOOLEAN || WhichType(rhs) != BOOLEAN) {
		throw "illegal type for 'and' operation";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && (contentOfA != lhs && contentOfA != rhs)) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) != 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs && contentOfA != rhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory multiplication
	printAssembly("", "IMU", ((contentOfA == lhs) ? rhs : lhs), "", WhichExternName(lhs) + " and " + WhichExternName(rhs));
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the and operation
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitOrCode(string rhs, string lhs) {
	if (WhichType(lhs) != BOOLEAN || WhichType(rhs) != BOOLEAN) {
		throw "illegal type for 'or' operation";
	}
   
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) == 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) != 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs && contentOfA != rhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory addition
	printAssembly("", "IAD", ((contentOfA == lhs) ? rhs : lhs), "", WhichExternName(lhs) + " or " + WhichExternName(rhs));
	
	string loc = getNextLocation();
	printAssembly("", "AZJ", loc, "+1", "Jump if 0");
	printAssembly(loc, "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the or operation
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitEqualsCode(string rhs, string lhs) {
	if (WhichType(lhs) != WhichType(rhs)) {
		throw "incompatible types for equals comparison";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) == 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) != 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs && contentOfA != rhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory subtraction
	printAssembly("", "ISB", ((contentOfA == lhs) ? rhs : lhs), "", WhichExternName(lhs) + " = " + WhichExternName(rhs));
	
	// print jumps to test if result of the subtraction is zero
	string loc = getNextLocation();
	printAssembly("", "AZJ", loc, "", "");
	printAssembly("", "LDA", "FALS", "", "");
	if (!isInSymbolTable("false")) {
		Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}
	printAssembly("", "UNJ", loc, "+1", "");
	printAssembly(loc, "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the equals comparison
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitLessThanCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER && WhichType(rhs) != INTEGER) {
		throw "both types must be integers for the '<' operator";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory subtraction
	printAssembly("", "ISB", rhs, "", WhichExternName(lhs) + " < " + WhichExternName(rhs));
	
	// print jumps to test if result of the subtraction is zero
	string loc = getNextLocation();
	printAssembly("", "AMJ", loc, "", "");
	printAssembly("", "LDA", "FALS", "", "");
	if (!isInSymbolTable("false")) {
		Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}
	printAssembly("", "UNJ", loc, "+1", "");
	printAssembly(loc, "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the equals comparison
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitGreaterThanCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER && WhichType(rhs) != INTEGER) {
		throw "both types must be integers for the '>' operator";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != rhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != rhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != rhs) {
		contentOfA = rhs;
		printAssembly("", "LDA", rhs, "", "");
	}
	
	// perform register-memory subtraction
	printAssembly("", "ISB", lhs, "", WhichExternName(lhs) + " > " + WhichExternName(rhs));
	
	// print jumps to test if result of the subtraction is zero
	string loc = getNextLocation();
	printAssembly("", "AMJ", loc, "", "");
	printAssembly("", "LDA", "FALS", "", "");
	if (!isInSymbolTable("false")) {
		Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}
	printAssembly("", "UNJ", loc, "+1", "");
	printAssembly(loc, "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the equals comparison
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitLessOrEqualsCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER && WhichType(rhs) != INTEGER) {
		throw "both types must be integers for the '<=' operator";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != rhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != rhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != rhs) {
		contentOfA = rhs;
		printAssembly("", "LDA", rhs, "", "");
	}
	
	// perform register-memory subtraction
	printAssembly("", "ISB", lhs, "", WhichExternName(lhs) + " <= " + WhichExternName(rhs));
	
	// print jumps to test if result of the subtraction is zero
	string loc = getNextLocation();
	printAssembly("", "AMJ", loc, "", "");
	printAssembly("", "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	printAssembly("", "UNJ", loc, "+1", "");
	printAssembly(loc, "LDA", "FALS", "", "");
	if (!isInSymbolTable("false")) {
		Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the equals comparison
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitGreaterOrEqualsCode(string rhs, string lhs) {
	if (WhichType(lhs) != INTEGER && WhichType(rhs) != INTEGER) {
		throw "both types must be integers for the '>=' operator";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && (contentOfA.at(0) == 'T' && contentOfA != "TRUE") && contentOfA != lhs) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory subtraction
	printAssembly("", "ISB", rhs, "", WhichExternName(lhs) + " >= " + WhichExternName(rhs));
	
	// print jumps to test if result of the subtraction is zero
	string loc = getNextLocation();
	printAssembly("", "AMJ", loc, "", "");
	printAssembly("", "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	printAssembly("", "UNJ", loc, "+1", "");
	printAssembly(loc, "LDA", "FALS", "", "");
	if (!isInSymbolTable("false")) {
		Insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the equals comparison
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitNotEqualsCode(string rhs, string lhs) {
	if (WhichType(lhs) != WhichType(rhs)) {
		throw "incompatible types for not equals comparison";
	}
	
	// if A-reg holds a temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) == 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		symbolTable.at(indexOfVariable(contentOfA)).alloc = YES;  // store the temp variable in the symbol table
		symbolTable.at(indexOfVariable(contentOfA)).units = 1;
		printAssembly("", "STA", contentOfA, "", "deassign AReg");  // emit code to store temp
		contentOfA = "";  // deassign the A-reg
	}
	
	// if A-reg holds a non-temp variable that is neither lhs or rhs
	if (!contentOfA.empty() && contentOfA.at(0) != 'T' && (contentOfA != lhs && contentOfA != rhs)) {
		contentOfA = "";  // deassign the A-reg
	}
	
	// if neither lhs nor rhs is in A-reg, load lhs into A-reg
	if (contentOfA != lhs && contentOfA != rhs) {
		contentOfA = lhs;
		printAssembly("", "LDA", lhs, "", "");
	}
	
	// perform register-memory subtraction
	printAssembly("", "ISB", ((contentOfA == lhs) ? rhs : lhs), "", WhichExternName(lhs) + " <> " + WhichExternName(rhs));
	
	// print jumps to test if result of the subtraction is zero
	string loc = getNextLocation();
	printAssembly("", "AZJ", loc, "+1", "");
	printAssembly(loc, "LDA", "TRUE", "", "");
	if (!isInSymbolTable("true")) {
		Insert("true", BOOLEAN, CONSTANT, "1", YES, 1);
	}
	
	// deassign all temporary variables involved
	if (lhs.at(0) == 'T' && lhs != "TRUE") {
		FreeTemp();
	}
	if (rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
	
	// A-reg now contains a new temporary variable, the result of the not equals comparison
	contentOfA = GetTemp();
	symbolTable.at(indexOfVariable(contentOfA)).dataType = BOOLEAN;
	
	// Push the new temporary variable onto the stack
	PushOperand(contentOfA);
}

void EmitAssignCode(string rhs, string lhs) {
	if (WhichType(lhs) != WhichType(rhs)) {
		throw "incompatible types for assignment operation";
	}
	
	if (symbolTable.at(indexOfVariable(lhs)).mode != VARIABLE) {
		throw "symbol on left-hand side of assignment must have a storage mode of VARIABLE";
	}
	
	if (lhs == rhs) {
		return;
	} else if (rhs != contentOfA) {
		printAssembly("", "LDA", rhs, "", "");
	}
	contentOfA = lhs;
	
	printAssembly("", "STA", lhs, "", WhichExternName(lhs) + " := " + WhichExternName(rhs));
	
	if (!rhs.empty() && rhs.at(0) == 'T' && rhs != "TRUE") {
		FreeTemp();
	}
}

void EmitThenCode(string opnd) {
	if (WhichType(opnd) != BOOLEAN) {
		throw "predicate of if statement must be boolean valued";
	}
	string tempLabel = getNextLocation();
	
	if (contentOfA != opnd) {
		printAssembly("", "LDA", opnd, "", "");		
	}
	
	printAssembly("", "AZJ", tempLabel, "", "if false jump to " + tempLabel);
	PushOperand(tempLabel);
	if (opnd.at(0) == 'T' && opnd != "TRUE") {
		FreeTemp();
	}
	contentOfA = "";
}

void EmitElseCode(string opnd) {
	string tempLabel = getNextLocation();
	printAssembly("", "UNJ", tempLabel, "", "jump to end if");
	printAssembly(opnd, "NOP", "", "", "else");
	PushOperand(tempLabel);
	contentOfA = "";
}

void EmitPostIfCode(string opnd) {
	printAssembly(opnd, "NOP", "", "", "end if");
	contentOfA = "";
}

void EmitWhileCode() {
	string tempLabel = getNextLocation();
	printAssembly(tempLabel, "NOP", "", "", "while");
	PushOperand(tempLabel);
	contentOfA = "";
}

void EmitDoCode(string opnd) {
	if (WhichType(opnd) != BOOLEAN) {
		throw "predicate of while statement must be boolean valued";
	}
	
	if (contentOfA != opnd) {
		printAssembly("", "LDA", opnd, "", "");		
	}
	
	string tempLabel = getNextLocation();
	printAssembly("", "AZJ", tempLabel, "", "do");
	PushOperand(tempLabel);
	if (opnd.at(0) == 'T' && opnd != "TRUE") {
		FreeTemp();
	}
	contentOfA = "";
}

void EmitPostWhileCode(string rhs, string lhs) {
	printAssembly("", "UNJ", lhs, "", "end while");
	printAssembly(rhs, "NOP", "", "", "");
	contentOfA = "";
}

void EmitRepeatCode() {
	string tempLabel = getNextLocation();
	printAssembly(tempLabel, "NOP", "", "", "repeat");
	PushOperand(tempLabel);
	contentOfA = "";
}

void EmitUntilCode(string opnd, string label) {
	if (WhichType(opnd) != BOOLEAN) {
		throw "predicate of until statement must be boolean valued";
	}
	
	if (contentOfA != opnd) {
		printAssembly("", "LDA", opnd, "", "");		
	}
	
	printAssembly("", "AZJ", label, "", "until");
	if (opnd.at(0) == 'T' && opnd != "TRUE") {
		FreeTemp();
	}
	contentOfA = "";
}


///////////////////////// Lexical Scanner /////////////////////////
string NextToken() { //returns the next token or end of file marker
	token = "";
	while (token == "") {
		if(charac == '{') { //process comment
			while (NextChar() != '}') {
				if (charac == END_OF_FILE) {
					//process error: unexpected end of file
					throw "unexpected end of file";
				}
			}
			
			NextChar();
		} else if(charac == '}') {
			//process error: '}' cannot begin token
			throw "'}' cannot begin token";
			NextChar();
		} else if(isspace(charac)) {
			NextChar();
		} else if(isSpecSym(charac)) {
			token += charac;
			NextChar();
			if (token == "<") {
				if (charac == '=' || charac == '>') {
					token += charac;
					NextChar();
				}
			} else if (token == ">" || token == ":") {
				if (charac == '=') {
					token += charac;
					NextChar();
				}
			}
		} else if(islower(charac)) {
			token += charac;
			while (islower(NextChar()) || isdigit(charac) || charac == '_') {
				if (token.back() == '_' && charac == '_') {
					throw "encountered consecutive underscores";
				}
				token += charac;
			}
			
			if (*(token.end()-1) == '_') {
				//process error: '_' cannot end token
				throw "'_' cannot end token";
			}
		} else if(isdigit(charac)) {
			token += charac;
			while (isdigit(NextChar())) {
				token += charac;
			}
		} else if(charac == END_OF_FILE) {
			token += charac;
		} else {
			//process error: illegal symbol
			throw "illegal symbol";
		}
	}
	
	// cout << token << endl;
	
	return token;
}

char NextChar() { //returns the next character or end of file marker
	sourceFile.get(charac);
	
	static char prevChar = '\n';
	
	if (sourceFile.eof()) {
		charac = END_OF_FILE;
		return charac;
	} else {
		if (prevChar == '\n') {
			listingFile << setw(5) << ++currentLine << '|';
		}
		listingFile << charac;
	}
	
	prevChar = charac;
	return charac;
}


///////////////////////// "Glue" Functions /////////////////////////
bool isKeyword(string str) {
	if (str == "program"
		|| str == "begin"
		|| str == "end"
		|| str == "var"
		|| str == "const"
		|| str == "integer"
		|| str == "boolean"
		|| str == "true"
		|| str == "false"
		|| str == "not"
		|| str == "mod"
		|| str == "div"
		|| str == "and"
		|| str == "or"
		|| str == "read"
		|| str == "write"
		|| str == "if"
		|| str == "then"
		|| str == "else"
		|| str == "repeat"
		|| str == "while"
		|| str == "do"
		|| str == "until") {
			return true;
	}
	return false;
}

bool isSpecToken(string str) {
	if (isSpecSym(str)
		|| str == ":="
		|| str == "<>"
		|| str == "<="
		|| str == ">=") {
			return true;
	}
	return false;
}

bool isSpecSym(string str) {
	if (str == "="
		|| str == ":"
		|| str == ","
		|| str == ";"
		|| str == "."
		|| str == "+"
		|| str == "-"
		|| str == "*"
		|| str == "<"
		|| str == ">"
		|| str == "("
		|| str == ")") {
			return true;
	}
	return false;
}

bool isSpecSym(char str) {
	if (str == '='
		|| str == ':'
		|| str == ','
		|| str == ';'
		|| str == '.'
		|| str == '+'
		|| str == '-'
		|| str == '*'
		|| str == '<'
		|| str == '>'
		|| str == '('
		|| str == ')') {
			return true;
	}
	return false;
}

bool isInteger(string str) {  // Does "+1" work?
	try {
		stoi(str);
	} catch (const invalid_argument& ia) {
		return false;
	}
	return true;
}

bool isBoolean(string str) {
	if (str == "true" || str == "false") {
		return true;
	}
	return false;
}

bool isNonKeyID(string str) {
	return !(isKeyword(str) || isSpecToken(str) || isInteger(str));
}

string printDataType(int i) {
	switch (symbolTable.at(i).dataType) {
		case INTEGER:
			return "INTEGER";
		case BOOLEAN:
			return "BOOLEAN";
		case PROG_NAME:
			return "PROG_NAME";
		case UNKNOWN:
			return "UNKNOWN";
	}
   
   return "THIS SHOULDN'T HAPPEN";
}

string printMode(int i) {
	switch (symbolTable.at(i).mode) {
		case VARIABLE:
			return "VARIABLE";
		case CONSTANT:
			return "CONSTANT";
	}
   
   return "THIS SHOULDN'T HAPPEN";
}

string printAlloc(int i) {
	switch (symbolTable.at(i).alloc) {
		case YES:
			return "YES";
		case NO:
			return "NO";
	}
   
   return "THIS SHOULDN'T HAPPEN";
}

string WhichInternName(string name) {  //tells which internalName a given externalName has
	name = name.substr(0,15);
	if (!name.empty() && name.at(0) == 'L') {
		return name;
	}
	for (uint i = 0; i < symbolTable.size(); i++) {
		if (symbolTable.at(i).externalName == name) {
			return symbolTable.at(i).internalName;
		}
	}
	throw "reference to undefined variable";
}

string WhichExternName(string name) {  //tells which externalName a given internalName has
	for (uint i = 0; i < symbolTable.size(); i++) {
		if (symbolTable.at(i).internalName == name) {
			return symbolTable.at(i).externalName;
		}
	}
	throw "reference to undefined variable";
	
}

bool isInSymbolTable(string name) {
	name = name.substr(0,15);
	for (uint i = 0; i < symbolTable.size(); i++) {
		if (symbolTable.at(i).externalName == name) {
			return true;
		}
	}
	return false;
}

uint indexOfVariable(string name) { // works for internal name or external name
	name = name.substr(0,15);
	for (uint i = 0; i < symbolTable.size(); i++) {
		if (symbolTable.at(i).externalName == name || symbolTable.at(i).internalName == name) {
			return i;
		}
	}
	return stoi(to_string(stoi("-1")));  // You found the hidden unoptimized return. You Win!
}

bool isRelOp(string name) {
	if (name == "="
	    || name == "<>"
		|| name == "<="
		|| name == ">="
		|| name == "<"
		|| name == ">") {
			return true;
	}
	return false;
}

bool isAddLevelOp(string name) {
	if (name == "+"
	    || name == "-"
		|| name == "or") {
			return true;
	}
	return false;
}
bool isMultLevelOp(string name) {
	if (name == "*"
	    || name == "div"
		|| name == "mod"
		|| name == "and") {
			return true;
	}
	return false;
}

void printAssembly(string location, string opCode, string address, string signNumber, string remarks) {
	objectFile << left << setw(6) << location; 
	objectFile << left << setw(4) << opCode;
	
	if (address != "" && isdigit(address.at(0))) { // if its a number you want to fill the left side with 0's
		objectFile << setfill('0') << setw(4) << right << address << setfill(' ');
	} else if (address != "" && address.at(0) == '-') {
		address.erase(address.begin());
		objectFile << '-' << setfill('0') << setw(3) << right << address << setfill(' ');
	} else {
		objectFile << left << setw(4) << address;
	}
	
	objectFile << left << setw(5) << signNumber; 
	objectFile << remarks << endl;
}

string getNextLocation() {
   static int currentLocCount = 0;
   return "L" + to_string(currentLocCount++);
}
