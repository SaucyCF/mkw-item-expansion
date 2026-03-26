#include <kamek.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <Extensions/ItemExpansion/ItemProbabilities.hpp>
#include <Race/ItemObjDrop.hpp>

static const u32 ORIGINAL_ITEM_COUNT = 19;
static const u32 EXPANDED_ITEM_COUNT = 23;

extern "C" {
    Item::Behavior expandedBehaviourTable[23];
    u16 expandedAccumulator[23];
}

extern "C" void* __nwa__FUl(u32 size);

static void* AllocZeroed(u32 size) {
    void* ptr = __nwa__FUl(size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}
kmCall(0x807baa28, AllocZeroed);

extern "C" {
    u8 compiledItemSlotBin[2200];
    u32 compiledItemSlotLen = 0;
    u8 compiledUnknownItemsBin[2200];
    u32 compiledUnknownItemsLen = 0;
}

extern "C" void BuildItemSlotBinary() {
    u32 offset = 0;
    compiledItemSlotBin[offset++] = (u8)ItemProbs::TABLE_COUNT;
    for (u32 t = 0; t < ItemProbs::TABLE_COUNT; t++) {
        const ItemProbs::TableDef& def = ItemProbs::ALL_TABLES[t];
        u32 cols = def.cols;
        compiledItemSlotBin[offset++] = (u8)cols;
        compiledItemSlotBin[offset++] = (u8)ItemProbs::ITEM_COUNT;
        u32 dataSize = ItemProbs::ITEM_COUNT * cols;
        memcpy(&compiledItemSlotBin[offset], def.data, dataSize);
        offset += dataSize;
    }
    compiledItemSlotLen = offset;
}

// "Unknown Items" mode: GP and VS tables get flat 30 for all active items,
// remaining tables (SPECIAL, battle) stay identical to normal.
// Table indices 0-4 are GP_PLAYER, GP_ENEMY, VS_PLAYER, VS_ENEMY, VS_ONLINE.
static const u32 UNKNOWN_ITEMS_PROB = 30;

extern "C" void BuildUnknownItemsBinary() {
    u32 offset = 0;
    compiledUnknownItemsBin[offset++] = (u8)ItemProbs::TABLE_COUNT;
    for (u32 t = 0; t < ItemProbs::TABLE_COUNT; t++) {
        const ItemProbs::TableDef& def = ItemProbs::ALL_TABLES[t];
        u32 cols = def.cols;
        compiledUnknownItemsBin[offset++] = (u8)cols;
        compiledUnknownItemsBin[offset++] = (u8)ItemProbs::ITEM_COUNT;
        u32 dataSize = ItemProbs::ITEM_COUNT * cols;
        if (t <= 4) {
            // GP and VS tables: flat 30 for all active items, 0 for gap/none
            for (u32 item = 0; item < ItemProbs::ITEM_COUNT; item++) {
                for (u32 col = 0; col < cols; col++) {
                    bool isActive = (item <= 18 || item == 21 || item == 22);
                    compiledUnknownItemsBin[offset++] = isActive ? (u8)UNKNOWN_ITEMS_PROB : 0;
                }
            }
        } else {
            // SPECIAL, battle tables: keep normal probabilities
            memcpy(&compiledUnknownItemsBin[offset], def.data, dataSize);
            offset += dataSize;
        }
    }
    compiledUnknownItemsLen = offset;
}

asmFunc DecideItemTableHook() {
    ASM(
    nofralloc;
    lis r26, expandedBehaviourTable @ha;
    addi r26, r26, expandedBehaviourTable @l;
    lis r16, expandedAccumulator @ha;
    addi r16, r16, expandedAccumulator @l;
    li r21, 0;
    li r17, 0;
    blr;
    )
}
kmBranch(0x807bb6b4, DecideItemTableHook);
kmPatchExitPoint(DecideItemTableHook, 0x807bb6cc);

asmFunc AccumulatorReadHook() {
    ASM(
    nofralloc;
    lis r4, expandedAccumulator @ha;
    rlwinm r3, r3, 0, 16, 31;
    addi r4, r4, expandedAccumulator @l;
    blr;
    )
}
kmBranch(0x807bb868, AccumulatorReadHook);
kmPatchExitPoint(AccumulatorReadHook, 0x807bb874);

asmFunc UseItemTableHook() {
    ASM(
    nofralloc;
    lis r3, expandedBehaviourTable @ha;
    addi r3, r3, expandedBehaviourTable @l;
    blr;
    )
}
kmBranch(0x80791960, UseItemTableHook);
kmPatchExitPoint(UseItemTableHook, 0x80791968);

asmFunc DefineCapacitiesTableHook() {
    ASM(
    nofralloc;
    lis r5, expandedBehaviourTable @ha;
    addi r5, r5, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x80790fcc, 0x60000000);
kmBranch(0x80790fdc, DefineCapacitiesTableHook);
kmPatchExitPoint(DefineCapacitiesTableHook, 0x80790fe0);

asmFunc ProcessTableTableHook() {
    ASM(
    nofralloc;
    lis r23, expandedBehaviourTable @ha;
    addi r23, r23, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x807baa3c, 0x60000000);
kmBranch(0x807baa44, ProcessTableTableHook);
kmPatchExitPoint(ProcessTableTableHook, 0x807baa48);

asmFunc CanSpawnTableHook() {
    ASM(
    nofralloc;
    lis r4, expandedBehaviourTable @ha;
    addi r4, r4, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x80799b34, 0x60000000);
kmBranch(0x80799b40, CanSpawnTableHook);
kmPatchExitPoint(CanSpawnTableHook, 0x80799b44);

asmFunc DropItemTableHook1() {
    ASM(
    nofralloc;
    lis r4, expandedBehaviourTable @ha;
    addi r4, r4, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x807bc3f8, 0x60000000);
kmBranch(0x807bc400, DropItemTableHook1);
kmPatchExitPoint(DropItemTableHook1, 0x807bc404);

asmFunc DropItemTableHook2() {
    ASM(
    nofralloc;
    lis r8, expandedBehaviourTable @ha;
    addi r8, r8, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x807bc454, 0x60000000);
kmBranch(0x807bc468, DropItemTableHook2);
kmPatchExitPoint(DropItemTableHook2, 0x807bc46c);

asmFunc InventoryInitTableHook() {
    ASM(
    nofralloc;
    lis r7, expandedBehaviourTable @ha;
    addi r7, r7, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x807bc90c, 0x60000000);
kmBranch(0x807bc918, InventoryInitTableHook);
kmPatchExitPoint(InventoryInitTableHook, 0x807bc91c);

asmFunc SetItemTableHook() {
    ASM(
    nofralloc;
    lis r6, expandedBehaviourTable @ha;
    addi r6, r6, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x807bc944, 0x60000000);
kmBranch(0x807bc950, SetItemTableHook);
kmPatchExitPoint(SetItemTableHook, 0x807bc954);

asmFunc UpdateItemArraysTableHook() {
    ASM(
    nofralloc;
    lis r22, expandedBehaviourTable @ha;
    addi r22, r22, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x8065f778, 0x60000000);
kmBranch(0x8065f788, UpdateItemArraysTableHook);
kmPatchExitPoint(UpdateItemArraysTableHook, 0x8065f78c);

asmFunc ScaleTableCopyHook() {
    ASM(
    nofralloc;
    lhz r0, 0x26(r4);
    sth r0, 0x26(r7);
    lhz r0, 0x28(r4);
    sth r0, 0x28(r7);
    lhz r0, 0x2A(r4);
    sth r0, 0x2A(r7);
    lhz r0, 0x2C(r4);
    sth r0, 0x2C(r7);
    addi r7, r7, 0x2E;
    blr;
    )
}
kmBranch(0x807badfc, ScaleTableCopyHook);
kmPatchExitPoint(ScaleTableCopyHook, 0x807bae00);

asmFunc ScaleTableClearHook() {
    ASM(
    nofralloc;
    sth r3, 0x26(r4);
    sth r3, 0x28(r4);
    sth r3, 0x2A(r4);
    sth r3, 0x2C(r4);
    addi r4, r4, 0x2E;
    blr;
    )
}
kmBranch(0x807bae74, ScaleTableClearHook);
kmPatchExitPoint(ScaleTableClearHook, 0x807bae78);

asmFunc SpawnLimitTableHook() {
    ASM(
    nofralloc;
    lis r4, expandedBehaviourTable @ha;
    addi r4, r4, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x80799c08, 0x60000000);
kmBranch(0x80799c10, SpawnLimitTableHook);
kmPatchExitPoint(SpawnLimitTableHook, 0x80799c14);

asmFunc BreakableObjectTableHook() {
    ASM(
    nofralloc;
    lis r4, expandedBehaviourTable @ha;
    addi r4, r4, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x8076ef58, 0x60000000);
kmBranch(0x8076ef60, BreakableObjectTableHook);
kmPatchExitPoint(BreakableObjectTableHook, 0x8076ef64);

asmFunc GetItemCountTableHook1() {
    ASM(
    nofralloc;
    lis r5, expandedBehaviourTable @ha;
    addi r5, r5, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x80798f20, 0x60000000);
kmBranch(0x80798f24, GetItemCountTableHook1);
kmPatchExitPoint(GetItemCountTableHook1, 0x80798f28);

asmFunc GetItemCountTableHook2() {
    ASM(
    nofralloc;
    lis r3, expandedBehaviourTable @ha;
    addi r3, r3, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x80798f58, 0x60000000);
kmBranch(0x80798f5c, GetItemCountTableHook2);
kmPatchExitPoint(GetItemCountTableHook2, 0x80798f60);

asmFunc AIItemTableHook() {
    ASM(
    nofralloc;
    lis r3, expandedBehaviourTable @ha;
    addi r3, r3, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x80733c40, 0x60000000);
kmBranch(0x80733c44, AIItemTableHook);
kmPatchExitPoint(AIItemTableHook, 0x80733c48);

asmFunc UseTypeQueryTableHook() {
    ASM(
    nofralloc;
    lis r3, expandedBehaviourTable @ha;
    addi r3, r3, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x80798270, 0x60000000);
kmBranch(0x80798274, UseTypeQueryTableHook);
kmPatchExitPoint(UseTypeQueryTableHook, 0x80798278);

asmFunc InventoryTickTableHook1() {
    ASM(
    nofralloc;
    lis r3, expandedBehaviourTable @ha;
    addi r3, r3, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x807bc7c0, 0x60000000);
kmBranch(0x807bc7c4, InventoryTickTableHook1);
kmPatchExitPoint(InventoryTickTableHook1, 0x807bc7c8);

asmFunc InventoryTickTableHook2() {
    ASM(
    nofralloc;
    lis r3, expandedBehaviourTable @ha;
    addi r3, r3, expandedBehaviourTable @l;
    blr;
    )
}
kmWrite32(0x807bc898, 0x60000000);
kmBranch(0x807bc89c, InventoryTickTableHook2);
kmPatchExitPoint(InventoryTickTableHook2, 0x807bc8a0);

// --- Item::ItemSlotData::Init (0x807baf48) ---
kmWrite32(0x807baf5c, 0x38600060);
kmWrite32(0x807baf80, 0x38600490);   
kmWrite32(0x807baf94, 0x38C00060);
kmWrite32(0x807bb284, 0x1C7D0060);
kmWrite32(0x807bb2a0, 0x38C00060);
kmWrite32(0x807bb338, 0x2C0A0017);  
kmWrite32(0x807bb340, 0x38C6002E);
kmWrite32(0x807bb344, 0x38E70060);

// --- ProcessTable (0x807ba9d8) ---
kmWrite32(0x807baa20, 0x1C7F002E);
kmWrite32(0x807bacc0, 0x38C6002E);
kmWrite32(0x807bacc4, 0x38E70060);

// --- DecideItem (0x807bb42c) ---
kmWrite32(0x807bb620, 0x1C64002E);
kmWrite32(0x807bb6a8, 0x3B600017);
kmWrite32(0x807bb6b0, 0x3B600017);

// --- calcRandomItem / DecideRouletteItem (0x807bb8d0) ---
kmWrite32(0x807bb8f8, 0x1C000060);
kmWrite32(0x807bb954, 0x1C000060);

// --- PostProcessVSTable / scaleTable (0x807bad20) ---
kmWrite32(0x807bad5c, 0x1C00002E);
kmWrite32(0x807bae0c, 0x1C89002E);

static void InitExpandedItemBehaviours() {
    memcpy(expandedBehaviourTable, Item::Behavior::behaviourTable, 
           ORIGINAL_ITEM_COUNT * sizeof(Item::Behavior));
    memset(&expandedBehaviourTable[UNKNOWN_0x13], 0, sizeof(Item::Behavior));
    memset(&expandedBehaviourTable[ITEM_NONE], 0, sizeof(Item::Behavior));
    memset(expandedAccumulator, 0, sizeof(expandedAccumulator));
}
RaceLoadHook InitExpanded(InitExpandedItemBehaviours);

static void ExpandedLoseItemFromDmg(Item::PlayerInventory& inventory) {
    ItemId itemId = inventory.currentItemId;
    if (itemId == BOO || itemId == FEATHER) {
        Item::Player* player = inventory.itemPlayer;
        const Vec3& pos = player->GetPosition();
        u8 playerId = player->id;
        Pulsar::Race::EjectDroppedItem(itemId, pos, playerId);
        inventory.currentItemId = ITEM_NONE;
        inventory.currentItemCount = 0;
        return;
    }
    inventory.LoseItemFromDmg();
}
kmCall(0x80798aac, ExpandedLoseItemFromDmg);

static void ConditionalEjectItems(Item::PlayerInventory& inventory) {
    ItemId itemId = inventory.currentItemId;
    if (itemId == BOO || itemId == FEATHER) {
        Item::Player* player = inventory.itemPlayer;
        const Vec3& pos = player->GetPosition();
        u8 playerId = player->id;
        Pulsar::Race::EjectDroppedItem(itemId, pos, playerId);
        inventory.currentItemId = ITEM_NONE;
        inventory.currentItemCount = 0;
    } else {
        inventory.EjectItems();
    }
}
kmCall(0x807bc6c4, ConditionalEjectItems);

extern "C" bool ExpandedCapacityCheck(ItemId id) {
    if (id == BOO) {
        if (Pulsar::Race::GetBooSpawnTimer() > 0) return false;
        u32 inPlay = Pulsar::Race::CountPlayersHoldingItem(BOO)
                   + Pulsar::Race::GetActiveDropCount(BOO);
        return inPlay < Pulsar::Race::BOO_MAX_COUNT;
    }
    if (id == FEATHER) {
        if (Pulsar::Race::GetFeatherSpawnTimer() > 0) return false;
        u32 inPlay = Pulsar::Race::CountPlayersHoldingItem(FEATHER)
                   + Pulsar::Race::GetActiveDropCount(FEATHER);
        return inPlay < Pulsar::Race::FEATHER_MAX_COUNT;
    }
    return Item::Manager::IsThereCapacityForItem(id);
}
kmCall(0x8072fda4, ExpandedCapacityCheck);
kmCall(0x807ba17c, ExpandedCapacityCheck);

extern "C" asmFunc DecideItemSkipTarget() {
    ASM(
    nofralloc;
    sthx r22, r16, r17;
    blr;
    )
}
kmBranch(0x807bb840, DecideItemSkipTarget);
kmPatchExitPoint(DecideItemSkipTarget, 0x807bb844);

extern "C" char sInstance__Q24Item7Manager;
extern "C" char GetTotalItemCount__Q24Item9ObjHolderCFv;
asmFunc DecideItemObjCapacityHook() {
    ASM(
    nofralloc;
    lbz r0, 0x0(r26);
    cmpwi r0, 0x0;
    beq skip_item;
    cmpwi r25, 0x10;
    bge check_expanded;
    lis r3, sInstance__Q24Item7Manager @ha;
    lwz r3, sInstance__Q24Item7Manager @l(r3);
    mulli r0, r25, 0x24;
    add r3, r3, r0;
    addi r19, r3, 0x48;
    or r3, r19, r19;
    lis r12, GetTotalItemCount__Q24Item9ObjHolderCFv @ha;
    addi r12, r12, GetTotalItemCount__Q24Item9ObjHolderCFv @l;
    mtspr CTR, r12;
    bctrl;
    lwz r0, 0x1c(r19);
    subf. r0, r3, r0;
    ble skip_item;
    b pass_item;
    check_expanded:
    mr r3, r21;
    lis r12, ExpandedCapacityCheck @ha;
    addi r12, r12, ExpandedCapacityCheck @l;
    mtspr CTR, r12;
    bctrl;
    cmpwi r3, 0;
    beq skip_item;
    b pass_item;
    skip_item:
    lis r3, DecideItemSkipTarget @ha;
    addi r3, r3, DecideItemSkipTarget @l;
    mtspr CTR, r3;
    bctr;
    pass_item:
    blr;
    )
}
kmBranch(0x807bb7b0, DecideItemObjCapacityHook);
kmPatchExitPoint(DecideItemObjCapacityHook, 0x807bb7e0);