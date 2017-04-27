#include <mpi.h>
#include <iostream>
#include <fstream>
#include <limits.h>     // Questionable include TODO: Reconsider
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1

#define DECADIC_BASE 10

#define MATRIX_INPUT_FILE_1 "mat1"
#define MATRIX_INPUT_FILE_2 "mat2"

#define ROOT_RANK 0

#define ELEMENT_COUNT 1
#define TAG_GROUP 1


void loadMatrixes();
void loadMatFile(char type);
void debug_showLoadedMatrixes();
int getElement(int x, int y, char type);
int getElementByWidth(int x, int y, int width, char type);
int calculateAMatrixOffset(int myid);

std::vector<int> matrixA;
int matrixAColumns;
int matrixARows;    // This is the number from MAT1 file

std::vector<int> matrixB;
int matrixBColumns; // This is the number from MAT2 file
int matrixBRows;

// Global matrix representation vectors
// Only ROOT_RANK process should access them
//std::vector<std::vector<int>> matrix1;
//std::vector<std::vector<int>> matrix2;

int main(int argc, char* argv[])
{
    // Initialization code ripped of:
    // http://www.fit.vutbr.cz/~ikalmar/PRL/odd-even-trans/odd-even.cpp
    int numprocs;
    int myid;
    MPI_Status stat;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    // Processors registers
    int C = 0;
    int A = INT_MIN;
    int B = INT_MIN;

    loadMatrixes();

    // Load matrixes from input files
    if (myid == ROOT_RANK)
    {

        if (DEBUG)
            debug_showLoadedMatrixes();

        // Root rank will distribute (in steps) numbers to other processors
        // I use matrixAColumns, because matrixAColums == matrixBRows in order to mutliply matrixes successfully
        for (int i = 0; i < matrixAColumns; i++)
        {
            // Every iteration, feed registers, multiply & store results, send register contents to others
            A = getElement(0, i,'A');
            B = getElementByWidth(0, i, matrixBColumns, 'B');
            C += (A*B);
        
            if (DEBUG)
                std::cout << "[P:" << myid << "|I:" << i << "] C+=" << A << "*"  << B << " My A:" << A << ", B:" << B << ", C:" << C << std::endl;

            // Distribution
            MPI_Send(&A, ELEMENT_COUNT, MPI_INT, 1, TAG_GROUP, MPI_COMM_WORLD);
            MPI_Send(&B, ELEMENT_COUNT, MPI_INT, matrixBColumns, TAG_GROUP, MPI_COMM_WORLD);
        }

    }
    else
    {
        // Other processors will enter loop to recv & send numbers further
        for (int i = 0; i < matrixAColumns; i++)
        {
            // Wait for numbers (if first-row || first-column processor, read one from vector)
            // Mesh layout
            // matrixARows * matrixBColumns
            if (myid < matrixBColumns) // First row, receive only from left neighbor
            {
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] First row waiting for myid-1 My A:" << A << ", B:" << B << ", C:" << C << std::endl;

                MPI_Recv(&A, ELEMENT_COUNT, MPI_INT, (myid-1), TAG_GROUP, MPI_COMM_WORLD, &stat);

                B = getElementByWidth(myid, i, matrixBColumns, 'B');

                if (DEBUG)
                    std::cout << "====I TAKE: " << B << "====" << std::endl;

                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] First row received My A:" << A << ", B:" << B << ", C:" << C << std::endl;
            }
            else if (myid % matrixBColumns == 0) // First column
            {
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] First column waiting for myid-matrixBcolumns My A:" << A << ", B:" << B << ", C:" << C << std::endl;

                MPI_Recv(&B, ELEMENT_COUNT, MPI_INT, (myid-matrixBColumns), TAG_GROUP, MPI_COMM_WORLD, &stat);
                    
                // TODO: This is the place where wrong memory mapping happens
                A = matrixA[(calculateAMatrixOffset(myid)+i)];
                
                    /* matrixA[(myid-((matrixAColumns-1)-i))] // NOPE */
                    /* getElement(0, myid+i, 'A'); // NOPE */
                    /* getElement(0, myid+i+1, 'A'); // NOPE */

                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] First column received; My A:" << A << ", B:" << B << ", C:" << C << std::endl;
            }   
            else // Normal 
            {
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Normal waiting for myid-1 My A:" << A << ", B:" << B << ", C:" << C << std::endl;
                
                MPI_Recv(&A, ELEMENT_COUNT, MPI_INT, (myid-1), TAG_GROUP, MPI_COMM_WORLD, &stat);
                
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Normal received for myid-1 My A:" << A << ", B:" << B << ", C:" << C << std::endl;
                
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Normal waiting for myid-matrixBcolumns My A:" << A << ", B:" << B << ", C:" << C << std::endl;
                
                MPI_Recv(&B, ELEMENT_COUNT, MPI_INT, (myid-matrixBColumns), TAG_GROUP, MPI_COMM_WORLD, &stat);
                
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Normal received My A:" << A << ", B:" << B << ", C:" << C << std::endl;
            }

            // Calculate and store
            C += (A*B);

            if (DEBUG)
                std::cout << "[P:" << myid << "|I:" << i << "] C+=" << A << "*"  << B << " My A:" << A << ", B:" << B << ", C:" << C << std::endl;
 
            // Distribute numbes to neighbors (or not, if last-row || last-column processor)
            if (myid >= matrixBColumns*(matrixARows-1) && !(myid == (matrixBColumns*matrixARows)-1)) // Last row
            {
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Last row sent to P"<<(myid+1)<<" My A:" << A << ", B:" << B << ", C:" << C << std::endl;
                
                MPI_Send(&A, ELEMENT_COUNT, MPI_INT, (myid+1), TAG_GROUP, MPI_COMM_WORLD);
            }
            else if ((myid+1) % matrixBColumns == 0 && !(myid == (matrixBColumns*matrixARows)-1)) // Last column
            {
                // Send only to the one under me
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Last column sent to P"<< (myid+matrixBColumns) <<" My A:" << A << ", B:" << B << ", C:" << C << std::endl;
                
                MPI_Send(&B, ELEMENT_COUNT, MPI_INT, (myid+matrixBColumns), TAG_GROUP, MPI_COMM_WORLD);
            }
            else if (myid == (matrixBColumns*matrixARows)-1) // The last mohican
            {
                if (DEBUG)
                    std::cout << "I AM THE LAST PROCESSOR. AND I ONLY CONSUME. I DO NOT SEND" << std::endl;
            }
            else // normal
            {
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Normal1 sent to P" << (myid+1) << " My A:" << A << ", B:" << B << ", C:" << C << std::endl;
                
                MPI_Send(&A, ELEMENT_COUNT, MPI_INT, (myid+1), TAG_GROUP, MPI_COMM_WORLD);
                
                if (DEBUG)
                    std::cout << "[P:" << myid << "|I:" << i << "] Normal2 sent to P"<< (myid+matrixBColumns) << " My A:" << A << ", B:" << B << ", C:" << C << std::endl;
                
                MPI_Send(&B, ELEMENT_COUNT, MPI_INT, (myid+matrixBColumns), TAG_GROUP, MPI_COMM_WORLD);
            }
        
        }
    }

    // Calculation is completed, now all the guys will send data to root
    // But first, we wait for it (just to be sure)
    MPI_Barrier(MPI_COMM_WORLD);


    if (myid == ROOT_RANK)
    {
        // Receive everything from everyone
        int tmpBuff;
        std::vector<int> finalMatrix;

        // Store my root number
        finalMatrix.push_back(C);
        
        for (int t = 1; t < numprocs; t++)
        {
            MPI_Recv(&tmpBuff, ELEMENT_COUNT, MPI_INT, t, TAG_GROUP, MPI_COMM_WORLD, &stat);
            finalMatrix.push_back(tmpBuff);
        }
        
        // Print it out in desired format
        std::cout << matrixARows  <<  ":"   << matrixBColumns << std::endl;
        int loopControl = 1;
        for (std::vector<int>::iterator it = finalMatrix.begin(); it != finalMatrix.end(); it++)
        {

            std::cout << *it;

            if ((loopControl%matrixBColumns) == 0)
            {
                std::cout << std::endl;
            }
            else
            {
                std::cout << " ";
            }

            loopControl++;
        }
    }
    else
    {
        // Other worker processes just send their C value
        MPI_Send(&C, ELEMENT_COUNT, MPI_INT, ROOT_RANK, TAG_GROUP, MPI_COMM_WORLD);
    }



    MPI_Finalize();
    return 0;
}


void loadMatrixes()
{
    // Process matrix1 file
    loadMatFile('A');

    // Process matrix2 file
    loadMatFile('B');
}

int calculateAMatrixOffset(int myid)
{
    // Divide number of all fields (processes) by ID and get boundary row id
    // For 2x5 its 2 (cause its second line), decrement by one to reflect 0-start indexing
    //int rowId = ((int) ((matrixARows*matrixBColumns)/myid))-1;
    int rowId = ((int)myid/matrixBColumns);

    // Now rowId * matrixAColumns = first element to be read which will be 
    // further incremented by loop control afterwards
    //return (rowId*matrixAColumns);
    return (rowId * matrixAColumns);
}

/**
 * Maps 2D accesing to 1D vector
 * int x        - row index
 * int y        - column index
 * char type    - matrix selection
 */
int getElement(int x, int y, char type)
{
   return (type == 'A') ? matrixA[(x*matrixAColumns+y)] : matrixB[(x*matrixBColumns+y)];
}

int getElementByWidth(int x, int y, int width, char type)
{
    return (type == 'A') ? matrixA[y*width+x] : matrixB[y*width+x];
}

// TODO: Add code checking that numbers in mat1 & mat2 are checking out
void loadMatFile(char type)
{

    std::string inputFile = (type == 'A') ? MATRIX_INPUT_FILE_1 : MATRIX_INPUT_FILE_2;

    std::fstream matrixFile;
    matrixFile.open(inputFile.c_str(),  std::ios::in);

    int matWidth;
    std::string strBuff;
    char* strtolErrorPtr;
    int firstLineNumber;
    int tmpElement;
    std::getline(matrixFile, strBuff);

    firstLineNumber = strtol(strBuff.c_str(), &strtolErrorPtr, DECADIC_BASE);
        if (strtolErrorPtr == strBuff)
        {
            fprintf (stderr, "Invalid number of matrix rows.");
            exit(1);
        }

    // For matrix A (mat1 file), the width must be calculated (init 0)
    // but the number of rows is given
    if (type == 'A')
    {
        matWidth = 0;
        
        // Process only as many lines as is expected to be within file
        for(int i = 0; i < firstLineNumber; i++)
        {
            std::getline(matrixFile, strBuff);

            std::istringstream iss(strBuff);
            std::string element;

            // Process line
            while (std::getline(iss, element, ' '))
            {
                // Parse tokenized string to integer
                tmpElement = strtol (element.c_str(), &strtolErrorPtr, DECADIC_BASE);
                    if (strtolErrorPtr == element.c_str())
                    {
                        fprintf (stderr, "Invalid number in matrix contents.");
                        exit(1);
                    }

                matrixA.push_back(tmpElement);
            }

            // Store number of columns for matrix
            if (matWidth == 0)
                    matWidth = matrixA.size();
        }

        matrixAColumns = matWidth;
        matrixARows = firstLineNumber;
    }
    // For matrix B (mat2 file), the width of matrix is given, but the number
    else
    {
        matrixBColumns = firstLineNumber;

        // Just push everything to vector and check whether modulus of matrix
        // is divisible by count of matrix B columns, if not prompt error
        int rowCount = 0;
        while (std::getline(matrixFile, strBuff))
        {
            std::istringstream iss(strBuff);
            std::string element;

            while(std::getline(iss, element, ' '))
            {
               // Parse tokenized string to integer
                tmpElement = strtol (element.c_str(), &strtolErrorPtr, DECADIC_BASE);
                    if (strtolErrorPtr == element.c_str())
                    {
                        fprintf (stderr, "Invalid number in matrix contents.");
                        exit(1);
                    }

                matrixB.push_back(tmpElement); 
            }

            rowCount++;
        }

        matrixBRows = rowCount;

        // Will not be tested TODO: Ensure processes call  MPI_Finalize()
        if (matrixB.size() % matWidth != 0)
        {
            fprintf (stderr, "Invalid number of elements for matrix.");
            exit(1);
        }
    }

    matrixFile.close();
}

void debug_showLoadedMatrixes()
{
    // Matrixes are stored as 1-D vector

    std::cout << "Matrix-A rows: " << matrixARows << std::endl;
    std::cout << "Matrix-A columns: " << matrixAColumns << std::endl;
    for (int i = 0; i < matrixA.size(); i++)
    {
        std::cout << matrixA.at(i) << " ";
        
        if ((i+1) % matrixAColumns == 0)
            std::cout << std::endl;
    }


    std::cout << "Matrix-B rows: " << matrixBRows << std::endl;
    std::cout << "Matrix-B columns: " << matrixBColumns << std::endl;
    for (int i = 0; i < matrixB.size(); i++)
    {
        std::cout << matrixB.at(i) << " ";

        if ((i+1) % matrixBColumns == 0)
            std::cout << std::endl;
    }
}