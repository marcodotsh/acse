# ACSE (Advanced Compiler System for Education)

ACSE is a complete toolchain consisting of a compiler (`acse`), an assembler
(`asrv32im`), and a simulator (`simrv32im`).
It provides a simple but reasonably accurate sandbox for learning how a
compilation toolchain works.

### [Downloads and quick reference](https://github.com/polimi-flc/acse/releases)
### [Full documentation](https://polimi-flc.github.io/acse-docs/index.html)

## How to use ACSE

ACSE was tested on the following operating systems:

- **Linux** (any recent 32 bit or 64 bit distribution should work)
- **macOS** (any recent version should work)
- **Windows** (both 32 bit or 64 bit) when built with
  [MinGW](http://www.mingw.org) under [MSYS2](https://www.msys2.org), or inside
  **Windows Services for Linux** (WSL).

If you are using **Linux** or **macOS**, ACSE requires the following programs
to be installed:

- a **C compiler** (for example *GCC* or *clang*)
- **GNU bison**
- **GNU flex**

If you use **Windows**, first you must install either the
[MSYS2](https://www.msys2.org) environment or **Windows Services for Linux**
(WSL). Both MSYS2 and Windows Services for Linux (WSL) provide a Linux-like
environment inside of Windows.

Once you have installed either MSYS2 or WSL, you can use the following
instructions just as if you were using Linux or macOS.

### Building ACSE

To build the ACSE compiler toolchain, open a terminal and type:

      make

The built executables will be located in the `bin` directory.

### Testing ACSE

To compile some examples (located in the directory `tests`) type:

      make tests

### Using ACSE

You can compile and run new Lance programs in this way (suppose you
have saved a Lance program in `myprog.src`):

      ./bin/acse myprog.src -o myprog.asm
      ./bin/asrv32im myprog.asm -o myprog.o
      ./bin/simrv32im myprog.o

Alternatively, you can add a test to the `tests` directory by following these
steps:

1. Create a new directory inside `tests`. You can choose whatever directory
   name you wish, as long as it is not `test`.
2. Move the source code files to be compiled inside the directory you have
   created. The extension of these files **must** be `.src`.
3. Run the command `make tests` to compile all tests, included the one you have
   just added.
   
The `make tests` command only runs the ACSE compiler and the assembler, you
will have to invoke the simulator manually.

All assembly files produced by ACSE are compatible with
[RARS](https://github.com/TheThirdOne/rars) so you can also run any compiled
program through it.

### Authors

ACSE is copyright (c) 2008-2024 Politecnico di Milano, and is licensed as
GNU GPL version 3. It has been developed by the following contributors:

- Andrea di Biagio (main author of the original non-RISC-V version)
- Giovanni Agosta (wrote the simulator in the original non-RISC-V version,
  advisor to the RISC-V version)
- Niccolò Izzo (maintainer 2015-2020)
- Daniele Cattaneo (maintainer from 2019, main author of the RISC-V version)

Additional help and input has been provided by:

- Alessandro Barenghi
- Luca Breveglieri
- Stefano Cherubin
- Massimo Fioravanti
- Gabriele Magnani
- Angelo Morzenti
- Michele Scandale
- Michele Scuttari
- Ettore Speziale
- Michele Tartara

Please report any suspected bugs or defects to
`daniele.cattaneo <at> polimi.it`.
