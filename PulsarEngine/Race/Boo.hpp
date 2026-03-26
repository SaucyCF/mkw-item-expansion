#ifndef _PUL_RACE_BOO_HPP_
#define _PUL_RACE_BOO_HPP_

#include <kamek.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>

namespace Pulsar {
namespace Race {

void UseBoo(Item::Player& itemPlayer);
bool IsPlayerInBooState(u8 playerId);
void ResetBooStates();

} // namespace Race
} // namespace Pulsar

#endif
