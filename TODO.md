# Docs & Comments
- Finish headerdoc
- Fix full stop at the end of sentences/capital letters at the beginning of sentences
- Convert all non-headerdoc comments to line comments
- Remove explanatory comments before functions in .c
- Rename all non-camel-case variables
- License file (gpl-v3)

# Structural coherency
- Replace int with bool where applicable
  - acse
  - as
  - sim
- Use C99 variable declarations where applicable
  - acse
  - as
  - sim

# Code quality
- Update copyright dates
- Handle malloc failures as aborts everywhere, not just in acse
  - sim
- clang-format everywhere (should be at least partially already done)
  - acse
  - as
  - sim
