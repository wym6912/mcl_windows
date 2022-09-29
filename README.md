# mcl_windows README

A compilable version in Windows.

## How to compile this in Windows?

The configure script must be used as without it the compilation will likely fail.

First, follow MSYS2 installation instructions at https://www.msys2.org/wiki/MSYS2-installation/

Then relaunch to MSYS2 shell using the "MSYS2 MinGW x64" executable. Once in that environment (check $MSYSTEM equals "MINGW64") install the compilers using `pacman -S` and the following package list:

```
autoheader autoconf autoreconf
base-devel mingw-w64-x86_64-toolchain
mingw-w64-x86_64-libdeflate mingw-w64-x86_64-zlib mingw-w64-x86_64-bzip2
mingw-w64-x86_64-xz mingw-w64-x86_64-curl mingw-w64-x86_64-autotools
mingw-w64-x86_64-tools-git
```

Finally, run `mcl.sh` or `mcl_noconfig.sh` (without running `./configure`). Once the compile and make process is finished, you will get `mcl` package programs on `/mcl` in MSYS2 installation folder, for example, you can find `/mcl/bin/mcl.exe`, it's the `mcl` program in Linux.

# What is MCL?

MCL, the Markov Cluster algorithm, also known as Markov Clustering, is a method and program for clustering weighted or simple networks, a.k.a. graphs.

## MCL website

[micans.org](micans.org/mcl)