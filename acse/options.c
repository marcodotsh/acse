/// @file options.c

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "utils.h"
#include "options.h"
#include "errors.h"
#include "target_info.h"

t_options compilerOptions;

/* generated by the makefile */
extern const char *axe_version;


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

int parseCompilerOptions(int argc, char *argv[])
{
  char *name = argv[0];
  int ch, res = 0;
  static const struct option options[] = {
      {"help", no_argument, NULL, 'h'},
  };

  compilerOptions.inputFileName = NULL;
  compilerOptions.outputFileName = "output.asm";

  while ((ch = getopt_long(argc, argv, "ho:", options, NULL)) != -1) {
    switch (ch) {
      case 'o':
        compilerOptions.outputFileName = optarg;
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

  if (argc > 1) {
    fprintf(stderr, "Cannot compile more than one file, exiting.\n");
    return 1;
  } else if (argc == 1) {
    compilerOptions.inputFileName = argv[0];
  }

#ifndef NDEBUG
  banner();
  printf("\n");
#endif
  return 0;
}

char *getLogFileName(const char *logType)
{
  char *outfn, *basename;
  int nameLen, lastDot, i;

  basename = strdup(compilerOptions.outputFileName);
  if (!basename)
    fatalError(ERROR_OUT_OF_MEMORY);

  lastDot = -1;
  for (i = 0; basename[i] != '\0'; i++) {
    if (basename[i] == '.')
      lastDot = i;
  }
  if (lastDot >= 0)
    basename[lastDot] = '\0';

  nameLen = strlen(basename) + strlen(logType) + 8;
  outfn = calloc(nameLen, sizeof(char));
  if (!outfn)
    fatalError(ERROR_OUT_OF_MEMORY);

  snprintf(outfn, nameLen, "%s_%s.log", basename, logType);
  free(basename);
  return outfn;
}
