# Project Philosophy

## Faithfulness to the Original Wolfenstein 3D
We strive to maintain the essence and gameplay experience of the original Wolfenstein 3D in our port, Wolf4SDL. Any modifications or enhancements should enhance the original feel rather than deviate from it.

## SDL 2.0 Code
All code contributions and modifications should be based on SDL 2.0 to ensure compatibility and consistency across platforms. This choice of library is essential for maintaining cross-platform support.

## Emphasis on Safe Code Practices
Safety is paramount in our development process. We strongly discourage the use of unsafe practices such as memory leaks due to mallocs without corresponding frees and the use of dangerous string functions that could lead to vulnerabilities.

## Tidiness and Consistency
Maintaining a clean, organized codebase is crucial for readability and maintainability. We encourage contributors to adhere to the existing formatting, indentation style, and overall structure of the codebase for consistency.

## License Compliance
To ensure legal compliance and respect for intellectual property rights, all contributors are required to adhere to the following licenses:
For the original source code of Wolfenstein 3D: Choose either license-id.txt or license-gpl.txt as appropriate.
For the OPL2 emulator: Choose either license-mame.txt (for fmopl.cpp) or license-gpl.txt (for dbopl.cpp) based on the specific component being modified. Use the USE_GPL define in version.h or set GPL=1 for Makefile accordingly.

By upholding these principles, we aim to create a robust, faithful, and legally compliant port of Wolfenstein 3D through Wolf4SDL.

### This file was shamelessly generated using perplexity.ai. What can I say? I write code, not books.