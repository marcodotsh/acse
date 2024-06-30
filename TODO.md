# Docs & Comments
- Finish headerdoc
- Fix full stop at the end of sentences/capital letters at the beginning of sentences
- Convert all non-headerdoc comments to line comments
- Remove explanatory comments before functions in .c
- Rename all non-camel-case variables
- License file (gpl-v3)

# Structural coherency
- Introduce error.c/h to acse/as/sim (essentially the same code)
- Remove acse.h
- Replace int with bool where applicable
  - acse
  - as
  - sim
- Use C99 variable declarations where applicable
  - acse
  - as
  - sim

# Code quality
- as: Transform Token type into struct
- as: Read entire file into memory in tokenizer?
- acse: Move log prints to main()?
- Update copyright dates
- Handle malloc failures as aborts everywhere, not just in acse
  - as
  - sim
