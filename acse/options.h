/// @file options.h
/// @brief Parsing of compilation options

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>

typedef struct t_options {
  char *outputFileName;
  char *inputFileName;
} t_options;

extern t_options compilerOptions;

extern int parseCompilerOptions(int argc, char *argv[]);
extern char *getLogFileName(const char *logType);

#endif
