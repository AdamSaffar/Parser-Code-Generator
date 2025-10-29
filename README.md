# Parser-Code-Generator
This project implements a simple compiler/interpreter for a tiny language. It includes a lexical analyzer, parser, symbol table management, and code generation for arithmetic expressions, control flow (if, while), and I/O operations.

# Requirements
* A C compiler (e.g., GCC, Clang, or an IDE like Visual Studio, Code::Blocks, or Xcode).
* A text editor to create or edit input files (e.g., VS Code, Notepad++, or built-in editors).

# Setup and Usage
1. Open your C development environment: This could be a terminal with a C compiler or an IDE that can build C projects.
2. Compile the lexical analyzer (scanner):
   * If using a terminal with GCC/Clang: gcc -O2 -std=c11 -o lex lex.c
   * If using an IDE: create a new project, add lex.c, and build/compile to produce lex
4. Compile the parser and code generator:
   * Terminal: gcc -O2 -std=c11 -o parsercodegen parsercodegen.c
   * IDE: add parsercodegen.c to your project and build to produce the executable.
5. Prepare your input file: Place your source code in a text file (see included sample input file for reference, e.g., inputfile.txt).
6. Run the lexical analyzer to generate tokens:
   * Terminal: ./lex inputfile.txt
   * IDE: run the lex executable with inputfile.txt as the argument.
8. Run the parser and code generator to produce the output:
   * Terminal: ./parsercodegen
   * IDE: run the parsercodegen executable.
10. Check output files: elf.txt (contains numeric instructions for the virtual machine)
