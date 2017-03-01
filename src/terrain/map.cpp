#include "./map.hpp"
#include "util.hpp"
#include "constants.hpp"
#include <iostream>
#include "engine/game.hpp"

Map::Map(std::vector<Platform> platforms, std::vector<Ledge> ledges)
    : platforms(platforms), ledges(ledges){};

bool Map::getClosestCollision(Pair const& start,
                              Pair const& end,
                              CollisionDatum& out,
                              Platform* ignoredCollision) {
    double closestDist = DOUBLE_INFINITY;
    PlatformSegment segment;
    Pair collisionPoint;
    bool anyCollision = false;

    for (Platform& p : platforms) {
        if (&p == ignoredCollision)
            continue;

        TerrainCollisionType t =
            p.checkCollision(start, end, collisionPoint, segment);

        double distance = (collisionPoint - start).euclid();
        if (t != NO_COLLISION && distance < closestDist) {
            closestDist = distance;
            out.type = t;
            out.segment = segment;
            out.position = collisionPoint;
            anyCollision = true;
        }
    }

    return anyCollision;
}

bool Map::getClosestEcbCollision(Ecb const& start,
                                 Ecb const& end,
                                 CollisionDatum& closestCollision) {
    double closestDist = DOUBLE_INFINITY;
    CollisionDatum collision;
    bool anyCollision = false;

    getClosestCollision(start.left, end.left, collision);
    if ((collision.position - start.left).euclid() < closestDist) {
        closestCollision = collision;
        closestCollision.position.x -= start.widthLeft;
        anyCollision = true;
    }

    getClosestCollision(start.right, end.right, collision);
    if ((collision.position - start.right).euclid() < closestDist) {
        closestCollision = collision;
        closestCollision.position.x += start.widthRight;
        anyCollision = true;
    }

    getClosestCollision(start.top, end.top, collision);
    if ((collision.position - start.top).euclid() < closestDist) {
        closestCollision = collision;
        closestCollision.position.y -= start.heightTop;
        anyCollision = true;
    }

    getClosestCollision(start.bottom, end.bottom, collision);
    if ((collision.position - start.bottom).euclid() < closestDist) {
        closestCollision = collision;
        closestCollision.position.y += start.heightBottom;
        anyCollision = true;
    }

    return anyCollision;
}

void Map::movePlayer(Player& player, Pair& requestedDistance) {
    grabLedges(player);

    std::cout << "movePlayer " << requestedDistance << std::endl;

    // if the player is grounded, move them along the platform
    Pair projectedPosition;
    if (player.isGrounded()) {
        // move player along platform
        projectedPosition = player.position;
        player.getCurrentPlatform()->groundedMovement(projectedPosition,
                                                      requestedDistance);

        if (requestedDistance.euclidSquared() > 0) {
            player.fallOffPlatform();
        }

        // add remaining distnace (for falling)
        projectedPosition += requestedDistance;
    }

    // if the player is not grounded, just move them according to requested
    // movement
    else {
        projectedPosition = player.position + requestedDistance;
    }

    std::cout << "projected position " << projectedPosition << std::endl;

    Ecb* currentEcb = &(player.currentCollision->playerModified);
    Ecb* projectedEcb = &(player.currentCollision->postCollision);
    projectedEcb->setOrigin(projectedPosition + PLAYER_ECB_OFFSET);

    Platform* currentPlatform = player.getCurrentPlatform();

    CollisionDatum collision;
    if (getClosestCollision(currentEcb->bottom, projectedEcb->bottom, collision,
                            currentPlatform)) {
        // if there is a collision with the floor, land
        std::cout << "collision bottom" << std::endl;
        if (collision.type == FLOOR_COLLISION) {
            std::cout << "triggering landing "
                      << collision.segment.getPlatform() << std::endl;

            player.land(collision.segment.getPlatform());
            projectedEcb->setBottom(collision.position);
        }
    }

    if (getClosestCollision(currentEcb->right, projectedEcb->right, collision,
                            currentPlatform)) {
        std::cout << "collision right" << std::endl;
        if (collision.type == WALL_COLLISION) {
            std::cout << "colliding with wall "
                      << collision.segment.getPlatform() << " at "
                      << collision.position << std::endl;

            projectedEcb->setRight(collision.position);

            if (!player.isGrounded()) {
                // TODO perform sliding
            }
        }
    }

    // reset plyer position to the projected Ecb
    player.moveTo(*projectedEcb);
}

void Map::grabLedges(Player& player) {
    if (player.canGrabLedge()) {
        for (Ledge& l : ledges) {
            Pair ledgebox_position = player.position + Pair(0, -LEDGEBOX_BASE);
            Pair diff = l.position - ledgebox_position;
            if (sign(diff.x) == sign(player.face) && player.face != l.facing &&
                std::abs(diff.x) < LEDGEBOX_WIDTH &&
                diff.y > -LEDGEBOX_HEIGHT && diff.y < 0) {
                player.grabLedge(&l);
                return;
            }
        }
    }
}

void Map::render(SDL_Renderer* r) {
    for (Platform& p : platforms) {
        p.render(r);
    }

    for (Ledge& l : ledges) {
        l.render(r);
    }
}

Platform* Map::getPlatform(size_t index) {
    return &(platforms[index]);
}
