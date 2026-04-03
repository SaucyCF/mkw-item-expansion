/*
 * ItemProbabilities.hpp
 *
 * ALL item probability tables defined in C++ code.
 * Edit these arrays to customize item distribution — no .bin file required.
 *
 * FORMAT:
 *   Each entry is a raw probability weight (u8, 0-200 scale).
 *   Column sums should total approximately 200 (= 100% probability).
 *   A value of 200 in a column makes that item guaranteed for that position.
 *
 * RACE TABLES  (12 columns): col 0 = 1st place, col 1 = 2nd, ... col 11 = 12th
 * BATTLE TABLES (3 columns): col 0 = tied/lead, col 1 = behind tier 1, col 2 = behind tier 2
 * SPECIAL TABLE (16 columns): each column = a specific Special Box variant
 *
 * ITEM INDICES:
 *    0 = Green Shell         7 = Blue Shell        14 = Thundercloud
 *    1 = Red Shell           8 = Lightning         15 = Bullet Bill
 *    2 = Banana              9 = Star              16 = Triple Green Shells
 *    3 = Fake Item Box      10 = Golden Mushroom   17 = Triple Red Shells
 *    4 = Mushroom           11 = Mega Mushroom     18 = Triple Bananas
 *    5 = Triple Mushroom    12 = Blooper           19 = UNKNOWN_0x13 — keep 0
 *    6 = Bob-omb            13 = POW Block         20 = ITEM_NONE — keep 0
 *                                                  21 = Boo
 *                                                  22 = Feather
 *                                                  23 = Triple FIB
 *
 * TABLE ORDER IN BINARY (must match vanilla for Init to parse correctly):
 *    0: GRAND_PRIX_PLAYER   4: VERSUS_ONLINE     8: COIN_PLAYER
 *    1: GRAND_PRIX_ENEMY    5: SPECIAL           9: COIN_ENEMY
 *    2: VERSUS_PLAYER       6: BALLOON_PLAYER   10: BALLOON_ONLINE
 *    3: VERSUS_ENEMY        7: BALLOON_ENEMY    11: COIN_ONLINE
 */

#pragma once
#include <types.hpp>

namespace ItemProbs {

static const u32 ITEM_COUNT = 24;
static const u32 TABLE_COUNT = 12;

//     1st  2nd  3rd  4th  5th  6th  7th  8th  9th 10th 11th 12th
static const u8 GRAND_PRIX_PLAYER[ITEM_COUNT][12] = {
    {   50,  35,  20,  10,   5,   0,   0,   0,   0,   0,   0,   0 }, //  0: Green Shell
    {   15,  50,  40,  30,  20,  15,  10,   5,   0,   0,   0,   0 }, //  1: Red Shell
    {   75,  35,  20,  10,   5,   0,   0,   0,   0,   0,   0,   0 }, //  2: Banana
    {   35,  20,  10,   5,   5,   0,   0,   0,   0,   0,   0,   0 }, //  3: Fake Item Box
    {   0,  20,  35,  40,  30,  25,  15,   5,   0,   0,   0,   0 }, //  4: Mushroom
    {   0,   0,  10,  20,  35,  50,  55,  65,  70,  50,  40,  25 }, //  5: Triple Mushroom
    {   0,   5,  15,  15,  10,  10,   0,   0,   0,   0,   0,   0 }, //  6: Bob-omb
    {   0,   0,   0,   5,  10,  15,  10,   5,   0,   0,   0,   0 }, //  7: Blue Shell
    {   0,   0,   0,   0,   0,   0,   0,   0,   5,  15,  25,  45 }, //  8: Lightning
    {   0,   0,   0,   0,   0,   0,  15,  25,  40,  45,  40,  30 }, //  9: Star
    {   0,   0,   0,   0,   0,  10,  30,  45,  55,  60,  55,  50 }, // 10: Golden Mushroom
    {   0,   0,   0,   5,  10,  20,  15,  10,   0,   0,   0,   0 }, // 11: Mega Mushroom
    {   0,   0,   0,   5,  10,  15,  15,  10,   5,   0,   0,   0 }, // 12: Blooper
    {   0,   0,   0,   0,  10,  10,  15,  10,   5,   0,   0,   0 }, // 13: POW Block
    {   0,   0,  10,  15,  15,  10,   5,   0,   0,   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0,   0,   0,   0,   0,  10,  20,  30,  40,  50 }, // 15: Bullet Bill
    {   0,  10,  25,  20,  10,   0,   0,   0,   0,   0,   0,   0 }, // 16: Triple Green Shells
    {   0,   0,   0,  15,  25,  20,  15,  10,   0,   0,   0,   0 }, // 17: Triple Red Shells
    {   25,  25,  15,   5,   0,   0,   0,   0,   0,   0,   0,   0 }, // 18: Triple Bananas
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 21: Boo
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 22: Feather
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 23: Triple FIB
};

//     1st  2nd  3rd  4th  5th  6th  7th  8th  9th 10th 11th 12th
static const u8 GRAND_PRIX_ENEMY[ITEM_COUNT][12] = {
    {  50,  40,  40,  35,  30,  30,  35,  30,  30,  20,  15,   5 }, //  0: Green Shell
    {  10,  65,  40,  35,  25,  20,  15,  10,   0,   0,   0,   0 }, //  1: Red Shell
    {  70,  40,  35,  35,  25,  20,  20,  20,  15,  10,   5,   0 }, //  2: Banana
    {  35,  20,  15,  15,  10,  10,   5,   0,   0,   0,   0,   0 }, //  3: Fake Item Box
    {  10,  15,  25,  30,  30,  35,  40,  45,  35,  35,  35,  10 }, //  4: Mushroom
    {   0,   0,  10,  15,  20,  25,  30,  35,  45,  65,  55,  25 }, //  5: Triple Mushroom
    {   0,   0,   5,  10,  10,  10,   5,   5,   0,   0,   0,   0 }, //  6: Bob-omb
    {   0,   0,   0,   0,   5,  10,  10,  10,  10,   0,   0,   0 }, //  7: Blue Shell
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  35 }, //  8: Lightning
    {   0,   0,   0,   0,   0,   0,   5,  15,  25,  30,  35,  40 }, //  9: Star
    {   0,   0,   0,   0,   0,   0,   0,   0,  15,  30,  35,  40 }, // 10: Golden Mushroom
    {   0,   0,   0,   0,   5,  10,  15,  10,  10,   0,   0,   0 }, // 11: Mega Mushroom
    {   0,   0,   0,   0,  10,  10,  10,  10,  10,   0,   0,   0 }, // 12: Blooper
    {   0,   0,   0,   0,  10,  10,  10,  10,   5,   0,   0,   0 }, // 13: POW Block
    {   0,   0,   5,  10,  10,   5,   0,   0,   0,   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,  10,  20,  45 }, // 15: Bullet Bill
    {   0,   0,  15,  15,  10,   5,   0,   0,   0,   0,   0,   0 }, // 16: Triple Green Shells
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 17: Triple Red Shells
    {  25,  20,  10,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 18: Triple Bananas
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 21: Boo
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 22: Feather
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 23: Triple FIB
};

//     1st  2nd  3rd  4th  5th  6th  7th  8th  9th 10th 11th 12th
static const u8 VERSUS_PLAYER[ITEM_COUNT][12] = {
    {   65,  35,  25,  15,   5,   0,   0,   0,   0,   0,   0,   0 }, //  0: Green Shell
    {   0,  50,  45,  40,  25,  15,  10,   0,   0,   0,   0,   0 }, //  1: Red Shell
    {   75,  35,  20,   5,   5,   0,   0,   0,   0,   0,   0,   0 }, //  2: Banana
    {   35,  20,  10,   5,   0,   0,   0,   0,   0,   0,   0,   0 }, //  3: Fake Item Box
    {   0,  20,  35,  45,  35,  30,  20,  15,   5,   0,   0,   0 }, //  4: Mushroom
    {   0,   0,   5,  15,  30,  45,  55,  65,  75,  65,  45,  15 }, //  5: Triple Mushroom
    {   0,   5,  10,  15,  10,  10,   0,   0,   0,   0,   0,   0 }, //  6: Bob-omb
    {   0,   0,   0,   5,  10,  15,  15,  10,   5,   0,   0,   0 }, //  7: Blue Shell
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  45 }, //  8: Lightning
    {   0,   0,   0,   0,   0,   0,  10,  25,  35,  50,  45,  35 }, //  9: Star
    {   0,   0,   0,   0,   0,   5,  20,  40,  50,  70,  70,  40 }, // 10: Golden Mushroom
    {   0,   0,   0,   5,  15,  20,  20,  10,  10,   0,   0,   0 }, // 11: Mega Mushroom
    {   0,   0,   0,   0,  10,  10,  10,  10,  10,   0,   0,   0 }, // 12: Blooper
    {   0,   0,   0,   0,  10,  10,  15,  10,  10,   0,   0,   0 }, // 13: POW Block
    {   0,   0,  10,  15,  15,  15,  10,   5,   0,   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,  15,  40,  65 }, // 15: Bullet Bill
    {   0,  10,  20,  20,  10,   5,   0,   0,   0,   0,   0,   0 }, // 16: Triple Green Shells
    {   0,   0,   5,  10,  20,  20,  15,  10,   0,   0,   0,   0 }, // 17: Triple Red Shells
    {   25,  25,  15,   5,   0,   0,   0,   0,   0,   0,   0,   0 }, // 18: Triple Bananas
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 21: Boo
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 22: Feather
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 23: Triple FIB
};

//     1st  2nd  3rd  4th  5th  6th  7th  8th  9th 10th 11th 12th
static const u8 VERSUS_ENEMY[ITEM_COUNT][12] = {
    {  50,  40,  40,  35,  30,  30,  35,  30,  30,  20,  15,   5 }, //  0: Green Shell
    {  10,  65,  35,  35,  30,  20,  15,  10,   0,   0,   0,   0 }, //  1: Red Shell
    {  70,  40,  35,  35,  30,  25,  20,  20,  15,  10,  10,   0 }, //  2: Banana
    {  35,  20,  15,  15,  10,  10,   5,   0,   0,   0,   0,   0 }, //  3: Fake Item Box
    {  10,  15,  25,  30,  35,  40,  40,  45,  35,  35,  35,  10 }, //  4: Mushroom
    {   0,   0,  10,  15,  20,  25,  30,  35,  40,  60,  65,  30 }, //  5: Triple Mushroom
    {   0,   0,   5,  10,  10,  10,   5,   5,   0,   0,   0,   0 }, //  6: Bob-omb
    {   0,   0,   0,   0,   5,  10,  10,  10,  10,   0,   0,   0 }, //  7: Blue Shell
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  25 }, //  8: Lightning
    {   0,   0,   0,   0,   0,   0,   5,  15,  25,  35,  40,  30 }, //  9: Star
    {   0,   0,   0,   0,   0,   0,   0,   0,  15,  25,  35,  50 }, // 10: Golden Mushroom
    {   0,   0,   0,   0,   5,  10,  15,  10,  10,   0,   0,   0 }, // 11: Mega Mushroom
    {   0,   0,   0,   0,  10,  10,  10,  10,  10,   5,   0,   0 }, // 12: Blooper
    {   0,   0,   0,   0,   0,   0,  10,  10,  10,  10,   0,   0 }, // 13: POW Block
    {   0,   5,  10,  10,   5,   5,   0,   0,   0,   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  50 }, // 15: Bullet Bill
    {   0,   0,  15,  15,  10,   5,   0,   0,   0,   0,   0,   0 }, // 16: Triple Green Shells
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 17: Triple Red Shells
    {  25,  15,  10,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 18: Triple Bananas
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 21: Boo
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 22: Feather
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 23: Triple FIB
};

//     1st  2nd  3rd  4th  5th  6th  7th  8th  9th 10th 11th 12th
static const u8 VERSUS_ONLINE[ITEM_COUNT][12] = {
    {  62,  35,  30,  10,   0,   0,   0,   0,   0,   0,   0,   0 }, //  0: Green Shell
    {   0,  50,  50,  40,  30,  20,   0,   0,   0,   0,   0,   0 }, //  1: Red Shell
    {  73,  40,  15,   5,   0,   0,   0,   0,   0,   0,   0,   0 }, //  2: Banana
    {  40,  15,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, //  3: Fake Item Box
    {   5,  25,  35,  48,  30,  25,  20,   0,   0,   0,   0,   0 }, //  4: Mushroom
    {   0,   0,   0,  10,  20,  30,  50,  67,  75,  55,  20,   0 }, //  5: Triple Mushroom
    {   0,   0,  10,  10,  15,  15,  10,   0,   0,   0,   0,   0 }, //  6: Bob-omb
    {   0,   0,   0,   5,  10,  15,  15,   0,   0,   0,   0,   0 }, //  7: Blue Shell
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  15,  40 }, //  8: Lightning
    {   0,   0,   0,   0,   0,   0,   0,  33,  43,  55,  55,  40 }, //  9: Star
    {   0,   0,   0,   0,   0,   5,  20,  45,  58,  70,  60,  45 }, // 10: Golden Mushroom
    {   0,   0,   0,   7,  15,  20,  20,  13,   0,   0,   0,   0 }, // 11: Mega Mushroom
    {   0,   0,   5,  10,  15,  15,  15,  12,  10,   5,   5,   5 }, // 12: Blooper
    {   0,   0,   0,   0,  10,  10,  15,  10,   0,   0,   0,   0 }, // 13: POW Block
    {   0,   0,   0,  15,  15,  15,  15,  10,   7,   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0,   0,   0,   0,   0,   0,   7,  15,  45,  70 }, // 15: Bullet Bill
    {   0,  10,  25,  20,  15,  10,   5,   0,   0,   0,   0,   0 }, // 16: Triple Green Shells
    {   0,   0,  10,  20,  25,  20,  15,  10,   0,   0,   0,   0 }, // 17: Triple Red Shells
    {  20,  25,  20,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 18: Triple Bananas
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 21: Boo
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 22: Feather
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 23: Triple FIB
};

static const u8 SPECIAL[ITEM_COUNT][16] = {
    {   0,   0,   0,   0,   0,   0, 200,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, //  0: Green Shell
    {   0,   0,   0,   0,   0,   0,   0,   0, 200,   0,   0,   0,   0,   0,   0,   0 }, //  1: Red Shell
    { 200,   0,   0,   0,   0, 100,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, //  2: Banana
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, //  3: Fake Item Box
    {   0, 200,   0,   0,   0, 100,   0,   0,   0,   0,   0,  10,   0,   0,   0,   0 }, //  4: Mushroom
    {   0,   0, 200,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, //  5: Triple Mushroom
    {   0,   0,   0,   0,   0,   0,   0, 200,   0,   0,   0,   0,   0,   0,   0,   0 }, //  6: Bob-omb
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, //  7: Blue Shell
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, //  8: Lightning
    {   0,   0,   0, 200,   0,   0,   0,   0,   0,   0,   0, 190,   0,   0,   0,   0 }, //  9: Star
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 10: Golden Mushroom
    {   0,   0,   0,   0,   0,   0,   0,   0,   0, 200,   0,   0,   0,   0,   0,   0 }, // 11: Mega Mushroom
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 12: Blooper
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 13: POW Block
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 200,   0,   0,   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 15: Bullet Bill
    {   0,   0,   0,   0, 200,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 16: Triple Green Shells
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 17: Triple Red Shells
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 18: Triple Bananas
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 21: Boo
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 22: Feather
    {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, // 23: Triple FIB
};

static const u8 BALLOON_PLAYER[ITEM_COUNT][3] = {
    {  50,  15,   0 }, //  0: Green Shell
    {  50,  60,  60 }, //  1: Red Shell
    {  25,  10,   0 }, //  2: Banana
    {  15,  10,   0 }, //  3: Fake Item Box
    {  15,  10,   0 }, //  4: Mushroom
    {   0,  10,  20 }, //  5: Triple Mushroom
    {  10,  10,  10 }, //  6: Bob-omb
    {   0,   0,  10 }, //  7: Blue Shell
    {   0,   0,  10 }, //  8: Lightning
    {   0,  10,  20 }, //  9: Star
    {   0,   0,   0 }, // 10: Golden Mushroom
    {  10,  15,  20 }, // 11: Mega Mushroom
    {   0,  10,   5 }, // 12: Blooper
    {   0,   0,   0 }, // 13: POW Block
    {   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0 }, // 15: Bullet Bill
    {  15,  15,  15 }, // 16: Triple Green Shells
    {   0,  15,  30 }, // 17: Triple Red Shells
    {  10,  10,   0 }, // 18: Triple Bananas
    {   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0 }, // 21: Boo
    {   0,   0,   0 }, // 22: Feather
    {   0,   0,   0 }, // 23: Triple FIB
};

static const u8 BALLOON_ENEMY[ITEM_COUNT][3] = {
    {  55,  35,  15 }, //  0: Green Shell
    {  40,  60,  80 }, //  1: Red Shell
    {  45,  15,   0 }, //  2: Banana
    {  25,  15,   0 }, //  3: Fake Item Box
    {   0,  10,   5 }, //  4: Mushroom
    {   0,   5,  10 }, //  5: Triple Mushroom
    {  10,  10,  10 }, //  6: Bob-omb
    {   0,   0,   5 }, //  7: Blue Shell
    {   0,   0,  10 }, //  8: Lightning
    {   0,   5,  10 }, //  9: Star
    {   0,   0,   0 }, // 10: Golden Mushroom
    {   5,  10,  15 }, // 11: Mega Mushroom
    {   5,  10,   5 }, // 12: Blooper
    {   0,   0,   0 }, // 13: POW Block
    {   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0 }, // 15: Bullet Bill
    {   5,  15,  25 }, // 16: Triple Green Shells
    {   0,   0,   0 }, // 17: Triple Red Shells
    {  10,  10,  10 }, // 18: Triple Bananas
    {   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0 }, // 21: Boo
    {   0,   0,   0 }, // 22: Feather
    {   0,   0,   0 }, // 23: Triple FIB
};

static const u8 COIN_PLAYER[ITEM_COUNT][3] = {
    {  40,  20,   0 }, //  0: Green Shell
    {  60,  60,  65 }, //  1: Red Shell
    {  25,  10,   0 }, //  2: Banana
    {  20,  10,   0 }, //  3: Fake Item Box
    {  10,   5,   0 }, //  4: Mushroom
    {   0,  10,  15 }, //  5: Triple Mushroom
    {  10,  10,  10 }, //  6: Bob-omb
    {   0,   0,   5 }, //  7: Blue Shell
    {   0,   0,   0 }, //  8: Lightning
    {   0,  10,  20 }, //  9: Star
    {   0,   0,  10 }, // 10: Golden Mushroom
    {  10,  15,  20 }, // 11: Mega Mushroom
    {   0,  10,   5 }, // 12: Blooper
    {   0,   0,   5 }, // 13: POW Block
    {   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0 }, // 15: Bullet Bill
    {  15,  15,  15 }, // 16: Triple Green Shells
    {   0,  15,  30 }, // 17: Triple Red Shells
    {  10,  10,   0 }, // 18: Triple Bananas
    {   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0 }, // 21: Boo
    {   0,   0,   0 }, // 22: Feather
    {   0,   0,   0 }, // 23: Triple FIB
};

static const u8 COIN_ENEMY[ITEM_COUNT][3] = {
    {  55,  40,  20 }, //  0: Green Shell
    {  45,  60,  80 }, //  1: Red Shell
    {  40,  15,   0 }, //  2: Banana
    {  25,  10,   0 }, //  3: Fake Item Box
    {   5,  10,   5 }, //  4: Mushroom
    {   0,   5,  10 }, //  5: Triple Mushroom
    {  10,  10,  10 }, //  6: Bob-omb
    {   0,   0,   5 }, //  7: Blue Shell
    {   0,   0,   0 }, //  8: Lightning
    {   0,   5,  10 }, //  9: Star
    {   0,   0,   0 }, // 10: Golden Mushroom
    {   5,  10,  15 }, // 11: Mega Mushroom
    {   5,  10,   5 }, // 12: Blooper
    {   0,   0,   5 }, // 13: POW Block
    {   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0 }, // 15: Bullet Bill
    {   0,  15,  25 }, // 16: Triple Green Shells
    {   0,   0,   0 }, // 17: Triple Red Shells
    {  10,  10,  10 }, // 18: Triple Bananas
    {   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0 }, // 21: Boo
    {   0,   0,   0 }, // 22: Feather
    {   0,   0,   0 }, // 23: Triple FIB
};

static const u8 BALLOON_ONLINE[ITEM_COUNT][3] = {
    {  45,  15,   0 }, //  0: Green Shell
    {  50,  60,  65 }, //  1: Red Shell
    {  25,   5,   0 }, //  2: Banana
    {  20,  10,   0 }, //  3: Fake Item Box
    {  15,  15,   0 }, //  4: Mushroom
    {   0,  10,  20 }, //  5: Triple Mushroom
    {  10,  10,  10 }, //  6: Bob-omb
    {   0,   0,   5 }, //  7: Blue Shell
    {   0,   0,  10 }, //  8: Lightning
    {   0,  10,  20 }, //  9: Star
    {   0,   0,   0 }, // 10: Golden Mushroom
    {  10,  15,  20 }, // 11: Mega Mushroom
    {   0,  10,   5 }, // 12: Blooper
    {   0,   0,   0 }, // 13: POW Block
    {   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0 }, // 15: Bullet Bill
    {  15,  15,  15 }, // 16: Triple Green Shells
    {   0,  15,  30 }, // 17: Triple Red Shells
    {  10,  10,   0 }, // 18: Triple Bananas
    {   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0 }, // 21: Boo
    {   0,   0,   0 }, // 22: Feather
    {   0,   0,   0 }, // 23: Triple FIB
};

static const u8 COIN_ONLINE[ITEM_COUNT][3] = {
    {  40,  15,   0 }, //  0: Green Shell
    {  55,  60,  70 }, //  1: Red Shell
    {  25,   5,   0 }, //  2: Banana
    {  20,  10,   0 }, //  3: Fake Item Box
    {  15,  15,   0 }, //  4: Mushroom
    {   0,  10,  15 }, //  5: Triple Mushroom
    {  10,  10,  10 }, //  6: Bob-omb
    {   0,   0,   5 }, //  7: Blue Shell
    {   0,   0,   0 }, //  8: Lightning
    {   0,  10,  20 }, //  9: Star
    {   0,   0,   5 }, // 10: Golden Mushroom
    {  10,  15,  20 }, // 11: Mega Mushroom
    {   0,  10,   5 }, // 12: Blooper
    {   0,   0,   5 }, // 13: POW Block
    {   0,   0,   0 }, // 14: Thundercloud
    {   0,   0,   0 }, // 15: Bullet Bill
    {  15,  15,  15 }, // 16: Triple Green Shells
    {   0,  15,  30 }, // 17: Triple Red Shells
    {  10,  10,   0 }, // 18: Triple Bananas
    {   0,   0,   0 }, // 19: UNKNOWN_0x13
    {   0,   0,   0 }, // 20: ITEM_NONE
    {   0,   0,   0 }, // 21: Boo
    {   0,   0,   0 }, // 22: Feather
    {   0,   0,   0 }, // 23: Triple FIB
};

struct TableDef {
    const u8* data;
    u32 cols;
};

static const TableDef ALL_TABLES[TABLE_COUNT] = {
    { &GRAND_PRIX_PLAYER[0][0], 12 },
    { &GRAND_PRIX_ENEMY[0][0],  12 },
    { &VERSUS_PLAYER[0][0],     12 },
    { &VERSUS_ENEMY[0][0],      12 },
    { &VERSUS_ONLINE[0][0],     12 },
    { &SPECIAL[0][0],           16 },
    { &BALLOON_PLAYER[0][0],     3 },
    { &BALLOON_ENEMY[0][0],      3 },
    { &COIN_PLAYER[0][0],        3 },
    { &COIN_ENEMY[0][0],         3 },
    { &BALLOON_ONLINE[0][0],     3 },
    { &COIN_ONLINE[0][0],        3 },
};

} // namespace ItemProbs
