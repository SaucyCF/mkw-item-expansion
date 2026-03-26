#ifndef _PUL_RACE_ITEMOBJDROP_HPP_
#define _PUL_RACE_ITEMOBJDROP_HPP_

#include <kamek.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/3D/Model/ModelDirector.hpp>

namespace Pulsar {
namespace Race {

static const u32 BOO_MAX_COUNT = 4;
static const u32 FEATHER_MAX_COUNT = 4;
static const u32 BOO_PULL_TIMER_FRAMES = 0;
static const u32 FEATHER_PULL_TIMER_FRAMES = 0;
static const float DROP_PICKUP_RADIUS_SQ = 22500.0f;
static const u32 MAX_DROPS_PER_TYPE = 8;
static const float DROP_HEIGHT_OFFSET = 0.0f;
static const float DROP_BOB_AMPLITUDE = 8.0f;
static const float DROP_BOB_SPEED = 0.05f;
static const float DROP_ROTATE_SPEED = 0.04f;
static const float BOO_DROP_MODEL_SCALE = 1.2f;
static const float FEATHER_DROP_MODEL_SCALE = 0.5f;
static const float EJECT_HORIZ_SPEED = 15.0f;
static const float EJECT_UP_SPEED = 10.0f;
static const float EJECT_GRAVITY = 1.2f;
static const float EJECT_FRICTION = 0.97f;
static const u32 EJECT_SETTLE_FRAMES = 45;

struct DroppedItem {
    bool active;
    ItemId type;
    Vec3 position;
    ModelDirector* model;
    u32 age;
    u8 ownerPlayerId;  
    float bobPhase;       
    float rotationAngle;
    Vec3 velocity;
    bool isEjected;   
    u32 ejectTimer;
    float spawnY;             
};

void InitDropSystem();
void UpdateDropSystem();
void SpawnDroppedItem(ItemId type, const Vec3& position, u8 ownerPlayerId);
void EjectDroppedItem(ItemId type, const Vec3& position, u8 ownerPlayerId);
u32 GetActiveDropCount(ItemId type);
u32 CountPlayersHoldingItem(ItemId type);
void ResetDropSystem();
u32 GetBooSpawnTimer();
u32 GetFeatherSpawnTimer();
void ResetBooSpawnTimer();
void ResetFeatherSpawnTimer();
void UpdateSpawnTimers();

} // namespace Race
} // namespace Pulsar

#endif
