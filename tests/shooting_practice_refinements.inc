// Regressions for permitted weapon sets, persistence and participant presentation.
stock TestMultipleWeapons()
{
    ResetTest(2);
    PracticeRules[0][PracticeWeaponMask] |= 1 << (31 - PRACTICE_WEAPON_FIRST);
    TestSlotWeapon[0][5] = 31; TestAmmo[0][5] = 100;
    ShowFirstTarget(); Shoot(true); AdvanceInterval();
    TestWeapon[0] = 31; PracticeTick(0, TestNow);
    Check(PracticeIsBusy(0) && PracticeSession[0][PracticeLastWeapon] == 31, "switching between permitted slots preserves the round");
    PracticeHandleShot(0, 31, BULLET_HIT_TYPE_PLAYER_OBJECT, 1000 + PracticeSession[0][PracticeObject], TestNow);
    Check(TestResult == 1 && TestResultHits == 2 && TestResultShots == 2, "both permitted weapons score in the same round");

    ResetTest(); PracticeRules[0][PracticeWeaponMask] |= 1 << (23 - PRACTICE_WEAPON_FIRST);
    TestWeapon[0] = 23; TestSlotWeapon[0][2] = 23; PracticeStart(0, 0);
    Check(PracticeIsBusy(0) && PracticeSession[0][PracticeLastWeapon] == 23, "a second selected weapon can start the round");
    TestWeapon[0] = 24; TestSlotWeapon[0][2] = 24; PracticeTick(0, TestNow);
    Check(PracticeIsBusy(0) && PracticeSession[0][PracticeLastWeapon] == 24, "permitted weapons sharing a slot are accepted");
    PracticeRules[0][PracticeWeaponMask] = 1 << (34 - PRACTICE_WEAPON_FIRST);
    TestWeapon[0] = 23; PracticeTick(0, TestNow);
    Check(PracticeIsBusy(0), "active round keeps its original allowed weapon set");
    TestWeapon[0] = 34; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestResult == 3, "newly changed global rule cannot authorize an active-round weapon");

    ResetTest(); ShowFirstTarget();
    PracticeHandleShot(0, 31, BULLET_HIT_TYPE_NONE, 0, TestNow);
    Check(!PracticeIsBusy(0) && TestResultShots == 0, "unpermitted shot cancels even when the reported held weapon is permitted");
    ResetTest(); PracticeStart(0, 0); TestWeapon[0] = 0; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0), "switching to fists while last permitted weapon has ammo cancels");
    ResetTest(); PracticeRules[0][PracticeWeaponMask] |= 1 << (31 - PRACTICE_WEAPON_FIRST);
    TestWeapon[0] = 31; TestSlotWeapon[0][5] = 31; TestAmmo[0][5] = 1;
    PracticeStart(0, 0); TestWeapon[0] = 0; TestAmmo[0][5] = 0; PracticeTick(0, TestNow);
    Check(PracticeIsBusy(0), "last permitted rifle exhaustion accepts fists despite another weapon having ammo");
    TestSlotWeapon[0][5] = 30; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0), "different weapon in exhausted slot cannot bypass weapon validation");
    ResetTest(); PracticeStart(0, 0); TestWeapon[0] = 0;
    TestSlotWeapon[0][2] = 0; TestAmmo[0][2] = 0; PracticeTick(0, TestNow);
    Check(PracticeIsBusy(0), "server-cleared exhausted weapon slot is accepted");

    ResetTest(); PracticeRules[0][PracticeWeaponMask] = PRACTICE_WEAPONS_ALL;
    PracticeShowAllowedWeapons(0, 0);
    Check(strfind(TestChat[0], "Pistola") != -1 && strfind(TestChat[0], "Rifle de francotirador") != -1
        && TestLongestMessage < 144, "all permitted names fit across complete help lines");
}

stock TestWeaponDialogs()
{
    ResetTest(); TestCommand_practicaconfig(0, "1"); MenuChoose(2); MenuChoose(0);
    MenuChoose(9);
    Check(PracticeWeaponCount(PracticeRules[0][PracticeWeaponMask]) == 2 && PracticeMenuPage[0] == SP_MENU_WEAPON
        && PracticeLoadConfiguration() && PracticeWeaponAllowed(31, PracticeRules[0][PracticeWeaponMask]),
        "weapon menu adds a choice, stays open and persists the set");
    PracticeShowWeapons(0); // Reload invalidates open dialogs rather than reusing stale row bindings.
    MenuChoose(2);
    Check(PracticeRules[0][PracticeWeaponMask] == (1 << (31 - PRACTICE_WEAPON_FIRST)), "weapon menu removes a permitted choice");
    new revision = PracticeRevision[0];
    MenuChoose(9); MenuChoose(-1); MenuChoose(13);
    Check(PracticeRevision[0] == revision && PracticeWeaponCount(PracticeRules[0][PracticeWeaponMask]) == 1,
        "last choice and invalid weapon rows cannot empty or corrupt the set");
    MenuBack(); Check(PracticeMenuPage[0] == SP_MENU_RULES, "weapon list returns to rules");
    PracticeShowWeapons(0); PracticeShowWeapons(1); MenuChoose(2, 1); MenuChoose(3);
    Check(!PracticeWeaponAllowed(25, PracticeRules[0][PracticeWeaponMask]) && PracticeMenuPage[0] == SP_MENU_MAIN,
        "stale weapon menu cannot mutate newer rules");
    PracticeShowWeapons(0); TestAdmin[0] = false; MenuChoose(3);
    Check(!PracticeWeaponAllowed(25, PracticeRules[0][PracticeWeaponMask]) && PracticeMenuPage[0] == SP_MENU_NONE,
        "lost RCON permission blocks a weapon change");
    TestAdmin[0] = true; PracticeStart(0, 0); PracticeShowWeapons(0); MenuChoose(3);
    Check(!PracticeWeaponAllowed(25, PracticeRules[0][PracticeWeaponMask]), "active practice blocks weapon changes");
    PracticeEnd(0, false, "test"); TestCommand_practicaconfig(0, "1"); MainChoose(SP_MAIN_RESET); MenuChoose(0);
    Check(PracticeRules[0][PracticeWeaponMask] == PRACTICE_WEAPON_DEFAULT, "reset restores only the default Desert Eagle");

    ResetTest(0); PracticeShowRules(0); MenuChoose(1);
    Check(PracticeMenuPage[0] == SP_MENU_RULE_INFO && strfind(TestDialogBody[0], "Primero añade") != -1,
        "empty layout explains prerequisite instead of displaying impossible input bounds");
    MenuChoose(0); Check(PracticeMenuPage[0] == SP_MENU_RULES, "prerequisite explanation returns to rules");
    ResetTest(3); TestSetNoShoot(1); PracticeShowRuleInput(0, PracticePassPoints);
    Check(strfind(TestDialogBody[0], "aciertos menos errores") != -1 && strfind(TestDialogBody[0], "hasta 2 puntos") != -1,
        "minimum score help explains net scoring and actual achievable maximum");
    MenuInput("2"); MenuChoose(2);
    Check(strfind(TestDialogBody[0], "incluso al aire") != -1 && strfind(TestDialogBody[0], "0 exige") != -1,
        "error help explains air shots and zero tolerance");
    MenuBack(); MenuChoose(3);
    Check(strfind(TestDialogBody[0], "cuenta regresiva") != -1 && strfind(TestDialogBody[0], "no cuentan") != -1,
        "preparation help explains countdown and excluded shots");
    MenuBack(); MenuChoose(4);
    Check(strfind(TestDialogBody[0], "Tiempo máximo") != -1 && strfind(TestDialogBody[0], "esperar sin disparar") != -1,
        "exposure help explains both target behaviors");
    MenuBack(); MenuChoose(5);
    Check(strfind(TestDialogBody[0], "reducen la precisión") != -1 && strfind(TestDialogBody[0], "0 elimina") != -1,
        "interval help explains zero and gap shots");
}

stock TestWeaponStorage()
{
    ResetTest();
    for (new version = 1; version <= 4; version++)
    {
        new header[32], rules[64];
        format(header, sizeof(header), "SARP_PRACTICE %d", version);
        if (version == 1) format(rules, sizeof(rules), "31 1 3 5 1");
        else if (version == 2) format(rules, sizeof(rules), "31 1 3 5 1 0 0 0 0");
        else format(rules, sizeof(rules), "31 1 3 5 1 0 0 0");
        TestWriteConfigFixture(header, rules);
        Check(PracticeLoadConfiguration() && PracticeRules[0][PracticeWeaponMask] == (1 << (31 - PRACTICE_WEAPON_FIRST))
            && PracticePoints[0][1][PracticePointX] == 8.25 && PracticePoints[0][1][PracticePointAngle] == 90.5,
            "each legacy format migrates exactly one selected weapon and preserves placement");
        new File:input = fopen(PRACTICE_CONFIG_FILE, io_read), savedHeader[32];
        PracticeReadLine(input, savedHeader, sizeof(savedHeader)); fclose(input);
        Check(!strcmp(header, savedHeader), "loading a legacy format does not rewrite its saved file");
    }
    Check(PracticePoints[0][1][PracticePointModel] == 1583 && PracticePoints[0][1][PracticePointRotX] == 15.25
        && PracticePoints[0][1][PracticePointRotY] == 36.5 && !strcmp(PracticePoints[0][1][PracticePointName], "Entrada lateral"),
        "v4 migration preserves custom model, complete orientation and label");
    PracticeRules[0][PracticeWeaponMask] = PRACTICE_WEAPONS_ALL; PracticeSaveConfiguration();
    PracticeRules[0][PracticeWeaponMask] = PRACTICE_WEAPON_DEFAULT;
    Check(PracticeLoadConfiguration() && PracticeRules[0][PracticeWeaponMask] == PRACTICE_WEAPONS_ALL,
        "v6 round trip retains every supported weapon including range endpoints");
    new invalidMasks[] = {0, -1, 8192, 8196, 2147483647};
    for (new i = 0; i < sizeof(invalidMasks); i++)
    {
        new rules[64]; format(rules, sizeof(rules), "%d 1 3 5 1 0 0 0", invalidMasks[i]);
        TestWriteConfigFixture("SARP_PRACTICE 5", rules);
        Check(!PracticeLoadConfiguration() && PracticeRules[0][PracticeWeaponMask] == PRACTICE_WEAPONS_ALL,
            "invalid v5 mask rejects entire file without replacing active configuration");
    }
    TestWriteConfigFixture("SARP_PRACTICE 4", "21 1 3 5 1 0 0 0");
    Check(!PracticeLoadConfiguration(), "unsupported legacy weapon cannot become a valid bitmask accidentally");
    Check(!PracticeWeaponAllowed(0, PRACTICE_WEAPONS_ALL) && !PracticeWeaponAllowed(35, PRACTICE_WEAPONS_ALL),
        "weapon IDs outside supported range never wrap into permitted bits");
}

stock TestCountdownAndDeparture()
{
    ResetTest(1); PracticeStart(0, 0);
    new PlayerText:countdown = PracticeCountdownText[0];
    Check(TestTDCount[0] == 1 && !TestTDCount[1] && TestTDVisible[_:countdown]
        && !strcmp(TestTDText[_:countdown], "Comienza en 3"), "countdown initially shows three only to participant");
    TestNow += 1000; PracticeTick(0, TestNow);
    Check(PracticeCountdownText[0] == countdown && !strcmp(TestTDText[_:countdown], "Comienza en 2"),
        "second countdown value updates the same owned textdraw");
    TestNow += 1000; PracticeTick(0, TestNow);
    Check(!strcmp(TestTDText[_:countdown], "Comienza en 1"), "visual countdown reaches one with its beep");
    TestNow += 1000; PracticeTick(0, TestNow);
    Check(!TestTDCount[0] && !TestTDVisible[_:countdown] && PracticeCountdownText[0] == PlayerText:INVALID_TEXT_DRAW,
        "first target removes countdown completely");
    Shoot(true); Check(!TestTDCount[0], "completed round has no countdown resource");

    ResetTest(); PracticeStart(0, 0); TestNow += 2500; PracticeTick(0, TestNow);
    Check(TestTDCount[0] == 1 && !strcmp(TestTDText[_:PracticeCountdownText[0]], "Comienza en 1")
        && TestSoundCount[0] == 2, "delayed tick synchronizes current number without stale flash or beep burst");
    TestPosition[0][0] = 4.0; TestPosition[2][0] = 50.0; TestWorld[3] = 1; TestInterior[4] = 1;
    PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && !TestTDCount[0] && !strcmp(TestLastMessage[1], "* Test Player 0 abandona el puesto de tiro."),
        "walking out during countdown removes overlay and announces named departure");
    Check(strfind(TestChat[0], "Incompleta: Cancelada automáticamente por salir del puesto de tiro.") >= 0,
        "automatic departure reports the exact private incomplete reason");
    Check(TestMessages[1] == 2 && TestMessages[2] == 1 && TestMessages[3] == 1 && TestMessages[4] == 1,
        "departure is delivered only to observers near the original station in the same dimension");
    PracticeTick(0, TestNow); PracticeEnd(0, false);
    Check(TestMessages[1] == 2, "departure is announced exactly once");

    ResetTest(); PracticeStart(0, 0); TestCommand_practica(0, "cancelar");
    Check(!TestTDCount[0] && strfind(TestChat[1], "abandona") == -1, "manual cancellation cleans countdown without claiming a departure");
    ResetTest(); PracticeStart(0, 0); TestHealth[0] = 0.0; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && !TestTDCount[0], "death during countdown removes overlay");
    ResetTest(); PracticeStart(0, 0); TestState[0] = PLAYER_STATE_DRIVER; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && !TestTDCount[0], "vehicle entry during countdown removes overlay");
    ResetTest(); PracticeStart(0, 0); TestDisconnect(0); PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && PracticeCountdownText[0] == PlayerText:INVALID_TEXT_DRAW,
        "disconnect clears countdown handle after native player resource disposal");
    TestConnected[0] = true; PracticeStart(0, 0);
    Check(TestTDCount[0] == 1 && !strcmp(TestTDText[_:PracticeCountdownText[0]], "Comienza en 3"),
        "reused player ID gets a fresh countdown without leaked widgets");
    PracticeCleanup(0); Check(!TestTDCount[0], "gamemode cleanup releases countdown");
    ResetTest(1); TestTDFails = true; ShowFirstTarget();
    Check(PracticeIsBusy(0) && !TestTDCount[0] && TestSoundCount[0] == 2,
        "textdraw allocation failure preserves audible countdown and functional round");
}

stock TestDetailedResults()
{
    ResetTest(4); TestSetNoShoot(2); ShowFirstTarget();
    Shoot(true); AdvanceInterval(); TestNow += 5000; PracticeTick(0, TestNow);
    AdvanceInterval(); TestNow += 5000; PracticeTick(0, TestNow); AdvanceInterval(); Shoot(false);
    Check(strfind(TestChat[0], "Puntuación neta: 0 {A8C4D9}|{FFFFFF} Veredicto: {FF0000}Reprobado{FFFFFF}", true) != -1
        && TestLastMessageColour[0] == PRACTICE_SECTION_COLOUR
        && strfind(TestChat[0], "Puntos mínimos: 1 {A8C4D9}|{FFFFFF} Errores máximos: 0") != -1,
        "only failed word is red, while label, trailing score and thresholds are white");
    Check(strfind(TestChat[0], "Aciertos: 1/2 {A8C4D9}|{FFFFFF} Sin impactar a tiempo: 1") != -1
        && strfind(TestChat[0], "Protegidos respetados: 1/2 {A8C4D9}|{FFFFFF} Errores de identificación: 1") != -1,
        "report separates missed shoot targets, correct restraint and identification errors");
    Check(strfind(TestChat[0], "Disparos: 2 {A8C4D9}|{FFFFFF} Precisión: 50.0%") != -1
        && strfind(TestChat[0], "% (") == -1 && strfind(TestChat[0], "Veredicto:") != -1,
        "accuracy has no attached explanation and verdict gets its own final line");
    Check(TestPrivateMessages[0] == 8 && !TestPrivateMessages[1] && TestLongestMessage < 144 && !TestResultColourWrong,
        "five white result rows and two soft blue dividers fit within chat limits");
    ResetTest(1); ShowFirstTarget(); TestNow += 5000; PracticeTick(0, TestNow);
    Check(strfind(TestChat[0], "Disparos: 0 {A8C4D9}|{FFFFFF} Precisión: no disponible") != -1 && !TestResultColourWrong,
        "zero-shot report leaves precision unavailable in white");
}

stock TestExtraShotPrecision()
{
    ResetTest(1); ShowFirstTarget(); Shoot(true);
    Check(TestResult == 1 && TestResultShots == 1 && TestResultHits == 1
        && strfind(TestChat[0], "Precisión: 100.0%") != -1,
        "one shot and one hit give full accuracy");
    ResetTest(1); ShowFirstTarget(); Shoot(false); Shoot(false); Shoot(true);
    Check(TestResult == 1 && TestResultShots == 3 && TestResultHits == 1
        && strfind(TestChat[0], "Precisión: 33.3%") != -1,
        "repeat attempts on one target lower accuracy while keeping the net-points verdict");
    ResetTest(2); PracticeRules[0][PracticePassPoints] = 2; ShowFirstTarget();
    Shoot(false); Shoot(true); Shoot(true); AdvanceInterval(); Shoot(true);
    Check(TestResult == 1 && TestResultHits == 2 && TestResultShots == 4 && !TestResultErrors
        && strfind(TestChat[0], "Precisión: 50.0%") != -1,
        "misses and interval shots all lower accuracy without adding identification errors");
}


stock TestMatchingReportBorders()
{
    ResetTest();
    new result[E_PRACTICE_SESSION];
    result[PracticeHits] = result[PracticeShootTotal] = result[PracticeNoShootTotal] = 10;
    result[PracticeCorrectIgnores] = result[PracticeIdentificationErrors] = 10;
    result[PracticeShots] = 99;
    PracticeReportResult(0, true, result, "");
    Check(TestPrivateMessages[0] == 8 && !strcmp(TestReportLines[0][0], TestReportLines[0][7])
        && strlen(TestReportLines[0][0]) == 94 && !strcmp(TestReportLines[0][1], "CAMPO DE TIRO"),
        "completed report has identical extended opening/closing borders and a separate colored title");
    new bool:withinBorder = true;
    for (new row = 2; row < 7; row++)
    {
        new visible;
        for (new i = 0; TestReportLines[0][row][i]; i++)
        {
            if (TestReportLines[0][row][i] == '{') { i += 7; continue; }
            visible++;
        }
        withinBorder = withinBorder && visible < strlen(TestReportLines[0][0]);
    }
    Check(withinBorder && TestLongestMessage < 144 && !TestResultColourWrong,
        "all paired result rows fit the extended separator and remain within the chat limit");
    TestPrivateMessages[0] = 0;
    PracticeReportResult(0, false, result, "Cancelada automáticamente por salir del puesto de tiro.");
    Check(TestPrivateMessages[0] == 4 && !strcmp(TestReportLines[0][0], TestReportLines[0][3])
        && !strcmp(TestReportLines[0][1], "CAMPO DE TIRO") && !TestResultColourWrong,
        "incomplete report reuses the same matching separators");
}
