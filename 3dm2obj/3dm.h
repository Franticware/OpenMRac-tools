#ifndef HLIDAC_3DM_H
#define HLIDAC_3DM_H

#include <cstring>

struct Mapobj {
    Mapobj() : vert_i(0), ang(0), id(0), r(0), f(0) { pos[0] = 0; pos[1] = 0; }
    unsigned int vert_i;
    float ang;
    float pos[2];
    unsigned int id;
    float r;
    float f;
};

class O3dm {
public:
    O3dm():p_i(0),p_sz(0),p_m(0){}
    ~O3dm(){delete[] p_i;}
    unsigned short* p_i; // indices
    unsigned int p_sz; // number of indices
    unsigned int p_m; // material index (+ 1?)
    char p_name[1024];
    float p_cen[3];
};

class T3dm {
public:
    typedef char Matname[256];
    void saveMtl(const char* fname) const;
    void saveObj(const char* fname) const;
    void saveExtra(const char* filename) const;
    void load(const char* fname);
    T3dm() : p_v(0), p_n(0), p_t(0), p_v_sz(0), p_o(0), p_sz(0), p_m(0), p_m_sz(0)/*, p_cen(0)*/ { }
    ~T3dm(){uncreate();}
    float* p_v; // v0 v1 v2
    float* p_n; // n0 n1 n2
    float* p_t; // t0 t1
    unsigned int p_v_sz; // number of vertices
    O3dm* p_o; // objects
    unsigned int p_sz; // number of objects
    Matname* p_m; // materials
    unsigned int p_m_sz; // number of materials
    void uncreate() {delete[] p_v; delete[] p_n; delete[] p_t; delete[] p_o; delete[] p_m;
        p_v=0;p_n=0;p_t=0;p_o=0;p_sz=0;p_m=0;}
};

#endif
