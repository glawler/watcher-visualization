#include <memory.h>
#include <stdio.h>
#include <queue>

using namespace std; 

#define BIT_CHUNK_TYPE unsigned char
#define BIT_CHUNK_SIZE (sizeof(BIT_CHUNK_TYPE)*8)
#define HIGH_ORDER_BIT (1<<(BIT_CHUNK_SIZE-1))
#define MAX_GM_NODES 256
typedef BIT_CHUNK_TYPE GroupRow[MAX_GM_NODES/BIT_CHUNK_SIZE];
typedef GroupRow GroupMatrix[MAX_GM_NODES];

GroupMatrix matrix; 

void setBit(const int &A, const int &B, bool value)
{                       
	if(value)  matrix[A][(B/BIT_CHUNK_SIZE)] |=  (HIGH_ORDER_BIT >> (B%BIT_CHUNK_SIZE));
	else       matrix[A][(B/BIT_CHUNK_SIZE)] &= ~(HIGH_ORDER_BIT >> (B%BIT_CHUNK_SIZE));
	printf("setBit(%d,%d,): matrix[%d][%d]>>%d == %s\n", A, B, A, (B/BIT_CHUNK_SIZE), B%BIT_CHUNK_SIZE, value ? "true" : "false"); 
}

void getGraphNeighbors(const int &root, GroupRow &nbrs)
{
	memset(&nbrs, 0, sizeof(GroupRow));

	// pretty much stolen line for line from pseudocode at http://en.wikipedia.org/wiki/Breadth-first_search, but without the search.
	queue<int> q;      // queue of bit indexes
	q.push(root); 
	nbrs[root/BIT_CHUNK_SIZE] |= (HIGH_ORDER_BIT >> (root%BIT_CHUNK_SIZE));

	while(!q.empty())
	{
		int v = q.front();
		q.pop();

		for(size_t j = 0; j < sizeof(GroupRow); j++)
		{
			for(size_t k = 0; k < BIT_CHUNK_SIZE; k++)
			{
				// a neighbor, but not yet visited
				if((matrix[v][j] & (HIGH_ORDER_BIT >> k)) && !(nbrs[j] & (HIGH_ORDER_BIT >> k)) )
				{
					nbrs[j] |= (HIGH_ORDER_BIT >> k);
					q.push((j*BIT_CHUNK_SIZE)+k);
				}
			}
		}
	}
}

void printRow(const GroupRow &row)
{
	for(size_t i = 0; i < sizeof(matrix[0]); i++)
	{
		for(size_t k = 0; k < BIT_CHUNK_SIZE; k++)
		{
			if(row[i] & (HIGH_ORDER_BIT >> k))
			{
				printf("%d ", ((i*BIT_CHUNK_SIZE)+k)); 
			}
		}
	}
	printf("\n"); 

}

void printIndex(const int &row)
{
	printf("row %d: ", row); 
	for(size_t j = 0; j < sizeof(matrix[0]); j++)
	{
		for(size_t k = 0; k < BIT_CHUNK_SIZE; k++)
		{
			if(matrix[row][j] & (HIGH_ORDER_BIT >> k))
			{
				printf("%d ", ((j*BIT_CHUNK_SIZE)+k));
			}
		}
	}
	printf("\n"); 

}

void printMatrix()
{
	GroupRow emptyRow;
	memset(&emptyRow, 0, sizeof(GroupRow)); 
	
	printf("Matrix:\n"); 
	for(size_t i = 0; i < MAX_GM_NODES; i++)
	{
		if(memcmp(&emptyRow, &matrix[i], sizeof(emptyRow)))
		{
			printIndex(i); 	
		}
	}
}

int main()
{
	setBit(102, 103, true); 
	setBit(103, 102, true); 

	printMatrix(); 

	GroupRow row;
	getGraphNeighbors(102, row); 
	printf("graph: "); 
	printRow(row); 

	setBit(107, 101, true); 
	setBit(107, 103, true); 
	setBit(107, 105, true); 

	setBit(103, 107, true); 

	printMatrix(); 
	
	getGraphNeighbors(102, row); 
	printf("graph from 102: "); 
	printRow(row); 

	setBit(107, 103, false); 
	setBit(103, 107, false); 
	
	getGraphNeighbors(102, row); 
	printf("graph from 102:"); 
	printRow(row); 

	getGraphNeighbors(107, row); 
	printf("graph from 107:"); 
	printRow(row); 

	return 0; 

}
