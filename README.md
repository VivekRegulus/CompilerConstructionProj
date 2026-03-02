Compiler Construction Project 
Overview
This repository contains the implementation of Stage 1 for the Compiler Construction project. It includes a fully functional Lexical Analyzer and a Table-Driven LL(1) Syntax Analyzer (Parser) built from scratch in C. The compiler processes a custom procedural programming language, validates its syntax against a strict LL(1) grammar, and generates an N-ary Parse Tree.

Key Features
Lexical Analysis (DFA-based): Implements a highly efficient Lexer using a Twin Buffer mechanism (BUFFER_SIZE = 50) to minimize disk I/O overhead. It strictly enforces custom regex constraints for identifiers, fields, records, and real numbers.

Syntax Analysis (LL(1) Predictive Parser): Uses a stack-based predictive parsing algorithm driven by pre-computed FIRST and FOLLOW sets.

Robust Error Recovery (Panic Mode): Incorporates textbook LL(1) Panic Mode recovery. It uses synchronization tokens (;, end, endif, endwhile, etc.) to gracefully discard invalid token sequences, suppress cascading errors, and resume parsing without crashing.

N-ary Parse Tree Generation: Dynamically builds an Abstract Syntax Tree (AST) using a "First-Child / Next-Sibling" pointer architecture. The tree is exported using an In-Order traversal strictly matching the required 7-column format.

Performance Tracking: Includes an execution time measurement module using CPU clock ticks.

File Structure
driver.c: Contains the main menu loop and file I/O handling.

lexer.c / lexer.h / lexerDef.h: Contains the Twin Buffer logic, DFA state machine, and token generation.

parser.c / parser.h / parserDef.h: Contains the Stack ADT, Tree Node ADT, LL(1) Parse Table, Parsing Engine, and Tree printing logic.

makefile: Automated build script.

t1.txt - t10.txt: Comprehensive test cases (both valid and erroneous) to test the compiler's resilience.

How to Build & Run
1. Compile the Project:
Ensure you have gcc and make installed. Run the following command in the terminal:

Bash
make clean
make
2. Execute the Compiler:
Run the generated executable, providing a source code text file and an output file for the parse tree:

Bash
./stage1exe t1.txt parsetreeOutFile.txt
3. Main Menu Options:
Upon running, the driver will present the following menu:

Option 0: Exit the program.

Option 1: Strip comments (%) from the source file and print the clean code to the console.

Option 2: Run the Lexical Analyzer and print the generated Token Stream.

Option 3: Run the Syntax Analyzer, print any lexical/syntax errors, and output the generated Parse Tree to the provided output file.

Option 4: Measure and print the total CPU execution time taken by the Lexer and Parser combined.

Error Handling
The compiler explicitly handles both Lexical Errors (e.g., unknown symbols, variable names exceeding character limits) and Syntax Errors (e.g., terminal/non-terminal mismatches). Errors are printed sequentially to the console with accurate line numbers, even when statements cross multiple lines.
