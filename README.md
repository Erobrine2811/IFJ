# IFJ25 Compiler

## Overview
The IFJ25 project is a compiler for the IFJ25 language, designed to translate IFJ25 code into IFJcode25. This project includes a scanner that processes the input code, generates tokens, and handles errors.

## Project Structure
```
ifj25
├── src
│   ├── scanner.c       # Implementation of the scanner for the IFJ25 language
│   ├── scanner.h       # Header file defining types, enums, and function prototypes for the scanner
│   ├── helper.c        # Utility functions for safe memory allocation and reallocation
│   ├── helper.h        # Header file declaring utility functions for memory management
│   └── error.h         # Defines error codes used throughout the project
├── Makefile             # Rules and commands for compiling the C programs
└── README.md            # Documentation for the project
```

## Building the Project
To compile the project, navigate to the project directory and run the following command:

```
make
```

This will generate an executable named `ifj25`.

## Running the Compiler
After building the project, you can run the compiler with the following command:

```
./ifj25 <input_file>
```

Replace `<input_file>` with the path to the IFJ25 source code file you want to compile.

## Cleaning Up
To remove the compiled object files and the executable, use the following command:

```
make clean
```

This will delete all object files and the `ifj25` executable.