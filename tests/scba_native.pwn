#include <a_samp>
#include <FCNPC>
#undef MAX_PLAYERS
#define MAX_PLAYERS (5)
#define CMD:%0(%1) stock TestCommand_%0(%1)
#pragma warning disable 203
#pragma warning disable 239

stock GetPlayerRoleplayName(playerid, name[], size)
{
    name[0] = EOS;
    return GetPlayerName(playerid, name, size);
}
#include "features/scba/header"

new NativeChecks, NativeFailures;

stock CheckNative(bool:condition, const message[])
{
    NativeChecks++;
    if (!condition) { NativeFailures++; printf("[SCBA native tests] FAIL: %s", message); }
}

main() {}

public OnGameModeInit()
{
    SetTimer("RunScbaNativeTests", 100, false);
    return 1;
}

forward RunScbaNativeTests();
public RunScbaNativeTests()
{
    CheckNative(ScbaModelsRegistered, "both supplied model pairs register with verified CRCs");
    new npc = FCNPC_Create("ScbaNativeProbe");
    CheckNative(npc != INVALID_PLAYER_ID, "an isolated native player slot can be created");
    if (npc != INVALID_PLAYER_ID)
    {
        CheckNative(FCNPC_Spawn(npc, 277, 100.0, 200.0, 10.0) != 0, "native fixture spawns");
        ScbaResetPlayer(npc);
        FCNPC_SetVirtualWorld(npc, 7);
        CheckNative(GetPlayerVirtualWorld(npc) == 7 && GetPlayerInterior(npc) == 0
            && !ScbaHostIsIndoors(npc, GetPlayerInterior(npc)), "a native virtual world can still be outdoors");
        ScbaPlayer[npc][ScbaMaskOn] = true;
        ScbaPlayer[npc][ScbaAirRemainingMs] = 0;
        ScbaPlayer[npc][ScbaLastAirUpdate] = GetTickCount();
        FCNPC_SetInterior(npc, 11);
        CallLocalFunction("OnPlayerInteriorChange", "iii", npc, 11, 0);
        CheckNative(GetPlayerInterior(npc) == 11 && ScbaHostIsIndoors(npc, GetPlayerInterior(npc))
            && ScbaPlayer[npc][ScbaBreathingIndoors], "a real interior ID and the production door callback enable indoor exposure");
        FCNPC_SetInterior(npc, 0);
        CallLocalFunction("OnPlayerInteriorChange", "iii", npc, 0, 11);
        CheckNative(GetPlayerInterior(npc) == 0 && !ScbaPlayer[npc][ScbaBreathingIndoors],
            "the door callback clears exposure on return to the native exterior");
        FCNPC_SetVirtualWorld(npc, 0);
        ScbaResetPlayer(npc);
        CallLocalFunction("OnPlayerFinishedDownloading", "ii", npc, 0);
        CheckNative(ScbaPlayer[npc][ScbaAssetsReady], "the download callback makes the gear and dial available");
        ScbaPlayer[npc][ScbaDizzinessLevel] = ScbaSymptoms[1][ScbaSwayLevel];
        SetPlayerDrunkLevel(npc, 400);
        CheckNative(ScbaPlayer[npc][ScbaDizzinessLevel] == SCBA_EXTERNAL_DIZZINESS,
            "the native host setter takes ownership of an active camera effect");
        ScbaClearSymptoms(npc);
        TogglePlayerControllable(npc, 0);
        CheckNative(ScbaPlayer[npc][ScbaHostControlsLocked], "the production hook observes the native host freeze");
        ScbaSetTruckControlLock(npc, true);
        TogglePlayerControllable(npc, 1);
        CheckNative(!ScbaPlayer[npc][ScbaHostControlsLocked] && ScbaPlayer[npc][ScbaTruckControlsLocked],
            "a host unlock is remembered while SCBA still holds the player's controls");
        ScbaSetTruckControlLock(npc, false);
        CheckNative(!ScbaPlayer[npc][ScbaTruckControlsLocked], "SCBA releases only its movement lock");
        CheckNative(SetPlayerAttachedObject(npc, 0, 1271, 7) != 0, "an unrelated accessory occupies its own slot");
        CheckNative(ScbaAttachPart(npc, SCBA_BACKPACK), "production attachment accepts the downloaded backpack model");
        CheckNative(ScbaAttachPart(npc, SCBA_MASK), "production attachment accepts the downloaded mask model");
        new back = ScbaAttachmentSlots[npc][SCBA_BACKPACK], mask = ScbaAttachmentSlots[npc][SCBA_MASK];
        CheckNative(back != mask && back != 0 && mask != 0, "production allocator keeps all three attachments separate");
        if (back != SCBA_EMPTY_SLOT && mask != SCBA_EMPTY_SLOT)
            CheckNative(IsPlayerAttachedObjectSlotUsed(npc, back) != 0 && IsPlayerAttachedObjectSlotUsed(npc, mask) != 0,
                "real SA-MP attachment slots contain both custom models");
        new const pose[E_SCBA_POSE] = {0.03, 0.08, 0.01, 2.0, -3.0, 4.0};
        CheckNative(ScbaSetCharacterFit(npc, SCBA_MASK, pose) && ScbaHasCharacterPose[npc][SCBA_MASK],
            "a loaded mask fit is applied to a real occupied attachment slot");
        ScbaPlayer[npc][ScbaEditingPart] = SCBA_MASK;
        CheckNative(funcidx("OnPlayerEditAttachedObject") >= 0, "the SA-MP attachment editor callback is exported under its full name");
        CallLocalFunction("OnPlayerEditAttachedObject", "iiiiifffffffff", npc, EDIT_RESPONSE_CANCEL, back,
            ScbaModels[SCBA_BACKPACK][ScbaCustomModel], ScbaModels[SCBA_BACKPACK][ScbaModelBone],
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
        CheckNative(ScbaPlayer[npc][ScbaEditingPart] == SCBA_MASK, "the native callback ignores a different attachment slot");
        CallLocalFunction("OnPlayerEditAttachedObject", "iiiiifffffffff", npc, EDIT_RESPONSE_CANCEL, mask,
            ScbaModels[SCBA_MASK][ScbaCustomModel], ScbaModels[SCBA_MASK][ScbaModelBone],
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 2.0, 2.0);
        CheckNative(ScbaPlayer[npc][ScbaEditingPart] == SCBA_PART_COUNT && !ScbaHasCharacterPose[npc][SCBA_BACKPACK],
            "the native callback reaches the matching SCBA editor and closes it");
        ScbaRemovePart(npc, SCBA_MASK); ScbaRemovePart(npc, SCBA_BACKPACK);
        if (back != SCBA_EMPTY_SLOT && mask != SCBA_EMPTY_SLOT)
            CheckNative(!IsPlayerAttachedObjectSlotUsed(npc, back) && !IsPlayerAttachedObjectSlotUsed(npc, mask),
                "production removal releases both native slots");
        CheckNative(IsPlayerAttachedObjectSlotUsed(npc, 0) != 0, "production removal preserves the unrelated native attachment");
        CheckNative(FCNPC_Destroy(npc) != 0, "the temporary native fixture is destroyed");
    }
    printf("[SCBA native tests] %d assertions, %d failures", NativeChecks, NativeFailures);
    if (!NativeFailures) print("[SCBA native tests] PASS");
    SetTimer("FinishScbaNativeTests", 100, false);
    return 1;
}

forward FinishScbaNativeTests();
public FinishScbaNativeTests()
{
    SendRconCommand("exit");
    return 1;
}
