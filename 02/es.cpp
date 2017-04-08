#include <mpi.h>
#include <iostream> 
#include <fstream>
#include <limits.h>

#define DEBUG 1

#define ELEMENT_COUNT 1
#define ROOT_RANK 0
#define Y1_PROC 1

#define TAG_GROUP 0

int main (int argc, char *argv[]) 
{
    // Inicializační kód převzat z:
    // http://www.fit.vutbr.cz/~ikalmar/PRL/odd-even-trans/odd-even.cpp
    int numprocs;               // Number of operating processors
    int myid;                   //muj rank -- //TODO: Should probably rename this to be more purpose reflecting
    int neighnumber;            // hodnota souseda // TODO: Reconsider if I really want to do it this way...
    int mynumber;               //moje hodnota - possibly wont be used, because we use X? (//TODO: REVIEW AND DELETE THIS)
    
    MPI_Status stat;        // MPI_Send() operation status structure

    // Registers
    int X = INT_MIN;
    int Y = INT_MIN;
    int C = 1;          // 1st step of algorhytm - Set all C registers to 1
    int Z;

    //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // pocet bezicich procesoru
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // id 'naseho' procesoru 

        std::fstream fin;
        std::string fileName = "numbers";
        int number;
        int invar = 1;          // Should probably start with 1, since we want to send numbers to all others expect for root


        if (myid == ROOT_RANK)
        {
           fin.open(fileName.c_str(), std::ios::in);
        }

        // Random number distribution - send only to workers, not the root itself
        while (invar <= 2*(numprocs-1))
        {
            // Root reads number and distributes it accordingly
            if (myid == ROOT_RANK)
            {
                number = fin.get();
                
                if (!fin.good())    // TODO: Restructualize this condition not to break the whole program
                    break;

                std::cout << number << " " << std::endl ;     // Generate first row of resulting output  // TODO: Include this in final version
                
                // Store number from input to X register of x_i processor
                MPI_Send (&number, ELEMENT_COUNT, MPI_INT, invar, TAG_GROUP, MPI_COMM_WORLD); 

                // Similarly, execute linear connection to x_1 processor and store the number in its Y register
                MPI_Send (&number, ELEMENT_COUNT, MPI_INT, Y1_PROC, TAG_GROUP, MPI_COMM_WORLD);

            }
            else                // Workers receive values and shift their registers
            {
                // WORKERS RECEIVE X, Y1 RECEIVES X, ALL REGISTERS SHIFT
                if (invar <= numprocs-1)    // Receiving only for as long as root is sending
                {
                    // Y_1 REGISTER RECEIVES VALUE FROM ROOT
                    if (myid == Y1_PROC)
                    {
                        MPI_Recv (&Y, ELEMENT_COUNT, MPI_INT, ROOT_RANK, TAG_GROUP, MPI_COMM_WORLD, &stat);
                    }


                    // WORKERS RECEIVE X VALUE
                    if (invar == myid)
                    {
                        // All workers should be capable of receiving value for their X registers
                        MPI_Recv (&X, ELEMENT_COUNT, MPI_INT, ROOT_RANK, TAG_GROUP, MPI_COMM_WORLD, &stat);
                    
                    }
                }

                // COMPARE X TO Y REGISTER
                if (invar >= myid && invar < ((numprocs-1)+myid))
                {
                    // Compare values if you can
                    // Test whether X and Y are not empty, then compare then and based on result increment C register
                    if (X != INT_MIN && Y != INT_MIN)
                    {
                        if (X > Y)
                        {
                            if (DEBUG)
                                std::cout << "[" << myid << ":" << invar << "]" << X << " was greater than X:" << X << ",Y:" << Y << " - so the C is incremented" << std::endl;

                            C++;

                            if (DEBUG)
                                 std::cout << "[" << myid << ":" << invar << "]" << "C is now:" << C << std::endl; 
                        }
                        else
                        {
                            if (DEBUG)
                                std::cout <<  "[" << myid << ":" << invar << "]" << "No incrementation executed, because X:" << X << ",Y:" << Y << std::endl;
                        }
                    }
                }


                // DO THE HEAVY-SHIFTING
                if (invar >= myid)
                {
                    // Shift registers (except for when you are the last register -> then do nothing)
                    if (myid != numprocs-1)
                    {
                        MPI_Send(&Y, ELEMENT_COUNT, MPI_INT, myid+1, TAG_GROUP, MPI_COMM_WORLD);

                        if (DEBUG)
                            std::cout <<  "[" << myid << ":" << invar << "]" <<  "Shifting Y(" << Y << ") from " << myid << " to " << (myid+1) << std::endl;
                    }
                }
                if (myid != 1 && invar >= myid-1)
                {

                    if (DEBUG)
                                std::cout <<  "[" << myid << ":" << invar << "]" <<  "Waiting Y(" << Y << ") from " << myid-1 << " on " << myid << std::endl;

                    MPI_Recv(&Y, ELEMENT_COUNT, MPI_INT, myid-1, TAG_GROUP, MPI_COMM_WORLD, &stat);

                    if (DEBUG)
                                std::cout <<  "[" << myid << ":" << invar << "]" <<  "Receving Y(" << Y << ") from " << myid-1 << " on " << myid << std::endl;
                }


                
            }
            invar++;
        }

        for (int t = 0; t <= numprocs-1; t++)
        {
            
        }

        //std::cout << std::endl;     // Generate line break after first output row

        fin.close();



    MPI_Finalize();
    return 0;
}