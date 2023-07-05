/// @file options.h
/// @brief Parsing of compilation options

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>

typedef struct t_options {
  const char *outputFileName;
  const char *inputFileName;
} t_options;

extern t_options compilerOptions;

int parseCompilerOptions(int argc, char *argv[]);
char *getLogFileName(const char *logType);

#endif
