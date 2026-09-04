#include "../include/obj-loader.h"

int LoadMeshFromFile(char *path, Triangle *mesh) {
    Vec3 positions[1000];
    Vec3 normals[1000];
    Vec2 textures[1000];

    FILE *ptr = fopen(path, "r");

    if (ptr == NULL) {
        printf("Error loading obj file: (can't find '%s')\n", path);
        return 0;
    }

    char data[100];

    int vertexCount = 0;
    int normalCount = 0;
    int textureCount = 0;

    int triangleCount = 0;

    int line = 0;
    
    while (fgets(data, sizeof(data), ptr)) {
        line++;
        
        if (data[0] == 'v') {
            if (data[1] == ' ') {
                int out = sscanf(data, " v %f %f %f", &positions[vertexCount].x, &positions[vertexCount].y, &positions[vertexCount].z);
                if (out != 3) {
                    printf("Error loading obj file: (invalid vertex at: '%s:%d')\n", path, line);
                    return 0;
                }
                vertexCount++;
            } else if (data[1] == 'n') {
                int out = sscanf(data, " vn %f %f %f", &normals[normalCount].x, &normals[normalCount].y, &normals[normalCount].z);
                if (out != 3) {
                    printf("Error loading obj file: (invalid vertex normals at: '%s:%d')\n", path, line);
                    return 0;
                }
                normalCount++;
            } else if (data[1] == 't') {
                int out = sscanf(data, " vt %f %f", &textures[textureCount].x, &textures[textureCount].y);
                if (out != 2) {
                    printf("Error loading obj file: (invalid vertex textures at: '%s:%d')\n", path, line);
                    return 0;
                }
                textureCount++;
            }
        } else if (data[0] == 'f') {
            int p1, n1, t1,
                p2, n2, t2,
                p3, n3, t3,
                p4, n4, t4;
	    if (strstr(data, "/") == NULL) {
		int out = sscanf(data, " f %d %d %d", &p1, &p2, &p3);
		if (out != 3) {
		    return 0;
		}

		mesh[triangleCount].vertices[0].pos = positions[p1 - 1];
		mesh[triangleCount].vertices[1].pos = positions[p2 - 1];
		mesh[triangleCount].vertices[2].pos = positions[p3 - 1];

		triangleCount++;

		continue;
	    }
            if (strstr(data, "//")) {
                int out = sscanf(data, " f %d//%d %d//%d %d//%d %d//%d", &p1, &n1, &p2, &n2, &p3, &n3, &p4, &n4);

                if (out != 8) {
                    out = sscanf(data, " f %d//%d %d//%d %d//%d", &p1, &n1, &p2, &n2, &p3, &n3);

                    if (out != 6) {
                        printf("Error loading obj file: (invalid face at: '%s:%d')\n", path, line);
                        return 0;
                    }

                    mesh[triangleCount].vertices[0].pos = positions[p1 - 1];
                    mesh[triangleCount].vertices[0].nor = normals[n1 - 1];

                    mesh[triangleCount].vertices[1].pos = positions[p2 - 1];
                    mesh[triangleCount].vertices[1].nor = normals[n2 - 1];

                    mesh[triangleCount].vertices[2].pos = positions[p3 - 1];
                    mesh[triangleCount].vertices[2].nor = normals[n3 - 1];

                    triangleCount++;

                    continue;
                }

                mesh[triangleCount].vertices[0].pos = positions[p1 - 1];
                mesh[triangleCount].vertices[0].nor = normals[n1 - 1];

                mesh[triangleCount].vertices[1].pos = positions[p2 - 1];
                mesh[triangleCount].vertices[1].nor = normals[n2 - 1];

                mesh[triangleCount].vertices[2].pos = positions[p3 - 1];
                mesh[triangleCount].vertices[2].nor = normals[n3 - 1];

                triangleCount++;

                mesh[triangleCount].vertices[0].pos = positions[p1 - 1];
                mesh[triangleCount].vertices[0].nor = normals[n1 - 1];

                mesh[triangleCount].vertices[1].pos = positions[p3 - 1];
                mesh[triangleCount].vertices[1].nor = normals[n3 - 1];

                mesh[triangleCount].vertices[2].pos = positions[p4 - 1];
                mesh[triangleCount].vertices[2].nor = normals[n4 - 1];

                triangleCount++;

                continue;
            }

            int out = sscanf(data, " f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &p1, &t1, &n1, &p2, &t2, &n2, &p3, &t3, &n3, &p4, &t4, &n4);

            if (out != 12) {
                out = sscanf(data, " f %d/%d/%d %d/%d/%d %d/%d/%d", &p1, &t1, &n1, &p2, &t2, &n2, &p3, &t3, &n3);
                if (out != 9) {
                    printf("Error loading obj file: (invalid face at: '%s:%d')\n", path, line);
                    return 0;
                }

                mesh[triangleCount].vertices[0].pos = positions[p1 - 1];
                mesh[triangleCount].vertices[0].nor = normals[n1 - 1];
                mesh[triangleCount].vertices[0].uv = textures[t1 - 1];

                mesh[triangleCount].vertices[1].pos = positions[p2 - 1];
                mesh[triangleCount].vertices[1].nor = normals[n2 - 1];
                mesh[triangleCount].vertices[1].uv = textures[t2 - 1];

                mesh[triangleCount].vertices[2].pos = positions[p3 - 1];
                mesh[triangleCount].vertices[2].nor = normals[n3 - 1];
                mesh[triangleCount].vertices[2].uv = textures[t3 - 1];

                triangleCount++;

                continue;
            }

            mesh[triangleCount].vertices[0].pos = positions[p1 - 1];
            mesh[triangleCount].vertices[0].nor = normals[n1 - 1];
            mesh[triangleCount].vertices[0].uv = textures[t1 - 1];

            mesh[triangleCount].vertices[1].pos = positions[p2 - 1];
            mesh[triangleCount].vertices[1].nor = normals[n2 - 1];
            mesh[triangleCount].vertices[1].uv = textures[t2 - 1];

            mesh[triangleCount].vertices[2].pos = positions[p3 - 1];
            mesh[triangleCount].vertices[2].nor = normals[n3 - 1];
            mesh[triangleCount].vertices[2].uv = textures[t3 - 1];

            triangleCount++;

            mesh[triangleCount].vertices[0].pos = positions[p1 - 1];
            mesh[triangleCount].vertices[0].nor = normals[n1 - 1];
            mesh[triangleCount].vertices[0].uv = textures[t1 - 1];

            mesh[triangleCount].vertices[1].pos = positions[p3 - 1];
            mesh[triangleCount].vertices[1].nor = normals[n3 - 1];
            mesh[triangleCount].vertices[1].uv = textures[t3 - 1];

            mesh[triangleCount].vertices[2].pos = positions[p4 - 1];
            mesh[triangleCount].vertices[2].nor = normals[n4 - 1];
            mesh[triangleCount].vertices[2].uv = textures[t4 - 1];

            triangleCount++;
        }
    }

    fclose(ptr);
    
    return triangleCount;
}