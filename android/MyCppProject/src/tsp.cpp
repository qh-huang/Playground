// C++ program to solve Traveling Salesman Problem 
// using Branch and Bound. 
//#include <bits/stdc++.h> 
#include <climits>
#include <stdio.h>
#include <string.h>
#include <iostream>     // std::cout
#include <cmath>
#include <iterator>     // std::next
#include <list>         // std::list
#include <algorithm>    // std::for_each
#include <deque>
#include <vector>
using namespace std; 
const int N = 30; 

// final_path[] stores the final solution ie, the 
// path of the salesman. 
int final_path[N+1]; 
vector<int> traverseOrder;

// visited[] keeps track of the already visited nodes 
// in a particular path 
bool visited[N]; 

// Stores the final minimum weight of shortest tour. 
int final_res = INT_MAX; 

void floyd(int **Map, int n)
{
    for(int k=0;k<n;++k)
        for(int i=0;i<n;++i)
            for(int j=0;j<n;++j)
                if(Map[i][j]>Map[i][k]+Map[k][j])
                    Map[i][j]=Map[i][k]+Map[k][j];
}

// Function to copy temporary solution to 
// the final solution 
void copyToFinal(int curr_path[], int n) 
{ 
	for (int i=0; i<n; i++) 
		final_path[i] = curr_path[i]; 
	final_path[n] = curr_path[0]; 
} 

// Function to find the minimum edge cost 
// having an end at the vertex i 
int firstMin(int **adj, int n, int i) 
{ 
	int min = INT_MAX; 
	for (int k=0; k<n; k++) 
		if (adj[i][k]<min && i != k) 
			min = adj[i][k]; 
	return min; 
} 

// function to find the second minimum edge cost 
// having an end at the vertex i 
int secondMin(int **adj, int n, int i) 
{ 
	int first = INT_MAX, second = INT_MAX; 
	for (int j=0; j<n; j++) 
	{ 
		if (i == j) 
			continue; 

		if (adj[i][j] <= first) 
		{ 
			second = first; 
			first = adj[i][j]; 
		} 
		else if (adj[i][j] <= second && 
				adj[i][j] != first) 
			second = adj[i][j]; 
	} 
	return second; 
} 

// function that takes as arguments: 
// curr_bound -> lower bound of the root node 
// curr_weight-> stores the weight of the path so far 
// level-> current level while moving in the search 
//		 space tree 
// curr_path[] -> where the solution is being stored which 
//			 would later be copied to final_path[] 
void TSPRec(int **adj, int n, int curr_bound, int curr_weight, 
			int level, int curr_path[]) 
{ 
	// base case is when we have reached level N which 
	// means we have covered all the nodes once 
	if (level==n) 
	{ 
		// check if there is an edge from last vertex in 
		// path back to the first vertex 
		if (adj[curr_path[level-1]][curr_path[0]] != 0) 
		{ 
			// curr_res has the total weight of the 
			// solution we got 
			int curr_res = curr_weight + 
					adj[curr_path[level-1]][curr_path[0]]; 

			// Update final result and final path if 
			// current result is better. 
			if (curr_res < final_res) 
			{ 
				copyToFinal(curr_path, n); 
				final_res = curr_res; 
			} 
		} 
		return; 
	} 

	// for any other level iterate for all vertices to 
	// build the search space tree recursively 
	for (int i=0; i<n; i++) 
	{ 
		// Consider next vertex if it is not same (diagonal 
		// entry in adjacency matrix and not visited 
		// already) 
		if (adj[curr_path[level-1]][i] != 0 && 
			visited[i] == false) 
		{ 
			int temp = curr_bound; 
			curr_weight += adj[curr_path[level-1]][i]; 

			// different computation of curr_bound for 
			// level 2 from the other levels 
			if (level==1) 
			curr_bound -= ((firstMin(adj, n, curr_path[level-1]) + 
							firstMin(adj, n, i))/2); 
			else
			curr_bound -= ((secondMin(adj, n, curr_path[level-1]) + 
							firstMin(adj, n, i))/2); 

			// curr_bound + curr_weight is the actual lower bound 
			// for the node that we have arrived on 
			// If current lower bound < final_res, we need to explore 
			// the node further 
			if (curr_bound + curr_weight < final_res) 
			{ 
				curr_path[level] = i; 
				visited[i] = true; 

				// call TSPRec for the next level 
				TSPRec(adj, n, curr_bound, curr_weight, level+1, 
					curr_path); 
			} 

			// Else we have to prune the node by resetting 
			// all changes to curr_weight and curr_bound 
			curr_weight -= adj[curr_path[level-1]][i]; 
			curr_bound = temp; 

			// Also reset the visited array 
			memset(visited, false, sizeof(visited)); 
			for (int j=0; j<=level-1; j++) 
				visited[curr_path[j]] = true; 
		} 
	} 
} 

// This function sets up final_path[] 
int tsp(int n, int **adj)
{ 
#if 1
	floyd(adj, n);
	int curr_path[n+1]; 

	// Calculate initial lower bound for the root node 
	// using the formula 1/2 * (sum of first min + 
	// second min) for all edges. 
	// Also initialize the curr_path and visited array 
	int curr_bound = 0; 
	memset(curr_path, -1, sizeof(curr_path)); 
	memset(visited, 0, sizeof(curr_path)); 

	// Compute initial bound 
	for (int i=0; i<n; i++) 
		curr_bound += (firstMin(adj, n, i) + secondMin(adj, n, i)); 

	// Rounding off the lower bound to an integer 
	curr_bound = (curr_bound&1)? curr_bound/2 + 1 : curr_bound/2; 

	// We start at vertex 1 so the first vertex 
	// in curr_path[] is 0 
	visited[0] = true; 
	curr_path[0] = 0; 

	// Call to TSPRec for curr_weight equal to 
	// 0 and level 1 
	TSPRec(adj, n, curr_bound, 0, 1, curr_path); 
#endif
	for (int i=0; i<=n; i++) 
	{
		//printf("%d ", final_path[i]); 
		traverseOrder.push_back(final_path[i]);
	}
	return 0;
} 


#if 0
// Driver code 
int main() 
{ 
	//Adjacency matrix for the given graph 
	
int adj[N][N]=
   { {0, 1, 1000, 1000, 1000, 1000, 1000, 1},
     {1, 0, 1, 1, 1000, 1000, 1000, 1000},
     {1000, 1, 0, 1000, 1, 1000, 1000, 1000},
     {1000, 1, 1000, 0, 1000, 1000, 1000, 1000},
     {1000, 1000, 1, 1000, 0, 1, 1, 1000},
     {1000, 1000, 1000, 1000, 1, 0, 1000, 1000},
     {1000, 1000, 1000, 1000, 1, 1000, 0, 1000},
     {1, 1000, 1000, 1000, 1, 1000, 1000, 0}
   };
	floyd(adj);
	TSP(adj); 

	printf("Minimum cost : %d\n", final_res); 
	printf("Path Taken : "); 
	for (int i=0; i<=N; i++) 
		printf("%d ", final_path[i]); 
	cout<<endl;

	return 0; 
} 
#endif
