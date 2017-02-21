#ifndef __GAME_MAINPLAYER
#define __GAME_MAINPLAYER

#include <SDL.h>
#include "engine/input.hpp"
#include "engine/sprite.hpp"
#include "terrain/platform.hpp"
#include "action.hpp"
#include "animationbank.hpp"
#include "playerconfig.hpp"
#include "playercollision.hpp"

#define FACE_LEFT -1;
#define FACE_RIGHT 1;

class Player : public Sprite {
    AnimationBank* bank;
    Platform* currentPlatform = NULL;

   public:
    PlayerCollision* previousCollision = new PlayerCollision();
    PlayerCollision* currentCollision = new PlayerCollision();

    Joystick* joystick;
    PlayerConfig config;

    int ecbFixedCounter;
    Pair ecbBottomFixedPosition = Pair(0, 0);

    Pair cVel = Pair(0, 0);
    Pair kVel = Pair(0, 0);
    Pair previousPosition;

    Action* action;
    ActionState actionState;
    bool fastfalled = false;
    bool grounded = false;
    int times_jumped = 0;
    int timer = 0;
    JumpType jumpType;
    bool isShortHop;
    bool actionable = true;
    double face = FACE_LEFT;
    int hitlagFrames = 0;

    void init() override;
    void render(SDL_Renderer* ren) override;
    void update() override;

    void fall(bool fast = false);
    void aerialDrift();
    void land(Platform* p, double y);
    void fixEcbBottom(int frames, Pair position);

    Player(std::string attributeFile, double x, double y);
    ~Player();

    void changeAction(ActionState state);
    double getXInput(int frames = 0);
    Platform* getCurrentPlatform();
    ActionState getActionState();
    Action* getAction();
};

#endif