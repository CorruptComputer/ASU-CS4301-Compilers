# ASU Team Juice Compiler

This compiler was created as a group project for our CS-4301 compilers class, myself and one other classmate worked on this. The compiler takes Pascallite code and generates the RAMM Assembly code object file and the listing file as outputs. This repository only has our final code for the compiler. 

Documentation given to us at each stage of the compiler can be found in the `documentation` folder, and the example data sets for each stage can be found in the `tests` folder.

To compile you can use g++: `g++ -o compiler -g -Wall -std=c++11 compiler.cpp`

To run it takes 3 arguments: `./compiler <data file input> <listing file output> <object file output>`

More information on RAMM Assembly can be found [here](http://www.cs.angelo.edu/ramm/).
