#ifndef GRAPH_H
#define GRAPH_H

#include <map>
#include <set>
#include <vector>
#include <stack>

template <class NODE>
class GRAPH {
public:
  typedef std::set<NODE> NODE_SET;
  typedef std::map<NODE, NODE_SET> ADJ_MAP;

private:
  ADJ_MAP _adj;

public:
  void Clear() {
    _adj.clear();
  }

  bool Has_node(const NODE& n) const {
    return _adj.find(n) != _adj.end();
  }

  void Add_node(const NODE& n) {
    if (!Has_node(n)) {
      _adj[n] = NODE_SET();
    }
  }

  void Add_edge(const NODE& a, const NODE& b) {
    if (a == b) return;

    Add_node(a);
    Add_node(b);

    _adj[a].insert(b);
    _adj[b].insert(a);
  }

  void Remove_edge(const NODE& a, const NODE& b) {
    typename ADJ_MAP::iterator ia = _adj.find(a);
    typename ADJ_MAP::iterator ib = _adj.find(b);

    if (ia != _adj.end()) ia->second.erase(b);
    if (ib != _adj.end()) ib->second.erase(a);
  }

  void Remove_node(const NODE& n) {
    typename ADJ_MAP::iterator it = _adj.find(n);
    if (it == _adj.end()) return;

    NODE_SET neighbors = it->second;

    typename NODE_SET::iterator nit;
    for (nit = neighbors.begin(); nit != neighbors.end(); ++nit) {
      _adj[*nit].erase(n);
    }

    _adj.erase(n);
  }

  unsigned Degree(const NODE& n) const {
    typename ADJ_MAP::const_iterator it = _adj.find(n);
    if (it == _adj.end()) return 0;
    return (unsigned)it->second.size();
  }

  const NODE_SET& Neighbors(const NODE& n) const {
    typename ADJ_MAP::const_iterator it = _adj.find(n);
    return it->second;
  }

  bool Adjacent(const NODE& a, const NODE& b) const {
    typename ADJ_MAP::const_iterator it = _adj.find(a);
    if (it == _adj.end()) return false;
    return it->second.find(b) != it->second.end();
  }

  unsigned Node_count() const {
    return (unsigned)_adj.size();
  }

  unsigned Edge_count() const {
    unsigned total = 0;

    typename ADJ_MAP::const_iterator it;
    for (it = _adj.begin(); it != _adj.end(); ++it) {
      total += (unsigned)it->second.size();
    }

    return total / 2;
  }

  void Nodes(std::vector<NODE>& out) const {
    out.clear();

    typename ADJ_MAP::const_iterator it;
    for (it = _adj.begin(); it != _adj.end(); ++it) {
      out.push_back(it->first);
    }
  }

  const ADJ_MAP& Adj_map() const {
    return _adj;
  }
};

#endif