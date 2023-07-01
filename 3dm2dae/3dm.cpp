#include "3dm.h"
#include "cstr1.h"

#include <glm/gtc/quaternion.hpp>

#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include <vector>
#include <set>
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

namespace T3DMs {

void procName(char* str)
{
    if (*str)
    {
        if (!isAlpha(*str))
        {
            *str = '_';
        }
        for (size_t i = 1; i < strlen(str); ++i)
        {
            if (!isAlNum(str[i]))
            {
                str[i] = '_';
            }
        }
    }
}

}

void T3dm::saveDae(const char* filename) const
{
    time_t now;
    time(&now);
    char timebuf[64] = {0};
    strftime(timebuf, 63, "%FT%T", gmtime(&now));
    FILE* fout = fopen(filename, "w");
    fputs("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n", fout);
    fputs("<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n", fout);
    fputs("  <asset>\n", fout);
    fputs("    <contributor>\n", fout);
    fputs("      <author>3dm2dae User</author>\n", fout);
    fputs("      <authoring_tool>3dm2dae</authoring_tool>\n", fout);
    fputs("    </contributor>\n", fout);
    fprintf(fout, "    <created>%s</created>\n", timebuf);
    fprintf(fout, "    <modified>%s</modified>\n", timebuf);
    fputs("    <unit name=\"meter\" meter=\"1\"/>\n", fout);
    fputs("    <up_axis>Z_UP</up_axis>\n", fout);
    fputs("  </asset>\n", fout);
    fputs("  <library_cameras>\n", fout);
    fputs("  </library_cameras>\n", fout);
    fputs("  <library_lights>\n", fout);
    fputs("  </library_lights>\n", fout);
    fputs("  <library_effects>\n", fout);
    for (size_t i = 1; i < p_m_sz; ++i)
    {
        T3dmName nm;
        memcpy(nm, p_m[i], sizeof nm);
        T3DMs::procName(nm);
        fprintf(fout, "    <effect id=\"%s-effect\">\n", nm);
        fprintf(fout, "      <profile_COMMON>\n");
        fprintf(fout, "        <newparam sid=\"%s-surface\">\n", nm);
        fprintf(fout, "          <surface type=\"2D\">\n");
        fprintf(fout, "            <init_from>%s</init_from>\n", nm);
        fprintf(fout, "          </surface>\n");
        fprintf(fout, "        </newparam>\n");
        fprintf(fout, "        <newparam sid=\"%s-sampler\">\n", nm);
        fprintf(fout, "          <sampler2D>\n");
        fprintf(fout, "            <source>%s-surface</source>\n", nm);
        fprintf(fout, "          </sampler2D>\n");
        fprintf(fout, "        </newparam>\n");
        fprintf(fout, "        <technique sid=\"common\">\n");
        fprintf(fout, "          <lambert>\n");
        fprintf(fout, "            <emission>\n");
        fprintf(fout, "              <color sid=\"emission\">0 0 0 1</color>\n");
        fprintf(fout, "            </emission>\n");
        fprintf(fout, "            <diffuse>\n");
        fprintf(fout, "              <texture texture=\"%s-sampler\" texcoord=\"UVMap\"/>\n", nm);
        fprintf(fout, "            </diffuse>\n");
        fprintf(fout, "            <index_of_refraction>\n");
        fprintf(fout, "              <float sid=\"ior\">1.45</float>\n");
        fprintf(fout, "            </index_of_refraction>\n");
        fprintf(fout, "          </lambert>\n");
        fprintf(fout, "        </technique>\n");
        fprintf(fout, "      </profile_COMMON>\n");
        fprintf(fout, "    </effect>\n");
    }
    fputs("  </library_effects>\n", fout);
    fputs("  <library_images>\n", fout);
    for (size_t i = 1; i < p_m_sz; ++i)
    {
        T3dmName nm;
        memcpy(nm, p_m[i], sizeof nm);
        T3DMs::procName(nm);
        fprintf(fout, "    <image id=\"%s\" name=\"%s\">\n", nm, nm);
        fprintf(fout, "      <init_from>%s</init_from>\n", p_m[i]);
        fprintf(fout, "    </image>\n");
    }
    fputs("  </library_images>\n", fout);
    fputs("  <library_materials>\n", fout);
    for (size_t i = 1; i < p_m_sz; ++i)
    {
        T3dmName nm;
        memcpy(nm, p_m[i], sizeof nm);
        T3DMs::procName(nm);
        fprintf(fout, "    <material id=\"%s-material\" name=\"%s\">\n", nm, nm);
        fprintf(fout, "      <instance_effect url=\"#%s-effect\"/>\n", nm);
        fprintf(fout, "    </material>\n");
    }
    fputs("  </library_materials>\n", fout);
    fputs("  <library_geometries>\n", fout);
    for (size_t i = 0; i != p_sz; ++i)
    {
        O3dm& obj = p_o[i];
        std::set<unsigned short> indexMapSet(obj.p_i, obj.p_i + obj.p_sz);
        std::vector<unsigned short> indexMap;
        indexMap.assign(indexMapSet.begin(), indexMapSet.end());
        std::vector<unsigned short> indexRev;
        if (indexMap.size() > 0)
        {
            indexRev.resize(indexMap[indexMap.size()-1]+1, 0);
            for (size_t j = 0; j != indexMap.size(); ++j)
            {
                indexRev[indexMap[j]] = j;
            }
        }
        T3dmName nm;
        memcpy(nm, obj.p_name, sizeof nm);
        T3DMs::procName(nm);
        fprintf(fout, "    <geometry id=\"%s-mesh\" name=\"%s\">\n", nm, obj.p_name);
        fprintf(fout, "      <mesh>\n");
        fprintf(fout, "        <source id=\"%s-mesh-positions\">\n", nm);
        fprintf(fout, "          <float_array id=\"%s-mesh-positions-array\" count=\"%u\">", nm, (unsigned int)(indexMap.size()*3));
        for (size_t j = 0; j != indexMap.size(); ++j)
        {
            size_t k = indexMap[j];
            fprintf(fout, "%f %f %f ", p_v[k * 3 + 2], p_v[k * 3 + 0], p_v[k * 3 + 1]);
        }
        fprintf(fout, "</float_array>\n");
        fprintf(fout, "          <technique_common>\n");
        fprintf(fout, "            <accessor source=\"#%s-mesh-positions-array\" count=\"%u\" stride=\"3\">\n", nm, (unsigned int)(indexMap.size()));
        fprintf(fout, "              <param name=\"X\" type=\"float\"/>\n");
        fprintf(fout, "              <param name=\"Y\" type=\"float\"/>\n");
        fprintf(fout, "              <param name=\"Z\" type=\"float\"/>\n");
        fprintf(fout, "            </accessor>\n");
        fprintf(fout, "          </technique_common>\n");
        fprintf(fout, "        </source>\n");
        fprintf(fout, "        <source id=\"%s-mesh-normals\">\n", nm);
        fprintf(fout, "          <float_array id=\"%s-mesh-normals-array\" count=\"%u\">", nm, (unsigned int)(indexMap.size()*3));
        for (size_t j = 0; j != indexMap.size(); ++j)
        {
            size_t k = indexMap[j];
            fprintf(fout, "%f %f %f ", p_n[k * 3 + 2], p_n[k * 3 + 0], p_n[k * 3 + 1]);
        }
        fprintf(fout, "</float_array>\n");
        fprintf(fout, "          <technique_common>\n");
        fprintf(fout, "            <accessor source=\"#%s-mesh-normals-array\" count=\"%u\" stride=\"3\">\n", nm, (unsigned int)(indexMap.size()));
        fprintf(fout, "              <param name=\"X\" type=\"float\"/>\n");
        fprintf(fout, "              <param name=\"Y\" type=\"float\"/>\n");
        fprintf(fout, "              <param name=\"Z\" type=\"float\"/>\n");
        fprintf(fout, "            </accessor>\n");
        fprintf(fout, "          </technique_common>\n");
        fprintf(fout, "        </source>\n");
        fprintf(fout, "        <source id=\"%s-mesh-map-0\">\n", nm);
        fprintf(fout, "          <float_array id=\"%s-mesh-map-0-array\" count=\"%u\">", nm, (unsigned int)(indexMap.size()*2));
        for (size_t j = 0; j != indexMap.size(); ++j)
        {
            size_t k = indexMap[j];
            fprintf(fout, "%f %f ", p_t[k * 2 + 0], p_t[k * 2 + 1]);
        }
        fprintf(fout, "</float_array>\n");
        fprintf(fout, "          <technique_common>\n");
        fprintf(fout, "            <accessor source=\"#%s-mesh-map-0-array\" count=\"%u\" stride=\"2\">\n", nm, (unsigned int)(indexMap.size()));
        fprintf(fout, "              <param name=\"S\" type=\"float\"/>\n");
        fprintf(fout, "              <param name=\"T\" type=\"float\"/>\n");
        fprintf(fout, "            </accessor>\n");
        fprintf(fout, "          </technique_common>\n");
        fprintf(fout, "        </source>\n");
        fprintf(fout, "        <vertices id=\"%s-mesh-vertices\">\n", nm);
        fprintf(fout, "          <input semantic=\"POSITION\" source=\"#%s-mesh-positions\"/>\n", nm);
        fprintf(fout, "        </vertices>\n");
        T3dmName mnm;
        memcpy(mnm, p_m[obj.p_m], sizeof mnm);
        T3DMs::procName(mnm);
        fprintf(fout, "        <triangles material=\"%s-material\" count=\"%u\">\n", mnm, obj.p_sz / 3);
        fprintf(fout, "          <input semantic=\"VERTEX\" source=\"#%s-mesh-vertices\" offset=\"0\"/>\n", nm);
        fprintf(fout, "          <input semantic=\"NORMAL\" source=\"#%s-mesh-normals\" offset=\"1\"/>\n", nm);
        fprintf(fout, "          <input semantic=\"TEXCOORD\" source=\"#%s-mesh-map-0\" offset=\"2\" set=\"1\"/>\n", nm);
        fprintf(fout, "          <p>");
        for (size_t j = 0; j != obj.p_sz; ++j)
        {
            unsigned int k = indexRev[obj.p_i[j]];
            fprintf(fout, "%u %u %u ", k, k, k);
        }
        fprintf(fout, "</p>\n");
        fprintf(fout, "        </triangles>\n");
        fprintf(fout, "      </mesh>\n");
        fprintf(fout, "    </geometry>\n");
    }
    fputs("  </library_geometries>\n", fout);
    fputs("  <library_visual_scenes>\n", fout);
    fputs("    <visual_scene id=\"Scene\" name=\"Scene\">\n", fout);
    const O3dm* m_o = 0;
    for (unsigned i = 0; i != p_sz; ++i)
    {
        if (strcmp("mapobject", p_o[i].p_name) == 0)
        {
            m_o = p_o + i;
        }
    }
    if (m_o)
    {
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
        std::stable_sort(p_mapobjs.begin(), p_mapobjs.end(), [](const Mapobj& m1, const Mapobj& m2) { return m1.id < m2.id; });
        for (size_t i = 0; i != p_mapobjs.size(); ++i)
        {
            const Mapobj& o = p_mapobjs[i];
            fprintf(fout, "      <node id=\"Mapobj_%d_%03d\" name=\"Mapobj.%d.%03d\" type=\"NODE\">\n", o.id, int(i), o.id, int(i));
            glm::mat4 objMat = glm::mat4(1.f);
            objMat = glm::translate(objMat, glm::vec3(o.pos[0], o.pos[1], 0));
            objMat = glm::rotate(objMat, -o.ang, glm::vec3(0.f, 0.f, 1.f));
            float m00 = objMat[0][0];
            float m01 = objMat[0][1];
            float m02 = objMat[0][2];
            float m03 = objMat[0][3];
            float m10 = objMat[1][0];
            float m11 = objMat[1][1];
            float m12 = objMat[1][2];
            float m13 = objMat[1][3];
            float m20 = objMat[2][0];
            float m21 = objMat[2][1];
            float m22 = objMat[2][2];
            float m23 = objMat[2][3];
            float m30 = objMat[3][0];
            float m31 = objMat[3][1];
            float m32 = objMat[3][2];
            float m33 = objMat[3][3];
            fprintf(fout, "        <matrix sid=\"transform\">%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f</matrix>\n",
                    m00, m10, m20, m30,  m01, m11, m21, m31,  m02, m12, m22, m32,  m03, m13, m23, m33);
            fprintf(fout, "      </node>\n");
        }
    }
    for (size_t i = 0; i != p_sz; ++i)
    {
        O3dm& obj = p_o[i];
        T3dmName nm;
        memcpy(nm, obj.p_name, sizeof nm);
        T3DMs::procName(nm);
        fprintf(fout, "      <node id=\"%s\" name=\"%s\" type=\"NODE\">\n", nm, obj.p_name);

        glm::mat4 objMat = glm::translate(glm::mat4(1.f), glm::vec3(obj.p_cen[2], obj.p_cen[0], obj.p_cen[1]));
        float m00 = objMat[0][0];
        float m01 = objMat[0][1];
        float m02 = objMat[0][2];
        float m03 = objMat[0][3];
        float m10 = objMat[1][0];
        float m11 = objMat[1][1];
        float m12 = objMat[1][2];
        float m13 = objMat[1][3];
        float m20 = objMat[2][0];
        float m21 = objMat[2][1];
        float m22 = objMat[2][2];
        float m23 = objMat[2][3];
        float m30 = objMat[3][0];
        float m31 = objMat[3][1];
        float m32 = objMat[3][2];
        float m33 = objMat[3][3];
        fprintf(fout, "        <matrix sid=\"transform\">%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f</matrix>\n",
                m00, m10, m20, m30,  m01, m11, m21, m31,  m02, m12, m22, m32,  m03, m13, m23, m33);
        fprintf(fout, "        <instance_geometry url=\"#%s-mesh\" name=\"%s\">\n", nm, obj.p_name);
        fprintf(fout, "          <bind_material>\n");
        fprintf(fout, "            <technique_common>\n");
        for (size_t j = 1; j < p_m_sz; ++j)
        {
            T3dmName mnm;
            memcpy(mnm, p_m[j], sizeof mnm);
            T3DMs::procName(mnm);
            fprintf(fout, "              <instance_material symbol=\"%s-material\" target=\"#%s-material\">\n", mnm, mnm);
            fprintf(fout, "                <bind_vertex_input semantic=\"UVMap\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>\n");
            fprintf(fout, "              </instance_material>\n");
        }
        fprintf(fout, "            </technique_common>\n");
        fprintf(fout, "          </bind_material>\n");
        fprintf(fout, "        </instance_geometry>\n");
        fprintf(fout, "      </node>\n");
    }
    fputs("    </visual_scene>\n", fout);
    fputs("  </library_visual_scenes>\n", fout);
    fputs("  <scene>\n", fout);
    fputs("    <instance_visual_scene url=\"#Scene\"/>\n", fout);
    fputs("  </scene>\n", fout);
    fputs("</COLLADA>\n", fout);
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
            p_m = new T3dmName[p_m_sz];
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
                float c0 = 0, c1 = 0, c2 = 0;
                if (sscanf(buff, "%f %f %f %u",
                    &c0, &c1, &c2,
                    &(p_o[object_i].p_m)) != 4)
                {
                }
                p_o[object_i].p_cen[0] = c0;
                p_o[object_i].p_cen[1] = c1;
                p_o[object_i].p_cen[2] = c2;
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
                }
                fgets(buff, 1024, fin); // read face count
                if (sscanf(buff, "f %u", &faces_n) != 1)
                {
                }
                p_o[object_i].p_sz = faces_n*3; // number of vertex indices
                p_o[object_i].p_i = new unsigned short[faces_n*3]; // allocate index array
                for (unsigned int i = 0; i != faces_n; ++i) // read face vertex indices
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
                    T3DMf::crossprod(normal, v0, v1); // compute face normal (not normalized, length proportional to face area)
                    for (unsigned int c = 0; c != 3; ++c) // repeat for each face vertex
                    {
                        unsigned int j = p_i[i*3+c]; // j <- current vertex index
                        while (j != 0 && nf[j]) // find first index in case of common normal
                            --j;
                        do { // add normal for each common vertex
                            p_n[j*3  ] += normal[0];
                            p_n[j*3+1] += normal[1];
                            p_n[j*3+2] += normal[2];
                            ++j;
                        } while (j != vertexnum && nf[j] == 1);
                    }
                }
                ++object_i;
            }
        }
    }
    fclose(fin);
    for (unsigned int i = 0; i != vertexnum; ++i) // normalize normals
    {
        T3DMf::normalize(p_n + i * 3);
    }
    // delete temporary arrays
    delete[] nf;
}
