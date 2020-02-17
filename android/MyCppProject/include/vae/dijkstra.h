#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <vector>
using namespace std;
// Data structure to store graph edges
struct Edge {
  int source, dest, weight;
};

// data structure to store heap nodes
class Node {
public:
  int vertex, weight;
};

// Comparison object to be used to order the heap
struct comp {
  bool operator()(const Node &lhs, const Node &rhs) const {
    return lhs.weight > rhs.weight;
  }
};

// class to represent a graph object
class Graph {
public:
  // construct a vector of vectors of Edge to represent an adjacency list
  vector<vector<Edge>> adjList;

  // Graph Constructor
  Graph(vector<Edge> const &edges, int N) {
    // resize the vector to N elements of type vector<Edge>
    adjList.resize(N);

    // add edges to the undirected graph
    for (Edge const &edge : edges) {
      // insert at end
      adjList[edge.source].push_back(edge);
    }
  }
};

#endif
