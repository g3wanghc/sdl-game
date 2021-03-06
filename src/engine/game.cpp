#include <iostream>

#include "game.hpp"
#include "input/input.hpp"
#include "util.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "engine/gl.h"

Game* EnG = nullptr;
SDL_Texture* FALLBACK_TEXTURE = nullptr;

// TODO use zoomLevel at all
Game::Game(unsigned int width,
           unsigned int height,
           Scene& initialScene,
           unsigned int zoomLevel)
    : currentScene(&initialScene) {
    // check if there is an existing global reference to Game. If there is
    // not, register this as the Game singleton
    if (EnG != nullptr) {
        std::cerr << "Tried to create another instance of singleton Game"
                  << std::endl;
        exit(1);
    } else {
        // init SDL
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        if (TTF_Init() == -1) {
            printf("TTF_Init: %s\n", TTF_GetError());
            exit(2);
        }
        EnG = this;
    }

    // init input system
    input.init();

    // create the game stuff
    win = makeWindow("poop", width, height);
    ctx = makeGlContext(win);
    ren = makeRenderer(win);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // TODO replace this with a string constant
    FALLBACK_TEXTURE = loadPNG("assets/fallback.png");
}

Game::~Game() {
    SDL_Quit();
}

/**
 * Makes the SDL window, exits program on error
 */
SDL_Window* Game::makeWindow(const std::string& name,
                             unsigned int width,
                             unsigned int height) {
    SDL_Window* win =
        SDL_CreateWindow(name.c_str(), 100, 100, (int)width, (int)height,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (win == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    return win;
}

SDL_GLContext Game::makeGlContext(SDL_Window* win) {
    SDL_GLContext context = SDL_GL_CreateContext(win);
    return context;
}

/**
 * Makes a renderer for the SDL window
 **/
SDL_Renderer* Game::makeRenderer(SDL_Window* win) {
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    if (ren == nullptr) {
        SDL_DestroyWindow(win);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError()
                  << std::endl;
        SDL_Quit();
        exit(1);
    }

    return ren;
}

/**
* Loads a BMP image into a texture on the rendering device
* @param file The BMP image file to load
* @param ren The renderer to load the texture onto
* @return the loaded texture, or nullptr if something went wrong.
*/
SDL_Texture* Game::loadPNG(const std::string& file) {
    // Initialize to nullptr to avoid dangling pointer issues
    SDL_Texture* texture = FALLBACK_TEXTURE;
    // Load the image
    SDL_Surface* loadedImage = IMG_Load(file.c_str());
    // If the loading went ok, convert to texture and return the texture
    if (loadedImage != nullptr) {
        texture = SDL_CreateTextureFromSurface(ren, loadedImage);
        SDL_FreeSurface(loadedImage);
        // Make sure converting went ok too
        if (texture == nullptr) {
            logSDLError(std::cout, "CreateTextureFromSurface");
        }
    } else {
        logSDLError(std::cout, "LoadIMG");
    }
    return texture;
}

void Game::start() {
    uint32_t lastTick, thisTick = SDL_GetTicks();
    this->currentScene->init();
    while (!readyToExit) {
        lastTick = thisTick;
        uint32_t thisTick = SDL_GetTicks(), tickDiff = thisTick - lastTick;

        if (this->fixedTickrate == 0) {
            // figure out the elased milliseconds
            this->elapsed = (tickDiff / 16.66666);
        } else {
            this->elapsed = fixedTickrate;
        }

        // Process SDL events
        input.clear();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            // If user closes the window, ready to exit
            if (e.type == SDL_QUIT) {
                readyToExit = true;
            }

            input.processEvent(&e);
        }

        // update the game's state
        currentScene->update();

        // update the frame buffer
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentScene->render();

        SDL_GL_SwapWindow(win);

        // Cap framerate at 60fps
        long int ticks = (SDL_GetTicks() - thisTick);
        long long int millis_to_delay = 16 - ticks;
        SDL_Delay(std::max(0LL, std::min(16LL, millis_to_delay)));
    }
}

SDL_Renderer* Game::getRenderer() {
    return ren;
}

SDL_Window* Game::getWindow() {
    return win;
}
