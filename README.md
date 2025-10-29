# Parser-Code-Generator
This project implements a simple compiler/interpreter for a tiny language. It includes a lexical analyzer, parser, symbol table management, and code generation for arithmetic expressions, control flow (if, while), and I/O operations.
# Setup
1. Open a terminal or command prompt on your system.
2. Compile the lexical analyzer (scanner): gcc -O2 -std=c11 -o lex lex.c
3. Compile the parser and code generator: gcc -O2 -std=c11 -o parsercodegen parsercodegen.c
4. Prepare your input file: Place your source code in a text file (see included sample input files for reference, e.g., inputfile.txt).
5. Run the lexical analyzer to generate tokens: ./lex inputfile.txt
6. Run the parser and code generator to produce the output: ./parsercodegen
7. Check output files: elf.txt (generated numeric instructions)
