#include <SDL.h>
#include <algorithm>
#include <iostream>

#include "entity.hpp"
#include "constants.hpp"
#include "game.hpp"
#include "scene.hpp"
#include "renderer/abstractrenderer.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

Scene::Scene() {
    entities = std::vector<Entity*>();
}

Scene::~Scene() {
    for (Entity* e : entities) {
        delete e;
    }
}

void Scene::init() {
    // init all the entities
    for (Entity* e : this->entities) {
        e->init();
    }
}

void Scene::start() {}

void Scene::update() {
    // update all the entities
    for (Entity* e : this->entities) {
        e->update();
    }
}

void Scene::render() {
    SDL_Window* w = EnG->getWindow();

    int width, height;
    SDL_GetWindowSize(w, &width, &height);
    projectionMatrix = glm::infinitePerspective(
        glm::pi<float>() * 0.4f, (float)width / (float)height, 0.0001f);

    glm::mat4 matrix = projectionMatrix * cameraMatrix;

    for (Entity* e : this->entities) {
        AbstractRenderer* r = e->getRenderer();
        if (r) {
            r->render(matrix);
        }
    }
}
