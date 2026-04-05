#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <MarioKartWii/Item/Obj/Gesso.hpp>
#include <MarioKartWii/Driver/DriverManager.hpp>
#include <MarioKartWii/Input/InputManager.hpp>
#include <MarioKartWii/CourseMgr.hpp>
#include <PulsarSystem.hpp>
#include <Settings/SettingsParam.hpp>
#include <MarioKartWii/Archive/ArchiveMgr.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <include/c_math.h>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartCollision.hpp>
#include <MarioKartWii/Item/Obj/KouraRed.hpp>
#include <core/egg/3D/Scn.hpp>
#include <Network/PacketExpansion.hpp>
#include <MarioKartWii/3D/Camera/CameraMgr.hpp>
#include <MarioKartWii/3D/Model/ModelDirector.hpp>
#include <Extensions/ItemExpansion/ItemObjDrop.hpp>
#include <MarioKartWii/Race/Racedata.hpp>
#include <Sound/BooBRSTM.hpp>

// Please make sure to credit SaucyCF (Saucy on Tockdom) if you decide to use or modify this code in your own project!

// Expanded behaviourTable in mod BSS (from ItemSlotExpansion.cpp)
extern "C" Item::Behavior expandedBehaviourTable[27];

namespace Pulsar {
namespace Race {

static const u32 BOO_DURATION_FRAMES = 450;      // 7.5 seconds at 60fps
static const u32 BOO_STEAL_DELAY_FRAMES = 120;   // 2 seconds delay before item given
static const u32 DRAW_OPTIONS_NORMAL = 0xA;
static const u32 DRAW_OPTIONS_TRANSPARENT = 1;

bool booActive[12] = {};
u32 booTimers[12] = {};
bool wasInBoo[12] = {};
bool booStealing[12] = {};
u32 booStealTimers[12] = {};
u8 booStolenVictim[12] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
ItemId booStolenItem[12] = {};
bool booUsed[12] = {};

static bool booInitialized = false;
static u32 booFramesSinceLoad = 0;
static u32 booLastSoundFrame = 0xFFFFFFFF;
static inline bool HasValidVtable(void* ptr) {
    if (ptr == nullptr) return false;
    u32 vtable = *(u32*)ptr;
    return (vtable >= 0x80000000 && vtable < 0x81800000) || 
           (vtable >= 0x90000000 && vtable < 0x94000000);
}

static const u32 BOO_SOUND_COOLDOWN_FRAMES = 180; // 3 seconds at 60fps

static bool CanPlayBooSoundThisFrame() {
    if (booLastSoundFrame != 0xFFFFFFFF &&
        (booFramesSinceLoad - booLastSoundFrame) < BOO_SOUND_COOLDOWN_FRAMES) return false;
    booLastSoundFrame = booFramesSinceLoad;
    return true;
}

static void playBooSoundIfAllowed() {
    if (Sound::PlayBooBRSTM()) return;
}

static bool IsBooModeActive() {
    if (!booInitialized) return false;
    if (Racedata::sInstance == nullptr || System::sInstance == nullptr) return false;
    if (!HasValidVtable(Racedata::sInstance) || !HasValidVtable(System::sInstance)) return false;
    return true;
}

bool IsPlayerInBooState(u8 playerId) {
    if (playerId >= 12 || !IsBooModeActive()) return false;
    return booActive[playerId];
}

static const u32 BOO_VICTIM_ATTACK_FRAMES = 162;
static const float BOO_MODEL_SCALE = 1.0f;

struct BooVictimScreen {
    u32 state;
    ModelDirector* model;
    u32 screenId;
    u16 targetPlayerId;
    u32 timer;
};

static BooVictimScreen booVictimScreenData[4];
static u32 booVictimScreenCount = 0;
static bool booVictimSystemReady = false;

static void ActivateBooVictimScreen(u8 victimPlayerId);

static void UpdateBooVictimPosition(BooVictimScreen& screen) {
    if (screen.targetPlayerId >= 12 || screen.model == nullptr) return;

    Kart::Manager* kartMgr = Kart::Manager::sInstance;
    if (kartMgr == nullptr) return;

    Kart::Player* kp = kartMgr->GetKartPlayer(screen.targetPlayerId);
    if (kp == nullptr) return;

    // Get kart position
    const Vec3& kartPos = kp->GetPosition();
    float posX = kartPos.x;
    float posY = kartPos.y;
    float posZ = kartPos.z;

    float fwdX, fwdZ;
    bool gotCamera = false;

    if (kp->HasCamera()) {
        RaceCameraMgr* camMgr = RaceCameraMgr::sInstance;
        if (camMgr != nullptr && camMgr->sortedCameras != nullptr) {
            u32 screenIdx = kp->GetScreenIdx();
            if (screenIdx < camMgr->cameraCount) {
                RaceCamera* cam = camMgr->sortedCameras[screenIdx];
                if (cam != nullptr) {
                    EGG::Matrix34f& viewMtx = cam->GetViewMatrix();
                    fwdX = viewMtx.n20;
                    fwdZ = viewMtx.n22;
                    gotCamera = true;
                }
            }
        }
    }
    if (!gotCamera) {
        const Mtx34& kartMtx = kp->GetMtx();
        fwdX = -kartMtx.n02;
        fwdZ = -kartMtx.n22;
    }
    float lenSq = fwdX * fwdX + fwdZ * fwdZ;
    if (lenSq < 0.0001f) return;
    float invLen = 1.0f / static_cast<float>(sqrt(static_cast<double>(lenSq)));
    fwdX *= invLen;
    fwdZ *= invLen;

    float rightX = fwdZ;
    float rightZ = -fwdX;

    nw4r::math::MTX34 booMtx;
    booMtx.n00 = rightX * BOO_MODEL_SCALE; booMtx.n01 = 0.0f;                   booMtx.n02 = fwdX * BOO_MODEL_SCALE;
    booMtx.n10 = 0.0f;                     booMtx.n11 = 1.0f * BOO_MODEL_SCALE;  booMtx.n12 = 0.0f;
    booMtx.n20 = rightZ * BOO_MODEL_SCALE; booMtx.n21 = 0.0f;                   booMtx.n22 = fwdZ * BOO_MODEL_SCALE;
    booMtx.n03 = posX + (-100.0f) * fwdX;
    booMtx.n13 = posY + 150.0f;
    booMtx.n23 = posZ + (-100.0f) * fwdZ;

    for (int j = 0; j < 2; ++j) {
        EGG::ScnMdlEx* ex = screen.model->scnMdlEx[j];
        if (ex != nullptr && ex->scnObj != nullptr) {
            ex->scnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, booMtx);
        }
    }
}

extern "C" void GessoManager_createInstance();
static void InitBooVictimSystem() {
    GessoManager_createInstance();
    booVictimSystemReady = false;
    booVictimScreenCount = 0;
    for (u32 i = 0; i < 4; ++i) {
        booVictimScreenData[i].state = 0;
        booVictimScreenData[i].model = nullptr;
        booVictimScreenData[i].screenId = i;
        booVictimScreenData[i].targetPlayerId = 0xC;
        booVictimScreenData[i].timer = 0;
    }

    if (!ModelDirector::BRRESExists(ARCHIVE_HOLDER_COMMON, "boo.brres")) return;

    nw4r::g3d::ResFile booRes;
    ModelDirector::BindBRRES(booRes, ARCHIVE_HOLDER_COMMON, "boo.brres");
    if (booRes.data == nullptr) return;

    u8 screenCount = Racedata::sInstance->racesScenario.screenCount;
    if (screenCount > 4) screenCount = 4;
    booVictimScreenCount = screenCount;

    for (u32 i = 0; i < booVictimScreenCount; ++i) {
        BooVictimScreen& screen = booVictimScreenData[i];

        screen.model = new ModelDirector(0xb, 8);
        screen.model->LoadWithAnm("boo", booRes, nullptr);

        for (u32 j = 0; j < 4; ++j) {
            if (j == i) screen.model->EnableScreen(j);
            else screen.model->DisableScreen(j);
        }

        screen.model->LinkAnimation(0, booRes, "attack", ANMTYPE_CHR, true,
            nullptr, ARCHIVE_HOLDER_COMMON, 0);

        screen.model->DisableScreen(i);
    }

    booVictimSystemReady = true;
}
kmCall(0x807994a4, InitBooVictimSystem);

static void ResetBooVictimScreens() {
    for (u32 i = 0; i < 4; ++i) {
        booVictimScreenData[i].state = 0;
        booVictimScreenData[i].targetPlayerId = 0xC;
        booVictimScreenData[i].timer = 0;
        if (booVictimScreenData[i].model != nullptr) {
            booVictimScreenData[i].model->DisableScreen(i);
        }
    }
}

static void ActivateBooVictimScreen(u8 victimPlayerId) {
    if (!booVictimSystemReady || booVictimScreenCount == 0) return;
    if (victimPlayerId >= 12) return;

    const RacedataPlayer& rp = Racedata::sInstance->racesScenario.players[victimPlayerId];
    if (rp.playerType != PLAYER_REAL_LOCAL) return;

    s8 hudSlot = rp.hudSlotId;
    if (hudSlot < 0 || (u32)hudSlot >= booVictimScreenCount) return;

    BooVictimScreen& screen = booVictimScreenData[hudSlot];
    if (screen.state != 0) return;

    screen.state = 1;
    screen.targetPlayerId = victimPlayerId;
    screen.timer = 0;

    screen.model->EnableScreen(hudSlot);
    screen.model->modelTransformator->PlayAnmNoBlend(0, 0.0f, 1.0f);
    UpdateBooVictimPosition(screen);
}

static void UpdateBooVictimScreens() {
    if (!booVictimSystemReady || !IsBooModeActive()) return;
    for (u32 i = 0; i < booVictimScreenCount; ++i) {
        BooVictimScreen& screen = booVictimScreenData[i];
        if (screen.state == 0) continue;
        screen.timer++;
        UpdateBooVictimPosition(screen);
        switch (screen.state) {
            case 1:
                if (screen.timer >= BOO_VICTIM_ATTACK_FRAMES) {
                    screen.state = 0;
                    screen.targetPlayerId = 0xC;
                    screen.timer = 0;
                    screen.model->DisableScreen(screen.screenId);
                }
                break;
        }
    }
}
RaceFrameHook BooVictimScreenUpdate(UpdateBooVictimScreens);

static u8 GetPlayerCount() {
    Kart::Manager* mgr = Kart::Manager::sInstance;
    return (mgr == nullptr) ? 0 : mgr->playerCount;
}

static u8 GetPlayerPosition(u8 playerId) {
    Raceinfo* info = Raceinfo::sInstance;
    if (info == nullptr || info->players == nullptr || playerId >= 12) return 12;
    RaceinfoPlayer* player = info->players[playerId];
    return (player == nullptr) ? 12 : player->position;
}

static u8 FindPlayersInFront(u8 myPlayerId, u8* candidateIds, u8 maxCandidates) {
    u8 count = 0;
    u8 myPosition = GetPlayerPosition(myPlayerId);
    u8 playerCount = GetPlayerCount();
    if (playerCount > 12) playerCount = 12;
    
    for (u8 i = 0; i < playerCount && count < maxCandidates; ++i) {
        if (i == myPlayerId) continue;
        if (GetPlayerPosition(i) < myPosition) {
            candidateIds[count++] = i;
        }
    }
    return count;
}

static ItemId GetPlayerItem(u8 playerId) {
    Item::Manager* mgr = Item::Manager::sInstance;
    if (mgr == nullptr || mgr->players == nullptr || playerId >= 12 || playerId >= mgr->playerCount) {
        return ITEM_NONE;
    }

    Item::Player& player = mgr->players[playerId];

    if (!player.isRemote) {
        return player.inventory.currentItemId;
    }
    return Network::GetNetworkPlayerItem(playerId);
}

static void ApplyGhostTransparency(Kart::Pointers& pointers, bool transparent) {
    u32 priority = transparent ? DRAW_OPTIONS_TRANSPARENT : DRAW_OPTIONS_NORMAL;

    if (pointers.kartBody != nullptr && pointers.kartBody->isModelLoaded && HasValidVtable(pointers.kartBody)) {
        pointers.kartBody->UpdateModelDrawPriority(priority);
    }

    DriverController* driver = pointers.driverController;
    if (driver != nullptr && HasValidVtable(driver)) {
        if (driver->driverModel != nullptr && HasValidVtable(driver->driverModel)) {
            driver->driverModel->UpdateDrawPriority(priority);
        }
        if (driver->driverModel_lod != nullptr && HasValidVtable(driver->driverModel_lod)) {
            driver->driverModel_lod->UpdateDrawPriority(priority);
        }
        if (driver->miiHeads != nullptr && HasValidVtable(driver->miiHeads)) {
            driver->miiHeads->UpdateDrawPriority(priority);
        }
        if (driver->ticoModel != nullptr && HasValidVtable(driver->ticoModel) && 
            driver->ticoModel->tico != nullptr && HasValidVtable(driver->ticoModel->tico)) {
            driver->ticoModel->tico->UpdateDrawPriority(priority);
        }
        if (driver->toadetteHair != nullptr && driver->toadetteHair->cb != nullptr && 
            driver->toadetteHair->cb->hair != nullptr && HasValidVtable(driver->toadetteHair->cb->hair)) {
            driver->toadetteHair->cb->hair->UpdateDrawPriority(priority);
        }
    }

    if (pointers.values != nullptr && pointers.wheels != nullptr) {
        u16 wheelCount = pointers.values->wheelCount0;
        if (wheelCount > 4) wheelCount = 4;
        
        for (u16 j = 0; j < wheelCount; ++j) {
            if (pointers.wheels[j] != nullptr && pointers.wheels[j]->isModelLoaded && HasValidVtable(pointers.wheels[j])) {
                pointers.wheels[j]->UpdateModelDrawPriority(priority);
            }
        }
    }

    if (pointers.values != nullptr && pointers.suspensions != nullptr) {
        u16 suspCount = pointers.values->wheelCount1;
        if (suspCount > 4) suspCount = 4;
        
        for (u16 j = 0; j < suspCount; ++j) {
            if (pointers.suspensions[j] != nullptr && pointers.suspensions[j]->isModelLoaded && HasValidVtable(pointers.suspensions[j])) {
                pointers.suspensions[j]->UpdateModelDrawPriority(priority);
            }
        }
    }

    if (pointers.kartShadow != nullptr && HasValidVtable(pointers.kartShadow)) {
        pointers.kartShadow->EnableDraw(!transparent);
    }
}

static void CompleteItemSteal(u8 stealerId) {
    if (stealerId >= 12) return;
    Item::Manager* mgr = Item::Manager::sInstance;
    if (mgr == nullptr || mgr->players == nullptr || stealerId >= mgr->playerCount) return;

    mgr->players[stealerId].inventory.SetItem(booStolenItem[stealerId], false);

    const RacedataPlayer& rp = Racedata::sInstance->racesScenario.players[stealerId];
    if (rp.playerType == PLAYER_REAL_LOCAL) {
        ActivateBooVictimScreen(stealerId);
       playBooSoundIfAllowed();
    }

    booStealing[stealerId] = false;
    booStealTimers[stealerId] = 0;
    booStolenVictim[stealerId] = 0xFF;
    booUsed[stealerId] = false;
}

void UseBoo(Item::Player& itemPlayer) {
    u8 playerId = itemPlayer.id;
    if (playerId >= 12 || booActive[playerId] || booStealing[playerId] || booUsed[playerId]) return;
    
    booUsed[playerId] = true;

    u8 candidateIds[12];
    u8 candidateCount = FindPlayersInFront(playerId, candidateIds, 12);
    
    ItemId chosenItem = MUSHROOM;
    u8 victimId = 0xFF;
    
    if (candidateCount > 0) {
        u8 bestVictim = 0xFF;
        u8 bestPosition = 0;
        ItemId bestItem = ITEM_NONE;

        for (u8 i = 0; i < candidateCount; ++i) {
            u8 candId = candidateIds[i];
            u8 candPosition = GetPlayerPosition(candId);
            ItemId candItem = GetPlayerItem(candId);

            if (candItem == ITEM_NONE) continue;
            if (candItem == POW_BLOCK) continue;
            if (candItem == BOO) continue;

            if (candPosition > bestPosition || bestItem == ITEM_NONE) {
                bestVictim = candId;
                bestPosition = candPosition;
                bestItem = candItem;
            }
        }

        if (bestVictim != 0xFF && bestItem != ITEM_NONE) {
            victimId = bestVictim;
            chosenItem = bestItem;
        }
    }

    booStolenItem[playerId] = chosenItem;
    booStolenVictim[playerId] = victimId;

    if (victimId != 0xFF) {
        Item::Manager* itemMgr = Item::Manager::sInstance;
        if (itemMgr != nullptr && itemMgr->players != nullptr && 
            victimId < itemMgr->playerCount && !itemMgr->players[victimId].isRemote) {
            ItemId victimCur = GetPlayerItem(victimId);
            if (victimCur != ITEM_NONE && victimCur != POW_BLOCK && victimCur != BOO) {
                itemMgr->players[victimId].inventory.ClearAll();
                const RacedataPlayer& victimRacePlayer = Racedata::sInstance->racesScenario.players[victimId];
                if (victimRacePlayer.playerType == PLAYER_REAL_LOCAL) {
                    playBooSoundIfAllowed();
                    ActivateBooVictimScreen(victimId);
                }
            }
        }
    }

    booStealing[playerId] = true;
    booStealTimers[playerId] = BOO_STEAL_DELAY_FRAMES;

    booActive[playerId] = true;
    booTimers[playerId] = BOO_DURATION_FRAMES;

    if (DriverMgr::isOnlineRace && !itemPlayer.isRemote) {
        Item::Obj::AddUseEVENTEntry(OBJ_THUNDER_CLOUD, playerId);
    }
    ResetBooSpawnTimer();
}

extern "C" void RedShellResetTargeting(Item::ObjKouraRed*);
static void UpdateBooStates() {
    if (!IsBooModeActive()) return;
    
    Kart::Manager* kartMgr = Kart::Manager::sInstance;
    Item::Manager* itemMgr = Item::Manager::sInstance;
    if (kartMgr == nullptr || itemMgr == nullptr) return;
    
    booFramesSinceLoad++;
    if (booFramesSinceLoad < 60) return;
    
    u8 playerCount = GetPlayerCount();
    if (playerCount > 12) playerCount = 12;
    
    for (u8 i = 0; i < playerCount; ++i) {
        Kart::Player* kartPlayer = kartMgr->GetKartPlayer(i);
        if (kartPlayer == nullptr) continue;

        if (booStealing[i] && booStealTimers[i] > 0) {
            booStealTimers[i]--;
            
            if (booStealTimers[i] == 0) {
                CompleteItemSteal(i);
            }
        }

        if (kartPlayer->pointers.kartStatus != nullptr) {
            u32 bf1 = kartPlayer->pointers.kartStatus->bitfield1;
            u32 bf2 = kartPlayer->pointers.kartStatus->bitfield2;
            if ((bf1 & 0x2) || (bf2 & 0x2000) || (bf2 & 0x4000)) {
                booActive[i] = false;
                booTimers[i] = 0;
                booStealing[i] = false;
                booStolenVictim[i] = 0xFF;
            }
        }

        bool hasBoo = booActive[i] && (booTimers[i] > 0);
        if (hasBoo) {
            booTimers[i]--;
            if (kartPlayer->pointers.kartMovement != nullptr) {
                kartPlayer->pointers.kartMovement->offroadInvincibilityFrames = 60;
                kartPlayer->pointers.kartMovement->kclSpeedFactor = 1.0f;
            }

            Item::ObjHolder& redShellHolder = itemMgr->itemObjHolders[OBJ_RED_SHELL];
            for (u32 j = 0; j < redShellHolder.capacity; ++j) {
                Item::Obj* obj = redShellHolder.itemObj[j];
                if (obj == nullptr || (obj->bitfield74 & 0x1)) continue;
                Item::ObjKouraRed* redShell = static_cast<Item::ObjKouraRed*>(obj);
                if (redShell->GetTargetedPlayerId() == i) {
                    RedShellResetTargeting(redShell);
                    *(u32*)((u32)redShell + 0x2b8) = 6;
                    *(u8*)((u32)redShell + 0x268) = 0xFF;
                    *(u8*)((u32)redShell + 0x269) = 1;
                }
            }
        } else if (wasInBoo[i]) {
            booActive[i] = false;
        }

        if (hasBoo != wasInBoo[i]) {
            wasInBoo[i] = hasBoo;
            ApplyGhostTransparency(kartPlayer->pointers, hasBoo);
        }
    }
}
RaceFrameHook BooUpdate(UpdateBooStates);

static void ConditionalUpdateOffroad(Kart::Movement& movement) {
    if (IsBooModeActive() && movement.GetPlayerIdx() < 12 && booActive[movement.GetPlayerIdx()]) {
        movement.kclSpeedFactor = 1.0f;
        movement.kclRotFactor = movement.GetStats().handlingFactors[0];
        return;
    }
    movement.UpdateOffroad();
}
kmCall(0x80578dc0, ConditionalUpdateOffroad);
kmCall(0x805792ec, ConditionalUpdateOffroad);
kmCall(0x8057a13c, ConditionalUpdateOffroad);

static void BooCheckPlayerCollision(Kart::Collision& collision, const Kart::Player& other) {
    Kart::Manager* mgr = Kart::Manager::sInstance;
    if (IsBooModeActive() && mgr != nullptr) {
        u8 otherIdx = other.GetPlayerIdx();
        if (otherIdx < 12 && booActive[otherIdx]) return;

        u8 count = GetPlayerCount();
        if (count > 12) count = 12;
        for (u8 i = 0; i < count; ++i) {
            Kart::Player* kp = mgr->GetKartPlayer(i);
            if (kp != nullptr && kp->pointers.kartCollision == &collision && booActive[i]) return;
        }
    }
    collision.CheckKartCollision(other);
}
kmCall(0x80596dc8, BooCheckPlayerCollision);

static void BooCheckObjectCollision(Kart::Collision& collision) {
    Kart::Manager* mgr = Kart::Manager::sInstance;
    if (IsBooModeActive() && mgr != nullptr) {
        u8 count = GetPlayerCount();
        if (count > 12) count = 12;
        for (u8 i = 0; i < count; ++i) {
            Kart::Player* kp = mgr->GetKartPlayer(i);
            if (kp != nullptr && kp->pointers.kartCollision == &collision && booActive[i]) {
                u32& bf2 = kp->pointers.kartStatus->bitfield2;
                u32 savedBf2 = bf2;
                bf2 |= 0x02000000;
                collision.CheckObjectCollision();
                bf2 = savedBf2;
                KartObjectCollision* objCol = kp->pointers.kartObject;
                const Mtx34& pose = kp->GetMtx();
                const Vec3& speed = kp->GetSpeed();
                objCol->Update(pose, speed);
                return;
            }
        }
    }
    collision.CheckObjectCollision();
}
kmCall(0x80596dfc, BooCheckObjectCollision);

static void BooCheckItemCollision(Kart::Collision& collision) {
    Kart::Manager* kartMgr = Kart::Manager::sInstance;
    if (IsBooModeActive() && kartMgr != nullptr) {
        u8 count = GetPlayerCount();
        if (count > 12) count = 12;
        for (u8 i = 0; i < count; ++i) {
            Kart::Player* kp = kartMgr->GetKartPlayer(i);
            if (kp != nullptr && kp->pointers.kartCollision == &collision && booActive[i]) {
                return;
            }
        }
    }
    collision.CheckItemCollision();
}
kmCall(0x80596e08, BooCheckItemCollision);

static bool BooCheckKartCollision(Item::Obj& obj, Kart::Player* kartPlayer, u32 r5) {
    if (IsBooModeActive() && kartPlayer != nullptr) {
        u8 playerId = kartPlayer->GetPlayerIdx();
        if (playerId < 12 && booActive[playerId]) {
            return false;
        }
    }
    return obj.CheckKartCollision(kartPlayer, r5);
}
kmCall(0x80799cf8, BooCheckKartCollision);
kmCall(0x80795464, BooCheckKartCollision);

extern "C" double RedShellEvalTarget(Item::ObjKouraRed*, u32, float*, int);
static double BooRedShellEvalTarget(Item::ObjKouraRed* shell, u32 targetPlayerId, float* param3, int param4) {
    if (IsBooModeActive() && targetPlayerId < 12 && booActive[targetPlayerId]) {
        return 0.0;
    }
    return RedShellEvalTarget(shell, targetPlayerId, param3, param4);
}
kmCall(0x807b3c48, BooRedShellEvalTarget);

static void ApplyLightningEffect(Kart::Movement& movement) {
    u8 playerIdx = movement.GetPlayerIdx();
    // Boo immunity
    if (IsBooModeActive() && playerIdx < 12 && booActive[playerIdx]) return;
    movement.ApplyLightning();
}
kmCall(0x80798790, ApplyLightningEffect);

static void ApplyPowEffect(Kart::Movement& movement) {
    u8 playerIdx = movement.GetPlayerIdx();
    if (IsBooModeActive() && playerIdx < 12 && booActive[playerIdx]) return;
    movement.calcPow();
}
kmCall(0x805795d8, ApplyPowEffect);

void ResetBooStates() {
    booFramesSinceLoad = 0;
    booLastSoundFrame = 0xFFFFFFFF;
    for (int i = 0; i < 12; i++) {
        booActive[i] = false;
        booTimers[i] = 0;
        wasInBoo[i] = false;
        booStealing[i] = false;
        booStealTimers[i] = 0;
        booStolenVictim[i] = 0xFF;
        booStolenItem[i] = ITEM_NONE;
        booUsed[i] = false;
    }
    Item::Behavior& booBehavior = expandedBehaviourTable[BOO];
    booBehavior.unknkown_0x0 = 1;
    booBehavior.unknkown_0x1 = 0;
    booBehavior.objId = OBJ_BOO;
    booBehavior.numberOfItems = 1;
    booBehavior.unknown_0xc = 0;
    booBehavior.unknown_0x10 = 0;
    booBehavior.useType = Item::ITEMUSE_USE;
    booBehavior.useFunction = UseBoo;
    ResetBooVictimScreens();
    booInitialized = true;
}
RaceLoadHook BooReset(ResetBooStates);

asmFunc EnableBooOpacity() {
    ASM(
    nofralloc;
    li r5, 0;
    ori r4, r4, 0x1;
    blr;
    )
}
kmCall(0x805b15dc, EnableBooOpacity);

} // namespace Race
} // namespace Pulsar
