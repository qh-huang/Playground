#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <string>
#include <vae/mygraph.h>

#if 0
struct VertexProperty {
    std::string label;
};

struct EdgeProperty {
    double distance; // in kilometers
};

typedef boost::adjacency_list<
    boost::listS, boost::vecS, boost::directedS,
    VertexProperty,
    EdgeProperty
> graph_t;

typedef graph_t::edge_descriptor edge_t;
typedef graph_t::vertex_descriptor vertex_t;
#endif

class my_visitor : boost::default_bfs_visitor {
protected:
  vertex_t destination_vertex_m;

public:
  my_visitor(vertex_t destination_vertex_l)
      : destination_vertex_m(destination_vertex_l){};
  void initialize_vertex(const vertex_t &s, const graph_t &g) const {}
  void discover_vertex(const vertex_t &s, const graph_t &g) const {}
  void examine_vertex(const vertex_t &s, const graph_t &g) const {}
  void examine_edge(const edge_t &e, const graph_t &g) const {}
  void edge_relaxed(const edge_t &e, const graph_t &g) const {}
  void edge_not_relaxed(const edge_t &e, const graph_t &g) const {}
  void finish_vertex(const vertex_t &s, const graph_t &g) const {
    if (destination_vertex_m == s)
      throw(2);
  }
};

#if 0
int a[8][8]=
   { {0, 1, 1000, 1000, 1000, 1000, 1000, 1},
     {1, 0, 1, 1, 1000, 1000, 1000, 1000},
     {1000, 1, 0, 1000, 1, 1000, 1000, 1000},
     {1000, 1, 1000, 0, 1000, 1000, 1000, 1000},
     {1000, 1000, 1, 1000, 0, 1, 1000, 1000},
     {1000, 1000, 1000, 1000, 1, 0, 1000, 1000},
     {1000, 1000, 1000, 1000, 1, 1000, 0, 1000},
     {1, 1000, 1000, 1000, 1, 1000, 1000, 0}
   };
#endif
graph_t make_graph(int cityNum, int **a) {
  graph_t g;
  // std::vector<std::string> cities = {"A", "B", "C", "D", "E" , "F", "G",
  // "H"};
  std::vector<std::string> cities;
  for (int i = 0; i < cityNum; i++) {
    cities.push_back(std::to_string(i));
  }
  std::vector<path> paths;
  path tmp;
  int i = 0, j = 0;
  for (i = 0; i < cityNum; i++) {
    for (j = 0; j < cityNum; j++) {
      tmp.F = i;
      tmp.T = j;
      tmp.distance = a[i][j];
      paths.push_back(tmp);
    }
  }

  for (const auto &p : paths) {
    add_edge(p.F, p.T, {p.distance}, g);
  }
  for (unsigned int i = 0; i < num_vertices(g); i++) {
    g[i].label = cities[i];
  }
  return g;
}

std::deque<vertex_t> get_path(vertex_t from, vertex_t to,
                              std::vector<vertex_t> parents) {
  std::deque<vertex_t> path;
  for (vertex_t v = to;; v = parents[v]) {
    path.push_front(v);
    if (v == from)
      break;
  }
  return path;
}

void print_path(graph_t g, std::deque<vertex_t> &path, double distance_to) {
  for (const auto &v : path) {
    std::cout << g[v].label;
    if (v != path.back()) {
      std::cout << " -> ";
    }
  }
  std::cout << "  (" << distance_to << " km)" << '\n';
}

void shortest_path(graph_t g, vertex_t from, vertex_t to,
                   std::deque<vertex_t> &path) {
  my_visitor vis(to);
  std::vector<vertex_t> parents(num_vertices(g));
  std::vector<double> distance(num_vertices(g));
  try {
    std::cout << "path-finding starts";
    boost::dijkstra_shortest_paths(
        g, from,
        boost::weight_map(boost::get(&EdgeProperty::distance, g))
            .predecessor_map(&parents[0])
            .distance_map(&distance[0])
            .visitor(vis));
  } catch (int exception) {
    std::cout << ", and then finishes";
  }
  std::cout << '\n';

  if (parents[to] == to) {
    std::cout << "no solultion found" << '\n';
  } else {
    std::cout << "shortest path found:" << '\n';
    path = get_path(from, to, parents);
    print_path(g, path, distance[to]);
  }
}

void getTwoCityShortestPath(int size, int **a, vertex_t from, vertex_t to,
                            std::deque<vertex_t> &path) {
  graph_t g = make_graph(size, a);
  std::cout << "number of (nodes, edges) in a graph = (" << num_vertices(g);
  std::cout << ", " << num_edges(g) << ")\n";
  std::cout << "(from, to) = (" << g[from].label << ", " << g[to].label
            << ")\n";
  shortest_path(g, from, to, path);
}

#if 0
int main()
{
	std::deque<vertex_t> path;
	getTwoCityShortestPath(H, C, path);
}
#endif
