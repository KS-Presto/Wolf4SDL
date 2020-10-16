This file explains how you can compile Wolf4SDL using Bloodshed's Dev-C++.

The steps explained in this document have been tested on Windows 10 x64.

Please make sure that you review the "Troubleshooting" section at the
end of the file should you get any issues when compiling.

-----------------
| Needed files: |
-----------------
 - "Dev-C++ 5.0 Beta 9.2 (4.9.9.2)" with Mingw/GCC 3.4.2 (about 9 MB)
   http://www.bloodshed.net/dev/devcpp.html
 - SDL-devel-1.2.15-mingw32.DevPak (85 kB)
 - SDL_mixer-devel-1.2.6-2-mingw32.DevPack (540 kB)
   (Both DevPacks can be found in the "devcpp_devpacks" folder in the source repo)

----------------
| Installation |
----------------
 - Install Dev-C++ to C:\Dev-Cpp
 - Open Wolf4SDL.dev
 - Go to "Tools" -> "Package Manager"
 - Click on the "Install" button in the toolbar
 - Select "SDL-devel-1.2.15-mingw32.DevPak" (wherever you saved it)
 - Some "Next" buttons and a "Finish" button later...
 - Click on the "Install" button in the toolbar
 - Select "SDL_mixer-devel-1.2.12.DevPak" (wherever you saved it)
 - Some "Next" buttons and a "Finish" button later...
 - Close the Package Manager
 
--------------------
| Data file setup: |
--------------------
 - Copy the data files (e.g. *.WL6) you want to use to the Wolf4SDL
   source code folder
 - If you want to use the data files of the full Activision version of
   Wolfenstein 3D v1.4, you can just skip to the next section
 - Otherwise open "version.h" and comment/uncomment the definitions
   according to the description given in this file
   
-----------------------
| Compiling Wolf4SDL: |
-----------------------
 - Compile via "Execute" -> "Compile"
 - No errors should be displayed
 - Run Wolf4SDL via "Execute" -> "Run"

--------------------
| Troubleshooting: |
--------------------
 - Issue #1:
   - Compiler will show "[Build Error] [obj/Wolf4SDL.exe] Error 1"
     
	 By looking at the "Compile Log", you will also see at the end a
     line like this: "gcc.exe: Internal error: Aborted (program collect2)"
   
 - Solution: 
   - Navigate to "C:\Dev-Cpp\libexec\gcc\mingw32\3.4.2\"
   - Find the file called "collect2.exe" and rename it to "collect2.exe.old"
     (you can also delete it)
   - Try to compile again and it will work.
   
 - Issue #2:
   - Compiler will show "undefined reference to `__cpu_features_init'"
   
 - Solution:
   - Make sure there is no C:\mingw folder. Otherwise Dev-C++ will mix different
     versions of MinGW libraries.
