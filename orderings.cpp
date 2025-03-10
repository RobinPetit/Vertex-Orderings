#include <mutex>
#include <stdexcept>
#include <vector>

#define OSTREAM_LL_GRAPH
#include <nautypp/nautypp>

#include "builder.hpp"
#include "utils.hpp"

using namespace nautypp;

template <typename T>
static std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << '{';
    bool first{true};
    for(auto const& x : vec) {
        if(not first)
            os << ", ";
        else
            first = false;
        os << x;
    }
    return os << '}';
}


const auto NB_THREADS{std::thread::hardware_concurrency()};

static inline bool has_odd_degree(const Graph& G) {
    for(auto [deg, count] : G.degree_distribution())
        if(odd(deg) and count > 0)
            return true;
    return false;
}

static inline size_t count_odd_degree_vertices(const Graph& G) {
    size_t ret{0};
    for(auto [deg, count] : G.degree_distribution())
        if(odd(deg))
            ret += count;
    return ret;
}

static inline size_t d_minus(const Graph& G, Vertex v, const std::vector<Vertex>& solution, size_t idx) {
    size_t ret{0};
    for(size_t j{0}; j < idx; ++j)
        if(G.has_edge(v, solution[j]))
            ++ret;
    return ret;
}

static inline bool odo_vertex(const Graph& G, Vertex v, const std::vector<Vertex>& solution, size_t idx) {
    size_t degree{d_minus(G, v, solution, idx)};
    return degree == 0 or odd(degree) == 1;
}

static inline bool oeo_vertex(const Graph& G, Vertex v, const std::vector<Vertex>& solution, size_t idx) {
    size_t degree{d_minus(G, v, solution, idx)};
    return degree == 0 or odd(degree) == 1 or (degree == G.degree(v));
}

template <bool omo>
class _OmoOeoFinder final {
public:
    _OmoOeoFinder(const Graph& graph):
            G{graph}, degrees(graph.degree()),
            nb_odd_vertices{0},
            taken(G.V(), false), solution(G.V(), 0) {
        for(Vertex v{0}; v < G.V(); ++v)
            if(degrees[v])
                ++nb_odd_vertices;
    }

    bool has_oeo_omo() {
        return found = _has_oeo_omo();
    }

    inline const std::vector<Vertex>& get_solution() const {
        if(not found)
            throw std::runtime_error("No solution.");
        return solution;
    }
private:
    const Graph& G;
    std::vector<size_t> degrees;
    size_t nb_odd_vertices;
    std::vector<bool> taken;
    std::vector<Vertex> solution;
    bool found{false};

    bool _has_oeo_omo(Vertex v_idx=0, size_t num_odd_taken=0) {
        if(v_idx == G.V()) {
            return true;
        } else {
            if constexpr(omo) {
                if(num_odd_taken == nb_odd_vertices)
                    return false;
            }
            for(Vertex v{0}; v < G.V(); ++v) {
                if(taken[v]) continue;
                if constexpr(omo) {
                    if(not odo_vertex(G, v, solution, v_idx)) {
                        if(all_neighbours_taken(v)) return false;
                        else                        continue;
                    }
                } else {
                    if(not oeo_vertex(G, v, solution, v_idx)) continue;
                }
                solution[v_idx] = v;
                taken[v] = true;
                if(_has_oeo_omo(v_idx+1, num_odd_taken + (G.degree(v)&1)))
                    return true;
                taken[v] = false;
            }
            solution[v_idx] = 0;
            return false;
        }
    }

    inline bool all_neighbours_taken(Vertex v) const {
        for(Vertex w : G.neighbours_of(v))
            if(not taken[w])
                return false;
        return true;
    }
};

using OmoFinder = _OmoOeoFinder<true>;
using OeoFinder = _OmoOeoFinder<false>;

bool is_omo(const Graph& G) {
    return OmoFinder(G).has_oeo_omo();
}

std::vector<Vertex> get_omo(const Graph& G) {
    OmoFinder finder(G);
    if(finder.has_oeo_omo())
        return finder.get_solution();
    else
        return {};
}

bool is_oeo(const Graph& G) {
    return OeoFinder(G).has_oeo_omo();
}

std::mutex mutex;

static inline void print_graph(const Graph& G, const std::string& prefix="") {
    std::unique_lock<std::mutex> lock(mutex);
    std::cout << prefix << G << std::endl;
}

static inline void test_graphs_oeo(const NautyParameters& params) {
    Nauty().run_async(
        [](const Graph& G) {
            if(not is_oeo(G))
                print_graph(G, "\t");
        },
        params, NB_THREADS, 200
    );
}

static inline void test_graphs_omo(const NautyParameters& params) {
    Nauty().run_async(
        [](const Graph& G) {
            if(not has_odd_degree(G))
                return;
            if(not is_omo(G))
                print_graph(G, "\t");
        },
        params, NB_THREADS, 200
    );
}

static inline void test_graphs_omo(const NautyParameters& params, std::function<bool(const Graph&)> filter) {
    Nauty().run_async(
        [&filter](const Graph& G) {
            if(not has_odd_degree(G) or not filter(G))
                return;
            if(not is_omo(G))
                print_graph(G, "\t");
        },
        params, NB_THREADS, 200
    );
}


static inline Builder cubic_graphs(int max_n) {
    return Builder(4, max_n).regular(3);
}

static inline Builder bipartite_graphs(int max_n) {
    return Builder(3, max_n).bipartite(true);
}

static inline Builder triangle_free_graphs(int max_n) {
    return Builder(3, max_n).triangle_free(true);
}

static inline Builder all_graphs(int max_n) {
    return Builder(1, max_n);
}

static inline bool not_2_edge_connected(const Graph& G) {
    return not Bridge(G).is_2_edge_connected();
}

static inline bool at_least_3_odd_vertices(const Graph& G) {
    return count_odd_degree_vertices(G) > 2;
}

template <GraphFunctionType Callback>
static void apply_on_counterexamples_omo(const NautyParameters& params,
        std::function<bool(const Graph&)> filter,
        Callback callback) {
    Nauty().run_async(
        [&filter, &callback](const Graph& G) {
            if(not has_odd_degree(G) or not filter(G))
                return;
            if(not is_omo(G))
                callback(G);
        },
        params, NB_THREADS, 200
    );
}

int main() {
    constexpr size_t Vmax{12};
    auto graphs{
        // all_graphs(Vmax).biconnected().bipartite()
        // all_graphs(Vmax).biconnected()//.triangle_free()
        all_graphs(Vmax).bipartite()
    };
    // graphs.biconnected = true;
    auto filter{
        [](const Graph& G) {
            return not_2_edge_connected(G) and at_least_3_odd_vertices(G);
        }
    };
    // test_graphs_omo(graphs, filter);
    // test_graphs_omo(graphs);
    // test_graphs_oeo(graphs);
    apply_on_counterexamples_omo(
        graphs,
        [](const Graph& G) {
            return not Bridge(G).is_2_edge_connected();
        },
        [](const Graph& G) {
            if(not Bridge(G).is_even_2_edge_connected())
                print_graph(G, "\t");
        }
    );
    return 0;
}
