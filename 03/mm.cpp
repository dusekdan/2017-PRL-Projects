#include <mpi.h>
#include <iostream>
#include <fstream>
#include <limits.h>     // Questionable include TODO: Reconsider

#define DEBUG 1

#define DECADIC_BASE 10

#define MATRIX_INPUT_FILE_1 "mat1"
#define MATRIX_INPUT_FILE_2 "mat2"

#define ROOT_RANK 0

void loadMatrixes();
void loadMatrixFromFile(std::string fileName);

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

    // Load matrixes from input files
    if (myid == ROOT_RANK)
        loadMatrixes();

    MPI_Finalize();
    return 0;
}


void loadMatrixes()
{
    // TODO: Consider creating a generic function loadMatrixFromFile to be called twice from here (less code reusage)
    // Process matrix1 file
    loadMatrixFromFile(MATRIX_INPUT_FILE_1);

    // Process matrix2 file
}

void loadMatrixFromFile(std::string fileName)
{
    std::fstream matrixFile;
    matrixFile.open(fileName.c_str(), std::ios::in);

    // Read number of matrix rows from first line of input file
    std::string strBuff;
    char* strtolErrorPtr;
    int numberOfRows;
    std::getline(matrixFile, strBuff);

    numberOfRows = strtol(strBuff.c_str(), &strtolErrorPtr, DECADIC_BASE);

    if (strtolErrorPtr == strBuff)
    {
        fprintf (stderr, "Invalid number of matrix rows.");
        exit(1);
    }

    std::cout << "Number of matrix rows for loaded file is " << numberOfRows << std::endl;

}