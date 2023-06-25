#include <cstdio>
#include <string>
#include "3dm.h"
#include "cstr1.h"

int main(int argc, char** argv)
{
    char filenameBuff[1024] = {0};
    char objBuff[1024] = {0};
    char mtlBuff[1024] = {0};
    char extBuff[1024] = {0};

    if (argc <= 1)
    {
        printf("3DM to OBJ conversion tool\n");
        printf("Converts OpenMRac 3D model format to Wavefront OBJ that can be imported in Blender\n");
        printf("Usage: 3dm2obj model1.3dm model2.3dm ...");
    }

    for (int i = 1; i < argc; ++i)
    {
        T3dm model;
        model.load(argv[i]);
        strncpy(filenameBuff, argv[i], 1023);
        toFilenameNoext(filenameBuff);
        strncpy(objBuff, filenameBuff, 1023);
        strncat(objBuff, ".obj", 1023);
        strncpy(mtlBuff, filenameBuff, 1023);
        strncat(mtlBuff, ".mtl", 1023);
        strncpy(extBuff, filenameBuff, 1023);
        strncat(extBuff, ".ext", 1023);
        model.saveMtl(mtlBuff);
        model.saveObj(objBuff);
        model.saveExtra(extBuff);
    }
    return 0;
}
