#ifndef __ENGINE_TEXT
#define __ENGINE_TEXT

#include "entity.hpp"
#include "pair.hpp"
#include <SDL.h>
#include <SDL_ttf.h>

class Text : public Entity {
   public:
    Pair position;
    TTF_Font* font;
    SDL_Color color;
    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_Rect rect;
    SDL_Renderer* ren;
    Text(SDL_Renderer* r,
         Pair position,
         TTF_Font* font,
         SDL_Color color,
         const char* initialText);
    ~Text();
    void updateText(const char* newText);
    virtual void init() override;
    virtual void update() override;
    virtual void preUpdate() override;
    virtual void postUpdate() override;
    virtual void render(SDL_Renderer* ren);
};

#endif
