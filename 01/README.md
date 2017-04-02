# Project 01 - Unranked, enviromental testing

Goal of this project was to try out how MPI runs on our development devices.

We simply created c/cpp (I chose cpp) file that would compile successfuly with g++ and tried to run MPI commands to verify it works.

To compile: mpic++ -o Hello Hello.cpp

I encapsulated this command call to Makefile, so just run 'make' to get it working. 

To run: mpirun -np 10 Hello
The '-np' switch specifies the number of processors that should run the code.