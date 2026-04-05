#include <kamek.hpp>
#include <MarioKartWii/Audio/RSARPlayer.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <Sound/BooBRSTM.hpp>

namespace Pulsar {
namespace Sound {

bool PlayBooBRSTM() {
    Audio::RaceRSARPlayer* rsarPlayer = static_cast<Audio::RaceRSARPlayer*>(Audio::RSARPlayer::sInstance);
    if (rsarPlayer == nullptr) return false;

    // SOUND_ID_GHOST_REPLAY is redirected to /Audio/boo.brstm in BRSTMExpansion.cpp.
    return rsarPlayer->PlaySound(SOUND_ID_GHOST_REPLAY, 0);
}

}  // namespace Sound
}  // namespace Pulsar
