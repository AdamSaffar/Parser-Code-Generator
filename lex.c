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
#include <ctype.h> 

#define MAX_TOKENS 1000


// TokenType Enumeration
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

//struct to store token data(used for token list)
typedef struct {
  TokenType type; // token type
  char lexeme[50]; // for identifiers
  int value; // for numbers
} Token;

typedef struct reservedWord {
  char *word;
  TokenType Tokensymbol;
} reservedWord;

//reserved word list Token numbers 20-34
reservedWord reservedWords[] = {
{"begin",beginsym},
{"end", endsym},
{"if", ifsym},
{"fi", fisym},
{"then", thensym},
{"while", whilesym},
{"do", dosym},
{"call", callsym},
{"const", constsym},
{"var", varsym},
{"procedure", procsym},
{"write", writesym},
{"read", readsym},
{"else", elsesym},
{"even", evensym},
{NULL, 0} 
};

// Function to print the lexeme and token type
void printFun(FILE *out , char *lexeme, TokenType token) {
  fprintf(out, "%s\t%d\n", lexeme, token);
}

int main(int argc, char *argv[]) {
  //if the argument count is not exactly one, print a message and exit
  if (argc != 2) {
    printf("Error, incorrect number of arguments\n");
    return -1;
  }
  //open text file
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    printf("Error opening file\n"); 
    return -1;
  }
  //open the output file
  FILE *out = fopen("tokens.txt", "w");
  if(out == NULL) {
    printf("Error creating output file\n");
    return -1;
  }
  Token tokenList[MAX_TOKENS]; //stores token number of each lexeme
  int tokenCount = 0; // used to iterate through the tokenList array
  int ch; 
  char buffer[256]; // temporary token buffer
  int i = 0;

  //fprintf(out, "Source Program:\n\n");
  //loop to print the contents of the file
  /*
  while ((ch = fgetc(fp)) != EOF) {
    fprintf(out, "%c",ch);
  }
  */

  //fprintf(out, "\n");
  rewind(fp);  // move the file pointer back to the beginning of the file
  //fprintf(out, "Lexeme Table:\n\n");
  //fprintf(out, "lexeme\ttoken type\n");

  //read/retrieve until end of file
  while ((ch = fgetc(fp)) != EOF) {
    // ignores any whitespace including(space, tab, newline, and carriage return)
    if (isspace(ch))  {
      continue; //skip
      }
    //check if character is a letter, if so, check reserved words list. If not a recognized word, return Identifier.
    else if (isalpha(ch)) {
      int count = 0;
      buffer[count++] = ch; // ch is the first letter of the token
      int next;
      // if the next character is a letter or a digit
      while ((next = fgetc(fp)) != EOF && (isalpha(next) || isdigit(next)) ) {
        buffer[count++] = next; // adds character to array
      }
      buffer[count] = '\0';  // null terminate
      
      // If identifier exceeds 11 characters
      if (count > 11) {
        //fprintf(out, "%s\tIdentifier too long\n", buffer);
        tokenList[tokenCount++].type = skipsym; //skip
      } 
      else {
        // Check reserved words
        int found = 0;
        for (int j = 0; reservedWords[j].word != NULL; j++) {
          if (strcmp(buffer, reservedWords[j].word) == 0) { //compares buffer to reserved words list
            tokenList[tokenCount++].type = reservedWords[j].Tokensymbol;
            //printFun(out, buffer, reservedWords[j].Tokensymbol);
            found = 1;
            break;
          }
        }
        // if not part of reserved word list, must be identifier
        if (!found) {
          tokenList[tokenCount].type = identsym;
          //printFun(out, buffer, identsym);
          strcpy(tokenList[tokenCount].lexeme, buffer);
          tokenCount++;
        }
      }
      if (next != EOF) 
        ungetc(next, fp); // re-read last read character
    }
    // Check if character is a number(0-9)
    else if (isdigit(ch)) {
      int count = 0;
      buffer[count++] = ch; //ch = first digit of the token
      int next;
      // continues looping if next character is a digit
      while ((next = fgetc(fp)) != EOF && isdigit(next)) {
        buffer[count++] = next; // adds number to array
      }
      buffer[count] = '\0';
      // numbers exceeding 5 digits
      if(count > 5) {
        //fprintf(out, "%s\tNumber too long\n", buffer);
        tokenList[tokenCount++].type = skipsym;
      } 
      // If number doesn't exceed 5  digits, print number and set token to numbersym
      else {
        tokenList[tokenCount].type = numbersym;
        //atoi converts string to int
        tokenList[tokenCount].value = atoi(buffer); // We also want to print the lexeme number value in token list
        //printFun(out, buffer, numbersym);
        tokenCount++;
      }
      if (next != EOF) 
        ungetc(next, fp); // re-read last read character
    }
    //checks for any special symbols
    else { 
      switch(ch) {
        case '+':
          tokenList[tokenCount++].type = plussym;
          //printFun(out, "+", plussym);
          break;
        case '-':
          tokenList[tokenCount++].type = minussym;
          //printFun(out, "-", minussym);
          break;
        case '*':
          tokenList[tokenCount++].type = multsym;
          //printFun(out, "*", multsym);
          break;
        case '/': {
          int next = fgetc(fp);
          if (next == '*') {
            // Beginning of comment
            int prev = 0;
            int curr = 0;
            while ((curr = fgetc(fp)) != EOF) {
                if (prev == '*' && curr == '/') {
                    // End of comment reached
                    break;
                }
                prev = curr;
            }
            // if comment found, then skip entirely
            break;
          } 
          // if not a comment, must be a slash
          else { 
            if (next != EOF) {
              ungetc(next, fp); //re-read the last read character
            }
            tokenList[tokenCount++].type = slashsym;
            //printFun(out, "/", slashsym);
            break;
          }
        }
        case '=':
          tokenList[tokenCount++].type = eqsym;
          //printFun(out, "=", eqsym);
          break;
        case '<': {
          int next = fgetc(fp);
          // if next character is >, token type = neqsym(<>)
          if(next == '>') { 
            tokenList[tokenCount++].type = neqsym;
            //printFun(out, "<>",neqsym);
            break;
          } 
          // if next character is =, token type = leqsym(<=)
          else if(next == '=') {
            tokenList[tokenCount++].type = leqsym;
            //printFun(out, "<=",leqsym);
          } 
          //otherwise, its just lessym(<)
          else {
            ungetc(next, fp); // put back the character if not part of <= or <>
            tokenList[tokenCount++].type = lessym;
            //printFun(out, "<",lessym);
          }
          break;
        }
        case '>': {
          int next = fgetc(fp);
          // if next character is =, token type = geqsym(>=)
          if(next == '=') {
            tokenList[tokenCount++].type = geqsym;
            //printFun(out, ">=", geqsym);
            break;
          }
          //otherise, token type = gtrsym(>)
          else {
            ungetc(next, fp);
            tokenList[tokenCount++].type = gtrsym;
            //printFun(out, ">", gtrsym);
          }
          break;
        }
        case '(':
          tokenList[tokenCount++].type = lparentsym;
          //printFun(out, "(", lparentsym);
          break;
        case ')':
          tokenList[tokenCount++].type =  rparentsym;
          //printFun(out, ")", rparentsym);
          break;
        case ',':
          tokenList[tokenCount++].type = commasym;
          //printFun(out, ",", commasym);
          break;
        case ';':
          tokenList[tokenCount++].type = semicolonsym;
          //printFun(out, ";", semicolonsym);
          break;
        case '.':
          tokenList[tokenCount++].type = periodsym;
          //printFun(out, ".", periodsym);
          break;
        case ':': {
          int next = fgetc(fp);
          // if next character is =, token type = becomessym(:=)
          if(next == '=') {
            tokenList[tokenCount++].type = becomessym;
            //printFun(out, ":=", becomessym);
            break;
          }
          // Since the individual ":" symbol is not the list, it is an invalid symbol
          else {
            tokenList[tokenCount++].type = skipsym; // ignore
            //fprintf(out, "%c\tinvalid symbol\n", ch);
            break;
          }
        }

        default:
          // if character does not equal any of the above symbols, it is invalid
          tokenList[tokenCount++].type = skipsym; // ignore
          //fprintf(out, "%c\tinvalid symbol\n", ch);
          break;
      }
    }

  }
  //fprintf(out, "\n");
  //fprintf(out, "Token List:\n\n");
  // print token list
  for(int i = 0; i < tokenCount; i++) {
    fprintf(out, "%d ", tokenList[i].type);
    //If token type is an identifier, we also print the identifier
    if(tokenList[i].type == identsym) {
      fprintf(out, "%s ", tokenList[i].lexeme);
    }
    //If token type is a number, we also print the lexeme number
    else if(tokenList[i].type == numbersym) {
      fprintf(out, "%d ", tokenList[i].value);
    }
    fprintf(out,"\n");
  }
  fprintf(out, "\n");
  fclose(fp); // close input file
  fclose(out); // close output file
}
