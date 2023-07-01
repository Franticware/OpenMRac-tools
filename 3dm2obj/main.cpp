#include <cstdio>
#include <string>
#include "3dm.h"
#include "cstr1.h"

bool contains(const char* str, char c)
{
    for (size_t i = 0; i != strlen(str); ++i)
    {
        if (str[i] == c)
            return true;
    }
    return false;
}

int main(int argc, char** argv)
{
    char filenameBuff[1024] = {0};
    char newFileBuff[1024] = {0};
    if (argc <= 1)
    {
        printf("3DM to DAE conversion tool\n");
        printf("Converts OpenMRac 3D model format to Collada DAE that can be imported in Blender\n");
        printf("Usage:\n");
        printf("3dm2dae model1.3dm model2.3dm ...\n");
    }
    for (int i = 1; i < argc; ++i)
    {
        T3dm model;
        model.load(argv[i]);
        strncpy(filenameBuff, argv[i], 1023);
        toFilenameNoext(filenameBuff);
        strncpy(newFileBuff, filenameBuff, 1023);
        strncat(newFileBuff, ".dae", 1023);
        model.saveDae(newFileBuff);
    }
    return 0;
}
