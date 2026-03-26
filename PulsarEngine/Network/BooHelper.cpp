#include <kamek.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/RKNet/ITEM.hpp>

namespace Pulsar {
namespace Network {

// Convert base item + mode to the proper ItemId for triple items
static ItemId ConvertToFullItemId(ItemId baseItem, u8 mode) {
    if (mode == 4) {
        switch (baseItem) {
            case GREEN_SHELL: return TRIPLE_GREEN_SHELL;
            case RED_SHELL: return TRIPLE_RED_SHELL;
            case BANANA: return TRIPLE_BANANA;
            case MUSHROOM: return TRIPLE_MUSHROOM;
            default: break;
        }
    }
    return baseItem;
}

// Get the network item for a remote player
ItemId GetNetworkPlayerItem(u8 playerId) {
    if (playerId >= 12) return ITEM_NONE;
    
    RKNet::ITEMHandler* handler = RKNet::ITEMHandler::sInstance;
    if (handler == nullptr) return ITEM_NONE;
    
    // Read from the received packet
    const RKNet::ITEMPacket& packet = handler->receivedPackets[playerId];
    ItemId storedItem = (ItemId)packet.storedItem;
    u8 mode = packet.mode;
    
    // Check for no item (mode 0)
    if (mode == 0) return ITEM_NONE;
    if (storedItem >= ITEM_NONE) return ITEM_NONE;
    
    // Convert to triple if needed
    return ConvertToFullItemId(storedItem, mode);
}

} // namespace Network
} // namespace Pulsar