Steps to compile.
1) Install lame library (ver 3.100)
   --> apt get install lame 
2) Check for the static library
   --> /us/include/libmp3lame.so
3) Install pthread if not provided in Linux 
   --> apt get install pthread
4) Link with the compiler
   --> g++ main.cpp -o lame_enocder.o -lmp3lame -lpthread 
   NOTE: Check the inlcude paths for the lame and pthread library and add it to g++/gcc linkr flags
