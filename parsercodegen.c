/*
Assignment:
HW3 - Parser and Code Generator for PL/0
Author(s): <Adam Saffar>
Language: C (only)
To Compile:
Scanner:
gcc -O2 -std=c11 -o lex lex.c
Parser/Code Generator:
gcc -O2 -std=c11 -o parsercodegen parsercodegen.c
To Execute (on Eustis):
./lex <input_file.txt>
./parsercodegen
where:
<input_file.txt> is the path to the PL/0 source program
Notes:
- lex.c accepts ONE command-line argument (input PL/0 source file)
- parsercodegen.c accepts NO command-line arguments
- Input filename is hard-coded in parsercodegen.c
- Implements recursive-descent parser for PL/0 grammar
- Generates PM/0 assembly code (see Appendix A for ISA)
- All development and testing performed on Eustis
Class: COP3402 - System Software - Fall 2025
Instructor: Dr. Jie Lin
Due Date: Friday, October 31, 2025 at 11:59 PM ET
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_NAME 12
#define MAX_CODE 1000

typedef enum {
skipsym = 1 , // Skip / ignore token
identsym , // Identifier
numbersym , // Number
plussym , // +
minussym , // -
multsym , // *
slashsym , // /
eqsym , // =
neqsym , // <>
lessym , // <
leqsym , // <=
gtrsym , // >
geqsym , // >=
lparentsym , // (
rparentsym , // )
commasym , // ,
semicolonsym , // ;
periodsym , // .
becomessym , // :=
beginsym , // begin
endsym , // end
ifsym , // if
fisym , // fi
thensym , // then
whilesym , // while
dosym , // do
callsym , // call
constsym , // const
varsym , // var
procsym , // procedure
writesym , // write
readsym , // read
elsesym , // else
evensym // even
} TokenType ;

typedef struct {
  int kind; // const = 1, var = 2, proc = 3
  char name[12]; // name up to 11 chars
  int val; // number (ASCII value)
  int level; // L level
  int addr; // M address
  int mark; // to indicate unavailable or deleted
} symbol;

// Code instruction
typedef struct {
  int op;
  int l;
  int m;
} Instruction;

// Opcodes
typedef enum {
  OP_LIT = 1,
  OP_OPR = 2,
  OP_LOD = 3,
  OP_STO = 4,
  OP_CAL = 5,
  OP_INC = 6,
  OP_JMP = 7,
  OP_JPC = 8,
  OP_SYS = 9
} OpCodes;

// OPR M codes
enum { 
  OPR_RTN = 0,
  OPR_ADD = 1,
  OPR_SUB = 2,
  OPR_MUL = 3,
  OPR_DIV = 4,
  OPR_EQL = 5,
  OPR_NEQ = 6,
  OPR_LSS = 7,
  OPR_LEQ = 8,
  OPR_GTR = 9,
  OPR_GEQ = 10,
  OPR_EVEN = 11
};

// Global Variables
FILE *fp = NULL;
FILE *out = NULL;

symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
int symbol_table_size = 0;
int sym_table_index = 0;

char currentIdent[MAX_NAME];
int currentNumber = 0;
int currentToken = -1;

Instruction code[MAX_CODE];
int code_index = 0;



// Helper function prototypes
void getNextToken(); 
int symbolTableCheck(const char *name); 
void addSymbol(int kind, const char *name, int val, int level, int addr); 
void emit(int op, int l, int m); 
void writeElf(const char *fname); 
void print_error(const char* message); 
void printAssembly(); 
void printSymbolTable();

// Parser function prototypes
void program();
void block(); 
void const_declaration(); 
int var_declaration(); 
void statement(); 
void condition(); 
void expression(); 
void term(); 
void factor(); 


int main(int argc) {
  // input file given by lexical analyzer
  fp = fopen("tokens.txt", "r");
  if(fp == NULL) {
    printf("Error opening file\n");
    return 1;
  }
  // output file, contains instructions but in numeric format (opcode, L, M)
  out = fopen("elf.txt", "w");
  if (out == NULL) {
    printf("Error opening output.txt\n");
    fclose(fp);
    return 1;
  }

  program(); // call parser and code generator

  printAssembly(); // output assembly instructions to terminal
  printSymbolTable(); // output symbol talble to terminal
  writeElf("elf.txt"); // output instruction in numeric format to file

  fclose(fp); // close input
  fclose(out); // close output
}
// prints formatted assembly code to terminal
void printAssembly() {
  printf("Assembly Code:\n");
  printf("Line\tOP   L   M\n");
  for(int i = 0; i < code_index; i++) {
    Instruction *ins = &code[i];
    const char *m = "UNK";
    switch(ins->op) {
      case OP_LIT: 
        m = "LIT";
        break;
      case OP_OPR:
        m = "OPR";
        break;
      case OP_LOD:
        m = "LOD";
        break;
      case OP_STO:
        m = "STO";
        break;
      case OP_CAL:
        m = "CAL";
        break;
      case OP_INC:
        m = "INC";
        break;
      case OP_JMP:
        m = "JMP";
        break;
      case OP_JPC:
        m = "JPC";
        break;
      case OP_SYS:
        m = "SYS";
        break;
      default:
        break;
    }
    printf("%3d %6s %3d %3d\n", i, m, ins->l, ins->m); 
  }
  printf("\n");
}
// prints formatted symbol table to terminal
void printSymbolTable() {
  printf("Symbol Table:\n");
  printf("Kind | Name\t| Value | Level | Address | Mark\n");
  printf("------------------------------------------------\n");
  //loops through symbol table, printing(kind,name,val,level,addr, and mark) at each index
  for(int i = 0; i < symbol_table_size; i++) {
    symbol *s = &symbol_table[i];
    printf("%4d |%9s |%6d |%6d |%8d |%5d\n", s->kind, s->name, s->val, s->level, s->addr, s->mark);
  }
  printf("\n");
}
// print instructions to elf.txt
void writeElf(const char *fname) {
  FILE *out = fopen(fname, "w"); // open output file for writing
  if(!out) {
    printf("Error opening file"); // print error if file cannot be opened
    return;
  }

  // write each instruction in numeric format: OP, L, M
  for(int i = 0; i < code_index; i++) {
    fprintf(out, "%d %d %d\n", code[i].op,code[i].l,code[i].m);
  }
  fclose(out);
}

void print_error(const char* message) {
  // prints message to terminal
  printf("%s\n", message);
  // prints message to file
  if(out != NULL) {
    fprintf(out, "%s\n", message);
    fflush(out);
  }
  exit(1); // if error encountered, exit program
}

// Adds a new symbol to the symbol table
void addSymbol(int kind, const char *name, int val, int level, int addr) {
  if(sym_table_index > MAX_SYMBOL_TABLE_SIZE) {
    print_error("Symbol table overflow.");
  }
  symbol *s = &symbol_table[symbol_table_size++]; // Get next free symbol slot

  s->kind = kind;
  strncpy(s->name,name,MAX_NAME-1); // copy name safely
  s->name[MAX_NAME-1] = '\0'; // null terminator
  s->val = val;
  s->level = level;
  s->addr = addr;
  s->mark = 0;
}

// emits instruction to the code array
void emit(int op, int l, int m) {
  if(code_index >= MAX_CODE) {
    print_error("Code overflow");
  }
  code[code_index].op = op; // set opcode
  code[code_index].l = l; // set level(always 0)
  code[code_index].m = m; // set modifier
  code_index++; // move to next instruction slot
}

// Reads the next token from tokens.txt and stores its value
void getNextToken() {
  while(1) {
    int ch = fscanf(fp,"%d", &currentToken); // get next token
    if(ch != 1) {
      currentToken = -1; // EOF reached
      return;
    }
    
    if (currentToken == identsym) {
      fscanf(fp, "%s", currentIdent); // read identifier name
    } 
    else if (currentToken == numbersym) {
      fscanf(fp, "%d", &currentNumber); // read number value
    }

    if (currentToken == skipsym) {
      // if lexer encounters skipsym(1), print error
      print_error("Error: Scanning error detected by lexer (skipsym present)"); 
    }

    break; // successfully read token, exit loop
  }
}

// Checks if a symbol exists in the symbol table
int symbolTableCheck(const char *name) {
  for(int i = 0; i < symbol_table_size; ++i) {
      if(strcmp(symbol_table[i].name, name) == 0) {
        return i; // symbol found, return its index
      }
  }
  return -1; // symbol not found
}

// <program> ::= <block> "."
void program(){
  emit(OP_JMP,0,0); // placeholder jump
  int jmpIdx = code_index - 1; // jmp index

  getNextToken();
  block();

  code[jmpIdx].m = 3; // jmp to start of main code
  if(currentToken != periodsym) {
    print_error("Error: program must end with period"); 
  }
  emit(OP_SYS,0,3); //SYS 0 3 halts program
}

// <block> ::= <const-declaration> <var-declaration> <statement>
void block() {
  const_declaration();
  int numVars = var_declaration();
  emit(OP_INC, 0, 3 + numVars);
  statement();
}

// <const-declaration> ::= [ "const" <ident> "=" <number> {"," <ident> "=" <number>} ";"]
void const_declaration() {
  if(currentToken == constsym) { // proceed if "const" present
    do {
      getNextToken();
      if(currentToken != identsym) {
        print_error(" Error: const, var, and read keywords must be followed by identifier");
      }
      // save identifer name
      char savedName[MAX_NAME];
      strncpy(savedName, currentIdent, MAX_NAME-1);
      savedName[MAX_NAME - 1] = '\0';

      if(symbolTableCheck(savedName) != -1) { // check if symbol exists
        print_error("Error: symbol name has already been declared");
      }
      getNextToken();
      if(currentToken != eqsym) {
        print_error("Error: constants must be assigned with =");
      }
      getNextToken();
      if(currentToken != numbersym) {
        print_error("Error: constants must be assigned an integer value");
      }

      int value = currentNumber;
      addSymbol(1, savedName, value, 0,0); // add constant to symbol table
      getNextToken();
    }while(currentToken == commasym); // handle multiple constants seperated by ","

    if(currentToken != semicolonsym) {
      print_error("Error: constant and variable declarations must be followed by a semicolon");
    }
    getNextToken();
  }
}

// <var-declaration> ::= [ "var" <ident> {"," <ident>} ";"]
int var_declaration() {
  int numVars = 0;
  if(currentToken == varsym) { // proceed if "var" present
    do {
      numVars++;
      getNextToken();
      if(currentToken != identsym){
        print_error("Error: const, var, and read keywords must be followed by identifier");
      }
      // save identifier name
      char savedName[MAX_NAME];
      strncpy(savedName, currentIdent, MAX_NAME-1);
      savedName[MAX_NAME - 1] = '\0';

      if(symbolTableCheck(savedName) != -1) { // check if symbol exists
        print_error("Error: symbol name has already been declared");
      }

      int addr = 2 + numVars;
      addSymbol(2,savedName,0,0,addr); // add variable to symbol table

      getNextToken();
    } while(currentToken == commasym); // handle multiple variables seperated by ","

    if(currentToken != semicolonsym) {
      print_error("Error: constant and variable declarations must be followed by a semicolon");
    }
    getNextToken();
  }
  return numVars; // return num of variables declared
}

/*
<statement> ::=
[
<ident> ":=" <expression>
| "begin" <statement> { ";" <statement> } "end"
| "if" <condition> "then" <statement> "fi"
| "while" <condition> "do" <statement>
| "read" <ident>
| "write" <expression>
| empty
]
*/
void statement() {
  if(currentToken == identsym) { // assignment statement

    // save identifier name 
    char savedName[MAX_NAME];
    strncpy(savedName,currentIdent, MAX_NAME-1);
    savedName[MAX_NAME-1] = '\0';

    int symIdx = symbolTableCheck(savedName); // check if variable exists
    if(symIdx == -1) {
      print_error("Error: undeclared identifier");
    }
    if(symbol_table[symIdx].kind != 2) { // variable 
      print_error("Error: only variable values may be altered");
    }
    getNextToken();
    if(currentToken != becomessym) {
      print_error("Error: assignment statements must use :=");
    }
    getNextToken();
    expression();

    symbol_table[symIdx].mark = 1; // mark as used
    emit(OP_STO, 0,symbol_table[symIdx].addr);

    return;
  }
  if(currentToken == beginsym) { // begin-end block
    do{
      getNextToken();
      statement();
    }while(currentToken == semicolonsym);

    if(currentToken != endsym) {
      print_error("Error: begin must be followed by end");
    }
    getNextToken();
    return;
  }
  if(currentToken == ifsym) { // if statement
    getNextToken();
    condition();

    int jpcIndex = code_index;
    emit(OP_JPC, 0, 0); // Jump if false 

    if(currentToken != thensym) {
      print_error("Error: if must be followed by then");
    }
    getNextToken();
    statement();

    if(currentToken == fisym){  // if "fi" encountered, ignore it
      getNextToken();
    }
    
    code[jpcIndex].m = thensym; // fill jump address
    return;
  }
  if(currentToken == whilesym) { // while loop
    getNextToken();
    int loopIdx = code_index; // start of loop
    condition();

    if(currentToken != dosym) {
      print_error("Error: while must be followed by do");
    }
    getNextToken();
    int jpcIndex = code_index;
    emit(OP_JPC, 0, 0); // jump if false
    statement();
    emit(OP_JMP, 0, loopIdx); // jump back to loop start
    code[jpcIndex].m = code_index; // fill jump address

    return;
  }
  if(currentToken == readsym) { // read statement
    getNextToken();
    if(currentToken != identsym) {
      print_error("Error: const, var, and read keywords must be followed by identifier");
    }
    // save identifier name
    char savedName[MAX_NAME];
    strncpy(savedName, currentIdent, MAX_NAME-1);
    savedName[MAX_NAME-1];

    int symIdx = symbolTableCheck(savedName); // check if symbol exists
    if(symIdx == -1) {
      print_error("Error: undeclared identifier");
    }
    if(symbol_table[symIdx].kind != 2) { // variable
      print_error("Error: only variable values may be altered");
    }
    getNextToken();
    emit(OP_SYS, 0, 2); // SYS 0 2 equals READ
    emit(OP_STO, 0, symbol_table[symIdx].addr); // store input in variable
    symbol_table[symIdx].mark = 1; // mark as used

    return;
  }
  if(currentToken == writesym) { // write statement
    getNextToken();
    expression();
    emit(OP_SYS, 0, 1); // SYS 0 1 equals WRITE
    return;
  }
}

// <condition> ::= "even" <expression> | <expression> <rel-op> <expression>
void condition() {
  if(currentToken == evensym) { // "even" condition
    getNextToken();
    expression();
    emit(OP_OPR, 0,OPR_EVEN);
  } else {
    // checks for relation operator(=, <>, <, <=, >, >=)
    expression();
    if(currentToken == eqsym) { // =
      getNextToken();
      expression();
      emit(OP_OPR, 0, OPR_EQL); 
    }
    else if(currentToken == neqsym) { // <>
      getNextToken();
      expression();
      emit(OP_OPR, 0, OPR_NEQ);
    }
    else if(currentToken == lessym) { // <
      getNextToken();
      expression();
      emit(OP_OPR, 0, OPR_LSS);
    }
    else if(currentToken == leqsym) { // <=
      getNextToken();
      expression();
      emit(OP_OPR,0,OPR_LEQ);
    }
    else if (currentToken == gtrsym) { // >
      getNextToken();
      expression();
      emit(OP_OPR, 0, OPR_GTR);
    }
    else if(currentToken == geqsym) { // >=
      getNextToken();
      expression();
      emit(OP_OPR,0,OPR_GEQ);
    }
    else {
      print_error("Error: condition must contain comparison operator"); //missing relation operator
    }
  }
}

// <expression> ::= <term> { ("+" | "-") <term> }
void expression() {
  term(); // parse first term

  // parse any addititonal + or - operators
  while(currentToken == plussym || currentToken == minussym) {
    if(currentToken == plussym) {
      getNextToken();
      term();
      emit(OP_OPR,0,OPR_ADD); // additition
    } else {
      getNextToken();
      term();
      emit(OP_OPR,0,OPR_SUB); // subtraction
    }
  }

}

// <term> ::= <factor> { ("*" | "/" ) <factor> }
void term() { 
  factor(); // parse first factor
  
  // parse any addititonal * or / operators
  while(currentToken == multsym || currentToken == slashsym) { 
    if(currentToken == multsym) {
      getNextToken();
      factor(); // parse next factor
      emit(OP_OPR,0,OPR_MUL); // multiplication
    }
    else if(currentToken == slashsym) {
      getNextToken();
      factor(); // parse next factor
      emit(OP_OPR,0,OPR_DIV); // division
    }
  }
}

// <factor> ::= <ident> | <number> | "(" <expression> ")"
void factor() {
  if(currentToken == identsym) {
    // save identifier name
    char savedName[MAX_NAME];
    strncpy(savedName, currentIdent, MAX_NAME - 1);
    savedName[MAX_NAME - 1] = '\0';

    int symIdx = symbolTableCheck(savedName); // check if symbol exists
    if(symIdx == -1) {
      print_error("Error: undeclared identifier");
    }
    if(symbol_table[symIdx].kind == 1) { // constant
      emit(OP_LIT,0,symbol_table[symIdx].val); // load constant value 
    }
    else { // variable
      emit(OP_LOD,0,symbol_table[symIdx].addr); // load value from mem
    }
    symbol_table[symIdx].mark = 1; // mark as used
    getNextToken();
  }
  else if(currentToken == numbersym) {
    emit(OP_LIT,0,currentNumber); // load literal number
    getNextToken();
  }
  else if(currentToken == lparentsym) { // paranthesized expression
    getNextToken();
    expression();
    if(currentToken != rparentsym) { // expect right parethensis ")"
      print_error("Error: right parenthesis must follow left parenthesis");
    }
    getNextToken();
  }
  else {
    print_error("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols");

  }
}