
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include "cpu.h"
#include "memory.h"
#include "loader.h"
#include "supervisor.h"
#include "debugger.h"


void usage(const char *name)
{
   puts("ACSE RISC-V RV32IM simulator, (c) 2022 Politecnico di Milano");
   printf("usage: %s [options] executable\n\n", name);
   puts("Options:");
   puts("  -d, --debug           Enters debug mode before starting execution");
   puts("  -e, --entry=ADDR      Force the entry point to ADDR");
   puts("  -l, --load-addr=ADDR  Sets the executable loading address (only");
   puts("                          for executables in raw binary format)");
   puts("  -x, --prg-exit-code   Exits the simulator with the same exit code");
   puts("                          as the simulated program. In case of faults");
   puts("                          produces POSIX-style exit codes.");
   puts("  -h, --help            Displays available options");
}


typedef int t_exitCode;
enum {
   SIM_EXIT_SUCCESS,
   SIM_EXIT_HELP,
   SIM_EXIT_INVALID_ARGS,
   SIM_EXIT_INVALID_FILE,
   SIM_EXIT_SIGSEGV,
   SIM_EXIT_SIGILL,
   COUNT_SIM_EXIT
};

int exitCode(t_exitCode code, bool toPosix)
{
   static const int normalCodes[COUNT_SIM_EXIT] = {0, 0, 1, 2, 100, 101};
   static const int posixCodes[COUNT_SIM_EXIT] = {0, 126, 126, 126, 128+11, 128+4};
   if (code < 0 || code >= COUNT_SIM_EXIT)
      return code;
   if (toPosix)
      return posixCodes[code];
   return normalCodes[code];
}


int main(int argc, char *argv[])
{
   t_svStatus status;
   int ch;
   bool debug, entryIsSet, prgExitCode;
   char *tmpStr, *name;
   t_memAddress entry, load;
   t_ldrFileType excType;
   t_ldrError ldrErr;
   static const struct option options[] = {
      { "debug",         no_argument,       NULL, 'd' },
      { "entry",         required_argument, NULL, 'e' },
      { "help",          no_argument,       NULL, 'h' },
      { "load-addr",     required_argument, NULL, 'l' },
      { "prg-exit-code", no_argument,       NULL, 'x' },
   };

   name = argv[0];
   debug = false;
   entry = 0;
   entryIsSet = false;
   load = 0;
   prgExitCode = false;

   while ((ch = getopt_long(argc, argv, "de:hl:x", options, NULL)) != -1) {
      switch (ch) {
         case 'd':
            debug = true;
            break;
         case 'e':
            entryIsSet = true;
            entry = (t_memAddress)strtoul(optarg, &tmpStr, 0);
            if (tmpStr == optarg) {
               fprintf(stderr, "Invalid entry address\n");
               return 1;
            }
            break;
         case 'l':
            load = (t_memAddress)strtoul(optarg, &tmpStr, 0);
            if (tmpStr == optarg) {
               fprintf(stderr, "Invalid load address\n");
               return 1;
            }
            break;
         case 'x':
            prgExitCode = true;
            break;
         case 'h':
            usage(name);
            return exitCode(SIM_EXIT_HELP, prgExitCode);
         default:
            usage(name);
            return exitCode(SIM_EXIT_INVALID_ARGS, prgExitCode);
      }
   }
   argc -= optind;
   argv += optind;

   if (argc < 1) {
      usage(name);
      return exitCode(SIM_EXIT_INVALID_ARGS, prgExitCode);
   } else if (argc > 1) {
      fprintf(stderr, "Cannot load more than one file, exiting.\n");
      return exitCode(SIM_EXIT_INVALID_ARGS, prgExitCode);
   }

   if (debug)
      dbgEnable();
   
   excType = ldrDetectExecType(argv[0]);
   if (excType == LDR_FORMAT_BINARY) {
      if (!entryIsSet)
         entry = load;
      ldrErr = ldrLoadBinary(argv[0], load, entry);
   } else if (excType == LDR_FORMAT_ELF) {
      ldrErr = ldrLoadELF(argv[0]);
      if (entryIsSet)
         cpuSetRegister(CPU_REG_PC, entry);
   } else {
      fprintf(stderr, "Could not open executable, exiting.\n");
      return exitCode(SIM_EXIT_INVALID_FILE, prgExitCode);
   }

   if (ldrErr == LDR_INVALID_ARCH) {
      fprintf(stderr, "Not a valid RISC-V executable, exiting.\n");
      return exitCode(SIM_EXIT_INVALID_FILE, prgExitCode);
   } else if (ldrErr == LDR_INVALID_FORMAT) {
      fprintf(stderr, "Unsupported executable, exiting.\n");
      return exitCode(SIM_EXIT_INVALID_FILE, prgExitCode);
   } else if (ldrErr != LDR_NO_ERROR) {
      fprintf(stderr, "Error during executable loading, exiting.\n");
      return exitCode(SIM_EXIT_INVALID_FILE, prgExitCode);
   }

   status = svInit();

   if (debug)
      dbgRequestEnter();

   while (status == SV_STATUS_RUNNING) {
      status = svVMTick();
   }

   if (status == SV_STATUS_MEMORY_FAULT) {
      fprintf(stderr, "Memory fault at address 0x%08x, execution stopped.\n",
            memGetLastFaultAddress());
      return exitCode(SIM_EXIT_SIGSEGV, prgExitCode);
   } else if (status == SV_STATUS_ILL_INST_FAULT) {
      fprintf(stderr, "Illegal instruction at address 0x%08x\n",
            cpuGetRegister(CPU_REG_PC));
      return exitCode(SIM_EXIT_SIGILL, prgExitCode);
   }
   if (prgExitCode)
      return svGetExitCode();
   return 0;
}
