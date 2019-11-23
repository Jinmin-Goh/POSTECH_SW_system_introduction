// Student name: 고진민
// Student ID: 20160074
// POVIS ID: eric9709

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int blockSize, rowBlock, colBlock, r, c, temp = 0, temp2 = 0, temp3 = 0, temp4 = 0, d = 0;

    // in case of N == 32, using blocking method
    if (N == 32)
    {
        blockSize = 8;
        for(colBlock = 0; colBlock < N; colBlock += blockSize)
        {
            for(rowBlock = 0; rowBlock < M; rowBlock += blockSize)
            {
                for(r = rowBlock; r < rowBlock + blockSize; r++)
                {
                    for(c = colBlock; c < colBlock + blockSize; c++)
                    {
                        // in case of not diagonal
                        if(r != c)
                            B[c][r] = A[r][c];
                        // in case of block's diagonal
                        else 
                        {
                            temp = A[r][c]; // to decrease miss, use temp
                            d = r;
                        }
                    }
                    // in case of real diagonal
                    if (rowBlock == colBlock) 
                        B[d][d] = temp;
                }
            }
        }
    }

    // in case of N == 64, using blocking method and indivisual access to make minimum miss
    else if (N == 64)
    {   
        blockSize = 4;
        for(r = 0; r < N; r += blockSize)
        {
            for(c = 0; c < M; c += blockSize)
            {
                // for B[][r]
                temp = A[r][c];
                temp2 = A[r][c+1];
                temp3 = A[r][c+2];
                temp4 = A[r][c+3];                
                B[c][r] = temp;
                B[c+1][r] = temp2;
                B[c+2][r] = temp3;
                B[c+3][r] = temp4;
            
                // for B[][r+1]
                temp = A[r+1][c];
                temp2 = A[r+1][c+1];
                temp3 = A[r+1][c+2];
                temp4 = A[r+1][c+3];
                B[c][r+1] = temp;
                B[c+1][r+1] = temp2;
                B[c+2][r+1] = temp3;
                B[c+3][r+1] = temp4;

                // for B[][r+2]
                temp = A[r+2][c];
                temp2 = A[r+2][c+1];
                temp3 = A[r+2][c+2];
                temp4 = A[r+2][c+3];
                B[c][r+2] = temp;
                B[c+1][r+2] = temp2;
                B[c+2][r+2] = temp3;
                B[c+3][r+2] = temp4;
               
                // for B[][r+3]
                temp = A[r+3][c];
                temp2 = A[r+3][c+1];
                temp3 = A[r+3][c+2];
                temp4 = A[r+3][c+3];
                B[c][r+3] = temp;
                B[c+1][r+3] = temp2;
                B[c+2][r+3] = temp3;
                B[c+3][r+3] = temp4;
            }
        }
    }

    // in case of N == 61, using blocking method for not square matrix
    else 
    {
        blockSize = 16;
        for (colBlock = 0; colBlock < M; colBlock += blockSize)
        {
            for (rowBlock = 0; rowBlock < N; rowBlock += blockSize)
            {   
                for(r = rowBlock; (r < N) && (r < rowBlock + blockSize); r++)   // check for not square matrix
                {
                    for(c = colBlock; (c < M) && (c < colBlock + blockSize); c++)
                    {
                        // in case of not diagonal
                        if(r != c)
                            B[c][r] = A[r][c];
                        // in case of block's diagonal
                        else 
                        {
                            temp = A[r][c]; // to decrease miss, use temp
                            d = r;
                        }
                    }
                    // in case of real diagonal
                    if (rowBlock == colBlock) 
                        B[d][d] = temp;
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

