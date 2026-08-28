#include "../include/scene-loader.h"
#include "../include/obj-loader.h"

int LoadSceneFromFile(char *path, Object **scene, NameFunctionPair *funcs, int funcCount) {
    FILE *ptr = fopen(path, "r");

    if (ptr == NULL) {
        printf("Error loading scene: (can't find '%s')\n", path);
        return 0;
    }

    char data[100];
    int n = 0;

    int line = 0;

    while (fgets(data, sizeof(data), ptr)) {
        line++;

        char copy[100];
        strcpy(copy, data);

        char *split = strtok(copy, " ");

        if (strcmp(split, "LOAD") == 0) {
            char fileToLoad[100];

            int out = sscanf(data, "LOAD %s", fileToLoad);
            if (out != 1) {
                printf("Error loading scene: (invalid command at '%s:%d')\n", path, line);
                return 0;
            }

            (*scene) = realloc((*scene), (n + 1) * sizeof(Object));
            (*scene)[n].hasFunction = 0;

            (*scene)[n].mesh = malloc(1000 * sizeof(Triangle));
            int triCount = LoadMeshFromFile(fileToLoad, (*scene)[n].mesh);
            if (triCount == 0) {
                return 0;
            }

            (*scene)[n].triangleCount = triCount;

            (*scene)[n].scale = (Vec3){1,1,1};

            n++;
        }
        else if (strcmp(split, "MOVE") == 0) {
            int idx;
            float x, y, z;

            int out = sscanf(data, "MOVE %d %f/%f/%f", &idx, &x, &y, &z);
            if (out != 4) {
                printf("Error loading scene: (invalid command at '%s:%d')\n", path, line);
                return 0;
            }
            if (idx > (n - 1)) {
                printf("Error moving object: (acessing non-existent object at '%s:%d')\n", path, line);
                return 0;
            }

            (*scene)[idx].position = (Vec3){x, y, z};
        }
        else if (strcmp(split, "ROTATE") == 0) {
            int idx;
            float x, y, z;

            int out = sscanf(data, "ROTATE %d %f/%f/%f", &idx, &x, &y, &z);
            if (out != 4) {
                printf("Error loading scene: (invalid command at '%s:%d')\n", path, line);
                return 0;
            }
            if (idx > (n - 1)) {
                printf("Error rotating object: (acessing non-existent object at '%s:%d')\n", path, line);
                return 0;
            }

            (*scene)[idx].rotation = (Vec3){x, y, z};
        }
        else if (strcmp(split, "SCALE") == 0) {
            int idx;
            float x, y, z;

            int out = sscanf(data, "SCALE %d %f/%f/%f", &idx, &x, &y, &z);
            if (out != 4) {
                printf("Error loading scene: (invalid command at '%s:%d')\n", path, line);
                return 0;
            }
            if (idx > (n - 1)) {
                printf("Error scaling object: (acessing non-existent object at '%s:%d')\n", path, line);
                return 0;
            }

            (*scene)[idx].scale = (Vec3){x, y, z};
        }
        else if (strcmp(split, "ASSIGN") == 0) {
            int idx;
            char name[50];

            int out = sscanf(data, "ASSIGN %d %s", &idx, name);
            if (out != 2) {
                printf("Error assigning function to object: (invalid command at '%s:%d')\n", path, line);
                return 0;
            }
            if (idx > (n - 1)) {
                printf("Error assigning function to object: (acessing non-existent object at '%s:%d')\n", path, line);
                return 0;
            }

            int found = 0;
            for (int i = 0; i < funcCount; i++) {
                if (strcmp(name, funcs[i].name) == 0) {
                    (*scene)[idx].func = funcs[i].func;
                    (*scene)[idx].hasFunction = 1;
                    found = 1;
                    break;
                }
            }

            if (found == 0) {
                printf("Error assigning function to object: (non existent function '%s' at '%s:%d')\n", name, path, line);
                return 0;
            }
        }
    }

    fclose(ptr);

    return n;
}