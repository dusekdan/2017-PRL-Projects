#include <mpi.h>
#include <iostream> 
#include <fstream>
#include <limits.h>

#define DEBUG 0
#define TIME_MEASSUREMENT 0

#define ELEMENT_COUNT 1
#define ROOT_RANK 0
#define Y1_PROC 1

#define TAG_GROUP 0
#define TAG_GROUP1 1
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
    int C = 0;                  // Algorithm step 1 => constant complexity => O(1) 
    int Z;

    // Loop control
    int invar = 1;          // Should probably start with 1, since we want to send numbers to all others expect for root
    int i = 0;

    //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // pocet bezicich procesoru
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           // id 'naseho' procesoru 

    // Input file
    std::fstream fin;
    std::string fileName = "numbers";
    int number;

    // Meassurement setup
    double _startTime;
    double _endTime;
    double _resultTime; 
    

    bool isFirstNumber = true;

    if (myid == ROOT_RANK)
    {
       fin.open(fileName.c_str(), std::ios::in);
    }


    if (TIME_MEASSUREMENT)
    {
        // MEASSURED-SECTION-START
        _startTime = MPI_Wtime();
    }

    // Algorithm step 2, complexity => O(2*n)
    // Random number distribution - send only to workers, not the root itself
    while (invar <= 2*(numprocs-1))
    {
        // Root reads number and distributes it accordingly
        if (myid == ROOT_RANK)
        {
            number = fin.get();
            
            if (!fin.good())
                break;

            // Generate first line of output
            if (!TIME_MEASSUREMENT)
            {
                if (!isFirstNumber)
                {
                    std::cout << " " << number;
                }
                else
                {
                    std::cout << number;
                    isFirstNumber = false;
                }
            }
            
            // Store number from input to X register of x_i processor
            MPI_Send (&number, ELEMENT_COUNT, MPI_INT, invar, TAG_GROUP, MPI_COMM_WORLD); 

            // Similarly, execute linear connection to x_1 processor and store the number in its Y register
            MPI_Send (&number, ELEMENT_COUNT, MPI_INT, Y1_PROC, TAG_GROUP, MPI_COMM_WORLD);

        }
        // Workers receive values, compares and shift their registers
        else
        {
            // WORKERS RECEIVE X, Y1 RECEIVES X
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
            if (i<(numprocs-1))     // Duplicate numbers fix case
            {
                if (i < myid)
                {
                    // Test whether X and Y are not empty, then compare then and based on result increment C register
                    if (X != INT_MIN && Y != INT_MIN)
                    {
                        i++;
                        if (X >= Y)
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
                else    // Classical algorithm case
                {
                    if (X != INT_MIN && Y != INT_MIN)
                    {
                        i++;
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


    // No sorting until all processors reach this point
    //MPI_Barrier(MPI_COMM_WORLD);


    // Simulate linear connection and put X to Z register of proper processor
    // Constant complexity => O(1)
    if (myid != ROOT_RANK)
    {
        if (DEBUG)
            std::cout << "[" << myid << ",END]Sending X:" << X << "to Pc=" << C << std::endl;
        
        // Send to P[c] your X
        MPI_Send(&X, ELEMENT_COUNT, MPI_INT, C, TAG_GROUP1, MPI_COMM_WORLD);

        // Receive value from some other source to your Z
        MPI_Recv(&Z, ELEMENT_COUNT, MPI_INT, MPI_ANY_SOURCE, TAG_GROUP1, MPI_COMM_WORLD, &stat);
    }

    // Receive everything on master
    //if (myid )


    // Everything past this point is not a part of the enumeration sort algorithm
    // Now it is time to send value to ROOT & let it print it in order, NOTE that this should be left shift (since 0 is our root)
    
    if (DEBUG && myid != ROOT_RANK)
        std::cout << "[" << myid << "," << "END] I have "  << "Z:" << Z << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);



    if (myid == ROOT_RANK)
    {
        // Generate line break for first line of output
        if(!TIME_MEASSUREMENT)
            std::cout << std::endl;

        // Structure for received values
        int* orderedSet = new int [numprocs-1];

        // This could be really improved by using MPI_Gather, but the algorithm requires n steps to receive data, so here we go
        // Complexity: O(n)
        int tmpBuff;
        for (int t = 1; t <= numprocs-1; t++)
        {
            MPI_Recv(&tmpBuff, ELEMENT_COUNT, MPI_INT, t, TAG_GROUP, MPI_COMM_WORLD, &stat);
            orderedSet[t-1] = tmpBuff;
        }

        if (TIME_MEASSUREMENT)
        {
            // MEASSURED-SECTION-END
            _endTime = MPI_Wtime();

            std::cout << (_endTime - _startTime) << std::endl ;
        }
        else    // If we do not meassure script's time complexity, we are in production
        {
            for (int q = 0; q < numprocs-1; q++ )
            {
                std::cout << orderedSet[q] << std::endl;
            }
        }

        fin.close();    // Only the root opened the file, so only the root will now close it
    }
    else    // Worker sending data
    {
        MPI_Send(&Z, ELEMENT_COUNT, MPI_INT, ROOT_RANK, TAG_GROUP, MPI_COMM_WORLD);
    }



    MPI_Finalize();
    return 0;
}
