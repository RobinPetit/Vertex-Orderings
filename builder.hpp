#ifndef BUILDER_HPP
#define BUILDER_HPP

#include <nautypp/nautypp>

class Builder {
public:
    Builder(int Vmin, int Vmax):
        params{
            .tree=false,
            .connected=true,
            .V=Vmin,
            .Vmax=Vmax
        } {
    }

    inline Builder& bipartite(bool flag=true) { params.bipartite = flag; return *this; }
    inline Builder& regular(int d) { params.min_deg = params.max_deg = d; return *this; }
    inline Builder& biconnected(bool flag=true) { params.biconnected = flag; return *this; }
    inline Builder& triangle_free(bool flag=true) { params.triangle_free = flag; return *this; }

    inline const nautypp::NautyParameters& get() const { return params; }
    inline operator const nautypp::NautyParameters&() const { return get(); }
private:
    nautypp::NautyParameters params;
};

#endif
