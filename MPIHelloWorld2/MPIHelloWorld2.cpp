// MPIHelloWorld2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <ctime>
#include <random>
#include <iostream>



int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	int rank;
	int world_size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (rank == 0)
	{

		int slaveNodesSize = world_size - 1;
		//if (slaveNodesSize == 0){return -1;	//no slave nodes		}
		slaveNodesSize = 4;

		//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
	//https://diego.assencio.com/?index=6890b8c50169ef45b74db135063c227c //
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
		std::uniform_int_distribution<int> distribution(-100000000, 100000000);//distribution
		std::mt19937_64 twister(time(0));//mercien twister number seed//by request of dale I added cTime
		std::mt19937 generator(twister());



		int size = 10;//for now change later
			   

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

		//send data to other node
		for (int i = 0; i < slaveNodesSize; i++)
		{
			int ArraySize = sizeof(a[i]);//should be dividedArraySize * 2
			MPI_Send(a[i], ArraySize, MPI_INT, i + 1, 0, MPI_COMM_WORLD);//this might be buggy
		}
	}
	else if (rank != 0)
	{
		//use this to help you https://mpitutorial.com/tutorials/mpi-send-and-receive/

		char helloStr[12];
		MPI_Recv(helloStr, sizeof(helloStr), MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
		printf("Rank 1 recieved string %s from Rank 0\n", helloStr);

	}
	
	MPI_Finalize();
	return 0;
}

