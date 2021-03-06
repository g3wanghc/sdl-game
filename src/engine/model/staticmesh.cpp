#include "staticmesh.hpp"
#include "engine/gl.h"

void StaticMesh::init(const GLfloat* verts,
                      const GLfloat* colors,
                      size_t num_points) {
    this->num_points = num_points;

    if (verts) {
        glGenBuffers(1, &vertexbuffer);
    }

    if (colors) {
        glGenBuffers(1, &colorbuffer);
    }

    updateMesh(verts, colors);
}

void StaticMesh::updateMesh(const GLfloat* verts, const GLfloat* colors) {
    if (verts) {
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, num_points * 3 * sizeof(GLfloat), verts,
                     GL_STATIC_DRAW);
    }

    if (colors) {
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, num_points * 3 * sizeof(GLfloat), colors,
                     GL_STATIC_DRAW);
    }
}
