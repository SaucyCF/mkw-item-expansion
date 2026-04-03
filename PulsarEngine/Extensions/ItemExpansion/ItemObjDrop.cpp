#include <Extensions/ItemExpansion/ItemObjDrop.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Archive/ArchiveMgr.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/CourseMgr.hpp>
#include <PulsarSystem.hpp>
#include <Race/Boo.hpp>
#include <Race/Feather.hpp>
#include <include/c_math.h>

// Expanded behaviourTable in mod BSS (from ItemSlotExpansion.cpp)
extern "C" Item::Behavior expandedBehaviourTable[25];

namespace Pulsar {
namespace Race {

static DroppedItem booDrops[MAX_DROPS_PER_TYPE];
static DroppedItem featherDrops[MAX_DROPS_PER_TYPE];
static bool dropSystemInitialized = false;

static u32 booSpawnTimer = 0;
static u32 featherSpawnTimer = 0;

// Cached BRRES resources
static nw4r::g3d::ResFile booDropRes;
static nw4r::g3d::ResFile featherDropRes;
static bool booDropResLoaded = false;
static bool featherDropResLoaded = false;

u32 GetBooSpawnTimer()     { return booSpawnTimer; }
u32 GetFeatherSpawnTimer() { return featherSpawnTimer; }

void ResetBooSpawnTimer()     { booSpawnTimer = BOO_PULL_TIMER_FRAMES; }
void ResetFeatherSpawnTimer() { featherSpawnTimer = FEATHER_PULL_TIMER_FRAMES; }

void UpdateSpawnTimers() {
    if (booSpawnTimer > 0)     booSpawnTimer--;
    if (featherSpawnTimer > 0) featherSpawnTimer--;
}

static inline float DistanceSq(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
}

static DroppedItem* GetDropArray(ItemId type) {
    if (type == BOO) return booDrops;
    if (type == FEATHER) return featherDrops;
    return nullptr;
}

static u32 GetMaxCount(ItemId type) {
    if (type == BOO) return BOO_MAX_COUNT;
    if (type == FEATHER) return FEATHER_MAX_COUNT;
    return 0;
}

static bool IsResLoaded(ItemId type) {
    if (type == BOO) return booDropResLoaded;
    if (type == FEATHER) return featherDropResLoaded;
    return false;
}

static nw4r::g3d::ResFile& GetResFile(ItemId type) {
    if (type == BOO) return booDropRes;
    if (type == FEATHER) return featherDropRes;
    return booDropRes;
}

static const char* GetModelName(ItemId type) {
    if (type == BOO) return "boo";
    if (type == FEATHER) return "gesso";
    return "boo";
}

static const char* GetBRRESName(ItemId type) {
    if (type == BOO) return "boo.brres";
    if (type == FEATHER) return "feather.brres";
    return "boo.brres";
}

static float GetDropModelScale(ItemId type) {
    if (type == BOO) return BOO_DROP_MODEL_SCALE;
    if (type == FEATHER) return FEATHER_DROP_MODEL_SCALE;
    return 1.0f;
}

static void UpdateDropModelPosition(DroppedItem& drop) {
    if (drop.model == nullptr || !drop.active) return;

    float scale = GetDropModelScale(drop.type);

    float cosA = static_cast<float>(cos(static_cast<double>(drop.rotationAngle)));
    float sinA = static_cast<float>(sin(static_cast<double>(drop.rotationAngle)));

    nw4r::math::MTX34 mtx;
    mtx.n00 = cosA  * scale; mtx.n01 = 0.0f;          mtx.n02 = sinA * scale;
    mtx.n10 = 0.0f;          mtx.n11 = scale;          mtx.n12 = 0.0f;
    mtx.n20 = -sinA * scale;  mtx.n21 = 0.0f;          mtx.n22 = cosA * scale;
    mtx.n03 = drop.position.x;
    mtx.n13 = drop.position.y + DROP_HEIGHT_OFFSET;
    mtx.n23 = drop.position.z;

    for (int j = 0; j < 2; ++j) {
        EGG::ScnMdlEx* ex = drop.model->scnMdlEx[j];
        if (ex != nullptr && ex->scnObj != nullptr) {
            ex->scnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, mtx);
        }
    }
}

static bool pickedUpThisFrame[12];

static void OnPickup(u8 playerId, ItemId droppedType) {
    Item::Manager* mgr = Item::Manager::sInstance;
    if (mgr == nullptr || playerId >= 12 || playerId >= mgr->playerCount) return;

    if (pickedUpThisFrame[playerId]) return;

    Item::Player& player = mgr->players[playerId];

    bool hasExistingItem = (player.inventory.currentItemId != ITEM_NONE);

    if (hasExistingItem) {
        ItemId savedId = player.inventory.currentItemId;
        u32 savedCount = player.inventory.currentItemCount;
        bool savedForced = player.inventory.isItemForcedDueToCapacity;

        player.inventory.SetItem(droppedType, false);
        if (droppedType == BOO) {
            UseBoo(player);
        } else if (droppedType == FEATHER) {
            UseFeather(player);
        }

        player.inventory.SetItem(savedId, savedForced);
        player.inventory.currentItemCount = savedCount;
    } else {
        player.inventory.SetItem(droppedType, false);
        if (droppedType == BOO) {
            UseBoo(player);
        } else if (droppedType == FEATHER) {
            UseFeather(player);
        }
    }

    pickedUpThisFrame[playerId] = true;
}

static void CheckPickups() {
    Kart::Manager* kartMgr = Kart::Manager::sInstance;
    if (kartMgr == nullptr) return;

    u8 playerCount = kartMgr->playerCount;
    if (playerCount > 12) playerCount = 12;

    for (int typeIdx = 0; typeIdx < 2; ++typeIdx) {
        ItemId type = (typeIdx == 0) ? BOO : FEATHER;
        DroppedItem* drops = GetDropArray(type);
        if (drops == nullptr) continue;

        for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
            if (!drops[i].active) continue;

            for (u8 p = 0; p < playerCount; ++p) {
                if (p == drops[i].ownerPlayerId && drops[i].age < 60) continue;

                Kart::Player* kp = kartMgr->GetKartPlayer(p);
                if (kp == nullptr) continue;

                if (IsPlayerInBooState(p)) continue;

                const Vec3& kartPos = kp->GetPosition();
                float distSq = DistanceSq(kartPos, drops[i].position);

                if (distSq < DROP_PICKUP_RADIUS_SQ) {
                    OnPickup(p, type);

                    drops[i].active = false;
                    if (drops[i].model != nullptr) {
                        drops[i].model->ToggleVisible(false);
                    }
                    break;
                }
            }
        }
    }
}

void ResetDropSystem() {
    for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
        booDrops[i].model = nullptr;
        booDrops[i].active = false;
        booDrops[i].age = 0;
        booDrops[i].bobPhase = 0.0f;
        booDrops[i].rotationAngle = 0.0f;
        booDrops[i].velocity.x = 0.0f;
        booDrops[i].velocity.y = 0.0f;
        booDrops[i].velocity.z = 0.0f;
        booDrops[i].isEjected = false;
        booDrops[i].ejectTimer = 0;
        booDrops[i].spawnY = 0.0f;

        featherDrops[i].model = nullptr;
        featherDrops[i].active = false;
        featherDrops[i].age = 0;
        featherDrops[i].bobPhase = 0.0f;
        featherDrops[i].rotationAngle = 0.0f;
        featherDrops[i].velocity.x = 0.0f;
        featherDrops[i].velocity.y = 0.0f;
        featherDrops[i].velocity.z = 0.0f;
        featherDrops[i].isEjected = false;
        featherDrops[i].ejectTimer = 0;
        featherDrops[i].spawnY = 0.0f;
    }
    booSpawnTimer = 0;
    featherSpawnTimer = 0;
    dropSystemInitialized = false;
    booDropResLoaded = false;
    featherDropResLoaded = false;
}

static void DisableBillboard(ModelDirector* model) {
    if (model == nullptr) return;
    nw4r::g3d::ResMdl& mdl = model->rawMdl;
    u32 nodeCount = mdl.GetResNodeNumEntries();
    for (u32 n = 0; n < nodeCount; ++n) {
        nw4r::g3d::ResNode node = mdl.GetResNode(n);
        if (node.data != nullptr) {
            node.data->billboardMode = nw4r::g3d::ResNodeData::BILLBOARD_OFF;
        }
    }
}

static void InitDropModels(ItemId type) {
    const char* brresName = GetBRRESName(type);
    const char* mdlName = GetModelName(type);
    DroppedItem* drops = GetDropArray(type);
    nw4r::g3d::ResFile& res = GetResFile(type);
    bool& loaded = (type == BOO) ? booDropResLoaded : featherDropResLoaded;

    if (!ModelDirector::BRRESExists(ARCHIVE_HOLDER_COMMON, brresName)) return;

    ModelDirector::BindBRRES(res, ARCHIVE_HOLDER_COMMON, brresName);
    if (res.data == nullptr) return;
    loaded = true;

    if (!ModelDirector::MdlExists(mdlName, res)) return;

    u32 maxCount = GetMaxCount(type);
    for (u32 i = 0; i < maxCount && i < MAX_DROPS_PER_TYPE; ++i) {
        drops[i].model = new ModelDirector(0xb, 0);
        if (drops[i].model == nullptr) continue;
        drops[i].model->LoadWithAnm(mdlName, res, nullptr);
        DisableBillboard(drops[i].model);
        if (type == BOO) {
            drops[i].model->LinkAnimation(0, res, "wait", ANMTYPE_CHR, true,
                nullptr, ARCHIVE_HOLDER_COMMON, 0);
        }

        drops[i].model->ToggleVisible(false);
        drops[i].active = false;
        drops[i].age = 0;
        drops[i].bobPhase = static_cast<float>(i) * 1.57f;
        drops[i].rotationAngle = 0.0f;
    }
}

void InitDropSystem() {
    ResetDropSystem();
    InitDropModels(BOO);
    InitDropModels(FEATHER);
    dropSystemInitialized = true;
}

u32 GetActiveDropCount(ItemId type) {
    DroppedItem* drops = GetDropArray(type);
    if (drops == nullptr) return 0;
    u32 count = 0;
    for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
        if (drops[i].active) count++;
    }
    return count;
}

void SpawnDroppedItem(ItemId type, const Vec3& position, u8 ownerPlayerId) {
    if (!dropSystemInitialized) return;
    if (!IsResLoaded(type)) return;

    DroppedItem* drops = GetDropArray(type);
    if (drops == nullptr) return;

    u32 maxCount = GetMaxCount(type);
    u32 activeCount = GetActiveDropCount(type);

    if (activeCount >= maxCount) {
        u32 oldestIdx = 0;
        u32 highestAge = 0;
        for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
            if (drops[i].active && drops[i].age > highestAge) {
                highestAge = drops[i].age;
                oldestIdx = i;
            }
        }
        drops[oldestIdx].active = false;
        if (drops[oldestIdx].model != nullptr) {
            drops[oldestIdx].model->ToggleVisible(false);
        }
    }

    for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
        if (drops[i].active) continue;
        if (drops[i].model == nullptr) continue;

        drops[i].active = true;
        drops[i].type = type;
        drops[i].position = position;
        drops[i].age = 0;
        drops[i].ownerPlayerId = ownerPlayerId;
        drops[i].bobPhase = 0.0f;
        drops[i].rotationAngle = 0.0f;
        drops[i].velocity.x = 0.0f;
        drops[i].velocity.y = 0.0f;
        drops[i].velocity.z = 0.0f;
        drops[i].isEjected = false;
        drops[i].ejectTimer = 0;
        drops[i].spawnY = position.y;

        drops[i].model->ToggleVisible(true);
        if (type == BOO && drops[i].model->modelTransformator != nullptr) {
            drops[i].model->modelTransformator->PlayAnmNoBlend(0, 0.0f, 1.0f);
        }
        UpdateDropModelPosition(drops[i]);
        return;
    }
}

static u32 ejectAngleCounter = 0;

void EjectDroppedItem(ItemId type, const Vec3& position, u8 ownerPlayerId) {
    if (!dropSystemInitialized) return;
    if (!IsResLoaded(type)) return;

    DroppedItem* drops = GetDropArray(type);
    if (drops == nullptr) return;

    u32 maxCount = GetMaxCount(type);
    u32 activeCount = GetActiveDropCount(type);

    if (activeCount >= maxCount) {
        u32 oldestIdx = 0;
        u32 highestAge = 0;
        for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
            if (drops[i].active && drops[i].age > highestAge) {
                highestAge = drops[i].age;
                oldestIdx = i;
            }
        }
        drops[oldestIdx].active = false;
        if (drops[oldestIdx].model != nullptr) {
            drops[oldestIdx].model->ToggleVisible(false);
        }
    }

    for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
        if (drops[i].active) continue;
        if (drops[i].model == nullptr) continue;
        float angle = static_cast<float>(ejectAngleCounter++) * 2.39996f;
        float cosA = static_cast<float>(cos(static_cast<double>(angle)));
        float sinA = static_cast<float>(sin(static_cast<double>(angle)));

        drops[i].active = true;
        drops[i].type = type;
        drops[i].position = position;
        drops[i].age = 0;
        drops[i].ownerPlayerId = ownerPlayerId;
        drops[i].bobPhase = 0.0f;
        drops[i].rotationAngle = 0.0f;
        drops[i].velocity.x = cosA * EJECT_HORIZ_SPEED;
        drops[i].velocity.y = EJECT_UP_SPEED;
        drops[i].velocity.z = sinA * EJECT_HORIZ_SPEED;
        drops[i].isEjected = true;
        drops[i].ejectTimer = EJECT_SETTLE_FRAMES;
        drops[i].spawnY = position.y;

        drops[i].model->ToggleVisible(true);
        if (type == BOO && drops[i].model->modelTransformator != nullptr) {
            drops[i].model->modelTransformator->PlayAnmNoBlend(0, 0.0f, 1.0f);
        }
        UpdateDropModelPosition(drops[i]);
        return;
    }
}

u32 CountPlayersHoldingItem(ItemId targetItem) {
    Item::Manager* mgr = Item::Manager::sInstance;
    if (mgr == nullptr) return 0;
    u32 count = 0;
    for (u8 i = 0; i < mgr->playerCount; ++i) {
        if (mgr->players[i].inventory.currentItemId == targetItem) {
            count++;
        }
    }
    return count;
}

void UpdateDropSystem() {
    if (!dropSystemInitialized) return;

    memset(pickedUpThisFrame, 0, sizeof(pickedUpThisFrame));

    UpdateSpawnTimers();

    for (int typeIdx = 0; typeIdx < 2; ++typeIdx) {
        ItemId type = (typeIdx == 0) ? BOO : FEATHER;
        DroppedItem* drops = GetDropArray(type);
        if (drops == nullptr) continue;

        for (u32 i = 0; i < MAX_DROPS_PER_TYPE; ++i) {
            if (!drops[i].active) continue;

            drops[i].age++;

            if (drops[i].isEjected) {
                Vec3 prevPos = drops[i].position;

                drops[i].position.x += drops[i].velocity.x;
                drops[i].position.y += drops[i].velocity.y;
                drops[i].position.z += drops[i].velocity.z;

                drops[i].velocity.y -= EJECT_GRAVITY;

                if (drops[i].ejectTimer > 0) {
                    drops[i].velocity.x *= EJECT_FRICTION;
                    drops[i].velocity.z *= EJECT_FRICTION;
                    drops[i].ejectTimer--;
                } else {
                    drops[i].velocity.x *= 0.9f;
                    drops[i].velocity.z *= 0.9f;
                }

                bool landed = false;
                CourseMgr* course = CourseMgr::sInstance;
                if (course != nullptr) {
                    CollisionInfo colInfo;
                    memset(&colInfo, 0, sizeof(colInfo));
                    KCLTypeHolder kclType;
                    kclType.Reset();
                    KCLBitfield floorBits = (KCLBitfield)(
                        KCL_BITFIELD_ROAD | KCL_BITFIELD_SLIPPERY_ROAD |
                        KCL_BITFIELD_WEAK_OFFROAD | KCL_BITFIELD_NORMAL_OFFROAD |
                        KCL_BITFIELD_HEAVY_OFFROAD | KCL_BITFIELD_ICY_ROAD |
                        KCL_BITFIELD_BOOST_PANEL | KCL_BITFIELD_BOOST_RAMP |
                        KCL_BITFIELD_JUMP_PAD |
                        KCL_BITFIELD_ITEM_ROAD | KCL_BITFIELD_SOLID_FALL |
                        KCL_BITFIELD_MOVING_WATER |
                        KCL_BITFIELD_MOVING_ROAD | KCL_BITFIELD_STICKY_ROAD |
                        KCL_BITFIELD_ROAD2 | KCL_BITFIELD_HALFPIPE);

                    if (course->IsCollidingAddEntry(
                            drops[i].position, prevPos, floorBits,
                            &colInfo, &kclType, 0, 10.0f)) {
                        drops[i].position.x += colInfo.dirToClosestTri.x;
                        drops[i].position.y += colInfo.dirToClosestTri.y;
                        drops[i].position.z += colInfo.dirToClosestTri.z;
                        landed = true;
                    }
                    if (!landed) {
                        CollisionInfo wallInfo;
                        memset(&wallInfo, 0, sizeof(wallInfo));
                        KCLTypeHolder wallKclType;
                        wallKclType.Reset();

                        KCLBitfield wallBits = (KCLBitfield)(
                            KCL_BITFIELD_WALL | KCL_BITFIELD_INVISIBLE_WALL |
                            KCL_BITFIELD_ITEM_WALL);

                        if (course->IsCollidingAddEntry(
                                drops[i].position, prevPos, wallBits,
                                &wallInfo, &wallKclType, 0, 10.0f)) {
                            drops[i].position.x += wallInfo.dirToClosestTri.x;
                            drops[i].position.y += wallInfo.dirToClosestTri.y;
                            drops[i].position.z += wallInfo.dirToClosestTri.z;
                            drops[i].velocity.x *= -0.3f;
                            drops[i].velocity.z *= -0.3f;
                        }
                    }
                }

                if (landed) {
                    drops[i].isEjected = false;
                    drops[i].velocity.x = 0.0f;
                    drops[i].velocity.y = 0.0f;
                    drops[i].velocity.z = 0.0f;
                } else if (drops[i].position.y < drops[i].spawnY - 2000.0f) {
                    drops[i].active = false;
                    if (drops[i].model != nullptr) {
                        drops[i].model->ToggleVisible(false);
                    }
                }
            }

            UpdateDropModelPosition(drops[i]);
        }
    }

    CheckPickups();
}
RaceFrameHook DropSystemUpdate(UpdateDropSystem);

static void DropSystemRaceLoad() {
    InitDropSystem();
}
RaceLoadHook DropSystemInit(DropSystemRaceLoad);

} // namespace Race
} // namespace Pulsar
