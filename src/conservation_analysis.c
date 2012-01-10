//============================================================================
// Name        : Conservation.cpp
// Author      : Abiezer Tejeda
// Version     :
// Copyright   : USU LEFTLAB
// Description : Gene networks conservation analysis.
//============================================================================

#include <math.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include "simulation_method.h"

//using namespace std;

//FUNCTION PREAMBLE
int gsl_matrix_rank(gsl_matrix *A);
gsl_vector *deprows(gsl_matrix *A);
gsl_matrix *reorder(gsl_vector *I, gsl_matrix *A, SPECIES **speciesOrder);
gsl_matrix *gsl_matrix_pinv(gsl_matrix *A);
gsl_matrix *NR(gsl_matrix *A);
gsl_matrix *linkzero(gsl_matrix *L);
gsl_matrix *gamma_matrix(gsl_matrix *L);
void disp_mat(gsl_matrix *A);
gsl_matrix* conservation(gsl_matrix *S, SPECIES **speciesOrder);

//MAIN FUNCTION
/*
int main()
{
//----------------------------------------------------------------------------------------------------
	const int M = 16;
	const int N = 9;

/*
	double delta[M][N] = {{1, -1,  0,  0 , 0,  0,  0,  0,  0},
						  {0, -1, -1,  0,  0,  0,  0,  0,  0},
						  {0,  0,  1, -1,  0,  0, -1,  0,  0},
						  {0,  0,  1,  1, -1,  0,  0,  0,  0},
						  {0,  0,  0,  0,  1, -1,  0,  0,  0},
						  {0,  0,  0,  0,  0,  0,  1, -1,  0},
						 {-1, -1,  0,  0,  0,  1,  0,  1,  1},
						  {1,  1,  0,  0,  0, -1,  0, -1, -2},
						  {0,  0,  0,  0,  0,  0,  0,  0,  1},
						  {0,  0,  0,  0, -1,  0,  1,  0,  0},
						  {0,  0,  0,  0,  1,  0, -1,  0,  0}};
*/

/*
	double delta[M][N] ={{0, 0, 0, 0, 0, 1, 0, 0, 0},
						 {0, 0, 0, 0, 1, 0, 0, 0, 0},
						 {0, 0, 0, 0, 0, 0, 1, 0, 0},
						 {0, 0, 0, 0, 0, 0, 1, 0, 0},
						 {1, 0, 0, 0, 0, 0, 0, 0, 0},
						 {0, 1, 0, 0, 0, 0, 0, 0, 0},
						{-1, 0, 0,-1, 0, 0, 0, 1, 0},
						{-1,-1, 1, 0, 0, 0, 0, 0, 0},
						{-1, 0, 0, 0, 0,-1, 0, 0, 1},
						{-1, 0, 0, 0, 0, 0, 0, 0, 0},
						 {0, 1,-1, 0, 0, 0, 0, 0, 0},
						 {0, 0, 0, 0,-1, 0, 0, 0, 0},
						 {0, 0, 0, 0, 0, 0,-1, 0, 0},
						 {0,-1, 0, 0, 0, 0, 0, 0, 0},
						 {1, 0, 0, 1, 0, 0, 0,-1, 0},
						 {1, 0, 0, 0, 0, 1, 0, 0,-1}};
/

/*
	double delta[M][N] = {{0, 0, 0, 0, 0, 1, 0, 0, 0},  //% DA -> DA + MA
						  {0, 0, 0, 0, 0, 1, 0, 0, 0},  //% DAp -> DAp + MA
						  {0, 0, 0, 0, 0, 0, 0, 0, 1},  //% DR -> DR + MR
						  {0, 0, 0, 0, 0, 0, 0, 0, 1},  //% DRp -> DRp + MR
						  {1, 0, 0, 0, 0, 0, 0, 0, 0},  //% MA -> A + MA
						  {0, 1, 0, 0, 0, 0, 0, 0, 0},  //% MR -> R + MR
						 {-1, 0, 0,-1, 1, 0, 0, 0, 0},//% A + DA -> DAp
						 {-1,-1, 1, 0, 0, 0, 0, 0, 0},//% A + R -> C
						 {-1, 0, 0, 0, 0, 0,-1, 1, 0},//% A + DR -> DRp
						 {-1, 0, 0, 0, 0, 0, 0, 0, 0}, //% A -> empty
						  {0, 1,-1, 0, 0, 0, 0, 0, 0}, //% C -> R
						  {0, 0, 0, 0, 0,-1, 0, 0, 0}, //% MA -> empty
						  {0, 0, 0, 0, 0, 0, 0, 0, -1}, //% MR -> empty
						  {0,-1, 0, 0, 0, 0, 0, 0, 0}, //% R -> empty
						  {1, 0, 0, 1,-1, 0, 0, 0, 0}, //% DAp -> DA + A
						  {1, 0, 0, 0, 0, 0, 1,-1, 0}};//% DRp -> DR + A

/

	double delta[M][N] = {{0,     0,     0,     0,     1,     0,     0,     0,     0},
						  {0,     0,     0,     0,     1,     0,     0,     0,     0},
						  {0,     0,     0,     0,     0,     0,     1,     0,     0},
						  {0,     0,     0,     0,     0,     0,     1,     0,     0},
						  {1,     0,     0,     0,     0,     0,     0,     0,     0},
						  {0,     1,     0,     0,     0,     0,     0,     0,     0},
					     {-1,     0,     0,    -1,     0,     0,     0,     1,     0},
					     {-1,    -1,     1,     0,     0,     0,     0,     0,     0},
					     {-1,     0,     0,     0,     0,    -1,     0,     0,     1},
					     {-1,     0,     0,     0,     0,     0,     0,     0,     0},
						  {0,     1,    -1,     0,     0,     0,     0,     0,     0},
						  {0,     0,     0,     0,    -1,     0,     0,     0,     0},
						  {0,     0,     0,     0,     0,     0,    -1,     0,     0},
						  {0,    -1,     0,     0,     0,     0,     0,     0,     0},
						  {1,     0,     0,     1,     0,     0,     0,    -1,     0},
						  {1,     0,     0,     0,     0,     1,     0,     0,    -1}};

	/*
	const int M = 6;
	const int N = 5;
	double delta[M][N] = {{1, -1,  0,  0,  0},
						  {0,  1, -1,  0,  0},
						  {0,  0,  1, -1,  0},
						  {0,  0,  0,  1, -1},
						  {0,  0, -2,  1,  1},
						  {0, -1,  0,  0,  1}};
 /

	gsl_matrix *A = gsl_matrix_alloc(M,N);
	gsl_matrix *cpy_A = gsl_matrix_alloc(M,N);

        int i;
        int j;

	//Assign elements in "delta" to gsl_matrix A.
	for(i=0;i<M;i++)
	{
		for(j=0;j<N;j++)
		{
			gsl_matrix_set(A,i,j,delta[i][j]);
		}
	}

	gsl_matrix_memcpy (cpy_A, A);
	int rank = gsl_matrix_rank(A);
        printf("rank(delta) = %d\n", rank);


	// 1- Take the transpose of the stoichiometrix matrix to have the "species" in the row dimension.
	gsl_matrix *T = gsl_matrix_alloc(N,M); //remember to deallocate memory at the end...
	gsl_matrix_transpose_memcpy(T,cpy_A); // T holds the transpose of A (cpy_A)

	gsl_matrix *L = conservation(T);

	gsl_matrix *Lo = linkzero(L);
	printf("\nLo = \n");
	disp_mat(Lo);

	gsl_matrix *G = gamma(Lo);
        printf("\nG = \n");
	disp_mat(G);

	gsl_matrix_free(T);
	gsl_matrix_free(A);
	gsl_matrix_free(cpy_A);
	gsl_matrix_free(L);
	gsl_matrix_free(Lo);


   return 0;
}
*/
//*****************************************************************************************************

//**********************************************************************************************************
//***************************************CONSERVATION FUNCTION*********************************************
gsl_matrix* conservation(gsl_matrix *S, SPECIES **speciesOrder)
{

	// 1- Extract the linearly dependent rows of the stoichiometric matrix.
	gsl_vector *Index = deprows(S);

	// 2- Reorganize the stoichiometric matrix as N = [NR NO]^T, where NR is the subset of N containing
	// the linearly independent rows, while NO contains the dependent rows.
	gsl_matrix *S1 = reorder(Index,S,speciesOrder);

	//Compute the independent species matrix
	gsl_matrix *Nr = NR(S1);

	//Compute the pseudoinverse of the independent species matrix
	gsl_matrix *Pinv = gsl_matrix_pinv(Nr);//MxN matrix

	//Find the link matrix
	gsl_matrix * L = gsl_matrix_alloc(S1->size1,Pinv->size2);
	gsl_linalg_matmult(S1,Pinv,L);

	gsl_matrix_free(Pinv);
	gsl_matrix_free(Nr);
	gsl_matrix_free(S1);
	gsl_vector_free(Index);

	return L;
}


//**********************************************************************************************************


//***************************************RANK FUNCTION*************************************************
// This function finds the rank of any matrix passed as its argument.

//INPUT: gsl_matrix *A
//OUTPUT: int rank. THe output of this function is the row/column rank of any matrix
//After this function is called, the matrix passed as an argument (i.e. A) is no longer available.
//The contents of A are replaced with other values of the call to the gsl_linalg_SV_decomp();

int gsl_matrix_rank(gsl_matrix *A1)
{
	int rank = 0;
	int M = A1->size1; //Number of rows
	int N = A1->size2; //Number of columns
	double tol = 0.0001;
        int i;

	// Copy matrix A1 to A so that the A1 is not destroyed outside this function.
	gsl_matrix *A = gsl_matrix_alloc(M,N);
	gsl_matrix_memcpy (A, A1);

	// Case 1: M>=N:
	if(M>=N)
	{
		//These vectors and matrix are needed to be passed as function arguments to the svd_cmp function.
		gsl_matrix *V = gsl_matrix_alloc(N,N);
		gsl_vector *S = gsl_vector_alloc(N);
		gsl_vector *work = gsl_vector_alloc(N);

		//compute svd with the gsl_linalg_SV_decomp(A,V,S,work) function.
		gsl_linalg_SV_decomp(A,V,S,work);

		//Search for values of S less than the threshold 0.0001, and truncate to zero
		for(i=0;i<N;i++)
		{
			if(gsl_vector_get(S,i) < tol)
			{
				gsl_vector_set(S,i,0.0);
			}
		}
		//Search for non-zero singular values in vector S.
		for(i=0;i<N;i++)
		{
			if(gsl_vector_get(S,i) != 0.0)
			{
				rank += 1;
			}
		}
		//Deallocate memory
		gsl_matrix_free(V);
		gsl_matrix_free(A);
		gsl_vector_free(work);
		gsl_vector_free(S);

		return rank;
	}
	// Case 1: M<N:
	else
	{
		gsl_matrix *V = gsl_matrix_alloc(M,M);
		gsl_vector *S = gsl_vector_alloc(M);
		gsl_vector *work = gsl_vector_alloc(M);
		gsl_matrix *tmp = gsl_matrix_alloc(N,M); // To store transpose(A).
		gsl_matrix_transpose_memcpy(tmp,A);//Transpose(A) is in matrix tmp.
		gsl_linalg_SV_decomp(tmp,V,S,work);

		//Search for values of S less than the threshold 0.0001, and truncate to zero
		for(i=0;i<M;i++)
		{
			if(gsl_vector_get(S,i) < tol)
			{
				gsl_vector_set(S,i,0.0);
			}
		}
		//Search for non-zero singular values in vector S.
		for(i=0;i<M;i++)
		{
			if(gsl_vector_get(S,i) != 0.0)
			{
				rank += 1;
			}
		}
		//Deallocate memory
		gsl_matrix_free(A);
		gsl_matrix_free(V);
		gsl_matrix_free(tmp);
		gsl_vector_free(work);
		gsl_vector_free(S);

		return rank;
	}
}
//**********************************************************************************************************

//**********************************************************************************************************
//***************************************DEPROWS FUNCTION***************************************************
// This function finds the index location of the linearly dependent rows of the stoichiometric matrix and
// returns a gsl vector containing these addresses.

//INPUT: gsl_matrix *A.
//OUTPUT: gsl_vector containing the indexes of linearly dependent rows of matrix A.

gsl_vector *deprows(gsl_matrix *A)
{
	int M = A->size1; // Number of rows of matrix A.
	int N = A->size2; // Number of columns of matrix A.
	int r;
        int i;
        int j;
        int n;

	//Allocate memory.
	gsl_vector *index = gsl_vector_alloc(M);
	gsl_vector *k = gsl_vector_alloc(M);
	gsl_vector *v = gsl_vector_alloc(N);

	for(i=0;i<M; i++)
	{
		gsl_matrix *tmp = gsl_matrix_alloc(i+1,N);
		for(j=0;j<i+1 ; j++)
		{
			//copy the first i rows of A to the tmp matrix to iterate through it and find the dep rows.
			gsl_matrix_get_row (v, A, j);
			gsl_matrix_set_row (tmp, j, v);
		}
		r = gsl_matrix_rank(tmp);

		gsl_vector_set(k, i, i+1-r); //row-rank
		if(r<i+1)
		{
			int d = gsl_vector_get(k,i);
			int s = gsl_vector_get(k,i-1);
			if(d>s)
			{
				gsl_vector_set(index,i,i);
			}
			else
			{
				// This -1.0 is used as an invalid location so we can identify it and take out only
				// those that are of interest to us.
				gsl_vector_set(index,i,-1.0);
			}
		}
		else
		{
			gsl_vector_set(index,i,-1.0);
		}
		gsl_matrix_free(tmp);
	}

	//Extract valid indexes from gsl_vector_index.
	int sum = 0;
	for(n = 0;n<M;n++)
	{
		if(gsl_vector_get(index,n)!=-1.0)
		{
			sum += 1;
		}
	}
	gsl_vector *I = gsl_vector_alloc(sum);//This vector will contain the indexes of the linearly dependent rows.

	int qty = 0;
	for(n=0;n<M;n++)
	{
		if(gsl_vector_get(index,n)!=-1)
		{
			gsl_vector_set(I,qty,gsl_vector_get(index,n));
			qty++;
		}
	}

	gsl_vector_free(index);
	gsl_vector_free(k);
	gsl_vector_free(v);

	return I;
}
//**********************************************************************************************************

//**********************************************************************************************************
//***************************************REORDER FUNCTION***************************************************
// This function reorders the stoichiometric matrix as N = [NR NO]', where NR is composed of the linearly
// independent rows and NO is composed of the linearly dependent rows.
//
// INPUT: Stoichiometric matrix,
//		  Index: vector containing the index location of the dependent rows.
//
// OUTPUT: Reordered stoichiometric matrix.

gsl_matrix *reorder(gsl_vector *I, gsl_matrix *A1, SPECIES **speciesOrder)
{
	int L = I->size;
	int M = A1->size1; //number of rows
	int N = A1->size2; //number of columns
	int i;
	int j;

	gsl_matrix *A = gsl_matrix_alloc(M,N);
	gsl_matrix_memcpy (A, A1); // Copies A1 onto A.

	//=================================================================================================
	// In this block of code we check for zero-rows in the stoichiometry matrix. If any,
	// these rows are deleted from the stoichiometry matrix, then the matrix is reordered to have the
	// independent rows and dependent rows organized as [NR NO].
	// To delete the zero rows, we use a masking matrix E by which the stoichimery matrix is multiplied
	// by to accomplish our goal of deleting zero rows (i.e., new_stoichiometry = E*Stoichimetry).

	gsl_vector *tmp = gsl_vector_alloc(N);
	int counter = 0;
	//This for-loop counts how many zer-rows are there in matrix A (Stoichiometric matrix)
	for(i=0;i<M;i++)
	{
		gsl_matrix_get_row (tmp, A, i);

		if(!gsl_vector_isnull(tmp))
		{
			counter++;
		}
	}

	gsl_vector_free(tmp); //deallocate memory from the temporal vector "tmp"

	if(counter != 0)
	{

		//Vector "non_zero_rows contain the index of the non-zero rows of the stoichiometric matrix
		gsl_vector *non_zero_rows = gsl_vector_calloc(counter);
		SPECIES **newSpeciesOrder = malloc(sizeof(SPECIES*)*counter);
		gsl_vector * tmp = gsl_vector_alloc(N); //This vector will hold a row of A temporarily

		int k = 0; //this this an counter

		for(i = 0;i<M;i++)
		{
			gsl_matrix_get_row (tmp, A, i);
			if(!gsl_vector_isnull(tmp))
			{
				gsl_vector_set(non_zero_rows,k,i);
				newSpeciesOrder[k] = speciesOrder[i];
				k++;
			}
		}
		speciesOrder = &newSpeciesOrder;

		gsl_matrix * E = gsl_matrix_calloc(counter,M);
		//Now create the masking matrix to eliminate the zero rows from the stoichiometry matrix
		for(i = 0;i<counter;i++)
		{
			int column_index = gsl_vector_get(non_zero_rows,i);
			gsl_matrix_set(E,i,column_index,1.0);
		}

		//Now multiply the masking matrix E by the stoichiometry matrix A: new_Stoich = E*S
		gsl_matrix * new_Stoichiometry = gsl_matrix_alloc(counter,N);
		gsl_linalg_matmult(E,A,new_Stoichiometry);

		//Now reorder the new stoichiometry matrix and return that matrix.
		for(i=0;i<L;i++)
		{
			int k = gsl_vector_get(I,i);
			for(j=k-i;j<counter-1;j++)
			{
				gsl_matrix_swap_rows(new_Stoichiometry, j, j+1);
				SPECIES *species = speciesOrder[j];
				speciesOrder[j] = speciesOrder[j+1];
				speciesOrder[j+1] = species;
			}
		}

		gsl_vector_free(tmp);
		gsl_vector_free(non_zero_rows);
		gsl_matrix_free(E);
		gsl_matrix_free(A);

		return new_Stoichiometry;
	}
	//=================================================================================================

	for(i=0;i<L;i++)
	{
		int k = gsl_vector_get(I,i);
		for(j=k-i;j<M-1;j++)
		{
			gsl_matrix_swap_rows(A, j, j+1);
			SPECIES *species = speciesOrder[j];
			speciesOrder[j] = speciesOrder[j+1];
			speciesOrder[j+1] = species;
		}
	}

	return A;
}
//**********************************************************************************************************



//**********************************************************************************************************

//**********************************************************************************************************
//***************************************PSEUDOINVERSE FUNCTION*********************************************
// This function computes the pseudoinverse of a mtrix.
// INPUT: gsl_matrix *A(MxN).
// OUTPUT: gsl_matrix *pinv, which is the pseudoinverse of matrix A. The pseudoinverse is computed using
// the SVD decomposition as: US^{-1}V' for M>N or VS+U' for N>M. An important property is pinv(A) = pinv(A')'.
// The gsl_svd function is equivalent to [u s v] = svd(A,'0') in MATLAB.


gsl_matrix *gsl_matrix_pinv(gsl_matrix *A)
{
	int M = A->size1;
	int N = A->size2;
        int n;
        unsigned int i;
        unsigned int j;

	//make a copy of matrix A
	gsl_matrix *U = gsl_matrix_alloc(M,N);
	gsl_matrix_memcpy (U, A);
	//int rank = gsl_matrix_rank(A);

	//CASE I: M>=N
	if(M >= N)
	{
		gsl_matrix *V = gsl_matrix_alloc(N,N);
		gsl_vector *S = gsl_vector_alloc(N);
		gsl_vector *work = gsl_vector_alloc(N);
		gsl_linalg_SV_decomp(U,V,S,work);

		int L = S->size;
		// Make singular values less than "tol" equal to zero. 'tol = 0.0001'
		double tol = 0.0001;
		for(n=0;n<L;n++)
		{
			double s = gsl_vector_get(S,n);
			if(s<=tol)
			{
				gsl_vector_set(S,n,0.0);
			}
		}

		//Make singular != 0 equal to 1/s_i.
		for(i=0;i<L;i++)
		{
			double k = gsl_vector_get(S,i);
			double kk = k*k;
			if(k!=0)
			{
				gsl_vector_set(S,i,k/kk);
			}
		}

		//Construct the diagonal matrix containing the singular values of
		gsl_matrix *tmp_S = gsl_matrix_calloc(N,N);
		for(i=0;i<L;i++)
		{
			double k = gsl_vector_get(S,i);
			gsl_matrix_set(tmp_S,i,i,k);
		}


		//Get transpose of U.
		gsl_matrix *UT = gsl_matrix_alloc(N,M);
		gsl_matrix_transpose_memcpy(UT,U);

		// Now get the pseudo-inverse by multiplying: V*tmp_S*UT if they have the appropriate dimensions.
		gsl_matrix *VS = gsl_matrix_alloc(V->size1,tmp_S->size2);
		gsl_matrix *Pinv = gsl_matrix_alloc(VS->size1,UT->size2);
		gsl_linalg_matmult(V,tmp_S,VS);
		gsl_linalg_matmult(VS,UT,Pinv);

		//Deallocate memory
		gsl_matrix_free(VS);
		gsl_matrix_free(UT);
		gsl_matrix_free(tmp_S);
		gsl_matrix_free(V);
		gsl_matrix_free(U);
		gsl_vector_free(S);
		gsl_vector_free(work);

		return Pinv;
	}
	else //M<N
	{
		// 1- Transpose U to get a tall matrix NxM.
		// 2- Compute SVD(U).
		// 3- Compute the pseudo-inverse pinv.
		// 4- Return transpose(pinv).

		double tol = 0.0001; //Absolute values of the elements of matrices and vectors below this threshold will be zeroed out.

		gsl_matrix *UT = gsl_matrix_alloc(N,M);
		gsl_matrix_transpose_memcpy(UT,U);

		gsl_matrix *V = gsl_matrix_alloc(M,M);
		gsl_vector *S = gsl_vector_alloc(M);
		gsl_vector *work = gsl_vector_alloc(M);
		gsl_linalg_SV_decomp(UT,V,S,work);

		// Make singular values less than "tol" equal to zero. 'tol = 0.0001'
		for(n=0;n<M;n++)
		{
			double s = gsl_vector_get(S,n);
			if(s<=tol)
			{
				gsl_vector_set(S,n,0.0);
			}
		}

		//Make singular != 0 equal to 1/s_i.
		for(i=0;i<M;i++)
		{
			double k = gsl_vector_get(S,i);
			double kk = k*k;
			if(k!=0)
			{
				gsl_vector_set(S,i,k/kk);
			}
		}

		//Construct the diagonal matrix containing the singular values of
		gsl_matrix *tmp_S = gsl_matrix_calloc(M,M);
		for(i=0;i<M;i++)
		{
			double k = gsl_vector_get(S,i);
			gsl_matrix_set(tmp_S,i,i,k);
		}

		// Get transpose of UT
		gsl_matrix *UT_T = gsl_matrix_alloc(M,N);
		gsl_matrix_transpose_memcpy(UT_T,UT);

		// Now get the pseudo-inverse by multiplying: V*tmp_S*U if they have the appropriate dimensions.
		gsl_matrix *VS = gsl_matrix_alloc(M,M);
		gsl_linalg_matmult(V,tmp_S,VS);
		//Make elements less than tol equal to zero.

		gsl_matrix *Pinv = gsl_matrix_alloc(M,N);
		gsl_linalg_matmult(VS,UT_T,Pinv);

		//Transpose Pinv.
		gsl_matrix *PinvT = gsl_matrix_alloc(N,M);
		gsl_matrix_transpose_memcpy(PinvT,Pinv);

		//Make elements of PinvT less than tol equal to zero.
		for(i=0;i<PinvT->size1;i++)
		{
			for(j=0;j<PinvT->size2;j++)
			{
				if(abs(gsl_matrix_get(PinvT,i,j))<tol)
				{
					gsl_matrix_set(PinvT,i,j,0.0);
				}
			}
		}

		//Deallocate memory
		gsl_matrix_free(Pinv);
		gsl_matrix_free(V);
		gsl_matrix_free(VS);
		gsl_matrix_free(U);
		gsl_matrix_free(UT);
		gsl_matrix_free(tmp_S);
		gsl_vector_free(S);
		gsl_vector_free(work);printf("\n");

		return PinvT;
	}

	return 0;
}

//**********************************************************************************************************


//**********************************************************************************************************
//********************************************NR FUNCTION***************************************************
// This function computes the independent species submatrix from the reordered stoichiometric matrix.
// INPUT: Reordered stoichiometric matrix.
// OUTPUT: Independent species submatrix NR.

gsl_matrix *NR(gsl_matrix *A)
{
	//int M = A->size1;
	int N = A->size2;

	int r = gsl_matrix_rank(A);
        int i;
        int j;

	gsl_matrix *NR1 = gsl_matrix_alloc(r,N);
	// Take the first M-r rows of A.
	for(i=0;i<r;i++)
	{
		for(j=0;j<N;j++)
		{
			double k = gsl_matrix_get(A,i,j);
			gsl_matrix_set(NR1,i,j,k);
		}
	}

	return NR1;
}

//**********************************************************************************************************

//**********************************************************************************************************
//********************************************LINKZERO FUNCTION*********************************************
// This function recieves the link matrix, L, and returns the link zero matrix. L = [I Lo]^T.
// INPUT: Link matrix.
// OUTPUT: Link-zero matrix.

gsl_matrix *linkzero(gsl_matrix *L)
{
	int M = L->size1;//rows
	int N = L->size2;//columns

	int r = gsl_matrix_rank(L);
        int j;

	//Allocate memory for the link-zero matrix (Lo).
	gsl_matrix *Lo = gsl_matrix_alloc(M-r,N);

	//Temporal vector.
	gsl_vector * v = gsl_vector_alloc(N);

	for(j=0;j<M-r;j++)
	{
		gsl_matrix_get_row (v, L,r+j);
		gsl_matrix_set_row(Lo,j,v);
	}
	//Deallocate memeory
	gsl_vector_free(v);

	return Lo;
}
//**********************************************************************************************************

//**********************************************************************************************************
//***********************************************GAMMA FUNCTION*********************************************
//
// INPUT: Link matrix.
// OUTPUT: Conservation matrix (gamma).


gsl_matrix *gamma_matrix(gsl_matrix *cpy_L)
{
	int M = cpy_L->size1;
	int N = cpy_L->size2;
        unsigned int i;
        unsigned int j;

	//make a copy of cpy_L.
	gsl_matrix *L = gsl_matrix_calloc(M,N);
	gsl_matrix_memcpy(L,cpy_L);

	//Create a matrix with the appropriate size to hold [Lo I]
	gsl_matrix * Gamma = gsl_matrix_calloc(M,N+M);

	//Copy the elements of L to Gamma and invert them: Gamma = [-Lo I].
	for(i=0;i<L->size1;i++)
	{
		for(j=0;j<L->size2;j++)
		{
			double k = -gsl_matrix_get(L,i,j);
			gsl_matrix_set(Gamma,i,j,k);
			if(abs(gsl_matrix_get(Gamma,i,j))<0.0001)
			{
				gsl_matrix_set(Gamma,i,j,0.0);
			}
		}
	}

	//create the indentity matrix I.
	for(i= 0;i<M;i++)
	{
		gsl_matrix_set(Gamma,i,i+N,1.0);
	}

	gsl_matrix_free(L);

	return Gamma;
}


//**********************************************************************************************************

//**********************************************************************************************************
//**********************************************************************************************************
// This function takes in a gsl matrix and displays its contents
void disp_mat(gsl_matrix *A)
{
	int M = A->size1;
	int N = A->size2;
        int i;
        int j;

	for(i=0;i<M;i++)
	{
		for(j=0;j<N;j++)
		{
			printf("%f ", gsl_matrix_get(A,i,j));
		}
		printf("\n");
	}
}
//**********************************************************************************************************

