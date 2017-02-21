#include "player.hpp"
#include "../engine/game.hpp"
#include "../util.hpp"
#include "../scenes.hpp"
#include "action.hpp"
#include "playerconfig.hpp"
#include <algorithm>
#include <iostream>

Player::Player(std::string fpath, double x, double y)
    : config(PlayerConfig(fpath)), previousPosition(x, y) {
    action = ACTIONS[FALL];
    position.x = x;
    position.y = y;
    previousCollision->reset(position);
    currentCollision->reset(position);
}

Player::~Player() {}

void Player::init() {
    std::cout << "init player" << std::endl;
    joystick = EnG->input.getJoystick(0);
    bank = new AnimationBank();
    changeAction(FALL);
}

void Player::update() {
    previousPosition.x = position.x;
    previousPosition.y = position.y;

    Sprite::update();

    timer++;
    action->step(*this);

    // reset position when player presses START.
    if (joystick->down(7)) {
        position.x = 1.15;
        position.y = 0.6;
        currentPlatform = NULL;
        changeAction(FALL);
    }

    // swap previous and current collision, reset the new current collision
    PlayerCollision* tmp = previousCollision;
    previousCollision = currentCollision;
    currentCollision = tmp;
    // this should be from animation data
    currentCollision->reset(position + Pair(0, -0.2));

    // if we are currently grounded, adapt the x of cVel to
    // move along the platform
    if (action->isGrounded(*this) && currentPlatform != NULL) {
        Pair stepVel = cVel * EnG->elapsed;
        if (!currentPlatform->groundedMovement(position, stepVel)) {
            currentPlatform = NULL;
            changeAction(FALL);
            times_jumped = 1;
        }
        velocity = kVel + stepVel / EnG->elapsed;

    } else {
        if (actionState != ESCAPEAIR) {
            cVel.x = sign(cVel.x) *
                     std::min(std::abs(cVel.x),
                              config.getAttribute("max_aerial_h_velocity"));
        }
        velocity = cVel + kVel;
    }

    Sprite::updateMotion();

    if (action->isGrounded(*this)) {
        currentCollision->playerModified.bottom = position;
        currentCollision->postCollision.bottom = position;
    } else if (ecbFixedCounter > 0) {
        ecbFixedCounter--;
        Pair newPos = position + ecbBottomFixedPosition;
        currentCollision->playerModified.bottom = newPos;
        currentCollision->postCollision.bottom = newPos;
    }
}

// void Player::physics() {
//     if(hitlagFrames > 0) {
//         hitlagFrames--;

//         // launch the player
//         if (hitlagFrames == 0) {
//             // launch();
//         // while in hitlag (freeze frames, perform SDI)
//         } else {
//             // sdi();
//         }
//     }
// }

/** Fall the player slowly, and allow the player to fastfall */
void Player::fall(bool fast) {
    if (fastfalled)
        return;
    cVel.y += config.getAttribute("gravity");
    cVel.y = std::min(cVel.y, config.getAttribute("terminal_velocity"));

    if (fast || (joystick->axis(1) > 0.65 && joystick->axis(1, 3) < 0.1 &&
                 cVel.y > 0)) {
        std::cout << "fastfalling" << std::endl;
        fastfalled = true;
        cVel.y = config.getAttribute("fast_fall_terminal_velocity");
    }
}

/** Transition from falling to being on ground
    Determine what state to enter from the state we are in */
void Player::land(Platform* p, double y) {
    double yvel = cVel.y;
    position.y = y;
    cVel.y = 0;
    grounded = true;
    fastfalled = false;
    times_jumped = 0;

    currentPlatform = p;
    printf("landing on %p\n", p);

    currentCollision->postCollision.bottom = position;

    switch (action->getLandType(*this)) {
        case NORMAL:
            // Trigger landing when velocity > 1
            std::cout << "cVel : " << yvel << std::endl;
            changeAction(yvel > 1 ? LANDING : WAIT);
            break;
        case KNOCKDOWN:
            // TODO check the tech buffer
            break;
        case SPECIAL:
            action->onLanding(*this);
            break;
    }
}

/** Move the player horizontally when they are in the air */
void Player::aerialDrift() {
    bool joystickMoving = std::abs(joystick->axis(0)) > 0.3;
    float inputDrift =
        (joystickMoving)
            ? joystick->axis(0) * config.getAttribute("max_aerial_h_velocity")
            : 0;

    // std::cout << inputDrift << " ";

    // if the player is moving more than inputDrift, slow then with air friction
    if (abs(inputDrift) > abs(cVel.x) && sign(cVel.x) == sign(inputDrift)) {
        // std::cout << "too fast, dragging";
        if (cVel.x > 0) {
            cVel.x =
                std::max(cVel.x - config.getAttribute("air_friction"), 0.0);
        } else {
            cVel.x =
                std::min(cVel.x + config.getAttribute("air_friction"), 0.0);
        }
    }

    // otherwise, move them according to their aerial mobility
    else if (joystickMoving) {
        // std::cout << "moving according to mobility";
        cVel.x += joystick->axis(0) * config.getAttribute("aerial_mobility") +
                  sign(joystick->axis(0)) *
                      config.getAttribute("aerial_stopping_mobility");
    }

    // if the joystick is not moving, slow the player down with aerial friciton
    if (!joystickMoving) {
        // std::cout << "not moving, slowing with friction";
        if (cVel.x > 0) {
            cVel.x =
                std::max(cVel.x - config.getAttribute("air_friction"), 0.0);
        } else {
            cVel.x =
                std::min(cVel.x + config.getAttribute("air_friction"), 0.0);
        }
    }
    // std::cout << std::endl;
}

void Player::render(SDL_Renderer* ren) {
    SDL_SetRenderDrawColor(ren, 255, 215, 0, 255);
    previousCollision->postCollision.render(ren, PLAYER_SCALE);

    SDL_SetRenderDrawColor(ren, 255, 127, 80, 255);
    currentCollision->postCollision.render(ren, PLAYER_SCALE);

    SDL_Rect destination{(int)(position.x * PLAYER_SCALE) - 64,
                         (int)(position.y * PLAYER_SCALE) - 110, 128, 128};
    SDL_RenderCopyEx(ren, bank->getCurrentTexture(*this), NULL, &destination, 0,
                     NULL, face < 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    // SDL_RenderDrawRect(ren, &destination);

    SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);
    SDL_RenderDrawLine(ren, ((int)(position.x * PLAYER_SCALE)) - 3,
                       ((int)(position.y * PLAYER_SCALE)),
                       ((int)(position.x * PLAYER_SCALE)) + 3,
                       ((int)(position.y * PLAYER_SCALE)));
    SDL_RenderDrawLine(ren, ((int)(position.x * PLAYER_SCALE)),
                       ((int)(position.y * PLAYER_SCALE)) - 3,
                       ((int)(position.x * PLAYER_SCALE)),
                       ((int)(position.y * PLAYER_SCALE)) + 3);
}

void Player::changeAction(ActionState state) {
    std::cout << "change to state [" << state << "] "
              << "(" << actionStateName(state) << ")" << std::endl;
    timer = 0;
    action = ACTIONS[state];
    bank->playAnimation(state);
    action->step(*this);
    actionState = state;
}

void Player::fixEcbBottom(int frames, Pair position) {
    ecbFixedCounter = frames;
    ecbBottomFixedPosition = position;
}

ActionState Player::getActionState() {
    return actionState;
}

Action* Player::getAction() {
    return action;
}

double Player::getXInput(int frames) {
    return joystick->axis(frames) * face;
}

Platform* Player::getCurrentPlatform() {
    return currentPlatform;
}