# Turing Machine
Fully reworked version of my [old college project](https://github.com/ToshaUwU/ArchiveCollegeProjects-TuringMachine) with the same name and purpose.

## Description
The library represents simple [Turing machine](https://en.wikipedia.org/wiki/Turing_machine) implementation. It exposes class [`TM::Program`](./lib/src/Program.hpp) for compilation sources into [finite-state machine](https://en.wikipedia.org/wiki/Finite-state_machine), class [`TM::Tape`](./lib/src/Tape.hpp) which represents pseudo-infinite and class [`TM::TuringMachine`](./lib/src/TuringMachine.hpp) that takes program to launch and tape on which it should perform exectuion.

## Syntax and programs creation
Program source code - is the set of [state definitions](#state-definition), following each other and defining the result state machine. If anythig goes wrong during the compilation, the very detailed error description will be returned, pointing to the concrete line and column where error occured.

### Alphabet
The source code is allowed to use any symbol from printable characters subset of ASCII + space, tab (`\t`) and new line (`\n`), with some exceptions:
  * Symbol `;` starts comment, which continues until the end of the line (`\n`). Comments can contain any symbols, even those that is not allowed in other parts of the program.
  * State names allowed to contain only alpha-numeric symbols (`[a-zA-Z0-9]`), dashes (`-`) and underscores (`_`).
  * `Move` have discrete set of symbols that could be used, each with it's own meaning (see [State definition](#state-definition) section below).

### State definition
State defined by the sequence of tokens: `<state_name> <key> <replace_with> <move> <next_state_name>`. This definition could be decomposed into two parts:
  - `<state_name> <key>` - state id. Must be unique pair of values. Used for lookup, made by taking `(current_state, current_symbol_on_tape)` pair.
  - `<replace_with> <move> <next_state_name>` - actions, that correspond to state id.

Tokens could be separated by one or more spaces, tabs or line endings, but must preserve the order and couldn't be omitted.

### Tokens description:
  - `<state_name>` - name of the state, being defined. Can't be _final state_ name (see `<next_state_name>`).
  - `<key>` - the symbol for which ate being defined. `*` corresponds to any symbol, but used only if exact match is not defined i.e. used as default if no symbols is matched.
  - `<replace_with>` - the symbol that will be written onto the tape. `*` leaves current symbol unchanged.
  - `<move>` - defines in which direction _head (tape pointer to current symbol)_ should be moved. Following values avialable:
    * `l/L/-` - move at one position to the left.
    * `r/R/+` - move at one position to the right.
    * `*/s/S/0` - stay at the current position (don't move).
  - `<next_state_name>` - name of the state that will be set as current. Could be equal to `<state_name>`. If name equal to `HALT`(case-insensitive), then this is the _final state_ which ends program exectuion.

### Examples
Example programs could be found in [`./programs`](./programs) directory.

## Architecture
- **`TuringMachine`** - hothing special, just takes _**references**_ to Tape and Program, and performing exectuion with Runtime errors check.

- **`Tape`** - simple abstraction on `std::vector` with some sprecific properties:
 * Forbids random access. You have _head_ that points to current symbol. You could get this symbol or move head by some offset. It could by any number that fits into `int8_t`. Negative values mean movement backward (to the left).
 * It _guarantees_ that tape is always valid and points to valid symbol. By automatical resize (preformed in $O(n)$ from initial size, new capacity is equal to `n * Tape::resize_policy`) and allocating some minimal amount of initial space (not less than `Tape::initial_size`).
 * `Tape::getString()` method that returns `const char *` in $O(1)$ time. This is C-string that shows all tape cells, that was visited at least once. There are `Tape::trimResundantSpaces()` method that trims all leading and trailing "spaces" in $O(n)$ time in worst case.

- **`Program`** - the most important and complicated part. It takes source code and converts it into internal finite-state machine format for efficent state-key lookup (performed in $O(\log(k))$, where k is states count). If source code contains errors, compilation will end with failure, providing detailed error description with exact line and column numbers where error is occured. For more information about internal structure see [Program internal architecture](#program-class-internal-architecture).

## Build and launch
Project doesn't have any third-party dependencies and could be build with only standard library and pure C++17. The main library resides in [`./lib`](./lib) directory and have cmake file, which will create `turingm` target and propagate public includes.

Also, there are simple command line tool in root directory which could be used for turing machine programs exectuion. In [`./programs`](./programs) directory lie some examples. Programs use `.tmc` extension (stands for _"turing machine code"_), but they are just plain text files.

You could pass arguments in command line tool to set some of the options. They are parsed in the following order:
1. `<path_to_program>` - path to file with source code. Default value is `"HelloWorld.tmc"` (no actual I/O operations is performed, code is embedded in tool itself).
2. `<begin_state_name>` - name of the state that serves as entry point. Default value is `"0"`.
3. `<default_tape_symbol>` - symbol that will be used to fill all the "empty" space in tape. Default value is `'_'`.
4. `<tape_initial_data>` - string that will be printed on tape _before_ program exectuion. Initial position of _head_ will point to first symbol of string. Default value is `""` (empty string);
5. `<iterations_limit>`

As mentioned, each of them has default value, therefore they could be omitted. Note that if you omit one argument, you must omit all the following arguments too, because program relies only on order they are passed and doesn't makes any checks.

## Program class internal architecture
Program compilation is the most complex and intresting part of all this project. It going through all the code, symbol by symbol, which passed to so-called _parsers_. This is family of specific functions, where each one must assemble individual symbols into tokens to parse them. Together they form (ironically) the finite-state machine, where each node responsible for specific token parsing. After _exactly one_ iteration throughout source code, there are completness check performed. It goes through all states that was _referenced (i.e. specified as next state for one or more states definitions)_, and if it founds undefined state, the program compilation end with error. As result, we have $O(n + k\log(k))$ time complexity of compilation, where $n$ is the size of code in symbols, and $k$ is the states count.

We, actually, could greatly optimise compilation complexity in terms of $n$ by reducing overhead related to different miscellaneous operations such as functions calls. All we need is just move outer iteration on sources _inside_ parsers. For example, `StateNameParser` will be able to read all state name, insted of assembling it symbol by symbol, which costs one function call for _every_ symbol in name. But described issue is only relevant on very huge programs, because overhead is very little and on small programs will not have impact on perfomance. Because of that, this task will be left for possible future improvements.