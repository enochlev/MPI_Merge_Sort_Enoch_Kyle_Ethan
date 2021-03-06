// MPIHelloWorld2.cpp : Defines the entry point for the console application.
//mpiexec -hosts 3 localhost 192.168.0.1 C:\test\MPIHelloWorld2.exe
//smpd -d

#include "stdafx.h"
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <ctime>
#include <random>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

using namespace std;


void print(int arr[], int size, string location) {
	

	ofstream myfile;
	myfile.open(location);
	
	
	for (int i = 0; i < size; i++) {
		//std::cout << arr[i] << " \n";
		myfile << arr[i] << " \n";
	}
	std::cout << "\n\n";
	myfile.close();
	
}

int compare(const void* a, const void* b)
{
	return (*(int*)a - *(int*)b);
}

/******/
//merge function adapted from geeksforgeeks.com
/******/
//Merges two subarrays of arr[]
//First subarray is arr[l..m]
//Second subarray is arr[m+1..r]

/****
NOTE
CHANGES HAVE BEEN MADE FOR MPI, MUST TEST
CURRENTLY, I THINK IT WILL ONLY WORK FOR AN EVEN NUMBER OF SUBARRAYS
****/

void merge(int arr[], int l, int m, int r)
{
	int i, j, k;
	int n1 = m - l + 1;
	int n2 = r - m;
	/*create temp arrays*/

	//n1 and n2 are the size of the subArrays
	int *L = new int[n1];
	int* R = new int[n2];

	/*Copy data to temp arrays L[] and R[]*/
	for (i = 0; i < n1; i++)
		L[i] = arr[l + i];
	for (j = 0; j < n2; j++)
		R[j] = arr[m + 1 + j];

	/*Merge the temp arrays back into arr[l...r]*/
	i = 0; //initial index of first subarray
	j = 0; //initial index of second subarray
	k = l; //initial index of merged subarray
	while (i < n1 && j < n2)
	{
		if (L[i] <= R[j])
		{
			arr[k] = L[i];
			i++;
		}
		else
		{
			arr[k] = R[j];
			j++;
		}
		k++;
	}

	/*Copy the remaining elements of L[], if there are any*/
	while (i < n1)
	{
		arr[k] = L[i];
		i++;
		k++;
	}

	/*Copy the remaining elements of R[], if there are any*/
	while (j < n2)
	{
		arr[k] = R[j];
		j++;
		k++;
	}
}

void condense(int arr[], int **Tarr, int subSize, int numOfSub)
{
	int position = 0;
	for (int i = 0; i < numOfSub; i++) {
		for (int j = 0; j < subSize; j++) {
			arr[position] = Tarr[i][j];
			position++;
		}
	}
}

//for 2D arrays, let l be the first subarray (zero) and r be the last
void mergeSort(int arr[], int l, int r) {
	if (l < r)
	{
		//same as (l+r)/2, but avoids overflow for large l and h
		int m = l + (r - l) / 2;

		//Sort first and second halves
		//Input for the merge sort needs to be the subarrays, not thew indexes
		mergeSort(arr, l, m);
		mergeSort(arr, m + 1, r);

		//merge();
	}
}


int main(int argc, char* argv[])
{

	

	MPI_Init(&argc, &argv);
	int rank;
	int world_size;

	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (rank == 0)
	{

		int slaveNodesSize = world_size - 1;
		if (slaveNodesSize == 0)
		{return -1;			}
		

		//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
	//https://diego.assencio.com/?index=6890b8c50169ef45b74db135063c227c //
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
		uniform_int_distribution<int> distribution(-10000, 10000);//distribution
		mt19937_64 twister(time(0));//mercien twister number seed//by request of dale I added cTime
		mt19937 generator(twister());



		int size = std::stoi(argv[1]);
		

		int dividedArraySize = size / slaveNodesSize;//currently will round down if you ask for size = 100, with 3 nodes, each node will get 33, making a total of 99; I dont feel like fixing this right now...

		//2d arry, each array is send to a slave node
		int** a = new int* [slaveNodesSize];
		for (int i = 0; i < slaveNodesSize; ++i)
			a[i] = new int[dividedArraySize];



		//initilize random numbers
			for (int i = 0; i < slaveNodesSize; i++)
			{
				for (int j = 0; j < dividedArraySize; j++)
				{
					int num = distribution(generator);
					a[i][j] = num;
				}
			}


			//print(a[0], slaveNodesSize);

		time_t begin = clock();
			
		//send data to other node
		for (int i = 0; i < slaveNodesSize; i++)
		{
			int ArraySize = sizeof(a[i]);//should be dividedArraySize * 2
			//MPI_Send(a[i], ArraySize, MPI_INT, i + 1, 0, MPI_COMM_WORLD);//this might be buggy
			MPI_Send(a[i], dividedArraySize, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
		}


		//recieve all subarray
		for (int i = 0; i < slaveNodesSize; i++)
		{
			int ArraySize = sizeof(a[i]);//should be dividedArraySize * 2
			//MPI_Recv(a[i], ArraySize, MPI_INT, i + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//this might be buggy
			MPI_Recv(a[i], dividedArraySize, MPI_INT, i+1, 0, MPI_COMM_WORLD, &status);
		}
		
		//::::::::::::::Kyle your code goes here::::::::::::::://
		/*
		2dArray[slaveNodeRank][indexOf_Individual_Array]
		Each row is a sub array
		Merge is running in serial on master
			--Means all the subarrays will be available
		*/
		//MUST TEST WITH A POWER OF 2 for numOfSub
		//START
		int* sortedArray = new int[size];
		//NEED TO CHECK PARAMETERS TO MATCH
		//condense(sortedArray, a, subSize, numOfSub)

		//check this works
		condense(sortedArray, a, dividedArraySize, slaveNodesSize);//puts all sorted subarrays into one array
		//print(sortedArray, size, "C:\\test\\fileUnsorted.txt");

		int subarraySize = dividedArraySize;
		int totalSize = size;
		int numOfSub= slaveNodesSize;
		
		int newSubSize = subarraySize * 2;
		int reduced = numOfSub * 2;
		while (numOfSub != 1) {
			reduced = numOfSub / 2;
			numOfSub = 0;
			while (numOfSub != reduced) {
				int start = numOfSub * newSubSize; //make sure this is right
				//int mid = start + newSubSize - 1; //make sure this is right
				//int end = mid + (mid - start) + 1;
				int end = start + newSubSize - 1;
				int mid = start + ((end - start) / 2) ;
				merge(sortedArray, start, mid, end);
				numOfSub++;
			}
			newSubSize = 2 * newSubSize;
		}
		merge(sortedArray, 0, (totalSize / 2) - 1, totalSize - 1);


		time_t end = clock();

		double timedef = difftime(end, begin);
		cout << difftime(end, begin) << " ms\n\n";
		
		//print(sortedArray, size, "C:\\test\\fileSorted.txt");



		//mergeSort(sortedArray, 0, size - 1);
		//result will be sorted contents in single dimension array 
		//running through the entire sort again
		//we should change from using a 2D array to simplify the merge
		//have merge working for single dimensional array
		//ALTERNATIVELY, try 
		/*
		condense(sortedArray, a, dividedArraySize, slaveNodesSize);
		merge(sortedArray, 0, (size / 2) - 1, size - 1);
		*/
		//STOP

		

		//

	}
	else if (rank != 0)
	{
		int inputedsize = stoi(argv[1]);
		int slaveNodesSize = world_size - 1;
		int subarraysize = inputedsize / slaveNodesSize;//should be 200


		int* subArray = new int[subarraysize];

		//MPI_RECV(   Pointer to subArray to recive the data ,  size of the data...(this is a requirment to know how much data to recieve , MPI_INT where using ints , orgin rank ( this case master), IDK , IDK)
		//MPI_Recv(&subArray, sizeof(int)/*<<Im trying to get the byte lenth of an inteiger here*/ * subarraysize , MPI_INT, 0, rank, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(subArray, subarraysize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		
		//use this to help you https://mpitutorial.com/tutorials/mpi-send-and-receive/

		//sort
		qsort(subArray, subarraysize, sizeof(int), compare);

		//send
		//MPI_Send(subArray, sizeof(int) * subarraysize, MPI_INT, 0 , rank, MPI_COMM_WORLD);
		MPI_Send(subArray, subarraysize, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	
	MPI_Finalize();
	return 0;
}

