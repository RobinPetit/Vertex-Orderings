#include <mutex>
#include <vector>

#define OSTREAM_LL_GRAPH
#include <nautypp/nautypp>

using namespace nautypp;

static inline bool odo_vertex(const Graph& G, Vertex v, const std::vector<Vertex>& solution, size_t idx) {
    size_t degree{0};
    for(size_t j{0}; j < idx; ++j)
        if(G.has_edge(v, solution[j]))
            ++degree;
    return degree == 0 or (degree & 1) == 1;
}

static inline bool oeo_vertex(const Graph& G, Vertex v, const std::vector<Vertex>& solution, size_t idx) {
    size_t d_minus{0};
    for(size_t j{0}; j < idx; ++j)
        if(G.has_edge(v, solution[j]))
            ++d_minus;
    return d_minus == 0 or (d_minus& 1) == 1 or (d_minus == G.degree(v));
}

template <bool omo>
bool _is_omo_oeo(const Graph& G, std::vector<bool>& taken, std::vector<Vertex>& solution, Vertex v_idx=0) {
    if(v_idx == G.V()) {
        return true;
    } else {
        for(Vertex v{0}; v < G.V(); ++v) {
            if(taken[v]) continue;
            if constexpr(omo) {
                if(not odo_vertex(G, v, solution, v_idx)) continue;
            } else {
                if(not oeo_vertex(G, v, solution, v_idx)) continue;
            }
            solution[v_idx] = v;
            taken[v] = true;
            if(_is_omo_oeo<omo>(G, taken, solution, v_idx+1))
                return true;
            taken[v] = false;
        }
        return false;
    }
}

bool is_omo(const Graph& G) {
    std::vector<Vertex> solution(G.V());
    std::vector<bool> taken(G.V(), false);
    return _is_omo_oeo<true>(G, taken, solution);
}

bool is_oeo(const Graph& G) {
    std::vector<Vertex> solution(G.V());
    std::vector<bool> taken(G.V(), false);
    return _is_omo_oeo<false>(G, taken, solution);
}

std::mutex mutex;

void print_graph(const Graph& G, const std::string& prefix="") {
    std::unique_lock<std::mutex> lock(mutex);
    std::cout << prefix << G << std::endl;
}

static inline bool has_odd_degree(const Graph& G) {
    for(auto [d, _] : G.degree_distribution())
        if(d&1 and _ > 0)
            return true;
    return false;
}

static void verify_subcubic_are_oeo(int max_n) {
    NautyParameters params{
        .tree=false,
        .connected=true,
        .V=4,
        .Vmax=max_n,
        .max_deg=3
    };
    Nauty().run_async(
        [](const Graph& G) {
            if(not is_oeo(G))
                print_graph(G, "\t");
        },
        params,
        12,
        200
    );
}

static inline void test_graphs_oeo(const NautyParameters& params) {
    Nauty nauty;
    nauty.run_async(
        [](const Graph& G) {
            if(not is_oeo(G))
                print_graph(G, "\t");
        },
        params,
        12,
        200
    );
}

static inline void test_graphs_omo(const NautyParameters& params) {
    Nauty nauty;
    nauty.run_async(
        [](const Graph& G) {
            if(not has_odd_degree(G))
                return;
            if(not is_omo(G))
                print_graph(G, "\t");
        },
        params,
        12,
        200
    );
}

static inline NautyParameters cubic_graphs(size_t max_n) {
    return {
        .tree=false,
        .connected=true,
        .V=4,
        .Vmax=static_cast<int>(max_n),
        .min_deg=1,
        .max_deg=3
    };
}

static inline NautyParameters bipartite_graphs(size_t max_n) {
    return {
        .tree=false,
        .connected=true,
        .bipartite=true,
        .V=3,
        .Vmax=static_cast<int>(max_n),
        .min_deg=1,
        .max_deg=static_cast<int>(max_n)
    };
}

static inline void test_cubic_graphs_omo(size_t max_n=18) {
    test_graphs_omo(cubic_graphs(max_n));
}

static inline void test_cubic_graphs_oeo(size_t max_n=18) {
    test_graphs_oeo(cubic_graphs(max_n));
}

static inline void test_bipartite_graphs_omo(size_t max_n=10) {
    test_graphs_omo(bipartite_graphs(max_n));
}

static inline void test_bipartite_graphs_oeo(size_t max_n=10) {
    test_graphs_oeo(bipartite_graphs(max_n));
}

int main() {
    // test_cubic_graphs(15);
    // verify_subcubic_are_oeo(15);
    // test_bipartite_graphs_omo(10);
    test_bipartite_graphs_oeo(14);
    return 0;
}
