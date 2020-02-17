#ifndef MYGRAPH_H
#define MYGRAPH_H

#include <string>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

struct path {
    int F, T;  // cities
    double distance; // in kilometers
};

enum { A, B, C, D, E, F, G, H, I };  // cities


struct VertexProperty 
{   
	std::string label;
};
struct EdgeProperty {   
	double distance; // in meters
};


typedef boost::adjacency_list<
    boost::listS, boost::vecS, boost::undirectedS,
    VertexProperty,
    EdgeProperty
> graph_t;


typedef graph_t::edge_descriptor edge_t;
typedef graph_t::vertex_descriptor vertex_t;
void getTwoCityShortestPath(int size, int **a, vertex_t from, vertex_t to, std::deque<vertex_t>& path);

#endif
