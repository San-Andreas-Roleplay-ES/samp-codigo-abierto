// Run as a separate SA-MP gamemode with sscanf 2.13.1 (never on the live server).
// Player, clock and streamer adapters are simulated; Pawn logic, parsing and file I/O are real.
#include <a_samp>
// Bound the production loops to the five simulated player slots in this harness.
#undef MAX_PLAYERS
#define MAX_PLAYERS (5)
#include <sscanf2>
#pragma warning disable 203
#pragma dynamic 8192

#define CMD:%0(%1) stock TestCommand_%0(%1)
#define Dialog:%0(%1) forward dialog_%0(%1); public dialog_%0(%1)
#define Dialog_Show(%0,%1, TestDialog(%0,
#define isnull(%0) (!(%0)[0])
#define STREAMER_TAG_OBJECT _
#define STREAMER_TAG_CP _
#define STREAMER_TAG_PICKUP _
#define STREAMER_TAG_3D_TEXT_LABEL _
#define INVALID_STREAMER_ID (0)
#define STREAMER_TYPE_OBJECT (0)
#define COLOR_FADE3 (-1)
#define COLOR_LIGHTGREEN (0x9EC73DFF)
#define COLOR_RED (0xFF0000FF)
#define COLOR_LIGHTORANGE (-1)
#define COLOR_GRAD1 (0xB4B5B7FF)
#define COLOR_WHITE (0xFFFFFFFF)
#define COLOR_HELP_1 (-1)
#define COLOR_PURPLE (-1)
#define TEST_PLAYERS (5)

new TestNow, TestFailures, TestAssertions;
new TestIntegrationStarts, TestIntegrationEnds, TestRepositorySaves, TestIntegrationStartSlot, TestIntegrationEndSlot;
new bool:TestIntegrationReject;
new bool:TestConnected[TEST_PLAYERS], bool:TestAdmin[TEST_PLAYERS], bool:TestNPC[TEST_PLAYERS];
new TestState[TEST_PLAYERS], TestWeapon[TEST_PLAYERS], TestAmmo[TEST_PLAYERS][13], TestSlotWeapon[TEST_PLAYERS][13], TestWorld[TEST_PLAYERS], TestInterior[TEST_PLAYERS];
new Float:TestPosition[TEST_PLAYERS][3], Float:TestHealth[TEST_PLAYERS];
new bool:TestObjects[256], TestNextObject, TestLiveObjects;
new bool:TestCreateFails, bool:TestVisible = true;
new TestResult, TestResultShots, TestResultHits, TestResultExpired;
new TestResultIgnored, TestResultErrors, TestResultShootTotal, TestResultNoShootTotal;
new TestResultPoints;
new TestMessages[TEST_PLAYERS], TestPrivateMessages[TEST_PLAYERS], TestLastMessage[TEST_PLAYERS][144];
new TestChat[TEST_PLAYERS][8192], TestLongestMessage, bool:TestResultColourWrong;
new TestReportLines[TEST_PLAYERS][8][144];
new TestLastMessageColour[TEST_PLAYERS];
#define TEST_TEXTDRAWS (4096)
new bool:TestTDAlive[TEST_TEXTDRAWS], bool:TestTDVisible[TEST_TEXTDRAWS], TestTDOwner[TEST_TEXTDRAWS], TestTDText[TEST_TEXTDRAWS][64], bool:TestTDFails;
new TestTDFailAt, TestTDFonts[TEST_TEXTDRAWS], TestTDModels[TEST_TEXTDRAWS];
new bool:TestTDSelectable[TEST_TEXTDRAWS], bool:TestCatalogue[TEST_PLAYERS], TestCancelSelections[TEST_PLAYERS];
new Float:TestTDPosition[TEST_TEXTDRAWS][2], Float:TestTDTextSize[TEST_TEXTDRAWS][2];
new bool:TestSelectionOrderWrong, bool:TestDeferredSelection;
new TestSelectionEchoes[TEST_PLAYERS], TestTDAlignment[TEST_TEXTDRAWS];
new Float:TestTDLetterSize[TEST_TEXTDRAWS][2];
new TestSounds[TEST_PLAYERS][128], TestSoundTimes[TEST_PLAYERS][128], TestSoundCount[TEST_PLAYERS];
new TestMaterialColours[256], TestMaterialText[256][16], bool:TestMaterialFails;
new TestStreamedObject, bool:TestCueBeforeReady;
new TestRandomSeed, TestRandomCalls;
new playerEditingObjectID[MAX_PLAYERS] = {-1, ...};
new TestObjectModels[256], TestObjectOwner[256], TestObjectWorld[256], TestObjectInterior[256];
new Float:TestObjectPosition[256][6];
new bool:TestCheckpoints[256], TestNextCP, TestLiveCP, TestCPOwner[256];
new Float:TestCPPosition[256][3];
new bool:TestLabels[256], TestNextLabel, TestLiveLabels, TestLabelOwner[256], TestLabelText[256][100];
new TestLabelFailAt, bool:TestCPFails;
new TestEditorObject[TEST_PLAYERS], TestEditorLock[TEST_PLAYERS], TestCancelEdits[TEST_PLAYERS];
new bool:TestEditFails, TestTDCount[TEST_PLAYERS], TestNextTD;
new TestDialogTitle[TEST_PLAYERS][80], TestDialogBody[TEST_PLAYERS][3072];
new bool:TestSaveFails;
new bool:TestPickups[1024], TestNextPickup, TestLivePickups, TestPickupFailAt;
new TestPickupModels[1024], TestPickupTypes[1024], TestPickupWorld[1024], TestPickupInterior[1024];
new Float:TestPickupPosition[1024][3];

stock File:TestStorageOpen(const name[], filemode:mode)
{
    if (TestSaveFails && mode == io_write) return File:0;
    return fopen(name, mode);
}

stock TestCreatePickup(modelid, type, Float:x, Float:y, Float:z, worldid, interiorid)
{
    if (TestPickupFailAt && TestNextPickup + 1 == TestPickupFailAt) return 0;
    new pickupid = ++TestNextPickup;
    TestPickups[pickupid] = true; TestLivePickups++;
    TestPickupModels[pickupid] = modelid; TestPickupTypes[pickupid] = type;
    TestPickupWorld[pickupid] = worldid; TestPickupInterior[pickupid] = interiorid;
    TestPickupPosition[pickupid][0] = x; TestPickupPosition[pickupid][1] = y; TestPickupPosition[pickupid][2] = z;
    return pickupid;
}
stock TestPickupValid(pickupid) { return pickupid > 0 && pickupid < 1024 && TestPickups[pickupid]; }
stock TestDestroyPickup(pickupid)
{
    if (TestPickupValid(pickupid)) { TestPickups[pickupid] = false; TestLivePickups--; }
    return 1;
}

stock TestDialog(playerid, style, const title[], const body[], const accept[], const cancel[])
{
    format(TestDialogTitle[playerid], 80, "%s", title);
    format(TestDialogBody[playerid], 3072, "%s", body);
    return 1;
}
stock TestHideDialog(playerid) { TestDialogTitle[playerid][0] = EOS; return 1; }
stock TestCreateCP(Float:x, Float:y, Float:z, Float:size, worldid, interiorid, playerid, Float:streamdistance)
{
    if (TestCPFails) return 0;
    new cp = ++TestNextCP; TestCheckpoints[cp] = true; TestLiveCP++; TestCPOwner[cp] = playerid;
    TestCPPosition[cp][0] = x; TestCPPosition[cp][1] = y; TestCPPosition[cp][2] = z;
    return cp;
}
stock TestCPValid(cp) { return cp > 0 && cp < 256 && TestCheckpoints[cp]; }
stock TestDestroyCP(cp)
{
    if (TestCPValid(cp)) { TestCheckpoints[cp] = false; TestLiveCP--; }
    return 1;
}
stock TestCreateLabel(const text[], color, Float:x, Float:y, Float:z, Float:drawdistance,
    attachedplayer, attachedvehicle, testlos, worldid, interiorid, playerid)
{
    if (TestLabelFailAt && TestNextLabel + 1 == TestLabelFailAt) return 0;
    new label = ++TestNextLabel; TestLabels[label] = true; TestLiveLabels++; TestLabelOwner[label] = playerid;
    format(TestLabelText[label], 100, "%s", text);
    return label;
}
stock TestLabelValid(label) { return label > 0 && label < 256 && TestLabels[label]; }
stock TestDestroyLabel(label)
{
    if (TestLabelValid(label)) { TestLabels[label] = false; TestLiveLabels--; }
    return 1;
}
stock TestEditObject(playerid, objectid) { TestEditorObject[playerid] = objectid; return !TestEditFails; }
stock TestCancelEdit(playerid) { TestEditorObject[playerid] = 0; TestCancelEdits[playerid]++; return 1; }
stock TestSetPVar(playerid, const name[], value) { TestEditorLock[playerid] = value; return 1; }
stock TestDeletePVar(playerid, const name[]) { TestEditorLock[playerid] = 0; return 1; }
stock PlayerText:TestCreateTD(playerid, Float:x, Float:y, const text[])
{
    if (TestTDFails || (TestTDFailAt && TestNextTD + 1 == TestTDFailAt) || TestNextTD + 1 >= TEST_TEXTDRAWS) return PlayerText:INVALID_TEXT_DRAW;
    new id = ++TestNextTD; TestTDCount[playerid]++; TestTDAlive[id] = true;
    TestTDVisible[id] = false; TestTDOwner[id] = playerid;
    TestTDSelectable[id] = false; TestTDFonts[id] = TestTDModels[id] = 0;
    TestTDAlignment[id] = 1;
    TestTDPosition[id][0] = x; TestTDPosition[id][1] = y;
    format(TestTDText[id], 64, "%s", text);
    return PlayerText:id;
}
stock TestDestroyTD(playerid, PlayerText:id)
{
    if (_:id > 0 && _:id < sizeof(TestTDAlive) && TestTDAlive[_:id] && TestTDOwner[_:id] == playerid)
    { TestTDAlive[_:id] = false; TestTDVisible[_:id] = false; TestTDCount[playerid]--; }
    return 1;
}
stock TestTDString(playerid, PlayerText:id, const text[])
{
    format(TestTDText[_:id], 64, "%s", text); return 1;
}
stock TestDisconnect(playerid)
{
    // The native server destroys player textdraws when a client disconnects.
    for (new id = 1; id <= TestNextTD; id++)
        if (TestTDAlive[id] && TestTDOwner[id] == playerid) TestDestroyTD(playerid, PlayerText:id);
    TestConnected[playerid] = false;
    return 1;
}
stock TestTDSize(playerid, PlayerText:id, Float:x, Float:y)
{ TestTDLetterSize[_:id][0] = x; TestTDLetterSize[_:id][1] = y; return 1; }
stock TestTDAlign(playerid, PlayerText:id, alignment)
{ TestTDAlignment[_:id] = alignment; return 1; }
stock TestTDSet(playerid, PlayerText:id, value) { return 1; }
stock TestTDShow(playerid, PlayerText:id)
{
    if (TestTDSelectable[_:id] && !TestCatalogue[playerid]) TestSelectionOrderWrong = true;
    TestTDVisible[_:id] = true; return 1;
}
stock TestTDFont(playerid, PlayerText:id, font) { TestTDFonts[_:id] = font; return 1; }
stock TestTDSelectability(playerid, PlayerText:id, value) { TestTDSelectable[_:id] = value != 0; return 1; }
stock TestTDTextDimensions(playerid, PlayerText:id, Float:x, Float:y)
{ TestTDTextSize[_:id][0] = x; TestTDTextSize[_:id][1] = y; return 1; }
stock TestTDPreviewModel(playerid, PlayerText:id, modelid) { TestTDModels[_:id] = modelid; return 1; }
stock TestTDPreviewRot(playerid, PlayerText:id, Float:x, Float:y, Float:z, Float:zoom) { return 1; }
stock TestSelectTextDraw(playerid, colour)
{
    for (new id = 1; id <= TestNextTD; id++)
        if (TestTDAlive[id] && TestTDOwner[id] == playerid && TestTDSelectable[id] && TestTDVisible[id])
            TestSelectionOrderWrong = true;
    TestCatalogue[playerid] = true;
    return 1;
}
stock TestCancelSelectTextDraw(playerid)
{
    TestCatalogue[playerid] = false;
    TestCancelSelections[playerid]++;
    // Support both immediate and delayed native cancellation acknowledgements.
    if (TestDeferredSelection) TestSelectionEchoes[playerid]++;
    else PracticeHandleCatalogueEscape(playerid);
    return 1;
}

stock TestDeliverSelectionEcho(playerid)
{
    if (!TestSelectionEchoes[playerid]) return 0;
    TestSelectionEchoes[playerid]--;
    return PracticeHandleCatalogueEscape(playerid);
}

stock TestSendMessage(playerid, color, const message[])
{
    TestMessages[playerid]++;
    if (strlen(message) > TestLongestMessage) TestLongestMessage = strlen(message);
    strcat(TestChat[playerid], message, 8192); strcat(TestChat[playerid], "\n", 8192);
    format(TestLastMessage[playerid], 144, "%s", message);
    TestLastMessageColour[playerid] = color;
    return 1;
}
stock va_SendClientMessage(playerid, color, const message[], {Float,_}:...)
{
    return 1;
}
stock TestIsConnected(playerid) { return playerid >= 0 && playerid < TEST_PLAYERS && TestConnected[playerid]; }
stock TestIsNPC(playerid) { return TestNPC[playerid]; }
stock TestIsAdmin(playerid) { return TestAdmin[playerid]; }
stock TestGetState(playerid) { return TestState[playerid]; }
stock TestGetWeapon(playerid) { return TestWeapon[playerid]; }
stock TestGetWorld(playerid) { return TestWorld[playerid]; }
stock TestGetInterior(playerid) { return TestInterior[playerid]; }
stock TestGetHealth(playerid, &Float:health) { health = TestHealth[playerid]; return 1; }
stock TestGetPos(playerid, &Float:x, &Float:y, &Float:z)
{
    x = TestPosition[playerid][0]; y = TestPosition[playerid][1]; z = TestPosition[playerid][2]; return 1;
}
stock TestGetAngle(playerid, &Float:angle) { angle = 90.0; return 1; }
stock TestGetWeaponData(playerid, slot, &weaponid, &ammo)
{
    weaponid = TestSlotWeapon[playerid][slot]; ammo = TestAmmo[playerid][slot]; return 1;
}
stock TestInRange(playerid, Float:range, Float:x, Float:y, Float:z)
{
    x -= TestPosition[playerid][0]; y -= TestPosition[playerid][1]; z -= TestPosition[playerid][2];
    return x*x + y*y + z*z <= range*range;
}
stock TestGetTick() { return TestNow; }
stock TestCreateObject(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, worldid, interiorid, playerid = -1)
{
    if (TestCreateFails) return 0;
    TestObjects[++TestNextObject] = true;
    TestObjectModels[TestNextObject] = modelid; TestObjectOwner[TestNextObject] = playerid;
    TestObjectWorld[TestNextObject] = worldid; TestObjectInterior[TestNextObject] = interiorid;
    TestObjectPosition[TestNextObject][0] = x; TestObjectPosition[TestNextObject][1] = y; TestObjectPosition[TestNextObject][2] = z;
    TestObjectPosition[TestNextObject][3] = rx; TestObjectPosition[TestNextObject][4] = ry; TestObjectPosition[TestNextObject][5] = rz;
    TestMaterialColours[TestNextObject] = 0;
    TestMaterialText[TestNextObject][0] = EOS;
    TestStreamedObject = 0;
    TestLiveObjects++;
    return TestNextObject;
}
stock TestObjectValid(objectid) { return objectid > 0 && objectid < sizeof(TestObjects) && TestObjects[objectid]; }
stock TestDestroyObject(objectid)
{
    if (TestObjectValid(objectid)) { TestObjects[objectid] = false; TestLiveObjects--; }
    return 1;
}
stock TestStreamerUpdate(playerid, type = -1) { TestStreamedObject = TestNextObject; return 1; }
stock TestObjectVisible(playerid, type, objectid) { return TestVisible && TestObjectValid(objectid); }
stock TestStreamerID(playerid, type, internalid)
{
    new id = internalid - 1000 - playerid * 100;
    return TestObjectValid(id) ? id : 0;
}
stock TestSetMaterial(objectid, materialindex, const text[], materialsize, const fontface[], fontsize, bold,
    fontcolor, backcolor, textalignment)
{
    if (TestMaterialFails || !TestObjectValid(objectid)) return 0;
    TestMaterialColours[objectid] = backcolor;
    format(TestMaterialText[objectid], 16, "%s", text);
    return 1;
}
stock TestRoleplayName(playerid, name[], size = sizeof(name))
{
    format(name, size, "Test Player %d", playerid);
    return 1;
}
stock TestRandom(maximum)
{
    TestRandomCalls++;
    TestRandomSeed = TestRandomSeed * 1103515245 + 12345;
    return (TestRandomSeed >>> 1) % maximum;
}

#define SendClientMessage TestSendMessage
#define IsPlayerConnected TestIsConnected
#define IsPlayerNPC TestIsNPC
#define IsPlayerAdmin TestIsAdmin
#define GetPlayerState TestGetState
#define GetPlayerWeapon TestGetWeapon
#define GetPlayerVirtualWorld TestGetWorld
#define GetPlayerInterior TestGetInterior
#define GetPlayerHealth TestGetHealth
#define GetPlayerPos TestGetPos
#define GetPlayerFacingAngle TestGetAngle
#define GetPlayerWeaponData TestGetWeaponData
#define IsPlayerInRangeOfPoint TestInRange
#define GetTickCount TestGetTick
#define CreateDynamicObject TestCreateObject
#define IsValidDynamicObject TestObjectValid
#define DestroyDynamicObject TestDestroyObject
#define Streamer_Update TestStreamerUpdate
#define Streamer_IsItemVisible TestObjectVisible
#define Streamer_GetItemStreamerID TestStreamerID
#define SetDynamicObjectMaterialText TestSetMaterial
#define GetPlayerRoleplayName TestRoleplayName
#define PlayerPlaySound TestPlaySound
#define random TestRandom
#define HidePlayerDialog TestHideDialog
#define CreateDynamicPickup TestCreatePickup
#define DestroyDynamicPickup TestDestroyPickup
#define IsValidDynamicPickup TestPickupValid
#define CreateDynamicCP TestCreateCP
#define DestroyDynamicCP TestDestroyCP
#define IsValidDynamicCP TestCPValid
#define CreateDynamic3DTextLabel TestCreateLabel
#define DestroyDynamic3DTextLabel TestDestroyLabel
#define IsValidDynamic3DTextLabel TestLabelValid
#define EditDynamicObject TestEditObject
#define CancelEdit TestCancelEdit
#define SetPVarInt TestSetPVar
#define DeletePVar TestDeletePVar
#define CreatePlayerTextDraw TestCreateTD
#define PlayerTextDrawDestroy TestDestroyTD
#define PlayerTextDrawLetterSize TestTDSize
#define PlayerTextDrawFont TestTDFont
#define PlayerTextDrawColor TestTDSet
#define PlayerTextDrawSetOutline TestTDSet
#define PlayerTextDrawShow TestTDShow
#define PlayerTextDrawAlignment TestTDAlign
#define PlayerTextDrawSetString TestTDString
#define PlayerTextDrawTextSize TestTDTextDimensions
#define PlayerTextDrawSetShadow TestTDSet
#define PlayerTextDrawBackgroundColor TestTDSet
#define PlayerTextDrawSetProportional TestTDSet
#define PlayerTextDrawSetSelectable TestTDSelectability
#define PlayerTextDrawUseBox TestTDSet
#define PlayerTextDrawBoxColor TestTDSet
#define PlayerTextDrawSetPreviewModel TestTDPreviewModel
#define PlayerTextDrawSetPreviewRot TestTDPreviewRot
#define SelectTextDraw TestSelectTextDraw
#define CancelSelectTextDraw TestCancelSelectTextDraw

#include "../gamemodes/features/shooting_practice/config.inc"
#include "../gamemodes/features/shooting_practice/registry.inc"
#include "../gamemodes/features/shooting_practice/sequence.inc"
// Capture the typed result at the presentation boundary, then run the actual
// formatter. Scoring assertions do not depend on translated chat wording.
#define PracticeReportResult TestRecordResult
#define PracticeIntegrationBeforeStart TestIntegrationStart
#define PracticeIntegrationOnEnd TestIntegrationEnd
#include "../gamemodes/features/shooting_practice/session.inc"
#undef PracticeReportResult
#undef PracticeIntegrationBeforeStart
#undef PracticeIntegrationOnEnd
#define PracticeSendResultLine TestSendResultMessage
#include "../gamemodes/features/shooting_practice/presentation.inc"
#undef PracticeSendResultLine
#include "../gamemodes/features/shooting_practice/editor.inc"
#define fopen TestStorageOpen
forward bool:TestRepositorySave();
#define PracticeRepositorySave TestRepositorySave
#include "../gamemodes/features/shooting_practice/storage.inc"
#undef fopen
#undef PracticeRepositorySave
#include "../gamemodes/features/shooting_practice/integration.inc"
#include "../gamemodes/features/shooting_practice/setup.inc"
#include "../gamemodes/features/shooting_practice/catalogue.inc"
#include "../gamemodes/features/shooting_practice/start_setup.inc"
#include "../gamemodes/features/shooting_practice/zone_setup.inc"
#include "../gamemodes/features/shooting_practice/discovery.inc"

new TestIntegrationFinal[E_PRACTICE_SESSION], TestIntegrationOrigin[E_PRACTICE_POINT];
new TestIntegrationModels[PRACTICE_MAX_TARGETS], TestIntegrationSlots[PRACTICE_MAX_TARGETS];
new bool:TestIntegrationTypes[PRACTICE_MAX_TARGETS];

stock bool:TestRepositorySave()
{
    TestRepositorySaves++;
    return PracticeRepositorySave();
}
stock bool:TestIntegrationStart(zone, const session[E_PRACTICE_SESSION],
    const origin[E_PRACTICE_POINT], startSlot, const slots[], const models[], const bool:noShoot[])
{
    TestIntegrationStarts++;
    TestIntegrationStartSlot = startSlot;
    if (TestIntegrationReject) return false;
    return PracticeIntegrationBeforeStart(zone, session, origin, startSlot, slots, models, noShoot);
}
stock E_PRACTICE_HISTORY_STATUS:TestIntegrationEnd(zone, bool:completed, const reason[],
    const session[E_PRACTICE_SESSION], const origin[E_PRACTICE_POINT], startSlot,
    const slots[], const models[], const bool:noShoot[])
{
    TestIntegrationEnds++;
    TestIntegrationEndSlot = startSlot;
    TestIntegrationFinal = session;
    TestIntegrationOrigin = origin;
    for (new i = 0; i < session[PracticeTotal]; i++)
    {
        TestIntegrationSlots[i] = slots[i];
        TestIntegrationModels[i] = models[i];
        TestIntegrationTypes[i] = noShoot[i];
    }
    return PracticeIntegrationOnEnd(zone, completed, reason, session, origin, startSlot, slots, models, noShoot);
}

stock TestSendResultMessage(playerid, color, const message[])
{
    if (TestPrivateMessages[playerid] < 8)
        format(TestReportLines[playerid][TestPrivateMessages[playerid]], 144, "%s", message);
    TestPrivateMessages[playerid]++;
    new bool:divider = strfind(message, "----------") == 0 || !strcmp(message, "CAMPO DE TIRO");
    if (color != (divider ? PRACTICE_SECTION_COLOUR : COLOR_WHITE)) TestResultColourWrong = true;
    return TestSendMessage(playerid, color, message);
}
stock TestRecordResult(playerid, bool:completed, const result[E_PRACTICE_SESSION], const reason[])
{
    TestResult = completed ? (PracticeResultPassed(result) ? 1 : 2) : 3;
    TestResultShots = result[PracticeShots];
    TestResultHits = result[PracticeHits];
    TestResultExpired = result[PracticeExpired];
    TestResultIgnored = result[PracticeCorrectIgnores];
    TestResultErrors = result[PracticeIdentificationErrors];
    TestResultShootTotal = result[PracticeShootTotal];
    TestResultNoShootTotal = result[PracticeNoShootTotal];
    TestResultPoints = PracticeResultPoints(result);
    return PracticeReportResult(playerid, completed, result, reason);
}

stock TestPlaySound(playerid, soundid, Float:x, Float:y, Float:z)
{
    if (TestSoundCount[playerid] < 128)
    {
        TestSounds[playerid][TestSoundCount[playerid]] = soundid;
        TestSoundTimes[playerid][TestSoundCount[playerid]] = TestNow;
    }
    TestSoundCount[playerid]++;
    if (soundid == PRACTICE_SOUND_TARGET)
    {
        new bool:ready;
        for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
            if (PracticeIsBusy(zone) && PracticeSession[zone][PracticeObject] == TestStreamedObject
                && TestObjectValid(TestStreamedObject) && TestVisible
                && (TestObjectModels[TestStreamedObject] != PRACTICE_TARGET_MODEL || TestMaterialColours[TestStreamedObject])) ready = true;
        if (!ready) TestCueBeforeReady = true;
    }
    return 1;
}

stock Check(bool:condition, const label[])
{
    TestAssertions++;
    if (!condition) { TestFailures++; printf("[Practice tests] FAIL: %s", label); }
}
stock ResetTest(targets = 3)
{
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++) PracticeCleanup(zone);
    new bool:noTextdraws = true;
    for (new playerid = 0; playerid < TEST_PLAYERS; playerid++)
    {
        PracticeResetUI(playerid);
        noTextdraws = noTextdraws && !TestTDCount[playerid];
    }
    Check(noTextdraws, "cleanup releases every owned textdraw");
    Check(!TestSelectionOrderWrong, "selection is configured before selectable textdraws are shown");
    PracticeResetDiscovery();
    Check(TestLiveObjects == 0 && !TestLiveCP && !TestLiveLabels && !TestLivePickups, "cleanup leaves no objects, checkpoints or labels");
    PracticeInitializeRegistry();
    PracticeCreateZone("Campo de tiro");
    PracticeZoneEnabled[0] = true;
    PracticeRevision[0] = 0;
    for (new playerid = 0; playerid < TEST_PLAYERS; playerid++)
    {
        PracticeSelectedZone[playerid] = 0;
        PracticeSelectedZoneID[playerid] = 1;
        PracticeMenuPage[playerid] = SP_MENU_NONE;
        PracticePlayerZone[playerid] = -1;
        TestDialogTitle[playerid][0] = TestDialogBody[playerid][0] = EOS;
    }
    fremove(PRACTICE_CONFIG_FILE); fremove(PRACTICE_CONFIG_TEMP); fremove(PRACTICE_CONFIG_BACKUP);
    new empty[E_PRACTICE_POINT];
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        PracticePoints[0][slot] = empty;
        if (slot <= targets)
        {
            PracticePoints[0][slot][PracticePointSet] = true;
            PracticePoints[0][slot][PracticePointX] = float(slot * 8);
            if (slot) PracticePoints[0][slot][PracticePointModel] = PRACTICE_TARGET_MODEL;
        }
    }
    new defaults[E_PRACTICE_RULE] = {PRACTICE_WEAPON_DEFAULT, 1, 3, 5000, 1000, 0, 0, 0};
    PracticeRules[0] = defaults;
    for (new playerid = 0; playerid < TEST_PLAYERS; playerid++)
    {
        TestConnected[playerid] = true; TestAdmin[playerid] = true; TestNPC[playerid] = false;
        TestState[playerid] = PLAYER_STATE_ONFOOT; TestWeapon[playerid] = 24;
        for (new slot = 0; slot < 13; slot++) { TestSlotWeapon[playerid][slot] = 0; TestAmmo[playerid][slot] = 0; }
        TestSlotWeapon[playerid][2] = 24; TestAmmo[playerid][2] = 30;
        TestWorld[playerid] = 0; TestInterior[playerid] = 0; TestHealth[playerid] = 100.0;
        TestPosition[playerid][0] = TestPosition[playerid][1] = TestPosition[playerid][2] = 0.0;
        TestMessages[playerid] = TestPrivateMessages[playerid] = TestSoundCount[playerid] = 0;
        TestCatalogue[playerid] = false; TestCancelSelections[playerid] = 0;
        TestSelectionEchoes[playerid] = PracticeCatalogueCancelEchoes[playerid] = 0;
        TestLastMessage[playerid][0] = TestChat[playerid][0] = EOS;
        playerEditingObjectID[playerid] = -1; TestCancelEdits[playerid] = 0;
    }
    TestIntegrationStarts = TestIntegrationEnds = TestRepositorySaves = 0;
    TestIntegrationReject = false; TestIntegrationStartSlot = TestIntegrationEndSlot = -1;
    TestNow = 1000; TestNextObject = 0; TestCreateFails = false; TestVisible = true;
    TestResult = 0; TestResultShots = 0; TestResultHits = 0; TestResultExpired = 0;
    TestResultIgnored = TestResultErrors = TestResultShootTotal = TestResultNoShootTotal = 0;
    TestResultPoints = 0;
    TestMaterialFails = false; TestCueBeforeReady = false; TestStreamedObject = 0;
    TestRandomSeed = 42; TestRandomCalls = 0;
    TestNextCP = TestNextLabel = TestNextTD = TestLabelFailAt = TestTDFailAt = 0;
    TestSelectionOrderWrong = TestDeferredSelection = false;
    TestNextPickup = TestPickupFailAt = 0;
    TestCPFails = TestEditFails = TestSaveFails = TestTDFails = false;
    TestLongestMessage = 0; TestResultColourWrong = false;
}
stock ShowFirstTarget()
{
    PracticeStart(0, 0);
    TestNow += PracticeRules[0][PracticePreparation] * 1000;
    PracticeTick(0, TestNow);
    Check(PracticeSession[0][PracticePhase] == PRACTICE_TARGET, "first target appears after countdown");
}
stock Shoot(bool:hit, playerid = 0)
{
    PracticeHandleShot(playerid, 24, hit ? BULLET_HIT_TYPE_PLAYER_OBJECT : BULLET_HIT_TYPE_NONE,
        1000 + playerid * 100 + PracticeSession[0][PracticeObject], TestNow);
}
stock AdvanceInterval()
{
    TestNow += PracticeRules[0][PracticeInterval];
    PracticeTick(0, TestNow);
}

stock bool:TestSequenceValid(const slots[PRACTICE_MAX_TARGETS], const bool:noShoot[PRACTICE_MAX_TARGETS], total)
{
    if (total != PracticeCountTargets(0)) return false;
    new visited, noShootCount;
    for (new index = 0; index < total; index++)
    {
        new slot = slots[index];
        if (slot < 1 || slot > PRACTICE_MAX_TARGETS || !PracticePoints[0][slot][PracticePointSet]) return false;
        if (visited & (1 << slot)) return false;
        visited |= 1 << slot;
        if (noShoot[index]) noShootCount++;
    }
    for (new index = total; index < PRACTICE_MAX_TARGETS; index++)
        if (slots[index] || noShoot[index]) return false;
    return noShootCount == PracticeCountNoShootTargets(0);
}

// Legacy score scenarios place the no-shoot targets last. Other tests below
// exercise arbitrary per-position types through the real administrator commands.
stock TestSetNoShoot(count)
{
    for (new slot = PRACTICE_MAX_TARGETS; slot > 0; slot--)
    {
        PracticePoints[0][slot][PracticePointNoShoot] = false;
        if (PracticePoints[0][slot][PracticePointSet] && count > 0)
        {
            PracticePoints[0][slot][PracticePointNoShoot] = true;
            count--;
        }
    }
}

stock TestWriteConfigFixture(const header[], const rules[], types = 0, invalidSlot = -1, invalidType = 2, positions = 14)
{
    // Each parser case deliberately has no recovery file; recovery has its own suite.
    fremove(PRACTICE_CONFIG_BACKUP); fremove(PRACTICE_CONFIG_TEMP);
    new File:fixture = fopen(PRACTICE_CONFIG_FILE, io_write);
    fwrite(fixture, header); fwrite(fixture, "\r\n");
    fwrite(fixture, rules); fwrite(fixture, "\r\n");
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        new line[160], marked = !slot || (positions & (1 << slot));
        new Float:x = !marked ? 0.0 : (slot == 1 ? 8.25 : float(slot * 8));
        format(line, sizeof(line), "%d %.6f %.6f %.6f %.6f 0 0", marked, x,
            slot == 1 ? 1.0 : 0.0, slot == 1 ? 2.0 : 0.0, slot == 1 ? 90.5 : 0.0);
        fwrite(fixture, line);
        if ((!strcmp(header, "SARP_PRACTICE 3") || !strcmp(header, "SARP_PRACTICE 4") || !strcmp(header, "SARP_PRACTICE 5") || !strcmp(header, "SARP_PRACTICE 6"))
            && !(slot == invalidSlot && invalidType == -1))
        {
            new pointType = slot == invalidSlot ? invalidType : ((types & (1 << slot)) ? 1 : 0);
            format(line, sizeof(line), " %d", pointType); fwrite(fixture, line);
        }
        if (!strcmp(header, "SARP_PRACTICE 4") || !strcmp(header, "SARP_PRACTICE 5") || !strcmp(header, "SARP_PRACTICE 6"))
        {
            format(line, sizeof(line), " %d 15.250000 36.500000\r\nNAME %s",
                slot && marked ? 1583 : 0, slot && marked ? ("Entrada lateral") : (""));
            fwrite(fixture, line);
        }
        fwrite(fixture, "\r\n");
    }
    fwrite(fixture, "END\r\n");
    fclose(fixture);
}

stock TestPresentation()
{
    ResetTest();
    TestPosition[2][0] = 50.0; TestWorld[3] = 1; TestInterior[4] = 1;
    PracticeStart(0, 0);
    Check(TestSoundCount[0] == 1 && TestSounds[0][0] == PRACTICE_SOUND_COUNTDOWN, "initial countdown beep");
    Check(TestSoundCount[1] == 1 && TestSoundCount[2] == 0 && TestSoundCount[3] == 0 && TestSoundCount[4] == 0,
        "sounds filtered by distance, world and interior");
    Check(TestMessages[0] == 1 && TestMessages[1] == 1 && !TestMessages[2] && !TestMessages[3] && !TestMessages[4],
        "one nearby roleplay start, no private start chatter");
    Check(strfind(TestLastMessage[0], "* Test Player 0 se prepara") == 0, "start uses roleplay name and action format");
    TestNow = 1500; PracticeTick(0, TestNow);
    Check(TestSoundCount[0] == 1, "half-second tick does not duplicate countdown");
    TestNow = 2000; PracticeTick(0, TestNow); PracticeTick(0, TestNow);
    Check(TestSoundCount[0] == 2 && TestSounds[0][1] == PRACTICE_SOUND_COUNTDOWN, "second countdown cue once");
    TestNow = 3000; PracticeTick(0, TestNow);
    Check(TestSoundCount[0] == 3 && TestSounds[0][2] == PRACTICE_SOUND_COUNTDOWN, "third countdown cue");
    TestNow = 4000; PracticeTick(0, TestNow);
    Check(TestSoundCount[0] == 4 && TestSounds[0][3] == PRACTICE_SOUND_TARGET, "first appearance is the sole start signal");
    Check(!TestCueBeforeReady, "target cue follows markings and streaming");
    Check(TestMaterialColours[PracticeSession[0][PracticeObject]] == PRACTICE_SHOOT_MATERIAL
        && !strcmp(TestMaterialText[PracticeSession[0][PracticeObject]], "DISPARAR"), "shoot target is red and labelled");
    Shoot(true);
    Check(TestSoundCount[0] == 4 && TestMessages[0] == 1, "hits produce no chat or extra sound");
    AdvanceInterval();
    Check(TestSoundCount[0] == 5 && TestSounds[0][4] == PRACTICE_SOUND_TARGET, "each later target has one appearance cue");
    TestCommand_practica(0, "cancelar");
    Check(TestSoundCount[0] == 6 && TestSounds[0][5] == PRACTICE_SOUND_END, "cancellation closes with a distinct cue");
    Check(TestPrivateMessages[0] == 4 && !TestPrivateMessages[1] && TestMessages[1] == 2, "only participant receives interruption reason");
    TestNow += 60000; PracticeTick(0, TestNow); PracticeEnd(0, false, "repeat");
    Check(TestSoundCount[0] == 6 && TestLiveObjects == 0, "no stale beeps or duplicate closure after cancellation");
    PracticeStart(0, 0);
    Check(TestSoundCount[0] == 7 && TestSounds[0][6] == PRACTICE_SOUND_COUNTDOWN, "immediate retry starts a fresh countdown");

    ResetTest(1); PracticeRules[0][PracticePreparation] = 5; PracticeStart(0, 0);
    for (new second = 1; second <= 5; second++) { TestNow += 1000; PracticeTick(0, TestNow); }
    Check(TestSoundCount[0] == 6 && TestSounds[0][4] == PRACTICE_SOUND_COUNTDOWN
        && TestSounds[0][5] == PRACTICE_SOUND_TARGET, "countdown respects configured duration");
    Shoot(true);
    Check(TestMessages[0] == 10 && TestPrivateMessages[0] == 8 && TestMessages[1] == 2,
        "normal round has start/end roleplay plus eight private result lines");
    Check(strfind(TestChat[0], "Veredicto: {9EC73D}Aprobado{FFFFFF}", true) != -1
        && strfind(TestChat[0], "Precisión: 100.0%") != -1 && TestLastMessageColour[0] == PRACTICE_SECTION_COLOUR,
        "only approved word is green, label and trailing score are white, accuracy has a percent sign");
    Check(TestLongestMessage < 144, "each result line fits the chat limit");

    ResetTest(1); PracticeStart(0, 0); TestNow += 2500; PracticeTick(0, TestNow);
    Check(TestSoundCount[0] == 2, "delayed tick skips missed countdown cues instead of bursting");
    TestNow += 500; PracticeTick(0, TestNow);
    Check(PracticeSession[0][PracticePhase] == PRACTICE_COUNTDOWN, "late last countdown cue gets a full second before start");
    TestNow += 500; PracticeTick(0, TestNow); Shoot(true);
    Check(TestSoundCount[0] == 4 && !TestCueBeforeReady, "late tick still produces one appearance and one closure");
    ResetTest(1); TestMaterialFails = true; ShowFirstTargetFailure();
    Check(TestResult == 3 && !TestLiveObjects && TestSoundCount[0] == 2
        && TestSounds[0][1] == PRACTICE_SOUND_END, "failed markings abort without exposing an unidentified target");
    ResetTest(1); TestVisible = false; ShowFirstTargetFailure();
    Check(TestResult == 3 && !TestCueBeforeReady && TestSoundCount[0] == 2, "streaming failure never plays appearance cue");

    ResetTest(); ShowFirstTarget(); TestConnected[0] = false; PracticeTick(0, TestNow);
    Check(TestSoundCount[0] == 2 && TestSoundCount[1] == 3 && TestMessages[1] == 2,
        "disconnect closes the station for nearby observers without messaging the disconnected player");
    ResetTest(); ShowFirstTarget(); TestWorld[0] = 1; PracticeTick(0, TestNow);
    Check(TestSoundCount[0] == 2 && TestSoundCount[1] == 3, "closure remains anchored to original station dimension");
}

stock ShowFirstTargetFailure()
{
    PracticeStart(0, 0); TestNow += PracticeRules[0][PracticePreparation] * 1000; PracticeTick(0, TestNow);
}

stock TestNoShootScoring()
{
    ResetTest(3); TestSetNoShoot(1); ShowFirstTarget();
    Shoot(true); AdvanceInterval(); TestNow += 5000; PracticeTick(0, TestNow); AdvanceInterval();
    Check(PracticeSession[0][PracticeNoShoot] && TestMaterialColours[PracticeSession[0][PracticeObject]] == PRACTICE_NO_SHOOT_MATERIAL
        && !strcmp(TestMaterialText[PracticeSession[0][PracticeObject]], "NO DISPARAR"), "no-shoot target is blue with a distinct label");
    Check(TestSounds[0][TestSoundCount[0] - 1] == PRACTICE_SOUND_TARGET, "no-shoot appearance uses identical sound");
    TestNow += 5000; PracticeTick(0, TestNow);
    Check(TestResult == 1 && TestResultHits == 1 && TestResultExpired == 1 && TestResultIgnored == 1 && !TestResultErrors,
        "ignoring no-shoot is correct, not an omission");
    Check(TestResultHits + TestResultExpired == TestResultShootTotal
        && TestResultIgnored + TestResultErrors == TestResultNoShootTotal, "completed mixed round partitions both target types");

    ResetTest(3); TestSetNoShoot(2); ShowFirstTarget(); Shoot(true); AdvanceInterval();
    Shoot(true, 1);
    Check(!PracticeSession[0][PracticeIdentificationErrors] && TestLiveObjects == 1, "bystander cannot penalize no-shoot target");
    Shoot(true);
    Check(PracticeIsBusy(0) && PracticeSession[0][PracticeHits] == 1 && PracticeSession[0][PracticeIdentificationErrors] == 1,
        "no-shoot impact adds one error, no point, and continues the exercise");
    Shoot(true);
    Check(PracticeSession[0][PracticeIdentificationErrors] == 1 && PracticeSession[0][PracticeShots] == 3,
        "repeat impact in interval does not duplicate identification error");
    AdvanceInterval(); TestNow += 5000; PracticeTick(0, TestNow);
    Check(TestResult == 2 && TestResultHits == 1 && TestResultErrors == 1 && TestResultIgnored == 1,
        "identification error fails default zero-error goal even with sufficient hits");
    Check(strfind(TestChat[0], "Precisión: 33.3%") != -1 && strfind(TestChat[0], "Errores de identificación: 1") != -1,
        "all shots remain in precision denominator and summary includes errors");

    ResetTest(3); TestSetNoShoot(1); PracticeRules[0][PracticeMaxErrors] = 1;
    ShowFirstTarget(); Shoot(true); AdvanceInterval(); Shoot(true); AdvanceInterval(); Shoot(true);
    Check(TestResult == 1 && TestResultErrors == 1 && TestResultHits == 2 && TestResultPoints == 1,
        "configured error allowance still requires sufficient net points");
    ResetTest(2); TestSetNoShoot(1); PracticeRules[0][PracticeMaxErrors] = 1;
    ShowFirstTarget(); Shoot(true); AdvanceInterval(); Shoot(false);
    Check(TestResult == 2 && TestResultErrors == 1 && TestResultPoints == 0 && TestResultShots == 2,
        "air shot during no-shoot subtracts a point and can fail even within error allowance");
    ResetTest(2); TestSetNoShoot(1); ShowFirstTarget();
    TestNow += 5000; PracticeTick(0, TestNow); AdvanceInterval(); Shoot(false);
    Check(TestResult == 2 && TestResultPoints == -1 && TestResultExpired == 1 && TestResultErrors == 1
        && strfind(TestChat[0], "Puntuación neta: -1") != -1, "negative score is retained and reported for trigger error without any hits");
    ResetTest(2); TestSetNoShoot(1); ShowFirstTarget(); Shoot(true); AdvanceInterval();
    PracticeHandleShot(0, 24, BULLET_HIT_TYPE_OBJECT, 999, TestNow);
    Check(TestResultErrors == 1 && TestResultPoints == 0 && !TestLiveObjects,
        "shot at unrelated scenery during no-shoot resolves the presentation as an error");
    ResetTest(2); TestSetNoShoot(1); ShowFirstTarget(); Shoot(true); AdvanceInterval();
    TestNow += 5000; Shoot(true);
    Check(TestResult == 1 && TestResultIgnored == 1 && !TestResultErrors && TestResultShots == 1,
        "hit at final no-shoot deadline cannot turn correct restraint into an error");
    ResetTest(3); TestSetNoShoot(2); ShowFirstTarget(); Shoot(true); AdvanceInterval();
    TestNow += 5000; Shoot(true);
    Check(PracticeSession[0][PracticeCorrectIgnores] == 1 && !PracticeSession[0][PracticeIdentificationErrors]
        && PracticeSession[0][PracticeShots] == 2, "late intermediate no-shoot hit counts only as a gap shot");
    TestCommand_practica(0, "cancelar");
    Check(TestResult == 3 && TestResultIgnored == 1 && !TestResultErrors && !TestResultExpired,
        "cancellation never classifies unpresented targets");
    ResetTest(2); TestSetNoShoot(1); ShowFirstTarget();
    Shoot(false); TestNow += 5000; PracticeTick(0, TestNow); AdvanceInterval(); TestNow += 5000; PracticeTick(0, TestNow);
    Check(TestResult == 2 && !TestResultHits && TestResultExpired == 1 && TestResultIgnored == 1,
        "ignoring every target does not satisfy required shoot hits");
}

stock TestRandomSequences()
{
    new slots[PRACTICE_MAX_TARGETS], bool:noShoot[PRACTICE_MAX_TARGETS], models[PRACTICE_MAX_TARGETS];
    ResetTest(5); PracticePoints[0][2][PracticePointSet] = false;
    PracticePoints[0][1][PracticePointNoShoot] = PracticePoints[0][4][PracticePointNoShoot] = true;
    new total = PracticeBuildSequence(0, slots, noShoot, models);
    Check(total == 4 && slots[0] == 1 && slots[1] == 3 && slots[2] == 4 && slots[3] == 5
        && noShoot[0] && !noShoot[1] && noShoot[2] && !noShoot[3], "fixed sparse sequence respects arbitrary assigned types");
    Check(!TestRandomCalls, "fixed configuration consumes no randomness");
    PracticeRules[0][PracticeRandomOrder] = 1;
    new bool:orderChanged, bool:valid = true;
    for (new seed = 1; seed <= 20; seed++)
    {
        TestRandomSeed = seed; total = PracticeBuildSequence(0, slots, noShoot, models);
        valid = valid && TestSequenceValid(slots, noShoot, total);
        for (new index = 0; index < total; index++) valid = valid && (noShoot[index] == (slots[index] == 1 || slots[index] == 4));
        if (slots[0] != 1 || slots[1] != 3 || slots[2] != 4 || slots[3] != 5) orderChanged = true;
    }
    Check(valid && orderChanged, "random order preserves unique slots, exact quota and fixed position types");
    PracticeRules[0][PracticeRandomOrder] = 0; PracticeRules[0][PracticeRandomTypes] = 1;
    new bool:typesChanged; valid = true;
    for (new seed = 1; seed <= 20; seed++)
    {
        TestRandomSeed = seed; total = PracticeBuildSequence(0, slots, noShoot, models);
        valid = valid && TestSequenceValid(slots, noShoot, total)
            && slots[0] == 1 && slots[1] == 3 && slots[2] == 4 && slots[3] == 5;
        if (!noShoot[0] || noShoot[1]) typesChanged = true;
    }
    Check(valid && typesChanged, "random types redistribute exact quota while order stays fixed");
    PracticeRules[0][PracticeRandomOrder] = 1;
    valid = true;
    for (new seed = 1; seed <= 50; seed++)
    {
        TestRandomSeed = seed; total = PracticeBuildSequence(0, slots, noShoot, models);
        valid = valid && TestSequenceValid(slots, noShoot, total);
    }
    Check(valid, "both random options preserve every position once and exact type quantities");
    Check(PracticePoints[0][1][PracticePointNoShoot] && PracticePoints[0][4][PracticePointNoShoot]
        && !PracticePoints[0][3][PracticePointNoShoot] && !PracticePoints[0][5][PracticePointNoShoot],
        "random rounds never overwrite saved per-position assignments");
    PracticeRules[0][PracticeRandomTypes] = 0; PracticeRules[0][PracticeRandomOrder] = 0;
    PracticeBuildSequence(0, slots, noShoot, models);
    Check(noShoot[0] && !noShoot[1] && noShoot[2] && !noShoot[3], "switching back to fixed restores saved types");
    PracticeRules[0][PracticeRandomTypes] = PracticeRules[0][PracticeRandomOrder] = 1;
    PracticeStart(0, 0); new callsAtStart = TestRandomCalls;
    TestNow += 3000; PracticeTick(0, TestNow);
    for (new index = 0; index < 4; index++)
    {
        if (PracticeSession[0][PracticeNoShoot]) { TestNow += 5000; PracticeTick(0, TestNow); }
        else Shoot(true);
        if (index < 3) AdvanceInterval();
    }
    Check(TestResult == 1 && TestResultHits == 2 && TestResultIgnored == 2 && TestRandomCalls == callsAtStart,
        "mixed random round follows its frozen plan without rerolling during callbacks");
    Check(!PracticeSequenceSlots[0][0] && !PracticeSequenceNoShoot[0][0], "cleanup clears sequence before player ID reuse");
}

#include "shooting_practice_ui.inc"
#include "shooting_practice_catalogue.inc"
#include "shooting_practice_integration.inc"
#include "shooting_practice_polish.inc"

stock TestStorageMigration()
{
    ResetTest(3); TestSetNoShoot(1); PracticeRules[0][PracticeRandomOrder] = 1;
    TestWriteConfigFixture("SARP_PRACTICE 1", "24 2 3 5 1");
    Check(PracticeLoadConfiguration() && PracticeRules[0][PracticePassPoints] == 2
        && !PracticeCountNoShootTargets(0) && !PracticeRules[0][PracticeMaxErrors]
        && !PracticeRules[0][PracticeRandomOrder] && !PracticeRules[0][PracticeRandomTypes]
        && PracticePoints[0][1][PracticePointX] == 8.25 && PracticePoints[0][1][PracticePointAngle] == 90.5,
        "legacy v1 preserves positions/rules and assigns all targets to shoot");
    Check(PracticeSaveConfiguration(), "legacy layout can be upgraded on next save");
    new File:input = fopen(PRACTICE_CONFIG_FILE, io_read), header[32];
    PracticeReadLine(input, header, sizeof(header)); fclose(input);
    Check(!strcmp(header, "SARP_PRACTICE 9"), "subsequent save upgrades storage to version 9");
    TestWriteConfigFixture("SARP_PRACTICE 2", "24 1 3 5 1 1 0 2 0");
    Check(!PracticeLoadConfiguration() && PracticeRules[0][PracticePassPoints] == 2 && !PracticeRules[0][PracticeRandomOrder],
        "invalid stored random flag cannot partially replace configuration");
    TestWriteConfigFixture("SARP_PRACTICE 2", "24 1 3 5 1 1 2 0 0");
    Check(!PracticeLoadConfiguration(), "legacy error allowance beyond quota is rejected");
    TestWriteConfigFixture("SARP_PRACTICE 2", "24 1 3 5 1 -1 0 0 0");
    Check(!PracticeLoadConfiguration(), "stored negative legacy quota is rejected");
    TestWriteConfigFixture("SARP_PRACTICE 2", "24 1 3 5 1");
    Check(!PracticeLoadConfiguration(), "v2 cannot silently accept missing fields");
    TestWriteConfigFixture("SARP_PRACTICE 2", "24 1 3 5 1 2 1 1 1", .positions = (1 << 1) | (1 << 4) | (1 << 9));
    Check(PracticeLoadConfiguration() && !PracticePoints[0][1][PracticePointNoShoot]
        && PracticePoints[0][4][PracticePointNoShoot] && PracticePoints[0][9][PracticePointNoShoot]
        && !PracticePoints[0][3][PracticePointSet] && PracticePoints[0][1][PracticePointX] == 8.25
        && PracticeRules[0][PracticeMaxErrors] == 1 && PracticeRules[0][PracticeRandomOrder] && PracticeRules[0][PracticeRandomTypes],
        "v2 migrates highest marked sparse slots and retains randomization preferences");
    Check(PracticeSaveConfiguration() && PracticeLoadConfiguration() && PracticeCountNoShootTargets(0) == 2
        && PracticePoints[0][4][PracticePointNoShoot] && PracticePoints[0][9][PracticePointNoShoot],
        "v2-to-v5 upgrade preserves the migrated assignment");
    TestWriteConfigFixture("SARP_PRACTICE 2", "24 1 3 5 1 3 2 0 0", .positions = 2);
    Check(PracticeLoadConfiguration() && PracticePoints[0][1][PracticePointNoShoot] && PracticeCountTargets(0) == 1,
        "incomplete legacy layouts retain marked positions even when old quota exceeds them");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 0 0 0", 2);
    Check(PracticeLoadConfiguration() && PracticePoints[0][1][PracticePointNoShoot]
        && !PracticePoints[0][3][PracticePointNoShoot], "v3 explicitly supports low-numbered no-shoot positions");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 0 0 0", 0, 2, 2);
    Check(!PracticeLoadConfiguration() && PracticePoints[0][1][PracticePointNoShoot] && !PracticePoints[0][2][PracticePointNoShoot],
        "invalid stored type rejects entire file without partial application");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 0 0 0", 0, 2, -1);
    Check(!PracticeLoadConfiguration(), "missing v3 type is not silently treated as shoot");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 0 0 0", 1);
    Check(!PracticeLoadConfiguration(), "station start cannot be a no-shoot target");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 0 0 0", 1 << 4);
    Check(!PracticeLoadConfiguration(), "unmarked slot cannot hold a hidden no-shoot assignment");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 0 0");
    Check(!PracticeLoadConfiguration(), "v3 requires every rule field");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 0 2 0");
    Check(!PracticeLoadConfiguration(), "v3 validates random flags");
    TestWriteConfigFixture("SARP_PRACTICE 3", "24 1 3 5 1 11 0 0");
    Check(!PracticeLoadConfiguration(), "v3 rejects out-of-bounds error allowance");
}

#include "shooting_practice_refinements.inc"
#include "shooting_practice_feedback.inc"
#include "shooting_practice_zones.inc"
#include "shooting_practice_starts.inc"
#include "shooting_practice_pause.inc"
#include "shooting_practice_discovery.inc"

main() {}
public OnGameModeInit()
{
    // Valid starts, occupancy, exact shot accounting and one point per presentation.
    ResetTest();
    PracticeStart(0, 0);
    Shoot(false);
    Check(PracticeSession[0][PracticeShots] == 0, "countdown shots excluded");
    PracticeStart(0, 1);
    Check(PracticeSession[0][PracticePlayer] == 0, "second player cannot steal station");
    PracticeStart(0, 0);
    Check(PracticeSession[0][PracticePhaseStarted] == 1000, "duplicate start does not restart countdown");
    TestNow = 4000; PracticeTick(0, TestNow);
    Shoot(true, 1);
    Check(PracticeSession[0][PracticeHits] == 0 && TestLiveObjects == 1, "bystander cannot score or retire target");
    Shoot(false); Shoot(true); Shoot(true);
    Check(PracticeSession[0][PracticeHits] == 1 && PracticeSession[0][PracticeShots] == 3, "repeat hit during gap adds no point");
    Check(TestLiveObjects == 0, "hit removes object");
    AdvanceInterval();
    TestNow += 5000; PracticeTick(0, TestNow);
    Check(PracticeSession[0][PracticeExpired] == 1, "timeout counts one omission");
    AdvanceInterval(); Shoot(true);
    Check(!PracticeIsBusy(0) && TestLiveObjects == 0, "completion releases station and object");
    Check(TestResult == 1 && TestResultShots == 4 && TestResultHits == 2 && TestResultExpired == 1, "final score and counters");
    PracticeStart(0, 1);
    Check(PracticeSession[0][PracticePlayer] == 1, "station immediately reusable");

    ResetTest(1); ShowFirstTarget();
    TestNow += 5000; Shoot(true);
    Check(TestResult == 2 && TestResultShots == 0 && TestResultHits == 0 && TestResultExpired == 1, "late final hit expires without division by zero");
    ResetTest(2); ShowFirstTarget();
    TestNow += 5000; Shoot(true);
    Check(PracticeSession[0][PracticeHits] == 0 && PracticeSession[0][PracticeExpired] == 1
        && PracticeSession[0][PracticeShots] == 1, "late hit before next target is a gap miss");
    AdvanceInterval();
    Check(PracticeSession[0][PracticePresented] == 2, "timeout and shot cannot skip a presentation");

    ResetTest(10); ShowFirstTarget();
    for (new i = 0; i < 10; i++) { Shoot(true); if (i < 9) AdvanceInterval(); }
    Check(TestResultHits == 10 && TestResultShots == 10 && TestResultExpired == 0, "ten target sequence");
    ResetTest(); PracticePoints[0][2][PracticePointSet] = false;
    ShowFirstTarget(); Shoot(true); AdvanceInterval();
    Check(PracticeSession[0][PracticeSlot] == 3 && PracticeSession[0][PracticeTotal] == 2, "sparse slots keep numeric order");
    ResetTest(1); PracticeRules[0][PracticeInterval] = 0; ShowFirstTarget(); Shoot(true);
    Check(!PracticeIsBusy(0), "zero interval still finishes last target");
    ResetTest(2); PracticeRules[0][PracticeInterval] = 0; ShowFirstTarget(); Shoot(true);
    Check(TestLiveObjects == 0, "zero interval does not spawn inside the shot callback");
    PracticeTick(0, TestNow); Shoot(true);
    Check(TestResult == 1 && TestResultHits == 2, "zero interval advances on the next tick");
    ResetTest(3); PracticeRules[0][PracticePassPoints] = 3; ShowFirstTarget(); Shoot(true);
    AdvanceInterval(); Shoot(true); AdvanceInterval(); TestNow += 5000; PracticeTick(0, TestNow);
    Check(TestResult == 2 && TestResultHits == 2, "passing threshold is enforced");

    // Invalid entry and every cleanup route used by the lifecycle hooks.
    ResetTest(); TestPosition[0][0] = 2.0; PracticeStart(0, 0);
    Check(!PracticeIsBusy(0), "reject start outside radius");
    TestPosition[0][0] = 0.0; TestWorld[0] = 1; PracticeStart(0, 0);
    Check(!PracticeIsBusy(0), "reject wrong world");
    TestWorld[0] = 0; TestWeapon[0] = 23; PracticeStart(0, 0);
    Check(!PracticeIsBusy(0), "reject wrong weapon");
    TestWeapon[0] = 24; TestState[0] = PLAYER_STATE_DRIVER; PracticeStart(0, 0);
    Check(!PracticeIsBusy(0), "reject vehicle");
    ResetTest(); ShowFirstTarget(); TestPosition[0][0] = 4.0; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestResult == 3 && TestLiveObjects == 0, "leaving cancels and cleans");
    ResetTest(); ShowFirstTarget(); TestInterior[0] = 1; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestResult == 3, "changing interior cancels");
    ResetTest(); ShowFirstTarget(); TestWorld[0] = 1; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0), "changing world cancels");
    ResetTest(); ShowFirstTarget(); TestWeapon[0] = 23; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0), "changing weapon cancels");
    ResetTest(); ShowFirstTarget(); TestHealth[0] = 0.0; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0), "death cancels");
    ResetTest(); ShowFirstTarget(); TestConnected[0] = false; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestLiveObjects == 0, "disconnect cleans");
    TestConnected[0] = true; PracticeStart(0, 0);
    Check(PracticeSession[0][PracticeShots] == 0 && PracticeSession[0][PracticePresented] == 0, "reused player ID has fresh session");
    ResetTest(); ShowFirstTarget(); TestCommand_practica(1, "cancelar");
    Check(PracticeIsBusy(0), "bystander cannot cancel");
    TestCommand_practica(0, "cancelar");
    Check(!PracticeIsBusy(0) && TestResult == 3, "explicit cancellation is incomplete");
    ResetTest(1); ShowFirstTarget(); TestAmmo[0][2] = 0; TestWeapon[0] = 0;
    PracticeTick(0, TestNow); Check(PracticeIsBusy(0), "empty ammo does not cancel");
    TestNow += 5000; PracticeTick(0, TestNow);
    Check(TestResult == 2 && TestResultExpired == 1, "empty ammo attempt expires normally");
    ResetTest(); TestCreateFails = true; PracticeStart(0, 0); TestNow += 3000; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestResult == 3 && TestLiveObjects == 0, "creation failure releases station");
    ResetTest(); TestVisible = false; PracticeStart(0, 0); TestNow += 3000; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestResult == 3 && TestLiveObjects == 0, "streaming failure cleans object");
    ResetTest(); ShowFirstTarget(); TestDestroyObject(PracticeSession[0][PracticeObject]); PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestResult == 3, "lost object marks attempt incomplete");
    ResetTest(1); TestNow = 2147483000; ShowFirstTarget(); TestNow += 5000; PracticeTick(0, TestNow);
    Check(TestResult == 2, "clock wraparound preserves deadlines");

    TestPresentation();
    TestNoShootScoring();
    TestRandomSequences();
    TestMenuConfiguration();
    TestVisualEditor();
    TestPreviews();
    TestStorageV4();
    TestStorageMigration();
    TestMultipleWeapons();
    TestWeaponDialogs();
    TestWeaponStorage();
    TestCountdownAndDeparture();
    TestDetailedResults();
    TestExtraShotPrecision();
    TestConditionalMain();
    TestPlacementRadius();
    TestStartPointPlacement();
    TestPersistentSetupView();
    TestPreviewProtection();
    TestDecimalTimings();
    TestCountdownPacing();
    TestZoneAdministration();
    TestZoneSelection();
    TestIndependentZones();
    TestZoneStorage();
    TestStartMenus();
    TestStartSessions();
    TestStartPreviews();
    TestStartStorage();
    TestConfigurationPause();
    TestPracticeDiscovery();
    TestIndicatorStorage();
    TestModernCatalogue();
    TestCatalogueTransitions();
    TestMatchingReportBorders();
    TestExpandedTargetModels();
    TestIntegrationPorts();
    ResetTest();
    PracticeCleanup(0);
    printf("[Practice tests] %d assertions, %d failures", TestAssertions, TestFailures);
    if (!TestFailures) print("[Practice tests] PASS");
    new callback[] = "PracticeTestExit";
    SetTimer(callback, 100, false);
    return 1;
}
forward PracticeTestExit();
public PracticeTestExit()
{
    new command[] = "exit";
    SendRconCommand(command);
    return 1;
}
