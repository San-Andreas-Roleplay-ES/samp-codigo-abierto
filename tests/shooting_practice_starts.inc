// Multiple public start points share one exclusive zone session.
stock TestSecondStart(zone = 0, Float:y = 20.0)
{
    new slot = PracticeStartSlot(1);
    PracticePoints[zone][slot] = PracticePoints[zone][0];
    PracticePoints[zone][slot][PracticePointY] = y;
    return slot;
}

stock TestStartMenus()
{
    ResetTest(); PracticeShowMain(0); MainChoose(SP_MAIN_START_POINT);
    Check(PracticeMenuPage[0] == SP_MENU_STARTS && PracticeStartRowCount[0] == 3
        && strfind(TestDialogBody[0], "Añadir punto") >= 0 && PracticePoints[0][0][PracticePointX] == 0.0,
        "root opens a point list without overwriting the existing start");
    TestPosition[0][1] = 6.0; MenuChoose(0);
    new second = PracticeStartSlot(1);
    Check(PracticeCountStarts(0) == 2 && PracticePoints[0][second][PracticePointY] == 6.0
        && PracticePoints[0][second][PracticePointAngle] == 90.0 && PracticeCountTargets(0) == 3
        && PracticeMenuPage[0] == SP_MENU_START && !PracticeConfigurationEditing(0),
        "adding a point captures position and facing and retains common targets");
    Check(TestLiveCP == 2 && TestLiveLabels == 5 && TestLiveObjects == 3,
        "point menus show all saved posts and common targets");
    MenuChoose(0); MenuInput("Caseta norte");
    Check(!strcmp(PracticePoints[0][second][PracticePointName], "Caseta norte")
        && strfind(TestDialogBody[0], "Caseta norte") >= 0, "point can be named through its menu");
    MenuChoose(0); MenuInput("{FF0000}bad");
    Check(!strcmp(PracticePoints[0][second][PracticePointName], "Caseta norte"), "point names reject color injection");
    MenuChoose(0); MenuInput("-");
    Check(!PracticePoints[0][second][PracticePointName][0] && strfind(TestDialogBody[0], "Punto 2") >= 0,
        "automatic point names can be restored");
    TestPosition[0][1] = 7.0; MenuChoose(1);
    Check(PracticePoints[0][second][PracticePointY] == 7.0 && PracticePoints[0][0][PracticePointY] == 0.0,
        "relocating a point updates only the selected post");
    MenuChoose(2); MenuBack();
    Check(PracticeCountStarts(0) == 2, "deleting a point requires confirmation");
    MenuChoose(2); MenuChoose(0);
    Check(PracticeCountStarts(0) == 1 && PracticeMenuPage[0] == SP_MENU_STARTS && PracticeCountTargets(0) == 3,
        "confirmed deletion retains targets and other posts");
    MenuChoose(1); MenuChoose(2); MenuChoose(0);
    Check(!PracticeCountStarts(0) && !PracticeZoneEnabled[0] && PracticeCountTargets(0) == 3,
        "deleting the final point closes the zone without erasing targets");
    Check(PracticeLoadConfiguration() && !PracticeCountStarts(0) && PracticeCountTargets(0) == 3,
        "partial configuration without starts survives reload");
    TestPosition[0][1] = 0.0; AddStart();
    Check(PracticeCountStarts(0) == 1 && PracticeCountTargets(0) == 3, "a start can be restored while retaining the targets");

    ResetTest(); TestPosition[0][0] = 1000.0; AddStart();
    Check(PracticeCountStarts(0) == 1, "distant post cannot be added to the same field");
    TestPosition[0][0] = 8.0; AddStart();
    Check(PracticeCountStarts(0) == 1, "post cannot overlap a target inside the three metre limit");
    TestPosition[0][0] = 0.0; TestWorld[0] = 9; AddStart();
    Check(PracticeCountStarts(0) == 1 && PracticePoints[0][0][PracticePointWorld] == 0,
        "additional post cannot change the field dimension");
    ResetTest(); second = TestSecondStart(0, 6.0); TestWorld[0] = 9; MarkExistingStart();
    Check(PracticeCountStarts(0) == 2 && PracticePoints[0][0][PracticePointWorld] == 0
        && PracticeCountTargets(0) == 3 && !PracticeConfigurationEditing(0),
        "multi-post relocation cannot clear common targets in another world");

    ResetTest();
    for (new i = 1; i < PRACTICE_MAX_STARTS; i++)
    {
        TestPosition[0][1] = float(i * 4); AddStart();
    }
    new revision = PracticeRevision[0]; AddStart();
    Check(PracticeCountStarts(0) == PRACTICE_MAX_STARTS && PracticeRevision[0] == revision
        && PracticeStartRowCount[0] == PRACTICE_MAX_STARTS + 2, "point list enforces its bounded capacity");

    ResetTest(); PracticeShowMain(0); MainChoose(SP_MAIN_START_POINT); MenuChoose(1); MenuChoose(2);
    PracticeShowMain(1); MainChoose(SP_MAIN_START_POINT, 1); MenuChoose(1, 1); MenuChoose(0, 1); MenuInput("Updated", 1);
    MenuChoose(0);
    Check(PracticeCountStarts(0) == 1 && !strcmp(PracticePoints[0][0][PracticePointName], "Updated"),
        "stale delete dialog cannot erase a newer configuration");
    PracticeShowMain(0); MainChoose(SP_MAIN_START_POINT); MenuChoose(1); MenuChoose(2);
    TestAdmin[0] = false; MenuChoose(0);
    Check(PracticeCountStarts(0) == 1 && PracticeMenuPage[0] == SP_MENU_NONE, "revoked RCON permission prevents point mutation");
}

stock TestStartSessions()
{
    ResetTest(); new second = TestSecondStart(), reason[144];
    TestPosition[0][1] = 20.0;
    Check(PracticeConfigurationReady(0, reason) && PracticeCanStart(0, 0), "every valid post can start the shared exercise");
    TestCommand_practica(0, "");
    Check(PracticeSessionStartSlot[0] == second && PracticeSessionOrigin[0][PracticePointY] == 20.0
        && PracticeIsBusy(0), "public command selects and freezes the nearest post");
    Check(TestSoundCount[0] == 1 && !TestSoundCount[1], "countdown sound is anchored to the selected post");
    TestCommand_practica(1, "");
    Check(PracticeSession[0][PracticePlayer] == 0 && PracticeGetPlayerZone(1) < 0
        && strfind(TestLastMessage[1], "práctica activa") >= 0,
        "a practice at one post blocks other posts of the same zone");
    PracticePoints[0][second][PracticePointY] = 25.0;
    Check(PracticeParticipantValid(0) && PracticeSessionOrigin[0][PracticePointY] == 20.0,
        "session movement checks use the frozen post rather than mutable configuration");
    PracticePoints[0][second][PracticePointY] = 20.0;
    TestNow += 3000; PracticeTick(0, TestNow);
    new objectid = PracticeSession[0][PracticeObject];
    Check(TestObjectOwner[objectid] == -1 && TestObjectWorld[objectid] == 0 && TestObjectInterior[objectid] == 0,
        "exercise target remains public in the field dimension");
    PracticeShowMain(1); MainChoose(SP_MAIN_START_POINT, 1); MenuChoose(1, 1); MenuChoose(2, 1); MenuChoose(0, 1);
    Check(PracticeCountStarts(0) == 2 && PracticeIsBusy(0), "point editing is blocked during the shared practice");
    TestPosition[0][1] = 0.0; PracticeTick(0, TestNow);
    Check(!PracticeIsBusy(0) && TestResult == 3 && strfind(TestChat[0], "salir del puesto") >= 0
        && !PracticeSessionOrigin[0][PracticePointSet] && PracticeSessionStartSlot[0] == -1,
        "moving to another configured post abandons the original practice");
    Check(strfind(TestChat[1], "abandona") < 0, "departure message remains anchored to the original selected post");
    TestAdmin[1] = false; TestCommand_practica(1, "");
    Check(PracticeIsBusy(0) && PracticeSessionStartSlot[0] == 0 && PracticeSession[0][PracticePlayer] == 1,
        "the field is released and can be used at another post without RCON");

    ResetTest(); second = TestSecondStart(0, 2.0); TestPosition[0][1] = 1.4;
    Check(PracticeNearestStart(0, 0, PRACTICE_START_RADIUS) == second, "overlapping start radii choose the nearest post");
    TestPosition[0][1] = 1.0;
    Check(PracticeNearestStart(0, 0, PRACTICE_START_RADIUS) == 0, "equal-distance posts use stable slot order");
    TestWorld[0] = 1;
    Check(PracticeNearestStart(0, 0, PRACTICE_START_RADIUS) < 0, "start proximity also checks virtual world");
    TestWorld[0] = 0; TestInterior[0] = 1;
    Check(PracticeNearestStart(0, 0, PRACTICE_START_RADIUS) < 0, "start proximity also checks interior");

    ResetTest(); second = TestSecondStart(0, 6.0); PracticePoints[0][0][PracticePointSet] = false;
    TestPosition[0][1] = 6.0; TestCommand_practica(0, "1");
    Check(PracticeIsBusy(0) && PracticeSessionStartSlot[0] == second, "sparse point list works after removing the original legacy start");
    ResetTest(); second = TestSecondStart(0, 6.0);
    PracticePoints[0][1][PracticePointY] = 6.0; PracticePoints[0][1][PracticePointX] = 1.0;
    Check(!PracticeConfigurationReady(0, reason), "common targets must be at least three metres from every post");
    ResetTest(); second = TestSecondStart(0, 6.0); OpenTarget(1); MenuChoose(3);
    EditorResult(EDIT_RESPONSE_FINAL, 1.0, 6.0);
    Check(PracticeMenuPage[0] == SP_MENU_EDIT_ERROR && PracticePoints[0][1][PracticePointX] == 8.0,
        "native final rejects a target valid for point one but too close to another post");
    MenuBack(); OpenTarget(1); MenuChoose(3); EditorResult(EDIT_RESPONSE_FINAL, 12.0, 2.0);
    Check(PracticePoints[0][1][PracticePointX] == 12.0 && PracticePoints[0][1][PracticePointY] == 2.0
        && !PracticeConfigurationEditing(0), "native final accepts a target valid from every post");
    ResetTest(); second = TestSecondStart(0, 6.0); PracticePoints[0][0][PracticePointSet] = false;
    TestPosition[0][1] = 6.0; OpenTarget(1);
    Check(PracticeHasDraft(0) && TestLiveCP == 1 && TestLiveLabels == 4,
        "target editing and preview use the remaining post when the original start is absent");
}

stock TestStartPreviews()
{
    ResetTest(); new second = TestSecondStart(0, 6.0);
    PracticeShowPreview(0); PracticeShowPreview(1);
    Check(TestLiveObjects == 6 && TestLiveCP == 4 && TestLiveLabels == 10,
        "each private preview owns all target and post markers");
    new cp = PracticePreviewCheckpoint[0][0], secondCP = PracticePreviewCheckpoint[0][second];
    new objectid = PracticePreviewObjects[0][0], otherCP = PracticePreviewCheckpoint[1][second];
    Check(TestCPOwner[secondCP] == 0 && TestCPPosition[secondCP][1] == 6.0
        && strfind(TestLabelText[PracticePreviewLabels[0][second]], "Punto 2") >= 0,
        "additional preview marker uses the actual post coordinates and name");
    PracticePoints[0][second][PracticePointY] = 7.0; PracticeConfigurationChanged(0);
    Check(PracticePreviewCheckpoint[0][0] == cp && PracticePreviewObjects[0][0] == objectid
        && PracticePreviewCheckpoint[0][second] != secondCP && PracticePreviewCheckpoint[1][second] != otherCP,
        "changing a post refreshes only its markers in each viewer");
    PracticeHidePreview(0);
    Check(TestLiveObjects == 3 && TestLiveCP == 2 && TestLiveLabels == 5, "hiding one preview retains all other viewer markers");
    PracticeStart(0, 2);
    Check(!TestLiveCP && !TestLiveLabels && !TestLiveObjects && PracticeIsBusy(0), "starting clears all previews before the public exercise");
    PracticeCleanup(0); PracticeShowPreview(0); TestWorld[0] = 1; PracticeEditorTick();
    Check(!TestLiveCP && !TestLiveLabels && !TestLiveObjects, "dimension change releases every preview marker");
}

// Exact v7 file format, independent of the v8 writer.
stock TestWriteV7Starts()
{
    new File:output = fopen(PRACTICE_CONFIG_FILE, io_write), line[256];
    fwrite(output, "SARP_PRACTICE 7\r\nREGISTRY 2 1\r\nZONE 1 1 0\r\nZONE_NAME Campo legado\r\n");
    format(line, sizeof(line), "%d 1 3 5000 1000 0 0 0\r\n", PRACTICE_WEAPON_DEFAULT); fwrite(output, line);
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        format(line, sizeof(line), "%d %.6f %.6f %.6f %.6f %d %d %d %d %.6f %.6f\r\n",
            PracticePoints[0][slot][PracticePointSet], PracticePoints[0][slot][PracticePointX], PracticePoints[0][slot][PracticePointY],
            PracticePoints[0][slot][PracticePointZ], PracticePoints[0][slot][PracticePointAngle], PracticePoints[0][slot][PracticePointInterior],
            PracticePoints[0][slot][PracticePointWorld], PracticePoints[0][slot][PracticePointNoShoot], PracticePoints[0][slot][PracticePointModel],
            PracticePoints[0][slot][PracticePointRotX], PracticePoints[0][slot][PracticePointRotY]);
        fwrite(output, line);
        format(line, sizeof(line), "NAME %s\r\n", PracticePoints[0][slot][PracticePointName]); fwrite(output, line);
    }
    fwrite(output, "END_ZONE\r\nEND\r\n"); fclose(output);
}

stock TestStartStorage()
{
    ResetTest(); new second = TestSecondStart(0, 6.0);
    format(PracticePoints[0][second][PracticePointName], PRACTICE_NAME_SIZE, "Caseta 100%%");
    PracticePoints[0][second][PracticePointAngle] = 181.25;
    Check(PracticeSaveConfiguration(), "v9 saves multiple starts");
    new empty[E_PRACTICE_POINT]; PracticePoints[0][0] = empty; PracticePoints[0][second] = empty;
    Check(PracticeLoadConfiguration() && PracticeCountStarts(0) == 2
        && PracticePoints[0][second][PracticePointY] == 6.0 && PracticePoints[0][second][PracticePointAngle] == 181.25
        && !strcmp(PracticePoints[0][second][PracticePointName], "Caseta 100%") && PracticeZoneEnabled[0],
        "round trip retains all point transforms and literal names");
    PracticePoints[0][0][PracticePointSet] = false; PracticeSaveConfiguration();
    Check(PracticeLoadConfiguration() && PracticeCountStarts(0) == 1 && PracticeFirstStart(0) == second,
        "sparse point slots survive reload");

    ResetTest(); format(PracticePoints[0][0][PracticePointName], PRACTICE_NAME_SIZE, "Puesto anterior");
    PracticePoints[0][0][PracticePointAngle] = 181.25; TestWriteV7Starts(); TestSecondStart(0, 6.0);
    Check(PracticeLoadConfiguration() && PracticeCountStarts(0) == 1
        && PracticePoints[0][0][PracticePointAngle] == 181.25
        && !strcmp(PracticePoints[0][0][PracticePointName], "Puesto anterior") && PracticeZoneEnabled[0]
        && PracticeCountTargets(0) == 3, "v7 migrates the existing start as point one without losing targets or rules");
    new File:input = fopen(PRACTICE_CONFIG_FILE, io_read), header[32]; PracticeReadLine(input, header); fclose(input);
    Check(!strcmp(header, "SARP_PRACTICE 7"), "loading legacy registry preserves the source file until a confirmed save");
    Check(PracticeSaveConfiguration(), "migrated registry can be upgraded on confirmed save");
    input = fopen(PRACTICE_CONFIG_FILE, io_read); PracticeReadLine(input, header); fclose(input);
    Check(!strcmp(header, "SARP_PRACTICE 9"), "confirmed save upgrades to v9");
    TestReplaceConfigLine(5 + 2 * second, "1 0 6 0 90 0 9 0 0 0 0");
    Check(!PracticeReadRegistry(PRACTICE_CONFIG_FILE), "corrupt added start in another world rejects the full registry");
    ResetTest(); TestSecondStart(0, 6.0); PracticeSaveConfiguration();
    TestReplaceConfigLine(5 + 2 * second, "1 0 6 0 90 0 0 1 0 0 0");
    Check(!PracticeReadRegistry(PRACTICE_CONFIG_FILE), "start record cannot acquire a target behavior");
    ResetTest(); TestSecondStart(0, 6.0); PracticeSaveConfiguration();
    TestReplaceConfigLine(5 + 2 * second, "1 0 6 0 90 0 0 0 1583 0 0");
    Check(!PracticeReadRegistry(PRACTICE_CONFIG_FILE), "start record cannot acquire a target model");
}
