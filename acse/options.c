/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * io_manager.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "options.h"
#include "errors.h"

t_options compilerOptions;


int parseCompilerOptions(int argc, char **argv)
{  
   argc--;
   argv++;

   if (argc > 0)
      compilerOptions.inputFileName = argv[0];

   if (argc == 2)
      compilerOptions.outputFileName = argv[1];
   else
      compilerOptions.outputFileName = "output.asm";

   return 0;
}

char *getLogFileName(const char *logType)
{
   char *outfn, *basename;
   int nameLen, lastDot, i;

   basename = strdup(compilerOptions.outputFileName);
   if (!basename)
      fatalError(AXE_OUT_OF_MEMORY);

   lastDot = -1;
   for (i = 0; basename[i] != '\0'; i++) {
      if (basename[i] == '.')
         lastDot = i;
   }
   if (lastDot >= 0)
      basename[lastDot] = '\0';
   
   nameLen = strlen(basename) + 24;
   outfn = calloc(nameLen, sizeof(char));
   if (!outfn)
      fatalError(AXE_OUT_OF_MEMORY);
   
   snprintf(outfn, nameLen, "%s_%s.log", basename, logType);
   free(basename);
   return outfn;
}

