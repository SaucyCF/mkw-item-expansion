#include <kamek.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <PulsarSystem.hpp>

// Expanded behaviourTable in mod BSS (from ItemSlotExpansion.cpp)
extern "C" Item::Behavior expandedBehaviourTable[27];

namespace Pulsar {
namespace Race {

static bool shroomStarActive[12] = {};

void UseShroomerStar(Item::Player& itemPlayer) {
    Kart::Movement* movement = itemPlayer.pointers->kartMovement;
    if (movement == nullptr) return;
    movement->ActivateMushroom();
    movement->ActivateStar();
    s16 mushroomDuration = movement->boost.mushroomBoostPanelFrames;
    if (mushroomDuration > 0) {
        movement->starTimer = mushroomDuration;
    }

    u8 playerId = itemPlayer.id;
    if (playerId < 12) {
        shroomStarActive[playerId] = true;
    }

    itemPlayer.inventory.RemoveItems(1);
}

static void UpdateShroomStarStates() {
    Kart::Manager* kartMgr = Kart::Manager::sInstance;
    if (kartMgr == nullptr) return;

    for (u32 i = 0; i < 12; i++) {
        if (!shroomStarActive[i]) continue;

        Kart::Player* kartPlayer = kartMgr->players[i];
        if (kartPlayer == nullptr) continue;

        Kart::Movement* movement = kartPlayer->pointers.kartMovement;
        if (movement == nullptr) continue;

        if (movement->boost.mushroomBoostPanelFrames <= 0) {
            movement->starTimer = 0;
            shroomStarActive[i] = false;
        }
    }
}

static void ResetShroomStarStates() {
    for (u32 i = 0; i < 12; i++) {
        shroomStarActive[i] = false;
    }
}

void FusionMushroomBoost(Item::Player& itemPlayer) {
    Kart::Movement* movement = itemPlayer.pointers->kartMovement;
    if (movement != nullptr) {
        movement->ActivateMushroom();
    }
}

static void RegisterFusionItemBehaviours() {
    ResetShroomStarStates();

    Item::Behavior& shroomStar = expandedBehaviourTable[SHROOM_STAR];
    shroomStar.unknkown_0x0 = 1;
    shroomStar.unknkown_0x1 = 0;
    shroomStar.objId = OBJ_STAR;
    shroomStar.numberOfItems = 1;
    shroomStar.unknown_0xc = 0;
    shroomStar.unknown_0x10 = 0;
    shroomStar.useType = Item::ITEMUSE_USE;
    shroomStar.useFunction = UseShroomerStar;

    Item::Behavior& greenShellMush = expandedBehaviourTable[GREEN_SHELL_MUSHROOM];
    greenShellMush.unknkown_0x0 = 1;
    greenShellMush.unknkown_0x1 = 1;
    greenShellMush.objId = OBJ_GREEN_SHELL;
    greenShellMush.numberOfItems = 1;
    greenShellMush.unknown_0xc = 0;
    greenShellMush.unknown_0x10 = 0;
    greenShellMush.useType = Item::ITEMUSE_FIRE;
    greenShellMush.useFunction = FusionMushroomBoost;

    Item::Behavior& bobombMush = expandedBehaviourTable[BOBOMB_MUSHROOM];
    bobombMush.unknkown_0x0 = 1;
    bobombMush.unknkown_0x1 = 1;
    bobombMush.objId = OBJ_BOBOMB;
    bobombMush.numberOfItems = 1;
    bobombMush.unknown_0xc = 0;
    bobombMush.unknown_0x10 = 0;
    bobombMush.useType = Item::ITEMUSE_FIRE;
    bobombMush.useFunction = FusionMushroomBoost;
}
RaceLoadHook RegisterFusionItems(RegisterFusionItemBehaviours);

static void UpdateFusionItems() {
    UpdateShroomStarStates();
}
RaceFrameHook FusionItemsUpdate(UpdateFusionItems);

} // namespace Race
} // namespace Pulsar
