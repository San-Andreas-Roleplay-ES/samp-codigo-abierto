// Multi-zone registry, public selection, isolated session/editor ownership and
// real file recovery. No SQL identity or permission adapter is simulated here.
stock TestAddZone(const name[], Float:x = 0.0, world = 0, targets = 2)
{
    new zone = PracticeCreateZone(name);
    if (zone < 0) return -1;
    for (new slot = 0; slot <= targets; slot++)
    {
        PracticePoints[zone][slot][PracticePointSet] = true;
        PracticePoints[zone][slot][PracticePointX] = x + float(slot * 8);
        PracticePoints[zone][slot][PracticePointWorld] = world;
        if (slot) PracticePoints[zone][slot][PracticePointModel] = PRACTICE_TARGET_MODEL;
    }
    PracticeZoneEnabled[zone] = true;
    return zone;
}

stock ZoneChoose(id, playerid = 0)
{
    for (new row = 0; row < PracticeZoneListCount[playerid]; row++)
        if (PracticeZoneListRows[playerid][row] == id) return MenuChoose(row, playerid);
    return 0;
}

stock TestZoneAdministration()
{
    ResetTest(); TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_ZONES && strfind(TestDialogBody[0], "#1: Campo de tiro") >= 0,
        "configuration command lists existing zone IDs and names");
    ZoneChoose(SP_ZONE_CREATE); MenuInput("LSSD Academy");
    Check(PracticeZoneID[1] == 2 && !PracticeZoneEnabled[1] && PracticeSelectedZoneID[0] == 2
        && !PracticeCountTargets(1) && PracticeMenuPage[0] == SP_MENU_MAIN,
        "administrative creation persists a named disabled zone with its own selection");
    MainChoose(SP_MAIN_TOGGLE_ZONE);
    Check(!PracticeZoneEnabled[1], "incomplete new zone cannot be activated");
    TestCommand_practicaconfig(0, "2"); MainChoose(SP_MAIN_RENAME_ZONE); MenuInput("LSSD Campo 100%");
    Check(!strcmp(PracticeZoneName[1], "LSSD Campo 100%") && !strcmp(PracticeZoneName[0], "Campo de tiro"),
        "renaming affects only the selected zone and keeps literal percent signs");
    MainChoose(SP_MAIN_RENAME_ZONE); MenuInput("{FF0000}bad");
    Check(!strcmp(PracticeZoneName[1], "LSSD Campo 100%"), "zone name rejects color injection");
    MainChoose(SP_MAIN_RENAME_ZONE); MenuInput("   ");
    Check(!strcmp(PracticeZoneName[1], "LSSD Campo 100%"), "zone name rejects whitespace-only input");
    MainChoose(SP_MAIN_START_POINT); MenuChoose(0);
    Check(PracticePoints[1][0][PracticePointSet] && PracticeCountTargets(0) == 3,
        "capturing start leaves other zone layout intact");
    PracticeShowMain(0); MainChoose(SP_MAIN_TARGETS); MenuChoose(0); SelectModel(1583); MenuChoose(0);
    EditorResult(EDIT_RESPONSE_FINAL, 12.0);
    TestCommand_practicaconfig(0, "2"); MainChoose(SP_MAIN_TOGGLE_ZONE);
    Check(PracticeZoneEnabled[1] && PracticeCountTargets(1) == 1 && PracticePoints[1][1][PracticePointModel] == 1583,
        "new zone can be configured with existing visual editor then activated");
    MainChoose(SP_MAIN_TOGGLE_ZONE);
    Check(!PracticeZoneEnabled[1] && !PracticeCanStart(1, 1), "deactivated zone rejects starts");
    MainChoose(SP_MAIN_TOGGLE_ZONE); MainChoose(SP_MAIN_RESET); MenuChoose(0);
    Check(!PracticeZoneEnabled[1] && !PracticeCountTargets(1) && PracticeZoneID[1] == 2,
        "reset preserves identity and automatically closes incomplete zone");
    TestCommand_practicaconfig(0, "2"); MainChoose(SP_MAIN_ARCHIVE_ZONE); MenuBack();
    Check(!PracticeZoneArchived[1], "archive cancellation preserves registry");
    MainChoose(SP_MAIN_ARCHIVE_ZONE); TestSaveFails = true; MenuChoose(0);
    Check(PracticeRegistrySavePending && PracticeSavePending[1], "failed archive save retains tombstone and pending state in memory");
    Check(PracticeZoneArchived[1] && !PracticeZoneEnabled[1] && PracticeSelectedZone[0] == -1
        && PracticeMenuPage[0] == SP_MENU_ZONES && strfind(TestDialogBody[0], "#2:") < 0,
        "confirmed archive clears owned UI and removes the zone from list");
    TestSaveFails = false; ZoneChoose(SP_ZONE_RETRY_SAVE);
    Check(!PracticeRegistrySavePending && !PracticeSavePending[1] && PracticeReadRegistry(PRACTICE_CONFIG_FILE),
        "global retry persists archived zone without a valid selected binding");
    ZoneChoose(SP_ZONE_CREATE); MenuInput("Private Range");
    Check(PracticeZoneID[2] == 3 && PracticeNextZoneID == 4, "archived IDs and slots are not recycled");
    TestCommand_practicaconfig(0, "2");
    Check(PracticeSelectedZoneID[0] == 3, "direct command cannot reopen archived zone");
    TestCommand_practicaconfig(0, "1");
    Check(PracticeSelectedZoneID[0] == 1 && PracticeMenuPage[0] == SP_MENU_MAIN,
        "direct ID command selects original zone without changing its layout");
    new badIds[][] = {"0", "-1", "1 2", "abc", "2147483648", "4294967297", "99999999999"};
    for (new i = 0; i < sizeof(badIds); i++)
    {
        TestCommand_practicaconfig(0, badIds[i]);
        Check(PracticeSelectedZoneID[0] == 1, "malformed or overflowing ID cannot redirect selection");
    }
    TestAdmin[1] = false; TestCommand_practicaconfig(1, "");
    Check(PracticeMenuPage[1] == SP_MENU_NONE, "nonadministrator cannot list configuration");
    TestCommand_practicaconfig(0, ""); ZoneChoose(SP_ZONE_CREATE); TestAdmin[0] = false; MenuInput("Unauthorized");
    Check(PracticeNextZoneID == 4 && PracticeMenuPage[0] == SP_MENU_NONE, "lost RCON cannot create via old input dialog");
    TestAdmin[0] = true; TestCommand_practicaconfig(0, ""); ZoneChoose(SP_ZONE_CREATE); TestHealth[0] = 0.0; MenuInput("Dead");
    Check(PracticeNextZoneID == 4, "dead administrator cannot create");
    TestHealth[0] = 100.0; TestCommand_practicaconfig(0, "");
    PracticeCreateZone("Concurrent admin"); ZoneChoose(1);
    Check(PracticeMenuPage[0] == SP_MENU_ZONES, "stale zone list refreshes rather than dispatching old row");

    ResetTest();
    for (new i = 0; i < 21; i++) PracticeCreateZone("Extra range");
    TestCommand_practicaconfig(0, "");
    Check(PracticeZoneListCount[0] == 12 && PracticeZoneListRows[0][1] == 1 && PracticeZoneListRows[0][10] == 10,
        "first page contains at most ten stable zone IDs plus navigation");
    ZoneChoose(SP_ZONE_NEXT);
    Check(PracticeZoneListPage[0] == 1 && PracticeZoneListRows[0][1] == 11 && PracticeZoneListCount[0] == 13,
        "middle page exposes previous and next navigation");
    ZoneChoose(12);
    Check(PracticeSelectedZoneID[0] == 12 && PracticeMenuPage[0] == SP_MENU_MAIN, "paginated row opens captured ID");
    TestCommand_practicaconfig(0, ""); ZoneChoose(SP_ZONE_NEXT); ZoneChoose(SP_ZONE_NEXT);
    Check(PracticeZoneListPage[0] == 2 && PracticeZoneListRows[0][1] == 21, "last page shows remaining zone IDs");
    ZoneChoose(SP_ZONE_PREVIOUS); Check(PracticeZoneListPage[0] == 1, "previous page reconstructs captured rows");
    for (new i = 22; i < PRACTICE_MAX_ZONES; i++) PracticeCreateZone("Capacity");
    Check(PracticeCreateZone("Overflow") == -1 && PracticeNextZoneID == PRACTICE_MAX_ZONES + 1,
        "bounded registry rejects capacity overflow without consuming an ID");
    PracticeZoneArchived[1] = true;
    Check(PracticeCreateZone("Still full") == -1, "archived records remain part of bounded file capacity");
}

stock TestZoneSelection()
{
    ResetTest(); new second = TestAddZone("Nearby range"), distant = TestAddZone("Distant", 200.0), other = TestAddZone("Other world", 0.0, 7);
    TestAdmin[0] = false; PracticeSelectedZone[0] = -1; PracticeSelectedZoneID[0] = 0;
    TestCommand_practica(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_PLAYER_ZONES && PracticeZoneListCount[0] == 2 && !PracticeIsBusy(0) && !PracticeIsBusy(second),
        "public overlapping starts require explicit choice and exclude distant/world zones");
    ZoneChoose(2);
    Check(PracticeGetPlayerZone(0) == second && PracticeSelectedZone[0] == -1 && !PracticeIsBusy(0),
        "public choice starts its ID without acquiring administrative selection");
    TestCommand_practica(0, "1"); Check(!PracticeIsBusy(0), "one player cannot occupy two zones");
    TestCommand_practica(1, "cancelar"); Check(PracticeIsBusy(second), "bystander cannot cancel another zone session");
    TestCommand_practica(0, "cancelar"); Check(!PracticeIsBusy(second) && PracticeGetPlayerZone(0) == -1, "cancel routes to participant's zone");
    TestCommand_practica(0, "3"); Check(!PracticeIsBusy(distant), "explicit ID cannot bypass start distance");
    TestCommand_practica(0, "4"); Check(!PracticeIsBusy(other), "explicit ID cannot bypass virtual world");
    TestCommand_practica(0, "ayuda"); Check(PracticeMenuPage[0] == SP_MENU_PLAYER_ZONES, "help can select among overlapping zones");
    ZoneChoose(2); Check(!PracticeIsBusy(second) && strfind(TestChat[0], "Desert Eagle") >= 0, "selected help shows rules without starting");
    TestCommand_practica(0, ""); TestPosition[0][0] = 4.0; ZoneChoose(2);
    Check(!PracticeIsBusy(second), "moving away after rendering selection cannot start");
    TestPosition[0][0] = 0.0; TestCommand_practica(0, ""); PracticeZoneEnabled[second] = false; ZoneChoose(2);
    Check(!PracticeIsBusy(second), "deactivation after rendering public selection cannot start");
    TestCommand_practica(0, ""); Check(PracticeGetPlayerZone(0) == 0, "one available nearby zone starts directly");
    TestCommand_practica(0, "cancelar");
    TestNPC[0] = true; TestCommand_practica(0, "1"); TestAdmin[0] = true; TestCommand_practicaconfig(0, "1");
    Check(!PracticeIsBusy(0) && !PracticeSelectedZoneValid(0), "NPC cannot start or configure a zone");
    TestNPC[0] = false; TestCommand_practica(0, "nope"); Check(!PracticeIsBusy(0), "unknown practice argument cannot start");
}

stock TestIndependentZones()
{
    ResetTest(2); new second = TestAddZone("Second", 200.0);
    PracticeRules[second][PracticePreparation] = 2;
    TestPosition[1][0] = 200.0; PracticeBindZone(1, second);
    PracticeStart(0, 0); PracticeStart(second, 1);
    Check(PracticeIsBusy(0) && PracticeIsBusy(second) && PracticeGetPlayerZone(0) == 0 && PracticeGetPlayerZone(1) == second,
        "different participants run independent zones concurrently");
    TestNow += 2000; PracticeTick(second, TestNow); PracticeTick(0, TestNow);
    Check(PracticeSession[0][PracticePhase] == PRACTICE_COUNTDOWN && PracticeSession[second][PracticePhase] == PRACTICE_TARGET
        && PracticeCountdownText[0] != PlayerText:INVALID_TEXT_DRAW && PracticeCountdownText[1] == PlayerText:INVALID_TEXT_DRAW,
        "independent preparation clocks and owned countdowns are respected");
    PracticeCleanup(second);
    Check(PracticeIsBusy(0) && PracticeCountdownText[0] != PlayerText:INVALID_TEXT_DRAW && PracticeGetPlayerZone(1) == -1,
        "cleaning one zone preserves other participant countdown");
    PracticeStart(second, 1); TestNow += 3000; PracticeTick(0, TestNow); PracticeTick(second, TestNow);
    new firstObject = PracticeSession[0][PracticeObject], secondObject = PracticeSession[second][PracticeObject];
    Check(TestLiveObjects == 2 && firstObject != secondObject && TestObjectPosition[firstObject][0] == 8.0
        && TestObjectPosition[secondObject][0] == 208.0 && TestObjectOwner[firstObject] == -1 && TestObjectOwner[secondObject] == -1,
        "session targets use different zone positions/handles and remain visible to observers");
    PracticeHandleShot(0, 24, BULLET_HIT_TYPE_PLAYER_OBJECT, 1000 + secondObject, TestNow);
    Check(PracticeSession[0][PracticeShots] == 1 && !PracticeSession[0][PracticeHits] && !PracticeSession[second][PracticeShots],
        "foreign target handle is a miss only in shooter's session");
    PracticeHandleShot(1, 24, BULLET_HIT_TYPE_PLAYER_OBJECT, 1100 + secondObject, TestNow);
    Check(PracticeSession[second][PracticeHits] == 1 && PracticeSession[second][PracticeShots] == 1 && PracticeSession[0][PracticeHits] == 0,
        "valid impact scores and removes only its owning zone target");
    Check(TestObjectValid(firstObject) && !TestObjectValid(secondObject), "another zone's target survives foreign completion");
    PracticeHandleShot(0, 24, BULLET_HIT_TYPE_PLAYER_OBJECT, 1000 + firstObject, TestNow);
    TestNow += 1000; PracticeTick(0, TestNow); PracticeTick(second, TestNow);
    PracticeHandleShot(0, 24, BULLET_HIT_TYPE_PLAYER_OBJECT, 1000 + PracticeSession[0][PracticeObject], TestNow);
    Check(!PracticeIsBusy(0) && PracticeIsBusy(second) && TestResultHits == 2 && TestResultShots == 3,
        "one round can finish while another retains its own counters and active target");
    PracticeHandleShot(1, 24, BULLET_HIT_TYPE_PLAYER_OBJECT, 1100 + PracticeSession[second][PracticeObject], TestNow);
    Check(!PracticeIsBusy(second) && TestResultHits == 2 && TestResultShots == 2 && !TestLiveObjects && !TestCueBeforeReady,
        "second round finishes independently with no cross-scoring or leaked target");

    ResetTest(2); second = TestAddZone("Editor zone", 200.0);
    PracticeBindZone(1, second); TestPosition[1][0] = 200.0;
    PracticeShowPreview(0); PracticeShowPreview(1); new preview = PracticePreviewObjects[1][0];
    PracticeStart(0, 2);
    Check(!PracticePreviewActive[0] && PracticePreviewActive[1] && TestObjectValid(preview),
        "starting a zone hides only its previews and preserves a different zone preview");
    Check(!PracticeMayChange(0) && PracticeMayChange(1), "busy zone blocks its editor while another zone remains editable");
    PracticeBeginDraft(1, 1); PracticeShowDraftMenu(1); MenuChoose(0, 1); MenuInput("Independent edit", 1); MenuChoose(6, 1);
    Check(!strcmp(PracticePoints[second][1][PracticePointName], "Independent edit") && PracticeIsBusy(0),
        "saving an unrelated zone during a round preserves the active session");
    PracticeCleanup(0);
    PracticeShowWeapons(0); PracticeShowWeapons(1); MenuChoose(9, 1); MenuChoose(1);
    Check(PracticeMenuPage[0] == SP_MENU_WEAPON && PracticeWeaponAllowed(23, PracticeRules[0][PracticeWeaponMask]),
        "unrelated zone mutation does not stale another zone's menu");
    PracticeResetUI(0); PracticeResetUI(1); PracticeBeginDraft(0, 1); PracticeBeginDraft(1, 1);
    Check(PracticeEditorOwner[0] == 0 && PracticeEditorOwner[second] == 1, "each zone has an independent writer lock");
    TestPosition[2][0] = 200.0; PracticeStart(second, 2);
    Check(!PracticeIsBusy(second), "editing zone blocks only its own start");
    PracticeDiscardDraft(0); PracticeStart(0, 2);
    Check(!PracticeIsBusy(0), "start still requires the participant to be at the correct physical point");
    TestPosition[2][0] = 0.0; PracticeStart(0, 2);
    Check(PracticeIsBusy(0) && PracticeHasDraft(1), "round elsewhere does not invalidate pending draft");
    PracticeResetUI(1); TestCommand_practicaconfig(1, "2"); MainChoose(SP_MAIN_ARCHIVE_ZONE, 1); MenuChoose(0, 1);
    Check(PracticeZoneArchived[second] && PracticeIsBusy(0), "archiving unused zone leaves unrelated round running");
    TestCommand_practicaconfig(0, "1"); MainChoose(SP_MAIN_ARCHIVE_ZONE); MenuChoose(0);
    Check(!PracticeZoneArchived[0] && PracticeIsBusy(0), "occupied zone cannot be archived");
    PracticeCleanup(0);
}

// Rewrite a single real file line, preserving every other record. This lets the
// parser reject corrupt input independently of the validated save boundary.
stock TestReplaceConfigLine(index, const replacement[])
{
    new File:input = fopen(PRACTICE_CONFIG_FILE, io_read), File:output = fopen("fixture.tmp", io_write);
    new line[256], current;
    while (fread(input, line, sizeof(line)))
    {
        if (current++ == index) { fwrite(output, replacement); fwrite(output, "\r\n"); }
        else fwrite(output, line);
    }
    fclose(input); fclose(output);
    PracticeCopyConfiguration("fixture.tmp", PRACTICE_CONFIG_FILE);
    fremove("fixture.tmp"); fremove(PRACTICE_CONFIG_BACKUP); fremove(PRACTICE_CONFIG_TEMP);
}

stock TestZoneStorage()
{
    ResetTest(); new second = TestAddZone("LSSD Academy 100%", 200.0, 7, 2), archived = TestAddZone("Archived", 500.0, 0, 1);
    PracticePoints[second][2][PracticePointNoShoot] = true;
    PracticePoints[second][2][PracticePointModel] = 1585;
    PracticePoints[second][2][PracticePointRotX] = 22.5;
    PracticePoints[second][2][PracticePointRotY] = 42.25;
    PracticePoints[second][2][PracticePointAngle] = 92.75;
    format(PracticePoints[second][2][PracticePointName], PRACTICE_NAME_SIZE, "Puerta norte");
    PracticeRules[second][PracticeExposure] = 700; PracticeRules[second][PracticeInterval] = 200;
    PracticeRules[second][PracticeWeaponMask] = PRACTICE_WEAPONS_ALL;
    PracticeRules[second][PracticeRandomOrder] = 1;
    PracticeZoneEnabled[archived] = false; PracticeZoneArchived[archived] = true;
    Check(PracticeSaveConfiguration() && PracticeReadRegistry(PRACTICE_CONFIG_FILE), "v8 writes validated complete registry");
    PracticeBindZone(1, second); PracticeZoneName[second][0] = EOS; PracticePoints[second][2][PracticePointX] = 0.0;
    Check(PracticeLoadConfiguration() && PracticeZoneID[second] == 2 && !strcmp(PracticeZoneName[second], "LSSD Academy 100%")
        && PracticeZoneEnabled[second] && PracticeZoneArchived[archived] && PracticeNextZoneID == 4 && PracticeSelectedZoneID[1] == 2,
        "registry reload preserves stable IDs, enabled/archive state, next ID and player binding");
    Check(PracticePoints[second][2][PracticePointX] == 216.0 && PracticePoints[second][2][PracticePointWorld] == 7
        && PracticePoints[second][2][PracticePointNoShoot] && PracticePoints[second][2][PracticePointModel] == 1585
        && PracticePoints[second][2][PracticePointRotX] == 22.5 && PracticePoints[second][2][PracticePointRotY] == 42.25
        && PracticePoints[second][2][PracticePointAngle] == 92.75 && !strcmp(PracticePoints[second][2][PracticePointName], "Puerta norte")
        && PracticeRules[second][PracticeExposure] == 700 && PracticeRules[second][PracticeInterval] == 200
        && PracticeRules[second][PracticeWeaponMask] == PRACTICE_WEAPONS_ALL && PracticeRules[second][PracticeRandomOrder],
        "multi-zone round trip preserves each target transform/model/name/type and independent timing/rules");
    PracticeStart(0, 0);
    Check(!PracticeLoadConfiguration() && PracticeIsBusy(0) && !PracticeStorageBlocked, "in-use reload cannot replace sessions or falsely mark valid file corrupt");
    PracticeCleanup(0); PracticeBeginDraft(0, 1);
    Check(!PracticeLoadConfiguration() && PracticeHasDraft(0) && !PracticeStorageBlocked, "reload cannot replace an editor draft");
    PracticeDiscardDraft(0);
    PracticeRules[0][PracticeExposure] = 900; PracticeSaveAndNotify(0);
    Check(PracticeReadRegistry(PRACTICE_CONFIG_BACKUP), "subsequent save retains a validated prior registry backup");
    TestReplaceConfigLine(1, "REGISTRY 4 65");
    Check(!PracticeLoadConfiguration() && PracticeStorageBlocked && PracticeZoneID[1] == 2 && PracticeRules[0][PracticeExposure] == 900,
        "over-capacity file cannot partially publish and blocks destructive overwrites");
    Check(!PracticeSaveConfiguration(), "invalid unrecoverable primary is not overwritten by a normal save");

    ResetTest(); second = TestAddZone("Second"); PracticeSaveConfiguration();
    PracticeRules[0][PracticeExposure] = 900; PracticeSaveConfiguration();
    new File:broken = fopen(PRACTICE_CONFIG_FILE, io_write); fwrite(broken, "torn primary\r\n"); fclose(broken);
    Check(PracticeLoadConfiguration() && PracticeRules[0][PracticeExposure] == 5000 && PracticeZoneID[second] == 2 && PracticeRegistrySavePending,
        "torn primary recovers previous validated registry and exposes repair save");
    PracticeSaveAndNotify(0);
    Check(!PracticeRegistrySavePending && PracticeReadRegistry(PRACTICE_CONFIG_FILE), "recovery retry repairs the primary file");
    TestSaveFails = true; PracticeRules[0][PracticeExposure] = 600; PracticeSaveAndNotify(0); TestSaveFails = false;
    Check(PracticeSavePending[0] && PracticeRegistrySavePending && PracticeReadRegistry(PRACTICE_CONFIG_FILE), "save failure retains in-memory edit and last valid primary");
    PracticeRules[0][PracticeExposure] = 5000; PracticeSaveAndNotify(0);
    Check(!PracticeSavePending[0] && !PracticeRegistrySavePending, "successful retry clears all registry pending flags");
    // Invalid live data fails at staging validation, before either durable file changes.
    PracticeRules[0][PracticeExposure] = 99;
    Check(!PracticeSaveConfiguration() && PracticeReadRegistry(PRACTICE_CONFIG_FILE) && PracticeReadRegistry(PRACTICE_CONFIG_BACKUP),
        "invalid stage preserves validated primary and backup");
    PracticeRules[0][PracticeExposure] = 5000;
    Check(PracticeLoadConfiguration() && PracticeRules[0][PracticeExposure] == 5000, "invalid abandoned stage never overrides valid primary");

    new corruptLines[][] = {"REGISTRY 1 2", "REGISTRY 3 -1", "REGISTRY 3 2 trailing", "ZONE 1 2 0 0", "ZONE 0 1 0 0", "ZONE 1 1 1 0", "ZONE 1 1 0 0 trailing", "ZONE_NAME {FF0000}bad"};
    for (new i = 0; i < sizeof(corruptLines); i++)
    {
        ResetTest(); TestAddZone("Second"); PracticeSaveConfiguration();
        TestReplaceConfigLine(i < 3 ? 1 : (i == 7 ? 3 : 2), corruptLines[i]);
        Check(!PracticeLoadConfiguration() && PracticeZoneID[1] == 2, "malformed registry metadata rejects the entire file without partial replacement");
    }
    ResetTest(); TestAddZone("Second"); PracticeSaveConfiguration(); TestReplaceConfigLine(6 + 2 * PRACTICE_POINT_COUNT, "ZONE 1 1 0 0");
    Check(!PracticeLoadConfiguration() && PracticeZoneID[1] == 2, "duplicate persistent IDs reject whole registry");
    ResetTest(); PracticeSaveConfiguration(); TestReplaceConfigLine(5, "NAME bad");
    Check(!PracticeLoadConfiguration(), "malformed v8 point line rejects file independently of save validation");

    ResetTest(0); PracticeZoneEnabled[0] = false; PracticeSaveConfiguration(); TestReplaceConfigLine(2, "ZONE 1 1 0 0");
    Check(PracticeLoadConfiguration() && !PracticeZoneEnabled[0] && PracticeRegistrySavePending,
        "incomplete active record loads configuration safely as inactive and requests repair");
    ResetTest(); PracticeSaveConfiguration(); PracticeCopyConfiguration(PRACTICE_CONFIG_FILE, PRACTICE_CONFIG_TEMP); fremove(PRACTICE_CONFIG_FILE);
    Check(PracticeLoadConfiguration() && PracticeRegistrySavePending && PracticeZoneID[0] == 1, "missing first primary recovers validated initial stage");

    ResetTest(); PracticeInitializeRegistry(); PracticeSelectedZone[0] = -1; PracticeSelectedZoneID[0] = 0;
    Check(PracticeSaveConfiguration() && PracticeLoadConfiguration() && PracticeNextZoneID == 1 && PracticeFindZone(1) == -1,
        "empty registry persists and reloads without inventing a default zone");
    PracticeNextZoneID = 2147483647;
    Check(PracticeCreateZone("ID overflow") == -1, "exhausted persistent ID counter fails without wrapping");
}
