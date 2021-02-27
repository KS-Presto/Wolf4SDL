This file explains how you can compile Wolf4SDL using CodeBlocks 20.03.

The steps explained in this document have been tested on Windows 10 x64.

If there are any issues with compilation, please refer to the "Troubleshooting"
section of the README-devcpp.txt

---------------
| *IMPORTANT* |
---------------
 Due to the fact that the latest versions of CodeBlocks also have updated compilers,
 there are issues compiling the project if we use newer versions of Mingw32.

 In order to be able to compile sucessfully using CodeBlocks, it will be required to
 install Dev-C++ 5.0 Beta 9.2 (4.9.9.2).

 In order to prepare Dev-C++ to be usable with CodeBlocks, please refer to the
 README-devcpp.txt.

 You only need to complete the "Needed Files" and "Installation" part.

-----------------
| Needed files: |
-----------------
 - "CodeBlocks 20.03" (codeblocks-20.03-setup.exe) (about 35 MB)
   http://www.codeblocks.org/downloads/binaries
   (You do not need to download the version with Mingw32 as we will not use it)

----------------
| Installation |
----------------
 - Complete the "Needed Files" and "Installation" part of the README-devcpp.txt
 - Install CodeBlocks 20.03 - Directory has no importance

-----------------------
| CodeBlocks Settings |
-----------------------
 - Open "Wolf4SDL.cbp"
 - Go to "Settings" -> "Compiler"
 - Find the tab "Toolchain Executables"
 - The "Compiler Installation Directories" should be pointed to "C:\Dev-Cpp\bin"
   (this assumes that you have installed Dev-C++ in the default directory)
   
-----------------------
| Compiling Wolf4SDL: |
-----------------------
 - Compile via "Build" -> "Build"
 - No errors should be displayed

--------------------
| Troubleshooting: |
--------------------
 - Issue #1:
   - Possible to get an error related to "Makefile"
   
 - Solution: 
   - Go to the root directory of the source repository
   - Delete "Makefile", "Makefile.dc" and "Makefile.win"
   - Try to compile again and it will work.