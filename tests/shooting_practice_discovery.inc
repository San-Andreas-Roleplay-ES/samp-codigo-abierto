// Public start indicators, private proximity hints and v9 migration.
stock TestPracticeDiscovery()
{
    ResetTest(); new second = TestSecondStart(0, 6.0);
    PracticeSynchronizeIndicators(0);
    Check(!PracticeZoneIndicators[0] && !TestLivePickups, "new zones have no public indicators by default");
    PracticeShowMain(0);
    Check(MainHasAction(SP_MAIN_INDICATORS) && strfind(TestDialogBody[0], "Indicadores de inicio") >= 0,
        "root exposes the per-zone indicator option");
    MainChoose(SP_MAIN_INDICATORS);
    new firstPickup = PracticeStartPickups[0][0], secondPickup = PracticeStartPickups[0][1];
    Check(PracticeZoneIndicators[0] && TestLivePickups == 2 && TestPickupValid(firstPickup) && TestPickupValid(secondPickup),
        "enabling creates one pickup at each marked post");
    Check(TestPickupModels[firstPickup] == 348 && TestPickupTypes[firstPickup] == 1
        && TestPickupPosition[firstPickup][2] == PRACTICE_START_PICKUP_Z
        && TestPickupPosition[secondPickup][1] == 6.0 && TestPickupWorld[firstPickup] == 0
        && TestPickupInterior[firstPickup] == 0 && !TestLiveLabels && !TestLiveObjects,
        "standalone pistol indicators use decorative type and the post dimension without labels");
    Check(TestPickupPosition[firstPickup][2] == -0.65, "pickup is lowered one meter relative to the former offset");
    PracticeSynchronizeIndicators(0);
    Check(TestNextPickup == 2 && PracticeStartPickups[0][0] == firstPickup, "unchanged pickups keep their native handles");
    PracticePoints[0][second][PracticePointY] = 7.0; PracticeConfigurationChanged(0);
    Check(PracticeStartPickups[0][0] == firstPickup && PracticeStartPickups[0][1] != secondPickup
        && !TestPickupValid(secondPickup) && TestLivePickups == 2,
        "moving a post replaces only its pickup");
    PracticePoints[0][second][PracticePointSet] = false; PracticeConfigurationChanged(0);
    Check(TestLivePickups == 1 && !PracticeStartPickups[0][1], "deleting a post removes its indicator");
    PracticeShowMain(0); MainChoose(SP_MAIN_TOGGLE_ZONE);
    Check(!TestLivePickups && PracticeZoneIndicators[0], "deactivating hides pickups while retaining the display preference");
    MainChoose(SP_MAIN_TOGGLE_ZONE);
    Check(TestLivePickups == 1, "reactivating restores configured indicators");
    MainChoose(SP_MAIN_INDICATORS);
    Check(!PracticeZoneIndicators[0] && !TestLivePickups, "disabling the option removes all indicators");

    ResetTest(); second = TestSecondStart(0, 6.0); PracticeZoneIndicators[0] = true;
    TestPickupFailAt = 2;
    Check(!PracticeSynchronizeIndicators(0) && !TestLivePickups && !PracticeStartPickups[0][0],
        "allocation failure rolls back the complete zone indicator set");
    TestPickupFailAt = 0; PracticeSynchronizeIndicators(0);
    Check(TestLivePickups == 2, "indicator allocation can recover without changing saved points");
    firstPickup = PracticeStartPickups[0][0]; TestDestroyPickup(firstPickup); PracticeSynchronizeIndicators(0);
    Check(TestLivePickups == 2 && PracticeStartPickups[0][0] != firstPickup, "lost native pickup is recreated");
    PracticeStart(0, 0); PracticeDiscoverPlayer(0);
    Check(TestLivePickups == 2 && PracticeIsBusy(0) && PracticeHintZoneID[0] == 1,
        "public indicators remain during practice without creating another session");
    PracticeCleanup(0);
    PracticeShowMain(0); MainChoose(SP_MAIN_RESET); MenuChoose(0);
    Check(!TestLivePickups && !PracticeZoneIndicators[0], "reset restores disabled indicators and releases native resources");

    ResetTest(); new other = TestAddZone("Otra zona", 200.0, 7, 2);
    for (new slot = 0; slot <= 2; slot++) PracticePoints[other][slot][PracticePointInterior] = 2;
    PracticeZoneIndicators[other] = true; PracticeSynchronizeIndicators(other);
    firstPickup = PracticeStartPickups[other][0];
    Check(TestLivePickups == 1 && TestPickupWorld[firstPickup] == 7 && TestPickupInterior[firstPickup] == 2,
        "independent zone indicator respects interior and virtual world");
    PracticeBindZone(0, other); PracticeArchiveZone(0);
    Check(!TestLivePickups && PracticeZoneArchived[other], "archiving releases public pickups");
    PracticeInitializeRegistry();
    Check(!TestLivePickups && !PracticeHintZoneID[0], "registry reset cleans discovery resources and player latches");

    ResetTest(); PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 1 && !TestMessages[1] && !TestLivePickups
        && strfind(TestLastMessage[0], "Usa /practica para iniciar una práctica.") >= 0,
        "proximity hint is private chat and independent of disabled indicators");
    for (new i = 0; i < 5; i++) PracticeDiscoverPlayer(0);
    TestPosition[0][0] = 1.6; PracticeDiscoverPlayer(0); TestPosition[0][0] = 0.0; PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 1, "remaining nearby and crossing the inner edge do not spam the chat");
    TestPosition[0][0] = 4.0; PracticeDiscoverPlayer(0); TestPosition[0][0] = 0.0; PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 2, "leaving the outer radius and returning shows a fresh hint");
    PracticeZoneIndicators[0] = true; PracticeSynchronizeIndicators(0); PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 2 && TestLivePickups == 1, "indicator toggle does not repeat the proximity hint");
    second = TestSecondStart(0, 6.0); TestPosition[0][1] = 6.0; PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 3, "a separate post has the same discovery behavior");
    PracticePoints[0][0][PracticePointSet] = false; PracticeHintZoneID[0] = 0; PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 4 && PracticeNearestStart(0, 0, PRACTICE_START_RADIUS) == second,
        "sparse start slots can advertise the command");

    ResetTest(); TestPosition[0][0] = 1.51; PracticeDiscoverPlayer(0);
    Check(!TestMessages[0], "hint enters only within the public command radius");
    TestPosition[0][0] = 0.0; TestWorld[0] = 1; PracticeDiscoverPlayer(0);
    TestWorld[0] = 0; TestInterior[0] = 1; PracticeDiscoverPlayer(0);
    TestInterior[0] = 0; TestNPC[0] = true; PracticeDiscoverPlayer(0);
    TestNPC[0] = false; TestState[0] = PLAYER_STATE_DRIVER; PracticeDiscoverPlayer(0);
    TestState[0] = PLAYER_STATE_ONFOOT; TestHealth[0] = 0.0; PracticeDiscoverPlayer(0);
    TestHealth[0] = 100.0; TestConnected[0] = false; PracticeDiscoverPlayer(0);
    Check(!TestMessages[0], "hint excludes wrong dimensions, NPCs, vehicles, dead and disconnected players");
    TestConnected[0] = true; PracticeZoneEnabled[0] = false; PracticeDiscoverPlayer(0);
    PracticeZoneEnabled[0] = true; PracticeZoneArchived[0] = true; PracticeDiscoverPlayer(0);
    PracticeZoneArchived[0] = false; PracticePoints[0][1][PracticePointSet] = false;
    PracticePoints[0][2][PracticePointSet] = false; PracticePoints[0][3][PracticePointSet] = false;
    PracticeDiscoverPlayer(0);
    Check(!TestMessages[0], "inactive, archived and incomplete fields do not invite participation");

    ResetTest(); TestAddZone("Superpuesta"); PracticeDiscoverPlayer(0); PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 1 && PracticeHintZoneID[0] == 1, "overlapping zones give one stable proximity hint");
    PracticeShowMain(0); PracticeHintZoneID[0] = 0; PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 1, "configuration dialogs defer discovery hints");
    PracticeMenuPage[0] = SP_MENU_NONE; PracticeStart(0, 0); PracticeDiscoverPlayer(0);
    Check(TestPrivateMessages[0] == 0 && PracticeHintZoneID[0] == 1, "active participant is not interrupted by discovery");
    PracticeCleanup(0); PracticeDiscoverPlayer(0);
    Check(TestMessages[0] == 2, "remaining at the post after practice does not repeat discovery");
}

stock TestIndicatorStorage()
{
    ResetTest(); TestSecondStart(0, 6.0);
    new other = TestAddZone("Sin iconos", 200.0); PracticeZoneIndicators[0] = true;
    Check(PracticeSaveConfiguration(), "v9 writes the per-zone indicator flag");
    PracticeZoneIndicators[0] = false; PracticeZoneIndicators[other] = true;
    Check(PracticeLoadConfiguration() && PracticeZoneIndicators[0] && !PracticeZoneIndicators[other]
        && TestLivePickups == 2 && PracticeCountStarts(0) == 2 && PracticeZoneEnabled[0],
        "reload retains independent flags and rebuilds each public pickup");
    new before = PracticeStartPickups[0][0];
    Check(PracticeReadRegistry(PRACTICE_CONFIG_FILE) && PracticeStartPickups[0][0] == before,
        "validation-only reads never mutate live pickup handles");
    TestReplaceConfigLine(2, "ZONE 1 1 0 2");
    Check(!PracticeReadRegistry(PRACTICE_CONFIG_FILE, true) && PracticeZoneIndicators[0]
        && PracticeStartPickups[0][0] == before, "invalid indicator flag rejects the whole registry without publication");

    ResetTest(); TestSecondStart(0, 6.0); PracticeZoneIndicators[0] = true; PracticeSaveConfiguration();
    TestReplaceConfigLine(0, "SARP_PRACTICE 8"); TestReplaceConfigLine(2, "ZONE 1 1 0");
    Check(PracticeLoadConfiguration() && !PracticeZoneIndicators[0] && !TestLivePickups
        && PracticeCountStarts(0) == 2 && PracticeCountTargets(0) == 3,
        "v8 migration defaults indicators off and preserves multiple starts");
    ResetTest(); TestWriteV7Starts(); PracticeZoneIndicators[0] = true; PracticeSynchronizeIndicators(0);
    Check(PracticeLoadConfiguration() && !PracticeZoneIndicators[0] && !TestLivePickups
        && PracticeCountStarts(0) == 1, "v7 migration also clears previous indicator state");
    for (new version = 1; version <= 6; version++)
    {
        ResetTest();
        new header[32], rules[80];
        format(header, sizeof(header), "SARP_PRACTICE %d", version);
        if (version == 1) format(rules, sizeof(rules), "24 1 3 5 1");
        else if (version == 2) format(rules, sizeof(rules), "24 1 3 5 1 0 0 0 0");
        else if (version <= 4) format(rules, sizeof(rules), "24 1 3 5 1 0 0 0");
        else if (version == 5) format(rules, sizeof(rules), "%d 1 3 5 1 0 0 0", PRACTICE_WEAPON_DEFAULT);
        else format(rules, sizeof(rules), "%d 1 3 5000 1000 0 0 0", PRACTICE_WEAPON_DEFAULT);
        TestWriteConfigFixture(header, rules); PracticeZoneIndicators[0] = true; PracticeSynchronizeIndicators(0);
        Check(PracticeLoadConfiguration() && !PracticeZoneIndicators[0] && !TestLivePickups,
            "legacy single-station formats all default indicators off");
    }

    ResetTest(); PracticeShowMain(0); PracticeShowMain(1); MainChoose(SP_MAIN_INDICATORS, 1);
    MainChoose(SP_MAIN_INDICATORS);
    Check(PracticeZoneIndicators[0], "stale dialog cannot revert another administrator's indicator setting");
    TestAdmin[0] = false; PracticeShowMain(0); MainChoose(SP_MAIN_INDICATORS);
    Check(PracticeZoneIndicators[0], "lost administrative permission cannot toggle indicators");
    TestAdmin[0] = true; PracticeStart(0, 0); PracticeShowMain(1); MainChoose(SP_MAIN_INDICATORS, 1);
    Check(PracticeZoneIndicators[0], "occupied zone rejects indicator configuration mutations");
    PracticeCleanup(0); PracticeShowMain(0); TestSaveFails = true; MainChoose(SP_MAIN_INDICATORS);
    Check(!PracticeZoneIndicators[0] && PracticeSavePending[0] && !TestLivePickups,
        "save failure keeps the new preference in memory and exposes retry");
    TestSaveFails = false; MainChoose(SP_MAIN_RETRY_SAVE);
    Check(!PracticeSavePending[0] && PracticeLoadConfiguration() && !PracticeZoneIndicators[0],
        "retry persists the disabled preference");
}
