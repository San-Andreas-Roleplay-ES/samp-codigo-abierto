#include <a_samp>
#undef MAX_PLAYERS
#define MAX_PLAYERS (5)
#define SCBA_TESTING
#define CMD:%0(%1) stock TestCommand_%0(%1)
#pragma warning disable 203
#pragma warning disable 239
#pragma dynamic 8192

#include "features/scba/config"

#define TEST_DRAW_COUNT (256)
#define TEST_VEHICLE_COUNT (32)
#define TEST_ROOM_INTERIOR (11)

new TestAssertions, TestFailures, TestNow = 1000;
new TestFirstModelResult = -1, TestModelRegistrations;
new bool:TestRejectModelLookup;
new bool:TestConnected[MAX_PLAYERS], bool:TestAdmin[MAX_PLAYERS], bool:TestNpc[MAX_PLAYERS];
new TestPlayerState[MAX_PLAYERS], TestSkin[MAX_PLAYERS], TestWorld[MAX_PLAYERS], TestInterior[MAX_PLAYERS];
new Float:TestPosition[MAX_PLAYERS][3], Float:TestHealth[MAX_PLAYERS];
new TestAttachments[MAX_PLAYERS][MAX_PLAYER_ATTACHED_OBJECTS], TestAttachmentBone[MAX_PLAYERS][MAX_PLAYER_ATTACHED_OBJECTS];
new TestAttachCalls, TestAttachFailAt;
new Float:TestPartOffset[MAX_PLAYERS][MAX_PLAYER_ATTACHED_OBJECTS][3];
new Float:TestPartScale[MAX_PLAYERS][MAX_PLAYER_ATTACHED_OBJECTS][3];
new TestEditedSlot[MAX_PLAYERS], TestClosedEditors, TestFitWrites, TestFitResets;
new bool:TestFailEditor, bool:TestFailFitWrite;
new TestCharacterId[MAX_PLAYERS], TestSavedPose[MAX_PLAYERS][SCBA_PART_COUNT][E_SCBA_POSE];
new bool:TestSavedPart[MAX_PLAYERS][SCBA_PART_COUNT];
#define MAX_ADMIN_OBJECTS (1)
new playerEditingObjectID[MAX_PLAYERS] = {-1, ...};

new TestMessages[MAX_PLAYERS], TestSounds[MAX_PLAYERS], TestLastSound[MAX_PLAYERS];
new Float:TestSoundPosition[MAX_PLAYERS][3], TestLastMessage[MAX_PLAYERS][144];
new TestAnimation[MAX_PLAYERS], TestClearedAnimations[MAX_PLAYERS], TestControl[MAX_PLAYERS];
new bool:TestTruckLoopSynced[MAX_PLAYERS];
new TestAnimationMisses[MAX_PLAYERS], TestAnimationAttempts[MAX_PLAYERS], TestPreloadMask[MAX_PLAYERS];
new TestStreamCount[MAX_PLAYERS], TestStreamUrl[MAX_PLAYERS][220], TestStreamPositional[MAX_PLAYERS];
new Float:TestStreamRadius[MAX_PLAYERS], TestDrunk[MAX_PLAYERS];
new Float:TestHeading[MAX_PLAYERS], Float:TestPartRotation[MAX_PLAYERS][MAX_PLAYER_ATTACHED_OBJECTS][3];
new TestVehicleModel[TEST_VEHICLE_COUNT], TestVehicleWorld[TEST_VEHICLE_COUNT];
new Float:TestVehiclePosition[TEST_VEHICLE_COUNT][3], Float:TestVelocity[TEST_VEHICLE_COUNT][3];
new Float:TestVehicleHealth[TEST_VEHICLE_COUNT];
new bool:TestDrawAlive[MAX_PLAYERS][TEST_DRAW_COUNT], bool:TestDrawVisible[MAX_PLAYERS][TEST_DRAW_COUNT];
new TestDrawFont[MAX_PLAYERS][TEST_DRAW_COUNT], TestDrawColour[MAX_PLAYERS][TEST_DRAW_COUNT];
new Float:TestDrawPosition[MAX_PLAYERS][TEST_DRAW_COUNT][2], Float:TestDrawSize[MAX_PLAYERS][TEST_DRAW_COUNT][2];
new TestDrawText[MAX_PLAYERS][TEST_DRAW_COUNT][48];
new TestDrawCalls, TestDrawFailAt, TestInvalidDrawCalls, TestHealthWrites;

stock Check(bool:condition, const message[])
{
    TestAssertions++;
    if (!condition) { TestFailures++; printf("[SCBA tests] FAIL: %s", message); }
}

stock TestIsConnected(playerid) { return playerid >= 0 && playerid < MAX_PLAYERS && TestConnected[playerid]; }
stock TestIsAdmin(playerid) { return TestAdmin[playerid]; }
stock TestIsNpc(playerid) { return TestNpc[playerid]; }
stock TestGetState(playerid) { return TestPlayerState[playerid]; }
stock TestGetSkin(playerid) { return TestSkin[playerid]; }
stock TestGetWorld(playerid) { return TestWorld[playerid]; }
stock TestGetInterior(playerid) { return TestInterior[playerid]; }
stock TestGetTick() { return TestNow; }
stock TestGetHealth(playerid, &Float:health) { health = TestHealth[playerid]; return 1; }
stock TestSetHealth(playerid, Float:health) { TestHealth[playerid] = health; TestHealthWrites++; return 1; }
stock TestGetPos(playerid, &Float:x, &Float:y, &Float:z)
{ x = TestPosition[playerid][0]; y = TestPosition[playerid][1]; z = TestPosition[playerid][2]; return 1; }
stock TestInRange(playerid, Float:radius, Float:x, Float:y, Float:z)
{
    new Float:dx = TestPosition[playerid][0] - x, Float:dy = TestPosition[playerid][1] - y, Float:dz = TestPosition[playerid][2] - z;
    return radius >= 0.0 && dx * dx + dy * dy + dz * dz <= radius * radius;
}
stock TestSlotUsed(playerid, slot) { return TestAttachments[playerid][slot] != 0; }
stock TestAttach(playerid, slot, model, bone, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:sx, Float:sy, Float:sz)
{
    TestAttachCalls++;
    if (TestAttachFailAt && TestAttachCalls == TestAttachFailAt) return 0;
    TestAttachments[playerid][slot] = model;
    TestAttachmentBone[playerid][slot] = bone;
    TestPartOffset[playerid][slot][0] = x; TestPartOffset[playerid][slot][1] = y; TestPartOffset[playerid][slot][2] = z;
    TestPartScale[playerid][slot][0] = sx; TestPartScale[playerid][slot][1] = sy; TestPartScale[playerid][slot][2] = sz;
    TestPartRotation[playerid][slot][0] = rx;
    TestPartRotation[playerid][slot][1] = ry;
    TestPartRotation[playerid][slot][2] = rz;
    return 1;
}
stock TestEditAttached(playerid, slot)
{
    if (TestFailEditor) return 0;
    TestEditedSlot[playerid] = slot;
    return 1;
}
stock TestCancelEdit(playerid)
{
    TestEditedSlot[playerid] = -1;
    TestClosedEditors++;
    return 1;
}
stock bool:TestSaveFitStat(playerid, E_SCBA_PART:part, const pose[E_SCBA_POSE])
{
    if (TestFailFitWrite) return false;
    new characterid = TestCharacterId[playerid];
    TestSavedPose[characterid][part] = pose;
    TestSavedPart[characterid][part] = true;
    TestFitWrites++;
    return true;
}
stock bool:TestResetFitStats(playerid)
{
    if (TestFailFitWrite) return false;
    new characterid = TestCharacterId[playerid];
    for (new E_SCBA_PART:part; part < SCBA_PART_COUNT; part++) TestSavedPart[characterid][part] = false;
    TestFitResets++;
    return true;
}
stock TestRemoveAttachment(playerid, slot) { TestAttachments[playerid][slot] = 0; return 1; }
stock TestChatText(playerid, colour, const message[])
{
    TestMessages[playerid]++;
    format(TestLastMessage[playerid], 144, "%s", message);
    return 1;
}
stock TestMessage(playerid, colour, const message[])
{
    new rendered[144];
    format(rendered, sizeof(rendered), message);
    return TestChatText(playerid, colour, rendered);
}
stock TestSound(playerid, soundid, Float:x, Float:y, Float:z)
{
    TestSounds[playerid]++; TestLastSound[playerid] = soundid;
    TestSoundPosition[playerid][0] = x; TestSoundPosition[playerid][1] = y; TestSoundPosition[playerid][2] = z;
    return 1;
}
stock TestStream(playerid, const url[], Float:x, Float:y, Float:z, Float:radius, positional)
{
    TestStreamCount[playerid]++;
    format(TestStreamUrl[playerid], 220, "%s", url);
    TestSoundPosition[playerid][0] = x; TestSoundPosition[playerid][1] = y; TestSoundPosition[playerid][2] = z;
    TestStreamRadius[playerid] = radius; TestStreamPositional[playerid] = positional;
    return 1;
}

stock GetPlayerRoleplayName(playerid, name[], size)
{
    format(name, size, "Test Person %d", playerid);
    return 1;
}
stock TestVehicleGetModel(vehicleid) { return vehicleid > 0 && vehicleid < TEST_VEHICLE_COUNT ? TestVehicleModel[vehicleid] : 0; }
stock TestVehicleGetWorld(vehicleid) { return TestVehicleWorld[vehicleid]; }
stock TestVehicleGetHealth(vehicleid, &Float:health) { health = TestVehicleHealth[vehicleid]; return 1; }
stock TestVehicleGetPos(vehicleid, &Float:x, &Float:y, &Float:z)
{ x = TestVehiclePosition[vehicleid][0]; y = TestVehiclePosition[vehicleid][1]; z = TestVehiclePosition[vehicleid][2]; return 1; }
stock TestVehicleGetVelocity(vehicleid, &Float:x, &Float:y, &Float:z)
{ x = TestVelocity[vehicleid][0]; y = TestVelocity[vehicleid][1]; z = TestVelocity[vehicleid][2]; return 1; }
stock TestApplyAnimation(playerid, const library[], const name[], Float:speed, loop, lockx, locky, freeze, time, sync)
{
    if (!strcmp(name, "null"))
    {
        if (!strcmp(library, "BOMBER")) TestPreloadMask[playerid] |= 1;
        if (!strcmp(library, "CAR")) TestPreloadMask[playerid] |= 2;
        return 1;
    }
    TestAnimationAttempts[playerid]++;
    if (TestAnimationMisses[playerid] > 0) { TestAnimationMisses[playerid]--; return 1; }
    if (!strcmp(library, "BOMBER") && !strcmp(name, "BOM_Plant_Loop"))
    {
        TestAnimation[playerid] = 168;
        TestTruckLoopSynced[playerid] = loop != 0 && sync == 1 && time == 0;
    }
    return 1;
}
stock TestAnimationIndex(playerid) { return TestAnimation[playerid]; }
stock TestAnimationName(index, library[], libsize, name[], namesize)
{
    if (index == 168) { format(library, libsize, "BOMBER"); format(name, namesize, "BOM_Plant_Loop"); }
    else { format(library, libsize, "PED"); format(name, namesize, "WALK"); }
    return 1;
}
stock TestClearAnimations(playerid) { TestAnimation[playerid] = 0; TestClearedAnimations[playerid]++; return 1; }

stock bool:TestDrawValid(playerid, PlayerText:draw)
{
    if (_:draw < 0 || _:draw >= TEST_DRAW_COUNT || !TestDrawAlive[playerid][_:draw])
    { TestInvalidDrawCalls++; return false; }
    return true;
}
stock PlayerText:TestCreateDraw(playerid, Float:x, Float:y, const text[])
{
    TestDrawCalls++;
    if (TestDrawFailAt && TestDrawCalls == TestDrawFailAt) return PlayerText:INVALID_TEXT_DRAW;
    for (new id = 0; id < TEST_DRAW_COUNT; id++)
        if (!TestDrawAlive[playerid][id])
        {
            TestDrawAlive[playerid][id] = true; TestDrawVisible[playerid][id] = false;
            TestDrawPosition[playerid][id][0] = x; TestDrawPosition[playerid][id][1] = y;
            TestDrawFont[playerid][id] = -1;
            format(TestDrawText[playerid][id], 48, "%s", text);
            return PlayerText:id;
        }
    return PlayerText:INVALID_TEXT_DRAW;
}
stock TestDestroyDraw(playerid, PlayerText:draw)
{
    if (!TestDrawValid(playerid, draw)) return 0;
    TestDrawAlive[playerid][_:draw] = false; TestDrawVisible[playerid][_:draw] = false; return 1;
}
stock TestDrawSet(playerid, PlayerText:draw, value) { return TestDrawValid(playerid, draw); }
stock TestDrawFontSet(playerid, PlayerText:draw, font)
{ if (!TestDrawValid(playerid, draw)) return 0; TestDrawFont[playerid][_:draw] = font; return 1; }
stock TestDrawColourSet(playerid, PlayerText:draw, colour)
{ if (!TestDrawValid(playerid, draw)) return 0; TestDrawColour[playerid][_:draw] = colour; return 1; }
stock TestDrawDimensions(playerid, PlayerText:draw, Float:x, Float:y)
{
    if (!TestDrawValid(playerid, draw)) return 0;
    TestDrawSize[playerid][_:draw][0] = x; TestDrawSize[playerid][_:draw][1] = y; return 1;
}
stock TestDrawString(playerid, PlayerText:draw, const text[])
{ if (!TestDrawValid(playerid, draw)) return 0; format(TestDrawText[playerid][_:draw], 48, "%s", text); return 1; }
stock TestDrawShow(playerid, PlayerText:draw)
{ if (!TestDrawValid(playerid, draw)) return 0; TestDrawVisible[playerid][_:draw] = true; return 1; }

stock TestRegisterModel(world, baseModel, model, const dff[], const txd[])
{
    new mutableDff[80], mutableTxd[80];
    format(mutableDff, sizeof(mutableDff), "%s", dff);
    format(mutableTxd, sizeof(mutableTxd), "%s", txd);
    new result = AddSimpleModel(world, baseModel, model, mutableDff, mutableTxd);
    if (!TestModelRegistrations) TestFirstModelResult = result;
    TestModelRegistrations++;
    return result;
}
stock TestFindModelName(crc, path[], size)
{
    if (TestRejectModelLookup) { path[0] = EOS; return 0; }
    return FindModelFileNameFromCRC(crc, path, size);
}

#define AddSimpleModel TestRegisterModel
#define FindModelFileNameFromCRC TestFindModelName
#define IsPlayerConnected TestIsConnected
#define IsPlayerAdmin TestIsAdmin
#define IsPlayerNPC TestIsNpc
#define GetPlayerState TestGetState
#define GetPlayerSkin TestGetSkin
#define GetPlayerVirtualWorld TestGetWorld
#define GetPlayerInterior TestGetInterior
#define GetPlayerHealth TestGetHealth
#define SetPlayerHealth TestSetHealth
#define GetPlayerPos TestGetPos
#define IsPlayerInRangeOfPoint TestInRange
#define GetTickCount TestGetTick
#define IsPlayerAttachedObjectSlotUsed TestSlotUsed
#define SetPlayerAttachedObject TestAttach
#define RemovePlayerAttachedObject TestRemoveAttachment
#define EditAttachedObject TestEditAttached
#define CancelEdit TestCancelEdit
#define SCBA_CUSTOM_FIT_STATS
#define ScbaHostSaveFit TestSaveFitStat
#define ScbaHostResetFits TestResetFitStats
#define SendClientMessage TestMessage
#define ScbaSendChatText TestChatText
#define PlayerPlaySound TestSound
#define PlayAudioStreamForPlayer TestStream
#define GetPlayerDrunkLevel TestGetDrunk
#define SetPlayerDrunkLevel TestSetDrunk
#define GetVehicleModel TestVehicleGetModel
#define GetVehicleHealth TestVehicleGetHealth
#define GetVehicleVirtualWorld TestVehicleGetWorld
#define GetVehiclePos TestVehicleGetPos
#define GetVehicleVelocity TestVehicleGetVelocity
#define SetPlayerFacingAngle TestSetHeading
#define TogglePlayerControllable TestSetControl
#define ApplyAnimation TestApplyAnimation
#define GetPlayerAnimationIndex TestAnimationIndex
#define GetAnimationName TestAnimationName
#define ClearAnimations TestClearAnimations
#define CreatePlayerTextDraw TestCreateDraw
#define PlayerTextDrawDestroy TestDestroyDraw
#define PlayerTextDrawFont TestDrawFontSet
#define PlayerTextDrawTextSize TestDrawDimensions
#define PlayerTextDrawColor TestDrawColourSet
#define PlayerTextDrawSetSelectable TestDrawSet
#define PlayerTextDrawLetterSize TestDrawDimensions
#define PlayerTextDrawAlignment TestDrawSet
#define PlayerTextDrawBackgroundColor TestDrawSet
#define PlayerTextDrawSetOutline TestDrawSet
#define PlayerTextDrawSetShadow TestDrawSet
#define PlayerTextDrawSetProportional TestDrawSet
#define PlayerTextDrawShow TestDrawShow
#define PlayerTextDrawSetString TestDrawString

#include "features/scba/header"

stock TestGetDrunk(playerid) { return TestDrunk[playerid]; }
stock TestSetDrunk(playerid, level) { ScbaYieldDizziness(playerid); TestDrunk[playerid] = level; return 1; }
stock TestSetHeading(playerid, Float:angle) { TestHeading[playerid] = angle; return 1; }
stock TestSetControl(playerid, toggle) { TestControl[playerid] = ScbaEffectiveControls(playerid, toggle); return 1; }

stock TestActiveDraws(playerid)
{
    new count;
    for (new i = 0; i < TEST_DRAW_COUNT; i++) if (TestDrawAlive[playerid][i]) count++;
    return count;
}

stock bool:TestGaugeShows(playerid, const texture[])
{
    if (!ScbaGaugeVisible[playerid]) return false;
    new id = _:ScbaGauge[playerid][SCBA_BAND_READING];
    return TestDrawAlive[playerid][id] && TestDrawVisible[playerid][id] && !strcmp(TestDrawText[playerid][id], texture);
}

stock ResetTest(interior = 0)
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        ScbaCleanup(playerid);
        ScbaResetPlayer(playerid);
        TestConnected[playerid] = true; TestAdmin[playerid] = false; TestNpc[playerid] = false;
        TestEditedSlot[playerid] = -1; playerEditingObjectID[playerid] = -1; TestCharacterId[playerid] = playerid;
        for (new E_SCBA_PART:part; part < SCBA_PART_COUNT; part++) TestSavedPart[playerid][part] = false;
        TestPlayerState[playerid] = PLAYER_STATE_ONFOOT; TestHealth[playerid] = 100.0; TestSkin[playerid] = 277;
        TestWorld[playerid] = 0; TestInterior[playerid] = interior;
        TestPosition[playerid][0] = 100.0; TestPosition[playerid][1] = 200.0; TestPosition[playerid][2] = 10.0;
        TestMessages[playerid] = TestSounds[playerid] = TestLastSound[playerid] = TestAnimation[playerid] = TestClearedAnimations[playerid] = 0;
        TestLastMessage[playerid][0] = EOS;
        TestControl[playerid] = 1;
        TestTruckLoopSynced[playerid] = false;
        TestAnimationMisses[playerid] = TestAnimationAttempts[playerid] = TestPreloadMask[playerid] = 0;
        TestStreamCount[playerid] = TestDrunk[playerid] = 0; TestStreamUrl[playerid][0] = EOS;
        ScbaPlayer[playerid][ScbaAssetsReady] = true;
        for (new slot = 0; slot < MAX_PLAYER_ATTACHED_OBJECTS; slot++) TestAttachments[playerid][slot] = 0;
    }
    for (new vehicle = 0; vehicle < TEST_VEHICLE_COUNT; vehicle++)
    {
        TestVehicleModel[vehicle] = TestVehicleWorld[vehicle] = 0;
        TestVehicleHealth[vehicle] = 1000.0;
        for (new axis = 0; axis < 3; axis++) TestVehiclePosition[vehicle][axis] = TestVelocity[vehicle][axis] = 0.0;
    }
    for (new i = 0; i < sizeof(ScbaSupplyPoints); i++) ScbaSupplyPoints[i][ScbaPointEnabled] = false;
    for (new i = 0; i < sizeof(ScbaSupplyVehicles); i++) ScbaSupplyVehicles[i][ScbaVehicleEnabled] = false;
    ScbaSupplyPoints[0][ScbaPointEnabled] = true;
    ScbaSupplyPoints[0][ScbaPointX] = 100.0; ScbaSupplyPoints[0][ScbaPointY] = 200.0; ScbaSupplyPoints[0][ScbaPointZ] = 10.0;
    ScbaSupplyPoints[0][ScbaPointWorld] = 0; ScbaSupplyPoints[0][ScbaPointInterior] = interior;
    ScbaSupplyPoints[0][ScbaPointRadius] = SCBA_INTERACTION_RADIUS;
    ScbaSupplyPoints[0][ScbaPointActions] = SCBA_ALL_ACTIONS;
    ScbaSupplyVehicles[0][ScbaVehicleEnabled] = false;
    ScbaSupplyVehicles[0][ScbaVehicleId] = 10; ScbaSupplyVehicles[0][ScbaVehicleExpectedModel] = 407;
    ScbaSupplyVehicles[0][ScbaVehicleInterior] = interior; ScbaSupplyVehicles[0][ScbaVehicleActions] = SCBA_ALL_ACTIONS;
    ScbaSupplyVehicles[0][ScbaVehicleRadius] = SCBA_INTERACTION_RADIUS;
    ScbaAudioBaseUrl[0] = EOS;
    TestDrawCalls = TestDrawFailAt = TestAttachCalls = TestAttachFailAt = TestInvalidDrawCalls = TestHealthWrites = 0;
    TestFailEditor = TestFailFitWrite = false;
    TestFitWrites = TestClosedEditors = TestFitResets = 0;
    TestNow = 1000;
    return 1;
}

stock EquipAndActivate(playerid = 0)
{
    ScbaHandleCommand(playerid, "equipar");
    ScbaHandleCommand(playerid, "mascara");
    ScbaHandleCommand(playerid, "aire");
}

stock PrepareVehicle()
{
    ScbaSupplyPoints[0][ScbaPointEnabled] = false;
    ScbaSupplyVehicles[0][ScbaVehicleEnabled] = true;
    TestVehicleModel[10] = 407;
    TestVehiclePosition[10][0] = 100.0; TestVehiclePosition[10][1] = 200.0; TestVehiclePosition[10][2] = 10.0;
}

stock TestCommandsAndPermissions()
{
    ResetTest();
    TestCommand_scba(0, "");
    Check(TestMessages[0] == 1 && strfind(TestLastMessage[0], "/scba equipar") >= 0
        && strfind(TestLastMessage[0], "/scba aire") >= 0 && strfind(TestLastMessage[0], "/scba info") == -1 && !TestActiveDraws(0), "bare command lists only the available actions");
    TestCommand_scba(0, "  INFO ");
    Check(TestMessages[0] == 2 && strfind(TestLastMessage[0], "Uso:") == 0 && strfind(TestLastMessage[0], "100%") == -1,
        "the retired info command follows normal help without reporting a cylinder");
    TestConnected[0] = false;
    TestCommand_scba(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "disconnected players cannot equip");
    TestConnected[0] = true;
    TestNpc[0] = true; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "NPCs cannot equip");
    TestNpc[0] = false; TestHealth[0] = 0.0; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "dead players cannot equip");
    TestHealth[0] = 100.0;
    new badCommands[][] = {"equipar otro", "mascara activar", "aire activar", "equiparxxxxxxxxxxxxxxxxxxx", "recargar basura"};
    for (new i = 0; i < sizeof(badCommands); i++)
    {
        ScbaHandleCommand(0, badCommands[i]);
        Check(!ScbaPlayer[0][ScbaEquipped], "extra arguments and oversized actions fail without mutation");
    }
    TestCommand_scba(0, "  EQUIPAR \t");
    Check(ScbaPlayer[0][ScbaEquipped] && !ScbaPlayer[0][ScbaMaskOn], "a regular player can equip without RCON using mixed case and whitespace");
    new calls = TestAttachCalls;
    ScbaHandleCommand(0, "equipar");
    Check(TestAttachCalls == calls, "repeated equip does not duplicate or replace attachments");
    ScbaPlayer[0][ScbaAirRemainingMs] -= 5000;
    ScbaHandleCommand(0, "guardar"); ScbaHandleCommand(0, "equipar");
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS - 5000, "storing and equipping retains the original cylinder reserve");
    TestPlayerState[0] = PLAYER_STATE_DRIVER; ScbaHandleCommand(0, "guardar");
    Check(ScbaPlayer[0][ScbaEquipped], "storage requires being on foot");
}

stock TestUnusedSourceRows()
{
    static const points[4][E_SCBA_POINT] =
    {
        {true, "Locker", 100.0, 200.0, 10.0, 0, 0, SCBA_INTERACTION_RADIUS, _:SCBA_EQUIP},
        SCBA_UNUSED_POINT_ROWS
    };
    static const vehicles[4][E_SCBA_VEHICLE] =
    {
        {true, 10, 407, 0, SCBA_INTERACTION_RADIUS, _:SCBA_REFILL},
        SCBA_UNUSED_VEHICLE_ROWS
    };
    for (new row = 1; row < sizeof(points); row++)
    {
        Check(!points[row][ScbaPointEnabled] && !points[row][ScbaPointActions]
            && !vehicles[row][ScbaVehicleEnabled] && !vehicles[row][ScbaVehicleActions]
            && vehicles[row][ScbaVehicleId] == INVALID_VEHICLE_ID,
            "rows following an active station or truck stay disabled and grant no service");
    }
}

stock bool:TestValidServiceConfig(bool:enabled, Float:radius, actions)
{
    return (actions & ~SCBA_ALL_ACTIONS) == 0 && (!enabled || (radius > 0.0 && actions != 0));
}

stock TestConfiguredSources()
{
    for (new point = 0; point < sizeof(ScbaSupplyPoints); point++)
    {
        Check(TestValidServiceConfig(ScbaSupplyPoints[point][ScbaPointEnabled],
            ScbaSupplyPoints[point][ScbaPointRadius], ScbaSupplyPoints[point][ScbaPointActions]),
            "station rows contain valid service settings, including unused capacity");
    }
    for (new slot = 0; slot < sizeof(ScbaSupplyVehicles); slot++)
    {
        new vehicleid = ScbaSupplyVehicles[slot][ScbaVehicleId];
        new bool:enabled = ScbaSupplyVehicles[slot][ScbaVehicleEnabled];
        Check(TestValidServiceConfig(enabled, ScbaSupplyVehicles[slot][ScbaVehicleRadius],
            ScbaSupplyVehicles[slot][ScbaVehicleActions]) && (!enabled || (vehicleid > 0 && vehicleid < MAX_VEHICLES)),
            "fleet rows contain valid service settings and every active unit has a vehicle id");
    }
}

stock TestSourcePolicies()
{
    ResetTest(); TestPosition[0][0] += SCBA_INTERACTION_RADIUS + 0.01;
    ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "outside the configured source radius cannot equip");
    TestPosition[0][0] = 100.0; TestWorld[0] = 1; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "same coordinates in another world are not a supply source");
    TestWorld[0] = 0; TestInterior[0] = 1; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "same coordinates in another interior are not a supply source");
    TestInterior[0] = 0; ScbaSupplyPoints[0][ScbaPointActions] = _:SCBA_REFILL; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "per-point action grants distinguish refill from equipment");
    ScbaSupplyPoints[0][ScbaPointActions] = SCBA_ALL_ACTIONS; ScbaSupplyPoints[0][ScbaPointEnabled] = false;
    ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "disabled pending coordinates never grant equipment");
    PrepareVehicle();
    ScbaSupplyVehicles[0][ScbaVehicleEnabled] = false;
    ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "a truck of the expected model needs an explicit registration");
    ScbaSupplyVehicles[0][ScbaVehicleEnabled] = true;
    ScbaHandleCommand(0, "equipar");
    Check(ScbaPlayer[0][ScbaEquipped], "an explicit vehicle registration supplies equipment");
    ScbaCleanup(0); TestVehicleModel[10] = 400; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "vehicle model mismatch invalidates a reused registration");
    TestVehicleModel[10] = 407; TestVelocity[10][0] = SCBA_VEHICLE_MAX_SPEED + 0.01;
    ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "moving vehicles cannot act as supply sources");
    TestVelocity[10][0] = 0.0; TestVehicleWorld[10] = 1; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "supply vehicles respect their current world");
}

stock bool:TestWearsPose(playerid, E_SCBA_PART:part, const pose[E_SCBA_POSE])
{
    new slot = ScbaAttachmentSlots[playerid][part];
    return slot != SCBA_EMPTY_SLOT && TestPartOffset[playerid][slot][0] == pose[ScbaOffsetX]
        && TestPartOffset[playerid][slot][1] == pose[ScbaOffsetY] && TestPartOffset[playerid][slot][2] == pose[ScbaOffsetZ]
        && TestPartRotation[playerid][slot][0] == pose[ScbaRotationX] && TestPartRotation[playerid][slot][1] == pose[ScbaRotationY]
        && TestPartRotation[playerid][slot][2] == pose[ScbaRotationZ];
}

stock TestFitResponse(playerid, E_SCBA_PART:part, response, const pose[E_SCBA_POSE])
{
    return ScbaFinishFitEdit(playerid, response, ScbaAttachmentSlots[playerid][part],
        ScbaModels[part][ScbaCustomModel], ScbaModels[part][ScbaModelBone], pose);
}

stock TestFitEditor()
{
    ResetTest();
    new const packPose[E_SCBA_POSE] = {0.12, -0.21, 0.31, 15.0, -55.0, 179.0};
    new const maskPose[E_SCBA_POSE] = {0.04, 0.09, 0.01, -8.0, 2.0, -161.0};
    new const defaultPack[E_SCBA_POSE] = {-0.079, 0.050, -0.020, 0.0, 90.0, 180.0};
    new const defaultMask[E_SCBA_POSE] = {0.0394, 0.0793, 0.0045, -9.46, -1.27, -164.10};
    new const incompleteCommands[][] = {"ajustar", "  AJUSTAR\t", "ajustar otro", "ajustar mascara extra", "ajustar reset otro", "ajustar equipoxxxxxxxxxxxxxxxxx"};
    new const equipHint[] = "Primero ponte el equipo con /scba equipar.";
    for (new i = 0; i < sizeof(incompleteCommands); i++)
    {
        new messages = TestMessages[0];
        TestCommand_scba(0, incompleteCommands[i]);
        Check(TestMessages[0] == messages + 1 && !strcmp(TestLastMessage[0], equipHint) && TestEditedSlot[0] == -1,
            "without the backpack each adjustment request sends only the equip hint");
    }
    TestCommand_scba(0, "ajusta equipo");
    Check(strfind(TestLastMessage[0], "Uso:") == 0 && TestEditedSlot[0] == -1, "ajusta follows general help");
    TestCommand_scba(0, "ajuste");
    Check(strfind(TestLastMessage[0], "Uso:") == 0, "ajuste is not a second adjustment command");
    TestCommand_scba(0, "ajustar reset");
    Check(!TestFitResets && !strcmp(TestLastMessage[0], equipHint), "reset first requires the SCBA backpack");
    TestCommand_scba(0, "ajustar equipo");
    Check(TestEditedSlot[0] == -1 && !ScbaHasEquipment(0) && !strcmp(TestLastMessage[0], equipHint),
        "naming a piece still requires the backpack");
    ScbaEquip(0);
    TestAttachments[0][ScbaAttachmentSlots[0][SCBA_BACKPACK]] = 0;
    TestCommand_scba(0, "ajustar");
    Check(!ScbaPlayer[0][ScbaEquipped] && !strcmp(TestLastMessage[0], equipHint), "a lost attachment cannot expose the adjustment options");

    ScbaEquip(0);
    for (new i = 0; i < sizeof(incompleteCommands); i++)
    {
        TestCommand_scba(0, incompleteCommands[i]);
        Check(TestEditedSlot[0] == -1 && strfind(TestLastMessage[0], "/scba ajustar equipo") >= 0
            && strfind(TestLastMessage[0], "/scba ajustar mascara") >= 0, "a wearer sees both choices for a missing or invalid piece");
    }
    TestCommand_scba(0, "ajustar mascara");
    Check(TestEditedSlot[0] == -1 && !ScbaPlayer[0][ScbaMaskOn] && strfind(TestLastMessage[0], "/scba mascara") >= 0,
        "a wearer requesting a removed mask receives the specific mask hint");
    ScbaToggleMask(0, TestNow);
    new backpack = ScbaAttachmentSlots[0][SCBA_BACKPACK], mask = ScbaAttachmentSlots[0][SCBA_MASK];
    Check(TestWearsPose(0, SCBA_BACKPACK, defaultPack) && TestWearsPose(0, SCBA_MASK, defaultMask),
        "a character without stats uses the approved offsets and rotations on both parts");
    TestFailEditor = true; TestCommand_scba(0, "ajustar equipo"); TestFailEditor = false;
    Check(ScbaPlayer[0][ScbaEditingPart] == SCBA_PART_COUNT, "a failed native editor does not leave a pending edit");
    TestCommand_scba(0, " AJUSTAR\t EQUIPO ");
    TestCommand_scba(0, "ajustar mascara");
    Check(TestEditedSlot[0] == backpack && ScbaPlayer[0][ScbaEditingPart] == SCBA_BACKPACK,
        "only one worn piece is edited at a time, using its actual slot");
    Check(!TestFitResponse(0, SCBA_MASK, EDIT_RESPONSE_FINAL, maskPose)
        && !ScbaFinishFitEdit(0, EDIT_RESPONSE_FINAL, backpack, 1271, 1, packPose)
        && !ScbaFinishFitEdit(0, EDIT_RESPONSE_FINAL, backpack, ScbaModels[SCBA_BACKPACK][ScbaCustomModel], 2, packPose)
        && !TestFitResponse(0, SCBA_BACKPACK, EDIT_RESPONSE_UPDATE, packPose) && !TestFitWrites,
        "foreign slots, models, bones and updates never write character stats");
    TestFitResponse(0, SCBA_BACKPACK, EDIT_RESPONSE_FINAL, packPose);
    Check(TestFitWrites == 1 && TestSavedPart[0][SCBA_BACKPACK] && !TestSavedPart[0][SCBA_MASK]
        && TestWearsPose(0, SCBA_BACKPACK, packPose) && TestWearsPose(0, SCBA_MASK, defaultMask),
        "accepting the backpack commits only that piece and leaves the mask calibrated");
    Check(TestPartScale[0][backpack][1] == 1.12 && !TestFitResponse(0, SCBA_BACKPACK, EDIT_RESPONSE_FINAL, maskPose),
        "a confirmed edit retains skin scale and cannot be replayed");
    TestCommand_scba(0, "ajustar mascara");
    Check(TestEditedSlot[0] == mask, "the mask command opens its own native editor");
    new airBeforeEdit = ScbaPlayer[0][ScbaAirRemainingMs];
    TestNow += SCBA_SECOND_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == airBeforeEdit && TestEditedSlot[0] == mask,
        "the worn mask can be fitted without connecting the regulator or consuming air");
    TestCommand_scba(0, "aire");
    TestNow += SCBA_SECOND_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == airBeforeEdit - SCBA_SECOND_MS && ScbaGaugeVisible[0],
        "adjusting a connected mask keeps breathing and the gauge running");
    TestFitResponse(0, SCBA_MASK, EDIT_RESPONSE_CANCEL, maskPose);
    Check(TestWearsPose(0, SCBA_MASK, defaultMask) && TestFitWrites == 1, "cancelling a first mask edit restores its default");
    TestCommand_scba(0, "ajustar mascara"); TestFitResponse(0, SCBA_MASK, EDIT_RESPONSE_FINAL, maskPose);
    TestCommand_scba(0, "ajustar equipo"); TestFitResponse(0, SCBA_BACKPACK, EDIT_RESPONSE_CANCEL, defaultPack);
    Check(TestWearsPose(0, SCBA_BACKPACK, packPose) && TestWearsPose(0, SCBA_MASK, maskPose)
        && TestPartScale[0][mask][2] == 1.07 && TestFitWrites == 2, "cancel restores the previous personal fit and both scales stay calibrated");
    TestCommand_scba(0, "ajustar equipo");
    TestFailFitWrite = true; TestFitResponse(0, SCBA_BACKPACK, EDIT_RESPONSE_FINAL, defaultPack); TestFailFitWrite = false;
    Check(TestFitWrites == 2 && TestWearsPose(0, SCBA_BACKPACK, packPose), "a rejected stats write rolls the displayed pose back");
    TestCommand_scba(0, "ajustar mascara");
    TestAttachFailAt = TestAttachCalls + 1; TestFitResponse(0, SCBA_MASK, EDIT_RESPONSE_FINAL, defaultMask); TestAttachFailAt = 0;
    Check(TestFitWrites == 2 && TestWearsPose(0, SCBA_MASK, maskPose), "failure to place the piece never commits stats");
    new invalid[E_SCBA_POSE];
    invalid[ScbaOffsetX] = SCBA_FIT_OFFSET_LIMIT + 1.0;
    TestCommand_scba(0, "ajustar equipo"); TestFitResponse(0, SCBA_BACKPACK, EDIT_RESPONSE_FINAL, invalid);
    invalid[ScbaOffsetX] = 0.0; invalid[ScbaRotationZ] = Float:0x7FC00000;
    Check(!ScbaSetCharacterFit(0, SCBA_MASK, invalid) && TestFitWrites == 2 && TestWearsPose(0, SCBA_BACKPACK, packPose),
        "invalid editor or imported values leave the prior fit intact");
    TestCommand_scba(0, "ajustar mascara"); ScbaToggleMask(0, TestNow);
    Check(TestClosedEditors == 1 && !TestFitResponse(0, SCBA_MASK, EDIT_RESPONSE_FINAL, defaultMask),
        "removing the selected piece closes its editor and ignores a late result");
    TestCommand_scba(0, "ajustar equipo"); TestSkin[0] = 279; ScbaTickPlayer(0, TestNow);
    Check(TestClosedEditors == 2 && TestWearsPose(0, SCBA_BACKPACK, packPose)
        && !TestFitResponse(0, SCBA_BACKPACK, EDIT_RESPONSE_FINAL, defaultPack), "skin changes cancel an outstanding edit and reapply the saved pose");

    ScbaPlayer[0][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS / 2;
    TestCommand_scba(0, "recargar"); TestCommand_scba(0, "ajustar equipo");
    Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_POINT && ScbaPlayer[0][ScbaEditingPart] == SCBA_PART_COUNT,
        "refill and attachment editing cannot run over one another");
    TestCommand_scba(0, "cancelar"); TestCommand_scba(0, "ajustar equipo"); TestCommand_scba(0, "recargar");
    Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE && ScbaPlayer[0][ScbaEditingPart] == SCBA_BACKPACK,
        "a refill command does not interrupt the active editor");
    playerEditingObjectID[0] = 7; ScbaTickPlayer(0, TestNow); TestCommand_scba(0, "ajustar equipo");
    Check(ScbaPlayer[0][ScbaEditingPart] == SCBA_PART_COUNT && TestClosedEditors == 2,
        "a mapping session keeps ownership of the native editor");
    playerEditingObjectID[0] = -1;
    TestCommand_scba(0, "guardar"); EquipAndActivate();
    Check(TestWearsPose(0, SCBA_BACKPACK, packPose) && TestWearsPose(0, SCBA_MASK, maskPose), "stored gear reapplies each personal fit when equipped again");
    TestCommand_scba(0, "ajustar mascara"); ScbaCleanup(0);
    Check(ScbaPlayer[0][ScbaEditingPart] == SCBA_PART_COUNT && !TestFitResponse(0, SCBA_MASK, EDIT_RESPONSE_FINAL, defaultMask),
        "spawn and death cleanup invalidate the editor without saving");
    EquipAndActivate(); Check(TestWearsPose(0, SCBA_MASK, maskPose), "personal calibration survives a respawn in the same connection");
    ScbaCleanup(0); ScbaResetPlayer(0); ScbaPlayer[0][ScbaAssetsReady] = true; TestCharacterId[0] = 2;
    EquipAndActivate(); Check(TestWearsPose(0, SCBA_BACKPACK, defaultPack) && TestWearsPose(0, SCBA_MASK, defaultMask),
        "a reused player slot has no previous character's overrides");
    TestCharacterId[1] = 0;
    for (new E_SCBA_PART:part; part < SCBA_PART_COUNT; part++)
        if (TestSavedPart[0][part]) ScbaSetCharacterFit(1, part, TestSavedPose[0][part]);
    EquipAndActivate(1);
    Check(TestWearsPose(1, SCBA_BACKPACK, packPose) && TestWearsPose(1, SCBA_MASK, maskPose) && TestFitWrites == 2,
        "a returning character reloads both saved pieces in another slot without triggering a stats write");
    new const zeroPose[E_SCBA_POSE] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Check(ScbaSetCharacterFit(1, SCBA_MASK, zeroPose) && TestWearsPose(1, SCBA_MASK, zeroPose) && TestWearsPose(1, SCBA_BACKPACK, packPose),
        "a valid all-zero pose imported on a worn part is distinct from the default");

    TestCommand_scba(1, "ajustar mascara"); TestCommand_scba(1, "ajustar reset");
    Check(!TestFitResets && ScbaPlayer[1][ScbaEditingPart] == SCBA_MASK, "reset does not discard an active edit");
    TestFitResponse(1, SCBA_MASK, EDIT_RESPONSE_CANCEL, zeroPose);
    TestAttachFailAt = TestAttachCalls + 2; TestCommand_scba(1, "ajustar reset"); TestAttachFailAt = 0;
    Check(!TestFitResets && TestWearsPose(1, SCBA_BACKPACK, packPose) && TestWearsPose(1, SCBA_MASK, zeroPose),
        "a failed second placement restores the previous fit of both pieces");
    TestFailFitWrite = true; TestCommand_scba(1, "ajustar reset"); TestFailFitWrite = false;
    Check(!TestFitResets && TestSavedPart[0][SCBA_BACKPACK] && TestSavedPart[0][SCBA_MASK]
        && TestWearsPose(1, SCBA_BACKPACK, packPose) && TestWearsPose(1, SCBA_MASK, zeroPose),
        "a declined deletion preserves both stored and displayed fits");
    TestCommand_scba(1, "ajustar RESET");
    Check(TestFitResets == 1 && !TestSavedPart[0][SCBA_BACKPACK] && !TestSavedPart[0][SCBA_MASK]
        && TestWearsPose(1, SCBA_BACKPACK, defaultPack) && TestWearsPose(1, SCBA_MASK, defaultMask),
        "reset deletes both overrides for the character and applies both skin defaults immediately");
    TestCommand_scba(1, "ajustar mascara"); TestFitResponse(1, SCBA_MASK, EDIT_RESPONSE_FINAL, maskPose);
    TestCommand_scba(1, "mascara"); TestCommand_scba(1, "ajustar reset"); TestCommand_scba(1, "guardar"); EquipAndActivate(1);
    Check(TestFitResets == 2 && !ScbaHasCharacterPose[1][SCBA_MASK] && !ScbaHasCharacterPose[1][SCBA_BACKPACK]
        && TestWearsPose(1, SCBA_MASK, defaultMask) && TestWearsPose(1, SCBA_BACKPACK, defaultPack),
        "reset also clears a removed mask, and both defaults survive storing and re-equipping");
    TestPartOffset[1][ScbaAttachmentSlots[1][SCBA_BACKPACK]][0] += 0.5;
    TestCommand_scba(1, "ajustar reset");
    Check(TestWearsPose(1, SCBA_BACKPACK, defaultPack) && TestWearsPose(1, SCBA_MASK, defaultMask),
        "repeating a reset also corrects a stale displayed pose when no override exists");
}

stock TestEquipmentAndHud()
{
    ResetTest();
    ScbaPlayer[0][ScbaAssetsReady] = false; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped] && !TestAttachCalls, "equipment waits for the model download acknowledgement");
    ScbaPlayer[0][ScbaAssetsReady] = true;
    TestAttachments[0][9] = 1234; TestAttachments[0][7] = 4321;
    ScbaHandleCommand(0, "equipar");
    Check(TestGaugeShows(0, "mdl-21002:air_s_off") && TestActiveDraws(0) == _:E_SCBA_GAUGE_LAYER,
        "the backpack immediately shows a full disconnected gauge on first equip");
    new beforeMask = TestDrawCalls;
    ScbaHandleCommand(0, "mascara");
    Check(ScbaAttachmentSlots[0][SCBA_BACKPACK] == 8 && ScbaAttachmentSlots[0][SCBA_MASK] == 6,
        "accessories take free slots without overwriting other equipment");
    Check(TestAttachments[0][8] == ScbaModels[SCBA_BACKPACK][ScbaCustomModel] && TestAttachmentBone[0][6] == 2,
        "backpack and mask bind the registered custom models to their configured bones");
    Check(floatabs(TestPartRotation[0][8][1] - 90.0) < 0.001 && floatabs(TestPartRotation[0][6][2]) > 150.0,
        "skin 277 receives the upright cylinder and forward-facing mask transforms");
    Check(TestGaugeShows(0, "mdl-21002:air_s_off"), "a worn mask with a detached regulator shows disconnected");
    ScbaHandleCommand(0, "aire");
    Check(ScbaGaugeVisible[0] && TestActiveDraws(0) == _:E_SCBA_GAUGE_LAYER,
        "the complete gauge stays visible while breathing");
    Check(TestDrawCalls == beforeMask && TestGaugeShows(0, "mdl-21002:air_s_on"), "connecting the regulator reuses the same gauge");
    new fullTime[12];
    format(fullTime, sizeof(fullTime), "%02d:%02d", SCBA_FULL_AIR_SECONDS / 60, SCBA_FULL_AIR_SECONDS % 60);
    Check(!strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_AIR_READING]], "100%")
        && !strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_TIME_READING]], fullTime), "gauge derives percentage and time from one full-air reserve");
    Check(TestDrawFont[0][_:ScbaGauge[0][SCBA_AIR_READING]] == 3 && TestDrawFont[0][_:ScbaGauge[0][SCBA_TIME_READING]] == 3,
        "both readouts use the original GTA HUD figures");
    for (new id = 0; id < TEST_DRAW_COUNT; id++)
        if (TestDrawAlive[0][id])
            Check(TestDrawFont[0][id] == 1 || TestDrawFont[0][id] == SCBA_GAUGE_DIGITS || TestDrawFont[0][id] == 4,
                "gauge uses the game HUD fonts and streamed sprites");
    ScbaHandleCommand(0, "mascara");
    Check(TestGaugeShows(0, "mdl-21002:air_s_off") && TestDrawCalls == beforeMask, "disconnecting retains the gauge and its handles");
    ScbaHandleCommand(0, "guardar");
    Check(!TestActiveDraws(0) && TestAttachments[0][9] == 1234 && TestAttachments[0][7] == 4321,
        "mask/storage cleanup releases only SCBA-owned resources");
    ResetTest();
    for (new slot = 0; slot < MAX_PLAYER_ATTACHED_OBJECTS; slot++) TestAttachments[0][slot] = 1234;
    ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "full attachment slots fail safely");
    ResetTest(); ScbaHandleCommand(0, "equipar");
    TestAttachFailAt = TestAttachCalls + 1; ScbaHandleCommand(0, "mascara");
    Check(!ScbaPlayer[0][ScbaMaskOn] && TestGaugeShows(0, "mdl-21002:air_s_off") && ScbaHasEquipment(0),
        "failed mask attachment preserves the backpack and its disconnected gauge");
    ResetTest(); TestDrawFailAt = 4; ScbaHandleCommand(0, "equipar");
    Check(!ScbaHasEquipment(0) && !TestActiveDraws(0) && ScbaAttachmentSlots[0][SCBA_BACKPACK] == SCBA_EMPTY_SLOT,
        "partial gauge allocation rolls back the complete UI and backpack");
    ScbaHandleCommand(0, "equipar");
    Check(ScbaHasEquipment(0) && TestGaugeShows(0, "mdl-21002:air_s_off"), "equip can be retried after partial allocation");
    Check(!TestInvalidDrawCalls, "gauge rollback never calls setters on invalid handles");
    ResetTest(); EquipAndActivate(); TestSkin[0] = 279; new calls = TestAttachCalls;
    TestNow += 100; ScbaTickPlayer(0, TestNow);
    Check(TestAttachCalls == calls + 2 && ScbaPlayer[0][ScbaAttachmentSkin] == 279,
        "skin changes reapply both configured fitting profiles");
}

stock TestRegulatorToggle()
{
    ResetTest();
    ScbaHandleCommand(0, "aire"); ScbaHandleCommand(0, "equipar"); ScbaHandleCommand(0, "aire");
    Check(!ScbaPlayer[0][ScbaRegulatorConnected] && !ScbaPlayer[0][ScbaMaskOn] && strfind(TestLastMessage[0], "/scba mascara") >= 0,
        "the air toggle requires a worn mask and never equips it implicitly");
    ScbaHandleCommand(0, "mascara");
    new mask = ScbaAttachmentSlots[0][SCBA_MASK], attachments = TestAttachCalls;
    Check(!ScbaPlayer[0][ScbaRegulatorConnected] && strfind(TestLastMessage[0], "/scba aire") >= 0,
        "putting on the mask leaves the regulator disconnected and explains how to connect it");
    TestNow += SCBA_FULL_AIR_MS; TestInterior[0] = TEST_ROOM_INTERIOR;
    ScbaUpdateBreathing(0, TestNow, TestInterior[0]);
    TestNow += SCBA_FULL_AIR_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS && !TestHealthWrites && !TestSounds[0],
        "standby preserves the full cylinder across a door, even after twice its rated duration");
    new fullTime[12];
    format(fullTime, sizeof(fullTime), "%02d:%02d", SCBA_FULL_AIR_SECONDS / 60, SCBA_FULL_AIR_SECONDS % 60);
    Check(TestGaugeShows(0, "mdl-21002:air_s_off") && !strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_AIR_READING]], "100%")
        && !strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_TIME_READING]], fullTime), "wearing only the mask leaves all displayed readings at standby");

    TestCommand_scba(0, "  AiRe\t");
    Check(ScbaPlayer[0][ScbaRegulatorConnected] && TestGaugeShows(0, "mdl-21002:air_s_on"),
        "the first toggle connects the regulator and updates the gauge immediately");
    ScbaHandleCommand(0, "aire extra");
    Check(ScbaPlayer[0][ScbaRegulatorConnected], "extra arguments cannot accidentally disconnect the regulator");
    TestNow += 1537; ScbaHandleCommand(0, "aire");
    new remaining = ScbaPlayer[0][ScbaAirRemainingMs];
    Check(remaining == SCBA_FULL_AIR_MS - 1537 && !ScbaPlayer[0][ScbaRegulatorConnected] && TestGaugeShows(0, "mdl-21002:air_s_off"),
        "the second toggle settles active time exactly, including the fraction since the last pulse");
    Check(ScbaPlayer[0][ScbaMaskOn] && TestSlotUsed(0, mask) && ScbaAttachmentSlots[0][SCBA_MASK] == mask && TestAttachCalls == attachments,
        "switching the regulator never removes or replaces the fitted mask");
    TestNow += SCBA_FULL_AIR_MS; TestInterior[0] = 0;
    ScbaUpdateBreathing(0, TestNow, TestInterior[0]); ScbaTickPlayer(0, TestNow);
    ScbaHandleCommand(0, "aire"); TestNow += 643; ScbaHandleCommand(0, "mascara");
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == remaining - 643 && !ScbaPlayer[0][ScbaRegulatorConnected] && !ScbaPlayer[0][ScbaMaskOn],
        "resuming skips the entire pause, and removing the mask closes the regulator");
    ScbaHandleCommand(0, "mascara"); TestNow += SCBA_FULL_AIR_MS; ScbaTickPlayer(0, TestNow);
    Check(!ScbaPlayer[0][ScbaRegulatorConnected] && ScbaPlayer[0][ScbaAirRemainingMs] == remaining - 643,
        "putting the mask back on always starts in standby");
}

stock TestAirAndDamage()
{
    ResetTest(TEST_ROOM_INTERIOR); ScbaHandleCommand(0, "equipar"); TestNow += 10000; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS, "carrying the backpack does not consume air");
    ScbaHandleCommand(0, "mascara"); ScbaHandleCommand(0, "aire"); TestNow += 1234; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS - 1234, "air follows elapsed milliseconds rather than timer pulse count");
    ScbaHandleCommand(0, "mascara"); new remaining = ScbaPlayer[0][ScbaAirRemainingMs];
    TestNow += 600000; ScbaTickPlayer(0, TestNow); ScbaHandleCommand(0, "mascara");
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == remaining, "mask pause and reactivation neither drain nor refill stored air");
    ScbaHandleCommand(0, "aire"); TestNow += remaining; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == 0 && TestHealth[0] == 100.0, "exact exhaustion reaches zero without immediate damage");
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(!TestHealthWrites, "empty-cylinder damage waits for the configured interval");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealth[0] == 100.0 - SCBA_DAMAGE_AMOUNT && TestHealthWrites == 1, "empty active mask applies only the configured damage");
    Check(strfind(TestLastMessage[1], "jadeos") != -1 && strfind(TestLastMessage[1], "Test Person") == -1,
        "the first actual air damage emits anonymous distress");
    TestNow += SCBA_DAMAGE_INTERVAL_MS * 20; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 2, "a late pulse does not replay a burst of missed damage ticks");
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaHandleCommand(0, "aire");
    Check(!ScbaPlayer[0][ScbaRegulatorConnected] && TestGaugeShows(0, "mdl-21002:air_s_empty") && TestHealthWrites == 2,
        "disconnecting an exhausted cylinder keeps the empty reading and the existing indoor damage deadline");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 3, "disconnecting does not postpone damage from an already empty cylinder indoors");
    ScbaHandleCommand(0, "mascara"); new writes = TestHealthWrites;
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == writes && TestGaugeShows(0, "mdl-21002:air_s_off")
        && !strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_AIR_READING]], "0%"),
        "the removed mask shows a disconnected, empty cylinder without applying an early hit");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == writes + 1, "removing an empty mask preserves the original indoor damage deadline");
    ScbaHandleCommand(0, "mascara");
    ScbaHandleCommand(0, "aire");
    Check(ScbaPlayer[0][ScbaMaskOn] && !ScbaPlayer[0][ScbaRegulatorConnected] && TestGaugeShows(0, "mdl-21002:air_s_empty")
        && strfind(TestLastMessage[0], "/scba recargar") >= 0, "an empty cylinder can be worn for fitting but cannot supply air");

    ResetTest(TEST_ROOM_INTERIOR); TestNow = 0x7FFFFD00; EquipAndActivate(); TestNow += 2000; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS - 2000, "elapsed air consumption survives signed GetTickCount wrap");

    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate(0); EquipAndActivate(1); TestNow += 10000; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS - 10000
        && ScbaPlayer[1][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS, "one player's tick does not consume another player's reserve");
    ScbaTickPlayer(1, TestNow); ScbaHandleCommand(0, "mascara"); TestNow += 5000; ScbaTickPlayer(1, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS - 10000
        && ScbaPlayer[1][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS - 15000, "independent players can pause and continue breathing separately");
}

stock TestIndoorAirDamage()
{
    ResetTest(); EquipAndActivate();
    TestWorld[0] = TestWorld[1] = 7;
    TestHealth[0] = 65.0;
    TestNow += SCBA_FULL_AIR_MS - SCBA_FULL_AIR_MS * SCBA_LOW_AIR_PERCENT / 100;
    ScbaTickPlayer(0, TestNow);
    Check(ScbaAirBand(0) == SCBA_AIR_LOW && TestLastSound[1] == SCBA_FALLBACK_RESERVE_SOUND,
        "the cylinder drains and the normal reserve alarm is audible outdoors");
    TestNow += ScbaPlayer[0][ScbaAirRemainingMs]; ScbaTickPlayer(0, TestNow);
    new nearbySounds = TestSounds[1], wearerMessages = TestMessages[0];
    TestNow += 8 * SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(!TestHealthWrites && TestHealth[0] == 65.0 && TestGaugeShows(0, "mdl-21002:air_s_empty")
        && !TestDrunk[0] && TestSounds[1] == nearbySounds && TestMessages[0] == wearerMessages,
        "an exhausted mask in an outdoor virtual world keeps an empty gauge without damage or symptoms");

    TestInterior[0] = TestInterior[1] = TEST_ROOM_INTERIOR;
    ScbaTickPlayer(0, TestNow);
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(!TestHealthWrites, "entering empty after a long time outdoors requires a full indoor interval");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 1 && TestHealth[0] == 65.0 - SCBA_DAMAGE_AMOUNT
        && strfind(TestLastMessage[1], "jadeos") >= 0 && !TestDrunk[0],
        "the first actual indoor hit starts the private and nearby breathing episode");

    TestNow += SCBA_DAMAGE_INTERVAL_MS / 2;
    TestInterior[0] = TestInterior[1] = 5;
    ScbaUpdateBreathing(0, TestNow, TestInterior[0]);
    TestNow += SCBA_DAMAGE_INTERVAL_MS - SCBA_DAMAGE_INTERVAL_MS / 2; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 2 && TestDrunk[0] > 0, "moving between native interiors keeps the existing damage cadence");

    new Float:healthAtExit = TestHealth[0];
    TestNow += SCBA_DAMAGE_INTERVAL_MS; TestInterior[0] = TestInterior[1] = 0;
    ScbaTickPlayer(0, TestNow);
    Check(TestHealth[0] == healthAtExit && TestHealthWrites == 2 && !TestDrunk[0] && !ScbaPlayer[0][ScbaHasDistressTime],
        "leaving at a hit deadline cancels damage and clears SCBA symptoms, without restoring HP");

    TestInterior[0] = TEST_ROOM_INTERIOR; ScbaUpdateBreathing(0, TestNow, TestInterior[0]);
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    // Two door changes can happen before the next timer pulse.
    TestInterior[0] = 0; ScbaUpdateBreathing(0, TestNow, TestInterior[0]);
    TestInterior[0] = TEST_ROOM_INTERIOR; ScbaUpdateBreathing(0, TestNow, TestInterior[0]);
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 2 && !TestDrunk[0], "a quick exit and re-entry discards the previous indoor deadline");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 3 && strfind(TestLastMessage[0], "Te falta el aire") == 0 && !TestDrunk[0],
        "re-entry begins a new symptom episode after a full interval");

    ResetTest(); EquipAndActivate();
    ScbaPlayer[0][ScbaAirRemainingMs] = 100;
    TestNow += 4 * SCBA_DAMAGE_INTERVAL_MS;
    TestInterior[0] = TEST_ROOM_INTERIOR; ScbaUpdateBreathing(0, TestNow, TestInterior[0]);
    ScbaTickPlayer(0, TestNow);
    Check(!ScbaPlayer[0][ScbaAirRemainingMs] && !TestHealthWrites, "crossing a door settles overdue outdoor consumption without overdue damage");
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(!TestHealthWrites, "the old outdoor exhaustion time cannot shorten the re-entry delay");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 1, "the late-processed door still starts the normal indoor cadence");

    ResetTest(); EquipAndActivate();
    TestInterior[0] = TEST_ROOM_INTERIOR; ScbaPlayer[0][ScbaAirRemainingMs] = SCBA_DAMAGE_INTERVAL_MS + 100;
    ScbaTickPlayer(0, TestNow);
    TestNow += SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(!TestHealthWrites && ScbaPlayer[0][ScbaAirRemainingMs] == 100, "entering a room with air never starts HP loss early");
    TestNow += 100 + SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(!TestHealthWrites, "when air runs out inside, its true exhaustion time starts the damage interval");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 1, "the indoor hit starts one full interval after the last air");
}

stock TestIndoorExposureRecovery()
{
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    TestNow += SCBA_FULL_AIR_MS; ScbaTickPlayer(0, TestNow);
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1;
    ScbaHandleCommand(0, "mascara"); ScbaHandleCommand(0, "mascara"); ScbaHandleCommand(0, "mascara");
    Check(!TestHealthWrites, "repeated mask changes cannot trigger an early first hit");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 1 && strfind(TestLastMessage[1], "jadeos entrecortados") >= 0,
        "the first hit still falls on time and the anonymous gasp also describes an uncovered face");

    ScbaHandleCommand(0, "recargar");
    TestNow += SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 2 && ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_POINT,
        "an indoor refill runs alongside the same damage clock");
    TestNow += ScbaPlayer[0][ScbaSupplyDurationMs] - SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS && ScbaPlayer[0][ScbaHasDistressTime],
        "finishing the refill restores the cylinder while keeping the breathing episode");
    new writes = TestHealthWrites;
    TestNow += SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    ScbaHandleCommand(0, "mascara"); TestNow += SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == writes + 2 && !ScbaPlayer[0][ScbaRegulatorConnected],
        "both a full cylinder and a refitted standby mask leave the wearer exposed");
    ScbaHandleCommand(0, "aire");
    new Float:healthAtRecovery = TestHealth[0];
    TestNow += 2 * SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(TestHealth[0] == healthAtRecovery && ScbaPlayer[0][ScbaAirRemainingMs] < SCBA_FULL_AIR_MS
        && !ScbaPlayer[0][ScbaHasDistressTime] && !TestDrunk[0],
        "breathing from the refilled cylinder ends damage and symptoms without restoring lost HP");

    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    TestNow += SCBA_FULL_AIR_MS; ScbaTickPlayer(0, TestNow);
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaHandleCommand(0, "guardar");
    TestNow++; ScbaTickPlayer(0, TestNow); ScbaTickPlayer(1, TestNow);
    Check(!ScbaPlayer[0][ScbaEquipped] && TestHealthWrites == 1 && TestHealth[1] == 100.0,
        "returning the equipment cannot reset exposure and an unrelated player in the room stays healthy");
    ScbaHandleCommand(0, "equipar"); ScbaHandleCommand(0, "guardar");
    TestNow += SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 2, "collecting and returning the gear again keeps the same deadline");
    TestInterior[0] = 0; ScbaTickPlayer(0, TestNow);
    TestInterior[0] = TEST_ROOM_INTERIOR; TestNow += 2 * SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 2 && !ScbaTracksBreathing(0), "leaving after storage ends the episode for the unequipped player");

    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    TestNow += SCBA_FULL_AIR_MS; ScbaHandleCommand(0, "mascara"); ScbaHandleCommand(0, "recargar");
    TestHealth[0] = SCBA_DAMAGE_AMOUNT; TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
    Check(TestHealth[0] == 0.0 && !ScbaTracksBreathing(0) && ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE
        && ScbaPlayer[0][ScbaAirRemainingMs] == 0, "fatal exposure cleans up an indoor refill before it can credit air");
}

stock TestEmptyCylinderCadence()
{
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    new emptyAt = TestNow + SCBA_FULL_AIR_MS;
    TestNow = emptyAt - 1; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == 1 && !TestHealthWrites, "the final millisecond of air protects the wearer");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(!ScbaPlayer[0][ScbaAirRemainingMs] && TestHealth[0] == 100.0, "running out starts a separate damage clock");

    new expectedHits = floatround(100.0 / SCBA_DAMAGE_AMOUNT, floatround_ceil);
    new bool:regular = true, firstDelay, Float:firstHealth;
    for (new hit = 1; hit <= expectedHits; hit++)
    {
        TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
        if (TestHealthWrites != hit - 1) regular = false;
        TestNow++; ScbaTickPlayer(0, TestNow);
        new Float:expectedHealth = 100.0 - hit * SCBA_DAMAGE_AMOUNT;
        if (expectedHealth < 0.0) expectedHealth = 0.0;
        if (TestHealthWrites != hit || floatabs(TestHealth[0] - expectedHealth) > 0.001) regular = false;
        if (!firstDelay && TestHealthWrites)
        {
            firstDelay = ScbaElapsed(TestNow, emptyAt);
            firstHealth = TestHealth[0];
        }
    }
    Check(regular && !ScbaPlayer[0][ScbaEquipped] && TestHealth[0] == 0.0,
        "every empty-cylinder hit occurs at its own deadline until health reaches zero");
    printf("[SCBA cadence] cylinder=%ds first=%dms first_hp=%.1f hits=%d last=%dms", SCBA_FULL_AIR_SECONDS,
        firstDelay, firstHealth, TestHealthWrites, ScbaElapsed(TestNow, emptyAt));
}

stock TestDamageTimerJitter()
{
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    new emptyAt = TestNow + SCBA_FULL_AIR_MS;
    TestNow = emptyAt; ScbaTickPlayer(0, TestNow);
    new jitter = min(SCBA_PULSE_INTERVAL_MS, SCBA_DAMAGE_INTERVAL_MS / 2);
    TestNow = emptyAt + SCBA_DAMAGE_INTERVAL_MS + jitter; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 1, "an ordinary late pulse applies one hit");
    TestNow = emptyAt + 2 * SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 1, "the next hit still waits for its original deadline");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 2, "routine timer jitter does not accumulate from one hit to the next");

    TestNow += 4 * SCBA_DAMAGE_INTERVAL_MS + jitter; ScbaTickPlayer(0, TestNow);
    ScbaHandleCommand(0, "recargar");
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 3, "a long stall discards missed hits and commands cannot replay them");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 4, "the hit following a long stall gets a full interval");

    ResetTest(TEST_ROOM_INTERIOR); TestNow = 0x7FFFFFFF - SCBA_DAMAGE_INTERVAL_MS / 2; EquipAndActivate();
    ScbaPlayer[0][ScbaAirRemainingMs] = 1;
    TestNow++; ScbaTickPlayer(0, TestNow);
    TestNow += SCBA_DAMAGE_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(!TestHealthWrites, "a damage deadline crossing signed clock wrap does not fire early");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestHealthWrites == 1, "the wrapped deadline applies exactly one hit");
}

stock TestNextAirDamage()
{
    TestNow += SCBA_DAMAGE_INTERVAL_MS;
    return ScbaTickPlayer(0, TestNow);
}

stock TestPrivateSymptoms()
{
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate(); ScbaPlayer[0][ScbaAirRemainingMs] = 0; TestHealth[0] = 75.0;
    TestNextAirDamage();
    Check(strfind(TestLastMessage[0], "Te falta el aire") == 0 && !TestDrunk[0] && strfind(TestLastMessage[1], "jadeos") >= 0,
        "first damage gives a private breathlessness message alongside the anonymous environment without sway");
    new messages = TestMessages[0], nearby = TestMessages[1];
    TestNextAirDamage();
    Check(TestMessages[0] == messages && TestMessages[1] == nearby && !TestDrunk[0], "above 60 HP the camera stays steady");
    TestNextAirDamage();
    Check(TestHealth[0] == 60.0 && strfind(TestLastMessage[0], "mareado") >= 0 && TestDrunk[0] == ScbaSymptoms[1][ScbaSwayOnset]
        && TestMessages[1] == nearby, "exactly 60 HP starts a mild dizzy spell in private");
    messages = TestMessages[0]; TestNextAirDamage();
    Check(TestMessages[0] == messages && TestDrunk[0] > ScbaSymptoms[1][ScbaSwayOnset]
        && TestDrunk[0] < ScbaSymptoms[1][ScbaSwayLevel] && ScbaSymptoms[1][ScbaSwayLevel] < 2700,
        "mild sway increases gradually below its former strength without repeating chat");
    TestHealth[0] = 35.0; TestNextAirDamage();
    Check(TestHealth[0] == 30.0 && strfind(TestLastMessage[0], "desvanecerte") >= 0 && TestDrunk[0] == ScbaSymptoms[2][ScbaSwayLevel]
        && TestDrunk[0] > ScbaSymptoms[1][ScbaSwayLevel], "at 30 HP the private warning and sway intensify");
    ScbaHandleCommand(0, "mascara");
    Check(TestDrunk[0] == ScbaSymptoms[2][ScbaSwayLevel] && ScbaPlayer[0][ScbaReportedSymptom] == 2,
        "removing the mask indoors preserves the current dizziness and symptom stage");

    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate(); ScbaPlayer[0][ScbaAirRemainingMs] = 0; TestHealth[0] = 30.0;
    TestNextAirDamage(); Check(!TestDrunk[0] && strfind(TestLastMessage[0], "Te falta") == 0, "even an injured player gets a first warning without dizziness");
    TestNextAirDamage(); Check(TestDrunk[0] == ScbaSymptoms[2][ScbaSwayLevel], "subsequent damage chooses the actual health band directly");
    TestPlayerState[0] = PLAYER_STATE_DRIVER; ScbaTickPlayer(0, TestNow);
    Check(!TestDrunk[0], "entering a vehicle releases SCBA sway ahead of the next damage tick");
    TestPlayerState[0] = PLAYER_STATE_ONFOOT;
    TestNextAirDamage(); TestSetDrunk(0, 900); ScbaHandleCommand(0, "mascara");
    Check(TestDrunk[0] == 900, "an external setter takes ownership even when it requests a weaker effect");

    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate(); ScbaPlayer[0][ScbaAirRemainingMs] = 0; TestHealth[0] = 50.0;
    TestSetDrunk(0, 5200); TestNextAirDamage(); TestNextAirDamage(); ScbaCleanup(0);
    Check(TestDrunk[0] == 5200, "an existing host effect survives both hypoxia and cleanup");
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate(); ScbaPlayer[0][ScbaAirRemainingMs] = 0; TestHealth[0] = 12.0;
    TestNextAirDamage(); TestNextAirDamage(); TestNextAirDamage();
    Check(TestHealth[0] == 0.0 && !TestDrunk[0] && !ScbaHasEquipment(0), "death releases SCBA sway and equipment");
}

stock TestGaspCadence()
{
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    ScbaPlayer[0][ScbaAirRemainingMs] = 0;
    TestPosition[2][0] += SCBA_EVENT_RADIUS + 1.0; TestWorld[3] = 1; TestInterior[4] = 1;
    new expectedSeconds[] = {2, 14, 20, 26, 30, 34, 38};
    new start = TestNow, heard = 0, nearbyChat = TestMessages[1];
    for (new hit = 0; hit < 20; hit++)
    {
        TestNextAirDamage();
        if (TestSounds[1] == heard) continue;
        Check(heard < sizeof(expectedSeconds) && TestNow - start == expectedSeconds[heard] * SCBA_SECOND_MS
            && TestLastSound[1] == SCBA_DISTRESS_SOUND, "gasps use the current health band's interval since the last audible gasp");
        heard = TestSounds[1];
    }
    Check(heard == sizeof(expectedSeconds) && heard == TestSounds[0] && !TestSounds[2] && !TestSounds[3] && !TestSounds[4],
        "increasing gasps reach only nearby listeners in the same world and interior");
    Check(TestMessages[1] == nearbyChat + 1 && strfind(TestLastMessage[1], "jadeos") >= 0,
        "the entire breathing episode adds only one environmental chat line");
    TestNextAirDamage();
    Check(TestSounds[1] == heard, "gasps stop at death");

    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate(); ScbaPlayer[0][ScbaAirRemainingMs] = 0;
    TestNextAirDamage(); ScbaHandleCommand(0, "mascara");
    heard = TestSounds[1]; TestNow += 30000; ScbaTickPlayer(0, TestNow);
    Check(TestSounds[1] == heard + 1, "the next due gasp remains audible after mask removal indoors");
    TestInterior[0] = 0; ScbaTickPlayer(0, TestNow);
    TestNow += 30000; ScbaTickPlayer(0, TestNow);
    Check(TestSounds[1] == heard + 1 && !ScbaPlayer[0][ScbaHasDistressTime], "going outside ends the maskless breathing episode");
}

stock TestGaugeThresholds()
{
    ResetTest(); EquipAndActivate();
    new thresholds[] = {SCBA_LOW_AIR_PERCENT, SCBA_CRITICAL_AIR_PERCENT};
    new E_SCBA_GAUGE_STATUS:above[] = {SCBA_STATUS_NORMAL, SCBA_STATUS_LOW};
    new E_SCBA_GAUGE_STATUS:at[] = {SCBA_STATUS_LOW, SCBA_STATUS_CRITICAL};
    for (new i = 0; i < sizeof(thresholds); i++)
    {
        new boundary = SCBA_FULL_AIR_MS / 100 * thresholds[i];
        ScbaPlayer[0][ScbaAirRemainingMs] = boundary + 1;
        Check(ScbaAirPercent(0) == thresholds[i] + 1 && ScbaCurrentGaugeStatus(0) == above[i],
            "the millisecond above a reserve threshold retains the higher reading and status");
        ScbaPlayer[0][ScbaAirRemainingMs] = boundary;
        Check(ScbaAirPercent(0) == thresholds[i] && ScbaCurrentGaugeStatus(0) == at[i],
            "the gauge and alarm band change together on the exact threshold");
    }
    ScbaPlayer[0][ScbaAirRemainingMs] = 1;
    Check(ScbaAirPercent(0) == 1 && ScbaCurrentGaugeStatus(0) == SCBA_STATUS_CRITICAL,
        "the final millisecond still reads one percent and very low air");
    ScbaPlayer[0][ScbaAirRemainingMs] = 0;
    Check(ScbaAirPercent(0) == 0 && ScbaCurrentGaugeStatus(0) == SCBA_STATUS_EMPTY,
        "an exhausted mask reads zero percent and no air");
}

stock TestAlarmPrivacy()
{
    ResetTest(); EquipAndActivate();
    TestPosition[2][0] += SCBA_EVENT_RADIUS + 1.0; TestWorld[3] = 1; TestInterior[4] = 1;
    for (new i = 0; i < MAX_PLAYERS; i++) TestMessages[i] = TestSounds[i] = 0;
    ScbaPlayer[0][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS * SCBA_LOW_AIR_PERCENT / 100;
    ScbaTickPlayer(0, TestNow);
    Check(TestSounds[0] == 1 && TestSounds[1] == 1 && !TestSounds[2] && !TestSounds[3] && !TestSounds[4],
        "alarms reach only nearby listeners in the same world and interior");
    Check(TestMessages[0] == 1 && TestMessages[1] == 1 && !TestMessages[2] && !TestMessages[3] && !TestMessages[4],
        "anonymous environment and sound use the same recipient filter");
    Check(strfind(TestLastMessage[1], "alarma") != -1 && strfind(TestLastMessage[1], "Test Person") == -1
        && strfind(TestLastMessage[1], "((") == -1, "alarm text contains no owner name or do-command author signature");
    Check(TestSoundPosition[1][0] == TestPosition[0][0] && TestSoundPosition[1][1] == TestPosition[0][1],
        "alarm audio originates at the wearer's position");
    TestNow += SCBA_ALARM_INTERVAL_MS - 1; ScbaTickPlayer(0, TestNow);
    Check(TestSounds[1] == 1 && TestMessages[1] == 1, "alarm cooldown avoids sound bursts and repeated environment messages");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestSounds[1] == 2 && TestMessages[1] == 1, "periodic alarm sound does not repeat the environment text");
    ScbaHandleCommand(0, "aire"); new sounds = TestSounds[1];
    TestNow += 2 * SCBA_ALARM_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(TestSounds[1] == sounds, "reserve alarms stay silent while the regulator is disconnected");
    ScbaHandleCommand(0, "aire");
    Check(TestSounds[1] == sounds + 1 && ScbaPlayer[0][ScbaReportedBand] == SCBA_AIR_LOW, "reconnecting at low air resumes the existing alarm band");
    ScbaHandleCommand(0, "mascara"); sounds = TestSounds[1];
    ScbaHandleCommand(0, "mascara"); ScbaHandleCommand(0, "aire");
    Check(TestSounds[1] == sounds, "mask toggling cannot bypass the alarm cooldown");
    TestNow += SCBA_HUD_INTERVAL_MS; ScbaPlayer[0][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS * SCBA_CRITICAL_AIR_PERCENT / 100;
    ScbaTickPlayer(0, TestNow);
    Check(ScbaAirBand(0) == SCBA_AIR_CRITICAL && ScbaPlayer[0][ScbaReportedBand] == SCBA_AIR_CRITICAL,
        "critical reserve is classified separately");
    Check(TestDrawColour[0][_:ScbaGauge[0][SCBA_BAND_READING]] == SCBA_COLOUR_CRITICAL, "critical air uses the configured gauge colour");
    new messages = TestMessages[1]; TestNow++; ScbaTickPlayer(0, TestNow);
    Check(TestMessages[1] == messages, "critical environment is emitted once per band transition");
    TestNpc[1] = true; TestNow += SCBA_ALARM_INTERVAL_MS; sounds = TestSounds[1]; ScbaTickPlayer(0, TestNow);
    Check(TestSounds[1] == sounds, "NPC listeners are excluded from alarm broadcasts");
}

stock TestStatusAndStorage()
{
    ResetTest();
    ScbaSupplyPoints[0][ScbaPointActions] = _:SCBA_EQUIP;
    ScbaHandleCommand(0, "equipar");
    Check(TestGaugeShows(0, "mdl-21002:air_s_off"), "the equipment shows the disconnected status without a mask");
    ScbaHandleCommand(0, "mascara");
    ScbaHandleCommand(0, "aire");
    Check(TestGaugeShows(0, "mdl-21002:air_s_on"), "the connected regulator has its own visible status");

    ScbaSupplyPoints[0][ScbaPointActions] = _:SCBA_REFILL;
    ScbaHandleCommand(0, "guardar");
    Check(ScbaHasEquipment(0) && ScbaPlayer[0][ScbaMaskOn], "a refill-only station cannot take the equipment");
    ScbaSupplyPoints[0][ScbaPointActions] = 4;
    ScbaHandleCommand(0, "guardar");
    Check(ScbaHasEquipment(0), "an old independent storage bit never grants permission");
    ScbaSupplyPoints[0][ScbaPointActions] = _:SCBA_EQUIP;
    ScbaHandleCommand(0, "guardar");
    Check(!ScbaHasEquipment(0), "equipment-only stations automatically accept returns");
    PrepareVehicle(); ScbaSupplyVehicles[0][ScbaVehicleActions] = _:SCBA_EQUIP;
    ScbaHandleCommand(0, "equipar"); ScbaSupplyVehicles[0][ScbaVehicleActions] = _:SCBA_REFILL;
    ScbaHandleCommand(0, "guardar");
    Check(ScbaHasEquipment(0), "a refill-only truck cannot take the equipment");
    ScbaSupplyVehicles[0][ScbaVehicleActions] = _:SCBA_EQUIP; ScbaHandleCommand(0, "guardar");
    Check(!ScbaHasEquipment(0), "vehicles that issue equipment accept returns under the same conditions");
}

stock TestCustomAlarms()
{
    ResetTest(); EquipAndActivate();
    format(ScbaAudioBaseUrl, sizeof(ScbaAudioBaseUrl), "http://audio.test/scba");
    TestPosition[2][0] += SCBA_EVENT_RADIUS + 1.0; TestWorld[3] = 1; TestInterior[4] = 1;
    ScbaPlayer[0][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS * SCBA_LOW_AIR_PERCENT / 100;
    ScbaTickPlayer(0, TestNow);
    Check(TestStreamCount[0] == 1 && TestStreamCount[1] == 1 && !TestStreamCount[2] && !TestStreamCount[3] && !TestStreamCount[4],
        "custom alarm tones stay within the sound radius, world and interior");
    Check(!strcmp(TestStreamUrl[1], "http://audio.test/scba/reserva.mp3") && !TestSounds[1]
        && TestStreamPositional[1] && TestStreamRadius[1] == SCBA_EVENT_RADIUS
        && TestSoundPosition[1][0] == TestPosition[0][0], "the reserve tone is a positional stream at the wearer");
    ScbaBroadcast(0, "", SCBA_FALLBACK_RESERVE_SOUND, SCBA_RESERVE_CLIP);
    Check(TestStreamCount[1] == 1, "overlapping alarms let the short cue finish on one client");
    TestNow += SCBA_ALARM_INTERVAL_MS; ScbaPlayer[0][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS * SCBA_CRITICAL_AIR_PERCENT / 100;
    ScbaTickPlayer(0, TestNow);
    Check(TestStreamCount[1] == 2 && !strcmp(TestStreamUrl[1], "http://audio.test/scba/critica.mp3"),
        "critical air selects the faster three-pulse alert");
    ScbaBroadcast(0, "", SCBA_DISTRESS_SOUND);
    Check(TestLastSound[1] == SCBA_DISTRESS_SOUND && TestStreamCount[1] == 2, "distress remains independent of the alarm stream");
}

stock TestSupplyAudio()
{
    for (new truck = 0; truck < 2; truck++)
    {
        ResetTest(); if (truck) PrepareVehicle();
        format(ScbaAudioBaseUrl, sizeof(ScbaAudioBaseUrl), "http://audio.test/scba");
        TestPosition[1][0] += SCBA_EVENT_RADIUS - 0.1; TestPosition[2][0] += SCBA_EVENT_RADIUS + 0.1;
        TestWorld[3] = 1; TestInterior[4] = 1;
        ScbaHandleCommand(0, "equipar"); ScbaHandleCommand(0, "recargar");
        Check(!TestStreamCount[0], "a full cylinder makes no service sound");
        ScbaPlayer[0][ScbaAirRemainingMs] = 1000;
        ScbaHandleCommand(0, "recargar");
        Check(TestStreamCount[0] == 1 && TestStreamCount[1] == 1 && !TestStreamCount[2] && !TestStreamCount[3] && !TestStreamCount[4],
            "a confirmed service reaches only audible nearby players in the same dimension");
        new url[80];
        if (truck) format(url, sizeof(url), "http://audio.test/scba/acople.mp3");
        else format(url, sizeof(url), "http://audio.test/scba/valvula.mp3");
        Check(!strcmp(TestStreamUrl[1], url) && TestStreamRadius[1] == SCBA_EVENT_RADIUS && TestStreamPositional[1]
            && TestSoundPosition[1][0] == TestPosition[0][0], "the service location selects the fitting or valve cue at its source");
        ScbaHandleCommand(0, "recargar"); TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
        Check(TestStreamCount[1] == 1 && TestGaugeShows(0, "mdl-21002:air_s_off")
            && !strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_AIR_READING]], "100%"),
            "one service plays once even with a repeat command and completion immediately updates the disconnected gauge");

        ScbaPlayer[0][ScbaAirRemainingMs] = 1000;
        ScbaHandleCommand(0, "recargar");
        ScbaBroadcast(0, "", SCBA_FALLBACK_CRITICAL_SOUND, SCBA_CRITICAL_CLIP, true);
        Check(TestStreamCount[1] == 3 && !strcmp(TestStreamUrl[1], "http://audio.test/scba/critica.mp3"),
            "an air alarm can take the listener's stream from a short fitting sound");
        ScbaHandleCommand(0, "cancelar"); ScbaHandleCommand(0, "recargar");
        Check(TestStreamCount[1] == 3, "recurring service sounds cannot interrupt an ongoing air alarm");
        ScbaHandleCommand(0, "cancelar"); TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
        Check(TestStreamCount[1] == 3 && ScbaPlayer[0][ScbaAirRemainingMs] == 1000,
            "cancelled service does not queue a sound or award air");
    }
}

stock TestRefillAndCancellation()
{
    ResetTest(); EquipAndActivate(); TestNow += 20000; ScbaTickPlayer(0, TestNow);
    ScbaHandleCommand(0, "recargar");
    Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE, "supply requires the mask to be removed");
    ScbaHandleCommand(0, "aire"); ScbaHandleCommand(0, "recargar");
    Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE, "disconnecting air alone does not remove the mask for service");
    ScbaHandleCommand(0, "mascara"); new remaining = ScbaPlayer[0][ScbaAirRemainingMs];
    ScbaHandleCommand(0, "recargar");
    Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_POINT && TestAnimation[0] == 168 && TestControl[0],
        "HQ refill starts the configured timed service and animation");
    ScbaHandleCommand(0, "aire");
    Check(!ScbaPlayer[0][ScbaRegulatorConnected] && ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_POINT,
        "the regulator command leaves an ongoing refill intact");
    TestNow += ScbaPlayer[0][ScbaSupplyDurationMs] - 1; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == remaining, "an incomplete service never restores air");
    TestNow++; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS
        && ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE && TestClearedAnimations[0] == 1,
        "completed HQ service restores one full charge and releases its animation");
    Check(ScbaPlayer[0][ScbaReportedBand] == SCBA_AIR_NORMAL && !ScbaPlayer[0][ScbaHasAlarmTime],
        "successful supply resets the reserve warning cycle");
    ScbaHandleCommand(0, "recargar");
    Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE, "a full cylinder cannot start another refill");

    for (new cause = 0; cause < 5; cause++)
    {
        ResetTest(); ScbaHandleCommand(0, "equipar"); ScbaPlayer[0][ScbaAirRemainingMs] = 10000;
        ScbaHandleCommand(0, "recargar");
        switch (cause)
        {
            case 0: TestPosition[0][0] += SCBA_SERVICE_MOVEMENT_RADIUS + 0.1;
            case 1: TestWorld[0] = 1;
            case 2: TestInterior[0] = 1;
            case 3: ScbaSupplyPoints[0][ScbaPointEnabled] = false;
            case 4: TestPlayerState[0] = PLAYER_STATE_DRIVER;
        }
        TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
        Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE && ScbaPlayer[0][ScbaAirRemainingMs] == 10000,
            "movement, dimension, disabled source and entering a vehicle cancel service without a free refill");
    }
    ResetTest(); PrepareVehicle(); ScbaHandleCommand(0, "equipar"); ScbaPlayer[0][ScbaAirRemainingMs] = 10000;
    ScbaHandleCommand(0, "recargar"); TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS, "registered stationary truck completes a cylinder replacement");
    for (new cause = 0; cause < 4; cause++)
    {
        ResetTest(); PrepareVehicle(); ScbaHandleCommand(0, "equipar"); ScbaPlayer[0][ScbaAirRemainingMs] = 10000;
        ScbaHandleCommand(0, "recargar");
        switch (cause)
        {
            case 0: TestVehicleModel[10] = 0;
            case 1: TestVelocity[10][0] = SCBA_VEHICLE_MAX_SPEED + 0.1;
            case 2: TestVehicleWorld[10] = 1;
            case 3: ScbaSupplyVehicles[0][ScbaVehicleEnabled] = false;
        }
        TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
        Check(ScbaPlayer[0][ScbaAirRemainingMs] == 10000 && ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE,
            "destroyed, moving, dimension-changed and revoked trucks cannot complete service");
    }
    ResetTest(); ScbaHandleCommand(0, "equipar"); ScbaPlayer[0][ScbaAirRemainingMs] = 10000;
    ScbaHandleCommand(0, "recargar"); TestAnimation[0] = 999;
    TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
    Check(!TestClearedAnimations[0] && TestAnimation[0] == 999, "service cleanup does not clear another module's replacement animation");
}

stock TestRefillDuration()
{
    new span = SCBA_SUPPLY_MAX_MS - SCBA_SUPPLY_MIN_MS;
    for (new truck = 0; truck < 2; truck++)
    {
        ResetTest(); if (truck) PrepareVehicle();
        ScbaHandleCommand(0, "equipar");
        for (new quarters = 0; quarters < 4; quarters++)
        {
            new remaining = SCBA_FULL_AIR_MS / 4 * quarters;
            ScbaPlayer[0][ScbaAirRemainingMs] = remaining;
            ScbaHandleCommand(0, "recargar");
            new expected = SCBA_SUPPLY_MIN_MS + span * (4 - quarters) / 4;
            Check(ScbaPlayer[0][ScbaSupplyDurationMs] == expected, "refill duration scales with the fraction missing at either source");
            TestNow += expected - 1; ScbaTickPlayer(0, TestNow);
            Check(ScbaPlayer[0][ScbaSupplySource] != SCBA_SOURCE_NONE && ScbaPlayer[0][ScbaAirRemainingMs] == remaining,
                "a partial charge is never awarded before its deadline");
            TestNow++; ScbaTickPlayer(0, TestNow);
            Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE && ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS,
                "the shorter or longer service finishes exactly at its own deadline");
        }
        ScbaPlayer[0][ScbaAirRemainingMs]--;
        ScbaHandleCommand(0, "recargar");
        Check(ScbaPlayer[0][ScbaSupplyDurationMs] == SCBA_SUPPLY_MIN_MS, "a nearly full cylinder has the minimum handling time");
    }
}

stock TestColdServiceAnimation()
{
    for (new truck = 0; truck < 2; truck++)
    {
        ResetTest();
        if (truck) PrepareVehicle();
        ScbaHandleCommand(0, "equipar");
        Check(TestPreloadMask[0] == 1, "equipping warms the shared crouching animation library after spawn");
        ScbaPlayer[0][ScbaAirRemainingMs] = 1000;
        TestAnimationMisses[0] = 1;
        ScbaHandleCommand(0, "recargar");
        Check(!TestAnimation[0] && TestAnimationAttempts[0] == 1 && !TestSounds[1], "a cold client can silently discard the first animation");
        TestNow += SCBA_ANIM_RETRY_MS - 1; ScbaTickPlayer(0, TestNow);
        Check(TestAnimationAttempts[0] == 1, "a cold client has time to load before the retry");
        TestNow++; ScbaTickPlayer(0, TestNow);
        Check(TestAnimationAttempts[0] == 2 && TestAnimation[0] == 168,
            "the same command starts the right animation automatically after a cold load");
        TestNow += SCBA_PULSE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
        Check(TestSounds[1] == 1 && TestLastSound[1] == (truck ? SCBA_FALLBACK_TRUCK_SOUND : SCBA_FALLBACK_STATION_SOUND),
            "delayed animation acknowledgement plays the location's native fallback once");
        TestNow += ScbaPlayer[0][ScbaSupplyDurationMs] - 1; ScbaTickPlayer(0, TestNow);
        Check(ScbaPlayer[0][ScbaAirRemainingMs] == 1000 && TestAnimationAttempts[0] == 2, "the full service duration starts when the clip is visible");
        TestNow++; ScbaTickPlayer(0, TestNow);
        Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS && TestControl[0] && !TestAnimation[0],
            "a recovered first service completes and releases its animation");
    }
    ResetTest(); PrepareVehicle(); ScbaHandleCommand(0, "equipar"); ScbaPlayer[0][ScbaAirRemainingMs] = 1000;
    TestAnimationMisses[0] = 100; ScbaHandleCommand(0, "recargar");
    TestNow += SCBA_ANIM_START_TIMEOUT_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE && ScbaPlayer[0][ScbaAirRemainingMs] == 1000
        && TestControl[0], "a client that never acknowledges releases the truck lock without a refill");
    ScbaHandleCommand(0, "recargar"); ScbaHandleCommand(0, "cancelar"); new attempts = TestAnimationAttempts[0];
    TestNow += SCBA_ANIM_RETRY_MS; ScbaTickPlayer(0, TestNow);
    Check(TestAnimationAttempts[0] == attempts && !TestSounds[1], "timeouts and cold cancellations cannot start a sound on a later pulse");
}

stock TestTruckServiceControls()
{
    ResetTest();
    PrepareVehicle();
    TestVehiclePosition[10][0] += 1.5;
    TestCommand_scba(0, "equipar");
    ScbaPlayer[0][ScbaAirRemainingMs] = 20000;
    TestCommand_scba(0, "recargar");
    Check(!TestControl[0] && TestTruckLoopSynced[0] && TestAnimation[0] == 168,
        "truck service holds controls and syncs the stationary crouching animation");
    Check(floatabs(floatcos(TestHeading[0], degrees)) < 0.001 && floatsin(TestHeading[0], degrees) < 0.0,
        "the player faces the truck");
    Check(TestGaugeShows(0, "mdl-21002:air_s_fill") && !TestControl[0],
        "the gauge shows supply in progress during locked truck service");
    TestCommand_scba(0, "cancelar");
    Check(TestControl[0] && TestAnimation[0] == 0 && ScbaPlayer[0][ScbaAirRemainingMs] == 20000
        && TestGaugeShows(0, "mdl-21002:air_s_off"),
        "deliberately cancelling frees the player without awarding air");
    TestSetControl(0, 0);
    TestCommand_scba(0, "recargar");
    TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
    Check(!TestControl[0] && ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS,
        "completing a truck exchange preserves a preexisting host movement lock");
    TestSetControl(0, 1);
    ScbaPlayer[0][ScbaAirRemainingMs] = 20000;
    TestCommand_scba(0, "recargar");
    TestSetControl(0, 0);
    ScbaTickPlayer(0, TestNow + SCBA_PULSE_INTERVAL_MS);
    TestAnimation[0] = 999;
    ScbaTickPlayer(0, TestNow + 2 * SCBA_PULSE_INTERVAL_MS);
    Check(!TestControl[0] && TestAnimation[0] == 999 && ScbaPlayer[0][ScbaAirRemainingMs] == 20000
        && ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE,
        "a replacement animation cancels the exchange and preserves both the host animation and its lock");
    TestCommand_scba(0, "recargar");
    TestSetControl(0, 1);
    Check(!TestControl[0], "external unlock requests do not prematurely release truck service");
    TestVelocity[10][0] = 1.0;
    ScbaTickPlayer(0, TestNow + SCBA_SUPPLY_MAX_MS);
    Check(TestControl[0] && ScbaPlayer[0][ScbaAirRemainingMs] == 20000,
        "losing the truck restores the latest requested control state without air");
    TestVelocity[10][0] = 0.0;
    TestCommand_scba(0, "recargar");
    TestAnimation[0] = 0;
    ScbaTickPlayer(0, TestNow + SCBA_SUPPLY_MAX_MS);
    Check(TestControl[0] && ScbaPlayer[0][ScbaAirRemainingMs] == 20000,
        "an animation that never started cannot complete a cylinder exchange");
}

stock TestLifecycleAndFailures()
{
    for (new cause = 0; cause < 4; cause++)
    {
        ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
        switch (cause)
        {
            case 0: TestAttachments[0][ScbaAttachmentSlots[0][SCBA_MASK]] = 0;
            case 1: TestHealth[0] = 0.0;
            case 2: TestPlayerState[0] = PLAYER_STATE_SPECTATING;
            case 3: TestAttachments[0][ScbaAttachmentSlots[0][SCBA_BACKPACK]] = 0;
        }
        TestNow += 10000; ScbaTickPlayer(0, TestNow);
        if (cause == 0)
            Check(ScbaHasEquipment(0) && !ScbaPlayer[0][ScbaMaskOn] && TestGaugeShows(0, "mdl-21002:air_s_off"),
                "losing only the mask keeps the backpack and disconnects the gauge");
        else
            Check(!ScbaPlayer[0][ScbaEquipped] && !ScbaPlayer[0][ScbaMaskOn] && !TestActiveDraws(0),
                "death, spectating and losing the backpack release equipment and UI");
        Check(!ScbaPlayer[0][ScbaRegulatorConnected] && !TestHealthWrites,
            "loss of a worn part or the player stops the regulator before empty-cylinder damage");
    }
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    ScbaPlayer[0][ScbaAirRemainingMs] = 0; ScbaPlayer[0][ScbaNextDamageAt] = TestNow + SCBA_DAMAGE_INTERVAL_MS;
    TestHealth[0] = SCBA_DAMAGE_AMOUNT; TestNow += SCBA_DAMAGE_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    Check(TestHealth[0] == 0.0 && !ScbaPlayer[0][ScbaEquipped] && !TestActiveDraws(0), "fatal air damage clamps health and immediately cleans up");
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    new initialDraws = TestDrawCalls, PlayerText:needle = ScbaGauge[0][SCBA_DIAL_NEEDLE];
    TestDrawFailAt = TestDrawCalls + 1; TestNow += SCBA_HUD_INTERVAL_MS * 60; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaMaskOn] && TestDrawCalls == initialDraws && ScbaGauge[0][SCBA_DIAL_NEEDLE] == needle,
        "moving the textured needle reuses its handle even when no draw slots are available");
    Check(!TestInvalidDrawCalls, "updating the needle uses a live draw handle");
    ResetTest(TEST_ROOM_INTERIOR); EquipAndActivate();
    ScbaCleanup(0); ScbaResetPlayer(0);
    Check(!ScbaPlayer[0][ScbaEquipped] && !ScbaPlayer[0][ScbaAssetsReady] && !TestActiveDraws(0)
        && ScbaAttachmentSlots[0][SCBA_BACKPACK] == SCBA_EMPTY_SLOT, "disconnect/reset cannot leak state into a reused player slot");
}

stock TestMultipleSources()
{
    ResetTest();
    PrepareVehicle();
    ScbaSupplyPoints[0][ScbaPointEnabled] = true;
    ScbaSupplyPoints[0][ScbaPointActions] = _:SCBA_EQUIP;
    new E_SCBA_SOURCE:source, index;
    Check(ScbaFindSource(0, SCBA_EQUIP, source, index) && source == SCBA_SOURCE_POINT && index == 0,
        "a fixed station offering the requested action takes precedence over a nearby truck");
    Check(ScbaFindSource(0, SCBA_REFILL, source, index) && source == SCBA_SOURCE_VEHICLE && index == 0,
        "the truck provides refill when the overlapping station only supplies equipment");

    ResetTest();
    for (new E_SCBA_POINT:field = E_SCBA_POINT:0; field < E_SCBA_POINT; field++)
        ScbaSupplyPoints[1][field] = ScbaSupplyPoints[0][field];
    ScbaSupplyPoints[0][ScbaPointEnabled] = false;
    ScbaSupplyPoints[1][ScbaPointX] = 500.0;
    ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "another configured station does not supply a remote player");
    TestPosition[0][0] = 500.0; ScbaHandleCommand(0, "equipar");
    Check(ScbaPlayer[0][ScbaEquipped], "a second configured station is usable without changing command logic");
    ScbaSupplyPoints[1][ScbaPointActions] = _:SCBA_REFILL; ScbaHandleCommand(0, "guardar");
    Check(ScbaPlayer[0][ScbaEquipped], "a second station's refill permission does not grant storage");
    ScbaPlayer[0][ScbaAirRemainingMs] = 1000; ScbaHandleCommand(0, "recargar");
    Check(ScbaPlayer[0][ScbaSupplyIndex] == 1, "service freezes the exact selected station");
    ScbaSupplyPoints[0][ScbaPointEnabled] = true;
    ScbaSupplyPoints[1][ScbaPointEnabled] = false;
    TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == 1000, "service cannot migrate to a different newly available station");

    ResetTest(); PrepareVehicle(); ScbaSupplyVehicles[0][ScbaVehicleEnabled] = false;
    ScbaSupplyVehicles[1][ScbaVehicleEnabled] = true;
    ScbaSupplyVehicles[1][ScbaVehicleId] = 10; ScbaSupplyVehicles[1][ScbaVehicleExpectedModel] = 407;
    ScbaSupplyVehicles[1][ScbaVehicleInterior] = 0; ScbaSupplyVehicles[1][ScbaVehicleRadius] = SCBA_INTERACTION_RADIUS;
    ScbaSupplyVehicles[1][ScbaVehicleActions] = _:SCBA_EQUIP | _:SCBA_REFILL;
    ScbaHandleCommand(0, "equipar"); Check(ScbaPlayer[0][ScbaEquipped], "a second fleet registration is usable");
    ScbaPlayer[0][ScbaAirRemainingMs] = 1000; ScbaHandleCommand(0, "recargar");
    TestVehicleModel[11] = 407; TestVehicleHealth[11] = 1000.0;
    TestVehiclePosition[11][0] = 100.0; TestVehiclePosition[11][1] = 200.0; TestVehiclePosition[11][2] = 10.0;
    ScbaSupplyVehicles[1][ScbaVehicleId] = 11;
    TestNow += SCBA_SUPPLY_MAX_MS; ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == 1000, "a replacement vehicle handle cannot complete the old cylinder change");

    ResetTest(); PrepareVehicle(); TestVehicleHealth[10] = 0.0; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "a destroyed vehicle cannot provide equipment");
    TestVehicleHealth[10] = 1000.0; ScbaSupplyVehicles[0][ScbaVehicleExpectedModel] = 0;
    Check(!ScbaHostSupplyVehicle(0), "invalid expected models cannot authorize an empty vehicle slot");
    ResetTest(); ScbaSupplyPoints[0][ScbaPointRadius] = 0.0; ScbaHandleCommand(0, "equipar");
    Check(!ScbaPlayer[0][ScbaEquipped], "invalid station radii fail closed");

    ResetTest();
    TestAdmin[0] = true;
    TestCommand_scba(0, "equipar");
    ScbaPlayer[0][ScbaAirRemainingMs] = 1000;
    TestCommand_scba(0, "recargar");
    TestAdmin[0] = false;
    TestNow += SCBA_SUPPLY_MAX_MS;
    ScbaTickPlayer(0, TestNow);
    Check(ScbaPlayer[0][ScbaAirRemainingMs] == SCBA_FULL_AIR_MS && ScbaPlayer[0][ScbaEquipped]
        && ScbaPlayer[0][ScbaSupplySource] == SCBA_SOURCE_NONE, "losing RCON while refilling has no effect on service");

    ResetTest(); EquipAndActivate(); ScbaPlayer[0][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS / 2;
    TestNow += SCBA_HUD_INTERVAL_MS; ScbaTickPlayer(0, TestNow);
    new Float:faceLeft = SCBA_GAUGE_LEFT, Float:faceRight = SCBA_GAUGE_LEFT + SCBA_GAUGE_WIDTH;
    new Float:faceTop = SCBA_GAUGE_TOP, Float:faceBottom = SCBA_GAUGE_TOP + SCBA_GAUGE_HEIGHT;
    for (new id = 0; id < TEST_DRAW_COUNT; id++)
        if (TestDrawAlive[0][id])
        {
            Check(TestDrawPosition[0][id][0] >= faceLeft && TestDrawPosition[0][id][0] <= faceRight
                && TestDrawPosition[0][id][1] >= faceTop && TestDrawPosition[0][id][1] <= faceBottom,
                "gauge dial, text anchors and needle stay inside their background");
        }
    new footer = _:ScbaGauge[0][SCBA_BAND_READING];
    Check(TestDrawFont[0][footer] == 4
        && floatabs(TestDrawPosition[0][footer][0] + TestDrawSize[0][footer][0] / 2.0 - SCBA_GAUGE_TEXT_X(128.0)) < 0.01
        && floatabs(TestDrawPosition[0][footer][1] + TestDrawSize[0][footer][1] / 2.0 - SCBA_GAUGE_TEXT_Y(230.5)) < 0.01,
        "the status sprite is centered on the dark footer along both axes");
    Check(ScbaNeedleStep(0) < SCBA_GAUGE_NEEDLE_STEPS && ScbaNeedleStep(0) > 0,
        "the needle changes with remaining air");
    new text[32];
    ScbaNeedleTexture(ScbaNeedleStep(0), text, sizeof(text));
    Check(!strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_DIAL_NEEDLE]], text)
        && !strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_DIAL_FACE]], "mdl-21002:air_dial"),
        "the gauge selects the downloaded dial and the frame matching the remaining air");
    ScbaPlayer[0][ScbaAirRemainingMs] = 0;
    ScbaUpdateGauge(0, TestNow);
    Check(!strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_DIAL_NEEDLE]], "mdl-21002:air_n00")
        && !strcmp(TestDrawText[0][_:ScbaGauge[0][SCBA_TIME_READING]], "00:00")
        && TestGaugeShows(0, "mdl-21002:air_s_empty"),
        "an empty cylinder reaches zero on the dial and on the digital readouts");
}

stock FinishTestRun()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) ScbaCleanup(playerid);
    printf("[SCBA tests] %d assertions, %d failures", TestAssertions, TestFailures);
    if (!TestFailures) print("[SCBA tests] PASS");
    SetTimer("FinishScbaTests", 100, false);
    return 1;
}

main() {}

public OnGameModeInit()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) ScbaResetPlayer(playerid);
    new bool:registered = ScbaRegisterModels();
    Check(registered, "actual SA-MP 0.3.DL model registration accepts both supplied DFF/TXD pairs");
    if (!registered) return FinishTestRun();
    Check(TestFirstModelResult == 0, "a successful first model registration may return zero on real SA-MP");
    TestRejectModelLookup = true;
    Check(!ScbaModelFilesRegistered(SCBA_BACKPACK), "missing CRC registrations are not mistaken for successful model loading");
    TestRejectModelLookup = false;
    TestConfiguredSources();
    TestUnusedSourceRows();
    TestCommandsAndPermissions();
    TestStatusAndStorage();
    TestSourcePolicies();
    TestEquipmentAndHud();
    TestFitEditor();
    TestRegulatorToggle();
    TestAirAndDamage();
    TestIndoorAirDamage();
    TestIndoorExposureRecovery();
    TestEmptyCylinderCadence();
    TestDamageTimerJitter();
    TestPrivateSymptoms();
    TestGaspCadence();
    TestGaugeThresholds();
    TestAlarmPrivacy();
    TestCustomAlarms();
    TestSupplyAudio();
    TestRefillAndCancellation();
    TestRefillDuration();
    TestColdServiceAnimation();
    TestTruckServiceControls();
    TestLifecycleAndFailures();
    TestMultipleSources();
    return FinishTestRun();
}

forward FinishScbaTests();
public FinishScbaTests()
{
    SendRconCommand("exit");
    return 1;
}
