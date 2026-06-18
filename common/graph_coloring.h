
#ifndef GRAPH_COLORING_H
#define GRAPH_COLORING_H

#include "graph.h"
#include <map>
#include <set>
#include <vector>
#include <stack>

template <class NODE>
class GRAPH_COLORING {
public:
  typedef std::map<NODE, int> COLOR_MAP;

  static bool Greedy_color(
      const GRAPH<NODE>& graph,
      int color_count,
      COLOR_MAP& colors)
  {
    colors.clear();

    std::vector<NODE> nodes;
    graph.Nodes(nodes);

    typename std::vector<NODE>::iterator nit;
    for (nit = nodes.begin(); nit != nodes.end(); ++nit) {
      NODE n = *nit;

      std::set<int> used;

      typename GRAPH<NODE>::NODE_SET::const_iterator ait;
      const typename GRAPH<NODE>::NODE_SET& adj = graph.Neighbors(n);

      for (ait = adj.begin(); ait != adj.end(); ++ait) {
        typename COLOR_MAP::const_iterator cit = colors.find(*ait);
        if (cit != colors.end()) {
          used.insert(cit->second);
        }
      }

      int chosen = -1;

      for (int c = 0; c < color_count; ++c) {
        if (used.find(c) == used.end()) {
          chosen = c;
          break;
        }
      }

      if (chosen < 0) {
        return false;
      }

      colors[n] = chosen;
    }

    return true;
  }

  static bool Simplify_color(
      const GRAPH<NODE>& graph,
      int color_count,
      COLOR_MAP& colors,
      std::set<NODE>& spills)
  {
    colors.clear();
    spills.clear();

    GRAPH<NODE> work = graph;
    std::stack<NODE> select_stack;

    while (work.Node_count() > 0) {
      std::vector<NODE> nodes;
      work.Nodes(nodes);

      NODE chosen = nodes[0];
      bool found_low_degree = false;

      typename std::vector<NODE>::iterator nit;
      for (nit = nodes.begin(); nit != nodes.end(); ++nit) {
        if (work.Degree(*nit) < (unsigned)color_count) {
          chosen = *nit;
          found_low_degree = true;
          break;
        }
      }

      if (!found_low_degree) {
        // Spill candidate: pick first for now.
        // Later improve with cost / degree heuristic.
        chosen = nodes[0];
        spills.insert(chosen);
      }

      select_stack.push(chosen);
      work.Remove_node(chosen);
    }

    while (!select_stack.empty()) {
      NODE n = select_stack.top();
      select_stack.pop();

      std::set<int> used;

      const typename GRAPH<NODE>::NODE_SET& adj = graph.Neighbors(n);
      typename GRAPH<NODE>::NODE_SET::const_iterator ait;

      for (ait = adj.begin(); ait != adj.end(); ++ait) {
        typename COLOR_MAP::const_iterator cit = colors.find(*ait);
        if (cit != colors.end()) {
          used.insert(cit->second);
        }
      }

      int chosen_color = -1;

      for (int c = 0; c < color_count; ++c) {
        if (used.find(c) == used.end()) {
          chosen_color = c;
          break;
        }
      }

      if (chosen_color < 0) {
        spills.insert(n);
      } else {
        colors[n] = chosen_color;
        spills.erase(n);
      }
    }

    return spills.empty();
  }
};

#endif