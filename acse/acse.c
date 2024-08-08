/// @file acse.c

/**
 * \mainpage ACSE: Advanced Compiler System for Education
 * 
 * ACSE (Advanced Compiler System for Education) is a simple compiler
 * developed for educational purposes for the course "Formal Languages and
 * Compilers". ACSE, together with its supporting tools, aims to be a
 * representative example of a complete computing system – albeit simplified –
 * in order to illustrate what happens behind the scenes when a program is
 * compiled and then executed.
 * 
 * This compiler which accepts a program in a simplified C-like language called
 * LANCE (Language for Compiler Education), and produces a compiled program in
 * standard RISC-V RV32IM assembly language.
 * 
 * ## Naming conventions and code style
 * 
 * All symbols are in lower camel case. Additionally, ACSE employs a consistent
 * naming convention for all functions, in order to introduce a simple form of
 * namespacing which groups together each function based on their role.
 * Specifically:
 *  - Functions that allocate and initialize a structure are named starting
 *    with "new"
 *  - Functions that allocate and initialize a structure, and then add it to
 *    a parent object are named starting with "create"
 *  - Functions that deinitialize and deallocate a structure are named
 *    starting with "delete"
 *  - Functions that operate on a given structure type are named with a prefix
 *    that represents the structure. Exception is made for functions that
 *    modify a t_program structure, which may have no specific prefix.
 *  - Functions that add instructions to the program are named starting with
 *    "gen".
 */

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include "target_info.h"
#include "program.h"
#include "cflow_graph.h"
#include "target_asm_print.h"
#include "target_transform.h"
#include "target_info.h"
#include "cflow_graph.h"
#include "reg_alloc.h"
#include "parser.h"
#include "errors.h"

/* generated by the makefile */
extern const char *axe_version;


char *getLogFileName(const char *logType, const char *fn)
{
  char *outfn, *basename;
  size_t nameLen;
  int lastDot, i;

  basename = strdup(fn);
  if (!basename)
    fatalError("out of memory");

  lastDot = -1;
  for (i = 0; basename[i] != '\0'; i++) {
    if (basename[i] == '.')
      lastDot = i;
  }
  if (lastDot >= 0)
    basename[lastDot] = '\0';

  nameLen = strlen(basename) + strlen(logType) + (size_t)8;
  outfn = calloc(nameLen, sizeof(char));
  if (!outfn)
    fatalError("out of memory");

  snprintf(outfn, nameLen, "%s_%s.log", basename, logType);
  free(basename);
  return outfn;
}


void banner(void)
{
  printf("ACSE %s compiler, version %s\n", TARGET_NAME, axe_version);
}

void usage(const char *name)
{
  banner();
  printf("usage: %s [options] input\n\n", name);
  puts("Options:");
  puts("  -o ASMFILE    Name the output ASMFILE (default output.asm)");
  puts("  -h, --help    Displays available options");
}

int main(int argc, char *argv[])
{
  char *name = argv[0];
  int ch, res = 0;
#ifndef NDEBUG
  FILE *logFile;
#endif
  static const struct option options[] = {
      {"help", no_argument, NULL, 'h'},
  };

  char *outputFn = "output.asm";

  while ((ch = getopt_long(argc, argv, "ho:", options, NULL)) != -1) {
    switch (ch) {
      case 'o':
        outputFn = optarg;
        break;
      case 'h':
        usage(name);
        return 1;
      default:
        usage(name);
        return 1;
    }
  }
  argc -= optind;
  argv += optind;

  if (argc < 1) {
    usage(name);
    return 1;
  } else if (argc > 1) {
    emitError(nullFileLocation, "cannot assemble more than one file");
    return 1;
  }

#ifndef NDEBUG
  banner();
  printf("\n");
#endif

  res = 1;

#ifndef NDEBUG
  fprintf(stderr, "Parsing the input program\n");
  fprintf(stderr, " -> Reading input from \"%s\"\n", argv[0]);
#endif
  t_program *program = parseProgram(argv[0]);
  if (!program) 
    goto fail;
#ifndef NDEBUG
  char *logFileName = getLogFileName("frontend", outputFn);
  logFile = fopen(logFileName, "w");
  if (logFile) {
    fprintf(stderr, " -> Writing the output of parsing to \"%s\"\n", logFileName);
    dumpProgram(program, logFile);
    fclose(logFile);
  }
  free(logFileName);
#endif

#ifndef NDEBUG
  fprintf(stderr, "Lowering of pseudo-instructions to machine instructions.\n");
#endif
  doTargetSpecificTransformations(program);

#ifndef NDEBUG
  fprintf(stderr, "Performing register allocation.\n");
  logFileName = getLogFileName("controlFlow", outputFn);
  logFile = fopen(logFileName, "w");
  if (logFile) {
    fprintf(stderr, " -> Writing the control flow graph to \"%s\"\n", logFileName);
    t_cfg *cfg = programToCFG(program);
    cfgComputeLiveness(cfg);
    cfgDump(cfg, logFile, true);
    deleteCFG(cfg);
    fclose(logFile);
  }
  free(logFileName);
#endif
  t_regAllocator *regAlloc = newRegAllocator(program);
  doRegisterAllocation(regAlloc);
#ifndef NDEBUG
  logFileName = getLogFileName("regAlloc", outputFn);
  logFile = fopen(logFileName, "w");
  if (logFile) {
    fprintf(stderr, " -> Writing the register bindings to \"%s\"\n", logFileName);
    dumpRegAllocation(regAlloc, logFile);
    fclose(logFile);
  }
  free(logFileName);
#endif
  deleteRegAllocator(regAlloc);

#ifndef NDEBUG
  fprintf(stderr, "Writing the assembly file.\n");
  fprintf(stderr, " -> Output file name: \"%s\"\n", outputFn);
  fprintf(stderr, " -> Code segment size: %d instructions\n",
      listLength(program->instructions));
  fprintf(stderr, " -> Data segment size: %d elements\n", listLength(program->symbols));
  fprintf(stderr, " -> Number of labels: %d\n", listLength(program->labels));
#endif
  bool ok = writeAssembly(program, outputFn);
  if (!ok) {
    emitError(nullFileLocation, "could not write output file");
    goto fail;
  }

  res = 0;
fail:
  deleteProgram(program);
#ifndef NDEBUG
  fprintf(stderr, "Finished.\n");
#endif
  return res;
}
