#ifndef __GAME_ACTION_HPP
#define __GAME_ACTION_HPP
class Player;
class Platform;

typedef enum LandType {
    NORMAL_LANDING,
    KNOCKDOWN_LANDING,
    SPECIAL_LANDING,
} LandType;

typedef enum JumpType {
    NO_JUMP,
    JUMP_STICK,
    JUMP_BUTTON,
} JumpType;

class Action {
   public:
    virtual void step(Player& p) = 0;
    virtual LandType getLandType(Player& p);
    virtual void onLanding(Player& p);
    virtual bool isGrounded(const Player& p);
    virtual bool isLandable(const Player& p, const Platform* plat);
    virtual bool canWalkOff(const Player& p);
    virtual bool canGrabLedge(const Player& p);
};

#define ACTION_STATE(x) x,
typedef enum {
#include "actionstates"
} ActionState;
#undef ACTION_STATE

const char* actionStateName(ActionState);

extern Action* ACTIONS[__NUM_ACTION_STATES];
extern const char* ACTION_STATE_NAMES[__NUM_ACTION_STATES + 1];

#endif
