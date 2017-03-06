#include <math.h>
#include "gtest/gtest.h"
#include "engine/pair.hpp"
#include "terrain/platform.hpp"
#include "terrain/map.hpp"
#include "lib/mock-player.hpp"
#include "util.hpp"

// void Map::getClosestCollision(
//             Pair const& start,
//             Pair const& end,
//             CollisionDatum & out) {
//     Platform * platform;
//     int segment;
//     Pair position;

using namespace Terrain;

TEST(Map, getClosestCollision) {
    std::vector<Platform> platforms = {Platform({Pair(1, 5), Pair(1, -5)}),
                                       Platform({Pair(2, 5), Pair(2, -5)}),
                                       Platform({Pair(3, 5), Pair(3, -5)}),
                                       Platform({Pair(4, 5), Pair(4, -5)})};
    Map m = Map(platforms, {});

    CollisionDatum collision;

    ASSERT_TRUE(m.getClosestCollision(Pair(0, 0), Pair(4, 0), collision));

    EXPECT_EQ(WALL_COLLISION, collision.type);
    EXPECT_EQ(m.getPlatform(0)->getSegment(0), collision.segment);
    EXPECT_EQ(Pair(1, 0), collision.position);
}

TEST(Map, getClosestCollision_Miss) {
    Map m = Map({Platform({Pair(1, -5), Pair(1, 5)}),
                 Platform({Pair(2, -5), Pair(2, 5)}),
                 Platform({Pair(3, -5), Pair(3, 5)}),
                 Platform({Pair(4, -5), Pair(4, 5)})},
                {});

    CollisionDatum collision;

    EXPECT_FALSE(m.getClosestCollision(Pair(0, 0), Pair(10, 0), collision));
}

TEST(Map, movePlayer_NoCollisions) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    Map m = Map({}, {});

    Pair requestedMotion = Pair(5, -2);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(Pair(15, 8), p.position);
}

TEST(Map, movePlayer_Grounded_Flat) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map({Platform({Pair(1, 10), Pair(20, 10)})}, {});

    p.land(m.getPlatform(0));
    Pair requestedMotion = Pair(5, 0);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(Pair(15, 10), p.position);
}

TEST(Map, movePlayer_Grounded_Flat_Left) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map({Platform({Pair(1, 10), Pair(20, 10)})}, {});

    p.land(m.getPlatform(0));
    Pair requestedMotion = Pair(-5, 0);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(Pair(5, 10), p.position);
}

TEST(Map, movePlayer_Grounded_Slant_Down_Right) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 1));
    p.init();
    Map m = Map({Platform({Pair(0, -9), Pair(20, 11)})}, {});

    p.land(m.getPlatform(0));
    Pair requestedMotion = Pair(5 * sqrt(2.0), 0);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(Pair(15, 6), p.position);
}

TEST(Map, movePlayer_Grounded_Slant_Down_Left) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 1));
    p.init();
    Map m = Map({Platform({Pair(0, 11), Pair(20, -9)})}, {});

    p.land(m.getPlatform(0));
    Pair requestedMotion = Pair(5 * sqrt(2.0), 0);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(Pair(15, -4), p.position);
}

TEST(Map, movePlayer_Grounded_Flat_Into_Wall) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map(
        {
            Platform({Pair(0, 10), Pair(20, 10)}),
            Platform({Pair(15, 20), Pair(15, -20)}),
        },
        {});

    p.land(m.getPlatform(0));
    Pair requestedMotion = Pair(10, 0);
    m.movePlayer(p, requestedMotion);

    EXPECT_NEAR(15, p.currentCollision->postCollision.right.x, 0.000001);
    EXPECT_NEAR(10, p.currentCollision->postCollision.bottom.y, 0.000001);
}

TEST(Map, movePlayer_Grounded_Slant_Down_Left_Into_Wall) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map(
        {
            Platform({Pair(0, 5), Pair(20, 15)}),
            Platform({Pair(15, 20), Pair(15, -20)}),
        },
        {});

    p.land(m.getPlatform(0));

    Pair requestedMotion = Pair(5 * sqrt(2.0), 0);
    m.movePlayer(p, requestedMotion);

    PlatformSegment segment = m.getPlatform(0)->getSegment(0);

    EXPECT_NEAR(15, p.currentCollision->postCollision.right.x, 0.000001);
    EXPECT_TRUE(onLine(*segment.firstPoint(), *segment.secondPoint(),
                       p.currentCollision->postCollision.bottom));
}

TEST(Map, movePlayer_Grounded_Slant_Down_Right_Into_Wall) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map(
        {
            Platform({Pair(0, 15), Pair(20, 5)}),
            Platform({Pair(15, 20), Pair(15, -20)}),
        },
        {});

    p.land(m.getPlatform(0));

    Pair requestedMotion = Pair(5 * sqrt(2.0), 0);
    m.movePlayer(p, requestedMotion);

    PlatformSegment segment = m.getPlatform(0)->getSegment(0);

    EXPECT_NEAR(15, p.currentCollision->postCollision.right.x, 0.000001);
    EXPECT_TRUE(onLine(*segment.firstPoint(), *segment.secondPoint(),
                       p.currentCollision->postCollision.bottom));
}

TEST(Map, movePlayer_Airborne_Flat_Into_Wall) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map(
        {
            Platform({Pair(15, 20), Pair(15, -20)}),
        },
        {});

    Pair requestedMotion = Pair(10, 0);
    m.movePlayer(p, requestedMotion);

    EXPECT_NEAR(15, p.currentCollision->postCollision.right.x, 0.000001);
    EXPECT_NEAR(p.currentCollision->playerModified.origin.y,
                p.currentCollision->postCollision.origin.y, 0.000001);
}

TEST(Map, movePlayer_Airborne_Slanted_Up_Into_Wall) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map(
        {
            Platform({Pair(11, 22), Pair(11, -22)}),
        },
        {});

    Pair requestedMotion = Pair(10, -5);
    m.movePlayer(p, requestedMotion);

    EXPECT_NEAR(p.currentCollision->postCollision.right.x, 11, 0.000001);

    EXPECT_NEAR(p.position.y, 5, 0.000001);
}

TEST(Map, movePlayer_Airborne_Slanted_Down_Into_Wall) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 10));
    p.init();
    Map m = Map(
        {
            Platform({Pair(11, 22), Pair(11, -22)}),
        },
        {});

    Pair requestedMotion = Pair(10, 5);
    m.movePlayer(p, requestedMotion);

    EXPECT_NEAR(p.currentCollision->postCollision.right.x, 11, 0.000001);

    EXPECT_NEAR(p.position.y, 15, 0.000001);
}

TEST(Map, movePlayer_Airborne_Slanted_Up_Into_Wall_Slip_Off) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 0));
    p.init();
    Map m = Map(
        {
            Platform({Pair(10, 1), Pair(10, -1)}),
        },
        {});

    // reset ECB so we are resting exactly on the wall
    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.setRight(Pair(10, 0));
    p.moveTo(tmpCollision);

    Pair requestedMotion = Pair(10, -20);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(19.5, -20));
}

TEST(Map, movePlayer_Airborne_Flat_Into_Corner_TopRight) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 0));
    p.init();
    Map m = Map(
        {
            Platform({Pair(10, -0.1), Pair(11, -0.1)}),
        },
        {});

    // reset ECB so we are resting exactly on the wall
    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.widthRight = 1;
    tmpCollision.heightTop = 1;
    tmpCollision.widthLeft = 1;
    tmpCollision.heightBottom = 1;
    tmpCollision.setRight(Pair(9, 0));
    p.moveTo(tmpCollision);

    Pair requestedMotion = Pair(10, 0);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(10.1, 0));
    EXPECT_EQ(p.currentCollision->postCollision.top, Pair(9.1, -1));
}

TEST(Map, movePlayer_Airborne_Diagonal_Down_Corner_TopRight) {
    /*
        ╱╲ ─────
        ╲╱ moving down and to the right, collide with the point
    */

    // setup scene
    Player p = makeMockPlayer(Pair(10, 0));
    p.init();
    Map m = Map(
        {
            Platform({Pair(9.9, -0.1), Pair(11, -0.1)}),
        },
        {});

    // reset ECB so we are resting exactly on the wall
    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.widthRight = 1;
    tmpCollision.heightTop = 1;
    tmpCollision.widthLeft = 1;
    tmpCollision.heightBottom = 1;
    tmpCollision.setRight(Pair(10, 0));
    p.moveTo(tmpCollision);

    // exact //

    Pair requestedMotion = Pair(10, 0.9);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(10.9, 0.9));
    EXPECT_EQ(p.currentCollision->postCollision.top, Pair(9.9, -0.1));

    // overshoot //

    p.moveTo(tmpCollision);

    requestedMotion = Pair(10, 1.8);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(15.9, 1.8));
    EXPECT_EQ(p.currentCollision->postCollision.top, Pair(14.9, 0.8));

    // undershoot //

    p.moveTo(tmpCollision);

    requestedMotion = Pair(0.1, 0.1);
    m.movePlayer(p, requestedMotion);

    // TODO is this right?
    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(10.1, 0.1));
    EXPECT_EQ(p.currentCollision->postCollision.top, Pair(9.1, -0.9));
}

TEST(Map, movePlayer_Airborne_Diagonal_Up_Corner_BottomRight) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 0));
    p.init();
    Map m = Map(
        {
            Platform({Pair(9.9, 0.1), Pair(11, 0.1)}),
        },
        {});

    // reset ECB so we are resting exactly on the wall
    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.widthRight = 1;
    tmpCollision.heightTop = 1;
    tmpCollision.widthLeft = 1;
    tmpCollision.heightBottom = 1;
    tmpCollision.setRight(Pair(10, 0));
    p.moveTo(tmpCollision);

    Pair requestedMotion = Pair(10, -0.9);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(10.9, -0.9));
    EXPECT_EQ(p.currentCollision->postCollision.top, Pair(9.9, -1.9));

    // overshoot //

    p.moveTo(tmpCollision);

    requestedMotion = Pair(10, -1.8);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(15.9, -1.8));
    EXPECT_EQ(p.currentCollision->postCollision.bottom, Pair(14.9, -0.8));

    // undershoot //

    p.moveTo(tmpCollision);

    requestedMotion = Pair(0.1, -0.1);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(10.1, -0.1));
    EXPECT_EQ(p.currentCollision->postCollision.top, Pair(9.1, -1.1));
}

TEST(Map, movePlayer_Airborne_DownY_BottomRight) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 0));
    p.init();
    Map m = Map(
        {
            Platform({Pair(0.1, 5), Pair(0.1, 10)}),
        },
        {});

    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.widthRight = 1;
    tmpCollision.heightTop = 1;
    tmpCollision.widthLeft = 1;
    tmpCollision.heightBottom = 1;
    tmpCollision.setOrigin(Pair(0, 0));
    p.moveTo(tmpCollision);

    // move down and try to slide off the point to the left.
    // Y should be unaffected, should slide to the left.
    p.moveTo(tmpCollision);
    Pair requestedMotion = Pair(0, 10);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(0.1, 10));
}

TEST(Map, movePlayer_Airborne_UpY_TopRight) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 0));
    p.init();
    Map m = Map(
        {
            Platform({Pair(0.1, -5), Pair(0.1, -10)}),
        },
        {});

    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.widthRight = 1;
    tmpCollision.heightTop = 1;
    tmpCollision.widthLeft = 1;
    tmpCollision.heightBottom = 1;
    tmpCollision.setOrigin(Pair(0, 0));
    p.moveTo(tmpCollision);

    // move down and try to slide off the point to the left.
    // Y should be unaffected, should slide to the left.
    p.moveTo(tmpCollision);
    Pair requestedMotion = Pair(0, -10);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(0.1, -10));
}

TEST(Map, movePlayer_Airborne_DownY_BottomRight_SecondSegment) {
    // setup scene
    Player p = makeMockPlayer(Pair(10, 0));
    p.init();
    Map m = Map(
        {
            Platform({Pair(0.1, 10), Pair(0.1, 5)}),
        },
        {});

    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.widthRight = 1;
    tmpCollision.heightTop = 1;
    tmpCollision.widthLeft = 1;
    tmpCollision.heightBottom = 1;
    tmpCollision.setOrigin(Pair(0, 0));
    p.moveTo(tmpCollision);

    // move down and try to slide off the point to the left.
    // Y should be unaffected, should slide to the left.
    p.moveTo(tmpCollision);
    Pair requestedMotion = Pair(0, 10);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.right, Pair(0.1, 10));
}

TEST(Map, movePlayer_Upward_Partial_Ledge_Collision) {
    /*
        Test that we can decide between sliding along the top
        right edge or the bottom right edge in this situation

        moving up, colliding with both
        the top right and bottom right ECb

        ╱╲
       ╱  ╲┌──── corner @ 3.4,  1.1
       ╲  ╱│      touching the ECB's right edge to start
        ╲╱ │

    */

    // setup scene
    Player p = makeMockPlayer(Pair(3.34, 1.1));
    p.init();
    Map m = Map(
        {
            Platform({
                Pair(3.4, 1.5), Pair(3.4, 1.1), Pair(3.5, 1.1),
            }),
        },
        {});

    Ecb tmpCollision = p.currentCollision->postCollision;
    tmpCollision.setOrigin(Pair(3.34, 1.1));
    tmpCollision.widthLeft = 0.06;
    tmpCollision.widthRight = 0.06;
    tmpCollision.heightTop = 0.1;
    tmpCollision.heightBottom = 0.1;

    // move down and try to slide off the point to the left.
    // Y should be unaffected, should slide to the left.
    p.moveTo(tmpCollision);
    Pair requestedMotion = Pair(0.0141667, -0.00375);
    m.movePlayer(p, requestedMotion);

    EXPECT_EQ(p.currentCollision->postCollision.origin, Pair(3.34225, 1.09625));
}
