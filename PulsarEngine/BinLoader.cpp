#include <MarioKartWii/Archive/ArchiveMgr.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>

extern "C" u8  compiledItemSlotBin[];
extern "C" u32 compiledItemSlotLen;
extern "C" void BuildItemSlotBinary();;

namespace Pulsar {
void *GetCustomItemSlot(ArchiveMgr *archive, ArchiveSource type, const char *name, u32 *length){
    if (compiledItemSlotLen == 0) BuildItemSlotBinary();
    if (length) *length = compiledItemSlotLen;
    return compiledItemSlotBin;
}
kmCall(0x807bb128, GetCustomItemSlot);
kmCall(0x807bb030, GetCustomItemSlot);
kmCall(0x807bb200, GetCustomItemSlot);
kmCall(0x807bb53c, GetCustomItemSlot);
kmCall(0x807bbb58, GetCustomItemSlot);
kmCall(0x807bbdd4, GetCustomItemSlot);
kmCall(0x807bbf50, GetCustomItemSlot);

} // namespace Pulsar