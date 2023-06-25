#include "3dm.h"
#include "cstr1.h"

#include <glm/gtc/quaternion.hpp>

#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

namespace T3DMf {

unsigned int maxuint(unsigned int a, unsigned int b)
{
    if (a > b)
        return a;
    else
        return b;
}

unsigned int minuint(unsigned int a, unsigned int b)
{
    if (a < b)
        return a;
    else
        return b;
}

void memsetf(float* fv, float f, unsigned int size)
{
    for (unsigned int i = 0; i != size; ++i)
        fv[i] = f;
}

void crossprod(float* n, const float* a, const float* b)
{
    n[0] = a[1]*b[2]-b[1]*a[2];
    n[1] = a[2]*b[0]-b[2]*a[0];
    n[2] = a[0]*b[1]-b[0]*a[1];
}

void normalize(float vect[3])
{
    float l = sqrt(vect[0] * vect[0] + vect[1] * vect[1] + vect[2] * vect[2]);
    vect[0] /= l;
    vect[1] /= l;
    vect[2] /= l;
    if (!std::isfinite(vect[0]) || !std::isfinite(vect[1]) || !std::isfinite(vect[2]))
    {
        vect[0] = 0.f;
        vect[1] = 0.f;
        vect[2] = 1.f;
    }
}

}

void T3dm::saveMtl(const char* fname) const
{
    FILE* fout = fopen(fname, "w");
    fprintf(fout, "# 3dm2obj\n# www.franticware.com\n");
    for (unsigned i = 1; i < p_m_sz; ++i)
    {
        fprintf(fout, "\nnewmtl %s\n", p_m[i]);
        fprintf(fout, "Ns 250.000000\nKa 1.000000 1.000000 1.000000\nKs 0.500000 0.500000 0.500000\nKe 0.000000 0.000000 0.000000\nNi 1.450000\nd 1.000000\nillum 2\n");
        fprintf(fout, "map_Kd %s\n", p_m[i]);
    }
    fclose(fout);
}

void T3dm::saveObj(const char* fname) const
{
    char filenameBuff[1024] = {0};
    toFilename(filenameBuff, 1023, fname);
    toFilenameNoext(filenameBuff);
    FILE* fout = fopen(fname, "w");
    fprintf(fout, "# 3dm2obj\n# www.franticware.com\nmtllib %s.mtl\n", filenameBuff);
    for (unsigned j = 0; j != p_v_sz; ++j)
    {
        fprintf(fout, "v %f %f %f\n", p_v[j * 3 + 0], p_v[j * 3 + 1], p_v[j * 3 + 2]);
    }
    for (unsigned j = 0; j != p_v_sz; ++j)
    {
        fprintf(fout, "vn %f %f %f\n", p_n[j * 3 + 0], p_n[j * 3 + 1], p_n[j * 3 + 2]);
    }
    for (unsigned j = 0; j != p_v_sz; ++j)
    {
        fprintf(fout, "vt %f %f\n", p_t[j * 2 + 0], p_t[j * 2 + 1]);
    }
    for (int i = 0; i != int(p_sz); ++i)
    {
        fprintf(fout, "o %s\n", p_o[i].p_name);
        fprintf(fout, "s 1\n");
        if (p_o[i].p_m)
        {
            fprintf(fout, "usemtl %s\n", p_m[p_o[i].p_m]);
        }

        for (unsigned j = 0; j != p_o[i].p_sz/3; ++j)
        {
            int v0 = p_o[i].p_i[j * 3 + 0] + 1;
            int v1 = p_o[i].p_i[j * 3 + 1] + 1;
            int v2 = p_o[i].p_i[j * 3 + 2] + 1;
            fprintf(fout, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", v0, v0, v0, v1, v1, v1, v2, v2, v2);
        }
    }
    fclose(fout);
}

static bool cmp_mapobj_id(const Mapobj& m1, const Mapobj& m2)
{
    return m1.id < m2.id;
}

void T3dm::saveExtra(const char* filename) const
{
    const O3dm* m_o = 0;
    for (unsigned i = 0; i != p_sz; ++i)
    {
        if (strcmp("mapobject", p_o[i].p_name) == 0)
        {
            m_o = p_o + i;
        }
    }
    if (m_o == 0) return;

    std::vector<Mapobj> p_mapobjs; // objektová mapa

    for (unsigned int i = 0; i != m_o->p_sz; ++i)
    {
        bool bfound = false;
        // projdou se všechny objekty v objektové mapě, aby se bod nevkládal 2x
        for (std::vector<Mapobj>::const_iterator it = p_mapobjs.begin(); it != p_mapobjs.end(); ++it)
        {
            if (it->vert_i == m_o->p_i[i] ||
                ( // pokud je to jiný bod ve stejném místě
                    p_v[it->vert_i*3+0] == p_v[m_o->p_i[i]*3+0] &&
                    p_v[it->vert_i*3+1] == p_v[m_o->p_i[i]*3+1] &&
                    p_v[it->vert_i*3+2] == p_v[m_o->p_i[i]*3+2]
                    )) // pokud už pole obsahuje bod s indexem, je nalezen
            {
                bfound = true;
                break;
            }
        }
        if (!bfound)
        {
            Mapobj mapobj;
            mapobj.vert_i = m_o->p_i[i];
            mapobj.ang = m_o->p_cen[1]+p_v[m_o->p_i[i]*3+1]*M_PI;
            mapobj.ang /= 10.f;
            mapobj.pos[0] = m_o->p_cen[0]+p_v[m_o->p_i[i]*3+2];
            mapobj.pos[1] = m_o->p_cen[2]+p_v[m_o->p_i[i]*3+0];
            const int texo_w = 8;
            const int texo_h = 8;
            int texo_x = 0;
            int texo_y = 0;
            texo_x = (int)floor(p_t[m_o->p_i[i]*2+0]*float(texo_w));
            texo_y = (int)floor(p_t[m_o->p_i[i]*2+1]*float(texo_h));
            int o_id = texo_x+texo_y*texo_w;
            --o_id;
            if (o_id >= 0)
            {
                mapobj.id = o_id;
                p_mapobjs.push_back(mapobj);
            }
        }
    }
    std::sort(p_mapobjs.begin(), p_mapobjs.end(), cmp_mapobj_id);

    FILE* fout = fopen(filename, "w");
    fprintf(fout, "# 3dm2obj\n# www.franticware.com\n");
    for (size_t i = 0; i != p_sz; ++i)
    {
        const O3dm& o = p_o[i];
        fprintf(fout, "o %s\n", o.p_name);
        fprintf(fout, "c %f %f %f\n", o.p_cen[0], o.p_cen[1], o.p_cen[2]);
    }
    for (size_t i = 0; i != p_mapobjs.size(); ++i)
    {
        const Mapobj& o = p_mapobjs[i];
        fprintf(fout, "e Mapobj.%d.%03d\n", o.id, int(i));
        fprintf(fout, "c %f 0 %f\n", o.pos[0], o.pos[1]);
        glm::quat q = glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), o.ang, glm::vec3(0.f, 0.f, 1.f));
        fprintf(fout, "q %f %f %f %f\n", q.x, q.y, q.z, q.w);
    }
    fclose(fout);
}

void T3dm::load(const char* fname)
{
    FILE* fin;
    uncreate(); // vymazání předchozího
    if (!(fin = fopen(fname, "r")))
        return;
    unsigned int vertexnum = 0; // celkový počet objektů
    char buff[1024]; // buffer pro načtení řádků
    while (fgets(buff, 1024, fin)) // sečtení všech vertexů
    {
        unsigned int vertexnum1 = 0;
        if (sscanf(buff, "v %u", &vertexnum1) == 1)
            vertexnum += vertexnum1;
    }
    rewind(fin);
    p_v_sz = vertexnum;
    p_v = new float[vertexnum*3]; // alokace pole vertexů
    p_n = new float[vertexnum*3]; // alokace pole normál
    T3DMf::memsetf(p_n, 0.0f, vertexnum*3); // vynulování pole normál (vlastní spec. fce memsetf pro floaty)
    p_t = new float[vertexnum*2]; // alokace pole texturovacích souřadnic
    unsigned char* nf = new unsigned char[vertexnum]; // normal flag (flag společné normály)
    memset(nf, 0, vertexnum); // počáteční vynulování všech normal flagů
    unsigned int objectnum = 0, texturenum = 0; // počet objektů, textur
    unsigned int object_i = 0; // index aktuálního objektu
    bool b_otnum = false; // načten počet objektů a textur
    bool b_eof = false; // konec souboru
    unsigned char* mainflags = 0; // flag hlavního objektu
    float* centers = 0; // středy objektů
    unsigned int points_i0 = 0; // mezivýpočet hodnoty počtu bodů
    unsigned int points_n = 0; // proměnná pro načtení počtu bodů
    unsigned int faces_n = 0; // proměnná pro načtení počtu pložek
    unsigned int points_i0_prev = 0; // spolupráce s points_i0 ve for
    float point_prev[3] = {0}; // uchování předchozího bodu (pro body se společnými normálami, kde je uvedeno pouze UV textury)
    while (!b_eof && fgets(buff, 1024, fin)) // načítání souboru po řádcích (vpodstatě přečtení 1. řádku a řádků s "o " na začátku)
    {

        if (sscanf(buff, "3dm %u %u", &objectnum, &texturenum) == 2) // načtení počtu objektů a textur
        {
            b_otnum = true; // je už načtený počet objektů a textur?
            p_o = new O3dm[objectnum]; // alokace objektů
            p_sz = objectnum; // přiřazení počtu objektů
            p_m_sz = texturenum+1;
            centers = new float[objectnum*3]; // alokace pole pro středy jednotlivých objektů
            mainflags = new unsigned char[objectnum]; // alokace pole flagů hlavních objektů
            memset(mainflags, 0, objectnum); // počáteční nulování flagů
            p_m = new Matname[p_m_sz];
            p_m[0][0] = '\0';
            for (unsigned int i = 0; i != texturenum; ++i) // načtení názvů textur
            {
                if (!fgets(buff, 1024, fin)) { b_eof = true; break; }
                // textury se zahazují (dodělat)
                strncpy(p_m[i+1], buff, 255);
                p_m[i+1][255] = '\0';
                for (int j = strlen(p_m[i+1]); j != 0; --j)
                {
                    if (!isSpace(p_m[i+1][j-1]))
                        break;
                    p_m[i+1][j-1] = '\0';
                }
            }
        }
        else if (b_otnum) // v případě načtení počtu objektů a textur
        {
            if (buff[0] == 'o' && buff[1] == ' ') // když řádek začíná "o "
            {
                strcpy(p_o[object_i].p_name, buff+2);
                rtrim(p_o[object_i].p_name);
                fgets(buff, 1024, fin); // načtení středu a čísla textury objektu
                if (sscanf(buff, "%f %f %f %u", &(centers[object_i*3]),
                    &(centers[object_i*3+1]), &(centers[object_i*3+2]),
                    &(p_o[object_i].p_m)) != 4)
                {
                }
                p_o[object_i].p_cen[0] = centers[object_i*3];
                p_o[object_i].p_cen[1] = centers[object_i*3+1];
                p_o[object_i].p_cen[2] = centers[object_i*3+2];
                fgets(buff, 1024, fin);
                if (sscanf(buff, "v %u", &points_n) != 1)
                {
                }
                points_i0_prev = points_i0; // od předchozího počtu začíná for načítající vertexy
                points_i0 += points_n; // počet načtených vertexů
                for (unsigned int i = points_i0_prev; i != points_i0; ++i) // načtení vertexů
                {
                    fgets(buff, 1024, fin);
                    if (*buff == ' ') // řádek začíná mezerou, je na něm jen UV, vertex a normála je společná s předchozím
                    {
                        p_v[i*3  ] = point_prev[0];
                        p_v[i*3+1] = point_prev[1];
                        p_v[i*3+2] = point_prev[2];
                        if (sscanf(buff, " %f %f", &(p_t[i*2]), &(p_t[i*2+1])) != 2)
                        {
                        }
                        nf[i] = 1;
                    } else { // řádek obsahuje vertex a UV
                        if (sscanf(buff, "%f %f %f %f %f",
                            &(p_v[i*3]), &(p_v[i*3+1]), &(p_v[i*3+2]),
                            &(p_t[i*2]), &(p_t[i*2+1])) != 5)
                        {
                        }
                        point_prev[0] = p_v[i*3  ];
                        point_prev[1] = p_v[i*3+1];
                        point_prev[2] = p_v[i*3+2];
                    }
                    if (!mainflags[object_i]) // posunutí vedlejšího objektu na střed
                    {
                        p_v[i*3  ] += centers[object_i*3+0];
                        p_v[i*3+1] += centers[object_i*3+1];
                        p_v[i*3+2] += centers[object_i*3+2];
                    }
                }
                fgets(buff, 1024, fin); // načtení počtu pložek
                if (sscanf(buff, "f %u", &faces_n) != 1)
                {
                }
                p_o[object_i].p_sz = faces_n*3; // počet indexů bodů
                p_o[object_i].p_i = new unsigned short[faces_n*3]; // alokace pole indexů
                for (unsigned int i = 0; i != faces_n; ++i) // načtení indexů vertexů plošek
                {
                    fgets(buff, 1024, fin);
                    unsigned int index0 = 0, index1 = 0, index2 = 0;
                    if (sscanf(buff, "%u %u %u", &index0,
                        &index1, &index2) != 3)
                    {
                    }
                    p_o[object_i].p_i[i*3] = index0;
                    p_o[object_i].p_i[i*3+1] = index1;
                    p_o[object_i].p_i[i*3+2] = index2;

                    p_o[object_i].p_i[i*3  ] += points_i0_prev;
                    p_o[object_i].p_i[i*3+1] += points_i0_prev;
                    p_o[object_i].p_i[i*3+2] += points_i0_prev;
                }

                for (unsigned int i = 0; i != faces_n; ++i)
                {
                    unsigned short* &p_i = p_o[object_i].p_i;
                    float normal[3];
                    float v0[3] = {
                        p_v[p_i[i*3+1]*3  ]-p_v[p_i[i*3]*3  ],
                        p_v[p_i[i*3+1]*3+1]-p_v[p_i[i*3]*3+1],
                        p_v[p_i[i*3+1]*3+2]-p_v[p_i[i*3]*3+2]};
                    float v1[3] = {
                        p_v[p_i[i*3+2]*3  ]-p_v[p_i[i*3]*3  ],
                        p_v[p_i[i*3+2]*3+1]-p_v[p_i[i*3]*3+1],
                        p_v[p_i[i*3+2]*3+2]-p_v[p_i[i*3]*3+2]};
                    T3DMf::crossprod(normal, v0, v1); // výpočet normály plošky (nenormalizované, velikost podle obsahu plošky)
                    for (unsigned int c = 0; c != 3; ++c) // zopakování pro každý index vertexu plo?ky
                    {
                        unsigned int j = p_i[i*3+c]; // j <- aktuální index vertexu
                        while (j != 0 && nf[j]) // nalezení prvního indexu v případě společné normály
                            --j;
                        do { // přičtení normály všem společným vrcholům (počet vrcholů = 0...n)
                            p_n[j*3  ] += normal[0];
                            p_n[j*3+1] += normal[1];
                            p_n[j*3+2] += normal[2];
                            ++j;
                        } while (j != vertexnum && nf[j] == 1);
                    }
                }
                ++object_i; // zvětšení hodnoty indexu objektu
            }
        }
    }
    fclose(fin);
    for (unsigned int i = 0; i != vertexnum; ++i) // normalizace normál
    {
        T3DMf::normalize(p_n + i * 3);
    }
    // smazání pracovních polí
    delete[] nf;
    delete[] centers;
    delete[] mainflags;
}
