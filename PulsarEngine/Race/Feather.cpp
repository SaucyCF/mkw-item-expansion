#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <MarioKartWii/Item/Obj/Gesso.hpp>
#include <MarioKartWii/Driver/DriverManager.hpp>
#include <MarioKartWii/Input/InputManager.hpp>
#include <MarioKartWii/CourseMgr.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <Extensions/ItemExpansion/ItemObjDrop.hpp>
#include <PulsarSystem.hpp>

// Expanded behaviourTable in mod BSS (from ItemSlotExpansion.cpp)
extern "C" Item::Behavior expandedBehaviourTable[23];

namespace Pulsar {
namespace Race {
//Credit CLF78 and Stebler, this is mostly a port of their version with slightly different hooks and proper arguments naming since this is C++
void UseFeather(Item::Player& itemPlayer) {
    const Kart::Pointers* pointers = itemPlayer.pointers;
    pointers->kartMovement->specialFloor |= 0x4; //JumpPad

    Kart::Status* status = pointers->kartStatus; //Hijacking bitfield1 14th bit to create a feather state
    u32 type = 0x7;
    if((status->bitfield1 & 0x4000) != 0) type = 0x2; //if already in a feather, lower vertical velocity (30.0f instead of 50.0 for type 7)
    status->jumpPadType = type;
    status->trickableTimer = 0x4;
    itemPlayer.inventory.RemoveItems(1);
    if(DriverMgr::isOnlineRace && itemPlayer.isRemote) Item::Obj::AddUseEVENTEntry(OBJ_BLOOPER, itemPlayer.id);
    ResetFeatherSpawnTimer();
};

//kmWrite32(0x805b68d8, 0x7DE97B78); //mr r9, r15 to get playercollision
static bool ConditionalIgnoreInvisibleWalls(float radius, CourseMgr& mgr, const Vec3& position, const Vec3& prevPosition,
    KCLBitfield acceptedFlags, CollisionInfo* info, KCLTypeHolder& kclFlags) {
        register Kart::Collision* collision;
        asm(mr collision, r15;);
        Kart::Status* status = collision->pointers->kartStatus;
        if(status->bitfield0 & 0x40000000 && status->jumpPadType == 0x7) {
            acceptedFlags = static_cast<KCLBitfield>(acceptedFlags & ~(1 << KCL_INVISIBLE_WALL));
        }
        //to remove invisible walls from the list of flags checked, these walls at flag 0xD and 2^0xD = 0x2000*
    return mgr.IsCollidingAddEntry(position, prevPosition, acceptedFlags, info, &kclFlags, 0, radius);
}
kmCall(0x805b68dc, ConditionalIgnoreInvisibleWalls);

u8 ConditionalFastFallingBody(const Kart::Sub& sub) {
        const Kart::PhysicsHolder& physicsHolder = sub.GetPhysicsHolder();
        const Kart::Status* status = sub.pointers->kartStatus;
        if(status->bitfield0 & 0x40000000 && status->jumpPadType == 0x7 && status->airtime >= 2 && (!status->bool_0x97 || status->airtime > 19)) {
            Input::ControllerHolder& controllerHolder = sub.GetControllerHolder();
            float input = controllerHolder.inputStates[0].stick.z <= 0.0f ? 0.0f :
                (controllerHolder.inputStates[0].stick.z + controllerHolder.inputStates[0].stick.z);
            physicsHolder.physics->gravity -= input * 0.39f;
    }
    return sub.GetPlayerIdx();
}
kmCall(0x805967ac, ConditionalFastFallingBody);


void ConditionalFastFallingWheels(float unk_float, Kart::WheelPhysicsHolder* wheelPhysicsHolder, Vec3& gravityVector, const Mtx34& wheelMat) {
        Kart::Status* status = wheelPhysicsHolder->pointers->kartStatus;
        if(status->bitfield0 & 0x40000000 && status->jumpPadType == 0x7) {
            if(status->airtime == 0) status->bool_0x97 = ((status->bitfield0 & 0x80) != 0) ? true : false;
            else if(status->airtime >= 2 && (!status->bool_0x97 || status->airtime > 19)) {
                const Input::ControllerHolder& controllerHolder = wheelPhysicsHolder->GetControllerHolder();
                float input = controllerHolder.inputStates[0].stick.z <= 0.0f ? 0.0f :
                    (controllerHolder.inputStates[0].stick.z + controllerHolder.inputStates[0].stick.z);
                gravityVector.y -= input * 0.39f;
        }
    }
    wheelPhysicsHolder->Update(gravityVector, wheelMat, unk_float);
}
kmCall(0x805973b4, ConditionalFastFallingWheels);

static void FeatherBehaviour() {
    Item::Behavior& featherBehavior = expandedBehaviourTable[FEATHER];
    featherBehavior.unknkown_0x0 = 1;
    featherBehavior.unknkown_0x1 = 0;
    featherBehavior.objId = OBJ_FEATHER;
    featherBehavior.numberOfItems = 1;
    featherBehavior.unknown_0xc = 0;
    featherBehavior.unknown_0x10 = 0;
    featherBehavior.useType = Item::ITEMUSE_USE;
    featherBehavior.useFunction = UseFeather;
}
RaceLoadHook PatchFeatherBehaviour(FeatherBehaviour);

//Kept as is because it's almost never used and that guarantees ghost sync
kmWrite32(0x808b5c24, 0x42AA0000); //increases min, max speed of jump pad type 0x7 as well as its vertical velocity
kmWrite32(0x808b5c28, 0x42AA0000);
kmWrite32(0x808b5c2C, 0x42960000);
}//namespace Race
}//namespace Pulsar