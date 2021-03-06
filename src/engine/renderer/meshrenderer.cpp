#include "meshrenderer.hpp"
#include <vector>
#include <iostream>
#include "engine/gl.h"
#include "engine/game.hpp"
#include "engine/model/staticmesh.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

MeshRenderer::MeshRenderer(BasicShader* shader, StaticMesh* mesh)
    : mesh(mesh), shader(shader) {}

void MeshRenderer::render(mat4& baseTransform) {
    // Draw the triangles
    glUseProgram(shader->programId);

    mat4 compoundTransform = baseTransform * modelTransform;
    glUniformMatrix4fv(shader->uniforms.baseTransform, 1, GL_FALSE,
                       &compoundTransform[0][0]);

    glEnableVertexAttribArray(shader->attributes.position);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexbuffer);
    glVertexAttribPointer(shader->attributes.position,
                          3,         // size
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void*)0   // array buffer offset
                          );

    glEnableVertexAttribArray(shader->attributes.color);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->colorbuffer);
    glVertexAttribPointer(shader->attributes.color,
                          3,         // size
                          GL_FLOAT,  // typ& e
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void*)0   // array buffer offset
                          );

    glDrawArrays(GL_TRIANGLES, 0, mesh->num_points);
    glDisableVertexAttribArray(shader->attributes.position);
    glDisableVertexAttribArray(shader->attributes.color);
}

void MeshRenderer::setModelTransform(mat4 t) {
    modelTransform = t;
}
