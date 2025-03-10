#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

#include <nautypp/nautypp>

static inline bool odd(size_t n) {
    return n&1;
}

static inline bool even(size_t n) {
    return not odd(n);
}

class Bridge {
public:
    Bridge(const nautypp::Graph& graph):
            G{graph}, preorder(G.V(), 0), lowest(G.V(), 0), counter{0} {
    }

    inline bool is_2_edge_connected() {
        return _is_2_edge_connected();
    }

    inline bool is_even_2_edge_connected() {
        return _is_even_2_edge_connected();
    }

private:
    const nautypp::Graph& G;
    std::vector<int> preorder;
    std::vector<int> lowest;
    int counter;

    bool _is_2_edge_connected(nautypp::Vertex u=0, nautypp::Vertex v=0) {
        preorder[v] = ++counter;
        lowest[v] = preorder[v];
        for(auto w : G.neighbours_of(v)) {
            if(preorder[w] == 0) {
                if(not _is_2_edge_connected(v, w))
                    return false;
                lowest[v] = std::min(lowest[v], lowest[w]);
                if(lowest[w] == preorder[w])
                    return false;
            } else if(w != u) {
                lowest[v] = std::min(lowest[v], preorder[w]);
            }
        }
        return true;
    }

    bool _is_even_2_edge_connected(nautypp::Vertex u=0, nautypp::Vertex v=0) {
        preorder[v] = ++counter;
        lowest[v] = preorder[v];
        for(auto w : G.neighbours_of(v)) {
            if(preorder[w] == 0) {
                if(not _is_even_2_edge_connected(v, w))
                    return false;
                lowest[v] = std::min(lowest[v], lowest[w]);
                if(lowest[w] == preorder[w]) {
                    if(not (odd(G.degree(v)) or odd(G.degree(w)))) {
                        std::cout << "Edge " << v << "-" << w << " is a bridge\n";
                        return false;
                    }
                }
            } else if(w != u) {
                lowest[v] = std::min(lowest[v], preorder[w]);
            }
        }
        return true;
    }
};

#endif
