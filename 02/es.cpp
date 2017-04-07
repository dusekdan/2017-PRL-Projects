#include <mpi.h>
#include <iostream> 
#include <fstream>

#define DEBUG 1

#define ELEMENT_COUNT 1
#define ROOT_RANK 0

int main (int argc, char *argv[]) 
{
    // Inicializační kód převzat z:
    // http://www.fit.vutbr.cz/~ikalmar/PRL/odd-even-trans/odd-even.cpp
    int numprocs;               //pocet procesoru
    int myid;                   //muj rank
    int neighnumber;            //hodnota souseda
    int mynumber;               //moje hodnota
    MPI_Status stat;            //struct- obsahuje kod- source, tag, error

    //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // pocet bezicich procesoru
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // id 'naseho' procesoru 

    // Cteni numbers souboru (opet castecne prevzato z vyse uvedene adresy)
    if (myid == ROOT_RANK)
    {
        std::string fileName = "numbers";
        int number;
        //int invar = 0;          // Identifikace procesoru prijimajiciho cislo
        int invar = 1;          // Should probably start with 1, since we want to send numbers to all others expect for root

        std::fstream fin;
        fin.open(fileName.c_str(), std::ios::in);

        // Random number distribution - send only to workers, not the root itself
        while (fin.good())
        {
            number = fin.get();
            
            if (!fin.good())
                break;

            if (DEBUG)
                std::cout << "Procesor " << invar << " dostava " << number << std::endl;
            

            MPI_Send (&number,
                     ELEMENT_COUNT, 
                     MPI_INT,
                     invar,
                     0, // tag
                     MPI_COMM_WORLD
                     ); 
            
            invar++;
        }

        fin.close();
    }
    // Workers acquire number to work with
    else
    {
        MPI_Recv (&mynumber,
                  ELEMENT_COUNT,
                  MPI_INT,
                  ROOT_RANK,
                  0, // tag
                  MPI_COMM_WORLD,
                  &stat
            );

        if (DEBUG)
            std::cout << "Processor #" << myid << " received " << mynumber << std::endl;
    
        // Enumeration sort comes here


    }
    


    MPI_Finalize();
    return 0;
}