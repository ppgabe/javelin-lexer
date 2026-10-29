# Javelin Language Lexer (jvlex)

## Overview

This project is a simple DFA-based implementation of the Javelin language's
lexer. The project is split into three parts:

1. **main.c** - The main entrypoint for the lexer.
2. **lexer.h** and **lexer.c** - DFA-based implementation of the lexer.
3. **tokens.h** - Header defining the `TokenType` enum and `Token` struct.

## Building from source

To build jvlex from source, you must have a C compiler capable of compiling
**C11** and up.

### On Linux with GCC

Simply run the following on your terminal:

```shell
gcc -Wall -Wextra -O2 *.c -o jvlex
```

### On macOS with Clang

```shell
clang -Wall -Wextra -O2 *.c -o jvlex
```

### On Windows...

_Use WSL2 to save your sanity and mine from configuring MinGW and/or VS Code
nonsense. On your WSL2 distro, simply run the same GCC command if appropriate._

_Or, if you're using CLion, open this folder as a project and run
it from there._


## Running the lexer

Once you have compiled the project successfully, you can now run it. jvlex
will only recognize Javelin source files ending with the `.jvln` extension.

### Run jvlex against a single Javelin source file

```shell
./jvlex sample1.jvln
```

### Or against multiple source files at once

```shell
./jvlex sample1.jvln sample2.jvln
```

## Viewing debug logs

If you wish to see a token-by-token log of the lexer, append the `--debug`
flag at the end of your command. For example:

```shell
./jvlex sample1.jvln sample2.jvln --debug
```

## Viewing the generated symbol table

jvlex generates the symbol table for each source file with the `.symb`
extension. You can open it with a text viewer, as it is simply a CSV
plaintext file.