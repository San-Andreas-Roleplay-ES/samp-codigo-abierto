// Behavior checks for conditional root actions, placement radius and stable edit references.
stock TestConditionalMain()
{
    ResetTest(0); PracticeShowMain(0);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE) && MainHasAction(SP_MAIN_RESET),
        "incomplete layout omits start while keeping reset reachable");
    MainChoose(SP_MAIN_RESET);
    Check(PracticeMenuPage[0] == SP_MENU_RESET, "shifted root row still opens reset rather than starting");
    MenuBack(); PracticePoints[0][1][PracticePointSet] = true;
    PracticePoints[0][1][PracticePointModel] = PRACTICE_TARGET_MODEL; PracticePoints[0][1][PracticePointX] = 8.0;
    PracticeShowMain(0);
    Check(MainHasAction(SP_MAIN_START_PRACTICE), "start becomes available after the layout is completed");

    TestPosition[0][0] = 2.0; PracticeShowMain(0);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE), "valid layout still omits start when participant is outside start radius");
    TestPosition[0][0] = 0.0; TestWeapon[0] = 23; PracticeShowMain(0);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE), "unpermitted equipped weapon omits start");
    TestWeapon[0] = 24; TestState[0] = PLAYER_STATE_DRIVER; PracticeShowMain(0);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE), "vehicle state omits start");
    TestState[0] = PLAYER_STATE_ONFOOT; TestHealth[0] = 0.0; PracticeShowMain(0);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE), "dead participant cannot start from the root");
    TestHealth[0] = 100.0; TestWorld[0] = 1; PracticeShowMain(0);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE), "wrong world omits start");
    TestWorld[0] = 0; PracticeShowMain(0); TestPosition[0][0] = 4.0;
    MainChoose(SP_MAIN_START_PRACTICE);
    Check(!PracticeIsBusy(0) && PracticeCountTargets(0) == 1 && !MainHasAction(SP_MAIN_START_PRACTICE),
        "moving after opening menu rechecks start without accidentally resetting shifted rows");
    TestSaveFails = true; PracticeConfigurationChanged(0); PracticeShowMain(0);
    MainChoose(SP_MAIN_RETRY_SAVE); TestSaveFails = false; MainChoose(SP_MAIN_RETRY_SAVE);
    Check(!PracticeSavePending[0] && PracticeLoadConfiguration() && PracticeCountTargets(0) == 1,
        "retry save works when start is omitted and optional rows shift");

    ResetTest(); OpenTarget(1); PracticeShowMain(1);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE, 1), "active configuration draft omits start");
    PracticeResetUI(0); PracticeStart(0, 0); PracticeShowMain(1);
    Check(!MainHasAction(SP_MAIN_START_PRACTICE, 1) && !MainHasAction(SP_MAIN_CANCEL_PRACTICE, 1),
        "occupied station exposes neither start nor another participant's cancellation");
    PracticeShowMain(0);
    Check(MainHasAction(SP_MAIN_CANCEL_PRACTICE) && !MainHasAction(SP_MAIN_START_PRACTICE),
        "participant sees cancel while their practice is running");
    MainChoose(SP_MAIN_CANCEL_PRACTICE);
    Check(!PracticeIsBusy(0) && MainHasAction(SP_MAIN_START_PRACTICE), "cancelling through mapped action makes the station reusable");
}

stock TestPlacementRadius()
{
    ResetTest(); TestPosition[0][0] = 2000.0; OpenTarget(0);
    Check(!PracticeConfigurationEditing(0) && !TestCatalogue[0] && !TestLiveObjects && PracticeCountTargets(0) == 3,
        "creation from another city is rejected before allocating wizard or objects");
    TestPosition[0][0] = 100.01; OpenTarget(1);
    Check(!PracticeHasDraft(0) && !PracticeEditObject[0], "editing beyond placement radius is rejected early");
    TestPosition[0][0] = -100.0; OpenTarget(0);
    Check(PracticeHasDraft(0) && PracticeTargetDistanceValid(PracticePoints[0][0], PracticeDraft[0])
        && PracticeDraft[0][PracticePointX] == -100.0,
        "boundary creation keeps the initial object inside the allowed radius");
    SelectModel(1271); MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, -100.0);
    Check(PracticePoints[0][4][PracticePointSet] && PracticePoints[0][4][PracticePointX] == -100.0,
        "outer boundary placement is accepted and saved");
    MenuBack(); TestPosition[0][0] = 0.0; OpenTarget(1); MenuChoose(3);
    EditorResult(EDIT_RESPONSE_FINAL, 100.01);
    Check(PracticeMenuPage[0] == SP_MENU_EDIT_ERROR && PracticePoints[0][1][PracticePointX] == 8.0,
        "native final callback cannot place a target outside the radius");
    MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, 2.99);
    Check(PracticeMenuPage[0] == SP_MENU_EDIT_ERROR && PracticePoints[0][1][PracticePointX] == 8.0,
        "native final callback rejects positions inside the firing area");
    MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, 3.0);
    Check(PracticePoints[0][1][PracticePointX] == 3.0, "inner target boundary is accepted");
    OpenTarget(1); MenuChoose(3); new staleEditor = PracticeEditObject[0];
    TestPosition[0][0] = 101.0; PracticeEditorTick();
    PracticeHandleObjectEdit(0, staleEditor, EDIT_RESPONSE_FINAL, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    Check(!PracticeConfigurationEditing(0) && !TestLiveObjects && !TestLiveCP && !TestLiveLabels
        && PracticePoints[0][1][PracticePointX] == 3.0,
        "walking outside placement radius cleans context and stale final cannot save");

    ResetTest(); TestPosition[0][2] = 101.0; OpenTarget(0);
    Check(!PracticeHasDraft(0), "placement uses three-dimensional distance rather than only map distance");
    TestPosition[0][2] = 0.0; TestInterior[0] = 1; OpenTarget(0);
    Check(!PracticeHasDraft(0) && !TestLiveObjects, "placement in another interior allocates no edit context");
}

stock TestPersistentSetupView()
{
    ResetTest(); TestCommand_practicaconfig(0, "1"); MenuChoose(1);
    new first = PracticePreviewObjects[0][0], neighbour = PracticePreviewObjects[0][1], last = PracticePreviewObjects[0][2];
    Check(PracticeSetupView[0] && !PracticePreviewActive[0] && TestLiveObjects == 3 && TestLiveCP == 1,
        "opening targets automatically shows all saved objects and the start");
    Check(!strcmp(TestLabelText[PracticePreviewLabels[0][1]], "Blanco 1\nDISPARAR"),
        "automatic object name has no duplicated numeric prefix");
    MenuChoose(1); MenuChoose(1);
    Check(TestObjectValid(first) && TestObjectValid(neighbour) && TestObjectValid(last),
        "model catalogue preserves saved objects as configuration references");
    SelectModel(1584); MenuChoose(3);
    new edited = PracticeEditObject[0];
    Check(!TestObjectValid(first) && TestObjectValid(edited) && PracticePreviewObjects[0][1] == neighbour
        && PracticePreviewObjects[0][2] == last && TestLiveObjects == 3,
        "native editor replaces only selected ghost while neighbour handles stay stable");
    EditorResult(EDIT_RESPONSE_UPDATE, 20.0);
    Check(PracticePreviewObjects[0][1] == neighbour && PracticePreviewObjects[0][2] == last,
        "dragging leaves neighbouring objects visible without recreating them");
    EditorResult(EDIT_RESPONSE_FINAL, 20.0);
    new saved = PracticePreviewObjects[0][0];
    Check(!TestObjectValid(edited) && TestObjectValid(saved) && TestObjectModels[saved] == 1584
        && TestObjectPosition[saved][0] == 20.0 && PracticePreviewObjects[0][1] == neighbour
        && PracticePreviewObjects[0][2] == last, "save refreshes the edited object and retains untouched reference handles");
    MenuChoose(2); MenuChoose(0); MenuInput("Puerta"); MenuChoose(7);
    Check(!strcmp(TestLabelText[PracticePreviewLabels[0][2]], "Puerta\nDISPARAR")
        && PracticePreviewObjects[0][0] == saved && PracticePreviewObjects[0][2] == last,
        "custom label has no numeric prefix and other saved references remain stable");
    MenuChoose(1); MenuChoose(3); EditorResult(EDIT_RESPONSE_CANCEL);
    Check(TestLiveObjects == 3 && TestObjectPosition[PracticePreviewObjects[0][0]][0] == 20.0,
        "native cancel restores the previously saved target among its neighbours");
    MenuBack();
    Check(PracticeMenuPage[0] == SP_MENU_MAIN && !TestLiveObjects && !TestLiveCP && !TestLiveLabels,
        "leaving target configuration clears automatic private references");

    PracticeShowPreview(0); first = PracticePreviewObjects[0][0]; neighbour = PracticePreviewObjects[0][1];
    PracticeShowTargets(0); MenuChoose(1); MenuBack(); MenuBack();
    Check(PracticePreviewActive[0] && !PracticeSetupView[0] && TestLiveObjects == 3
        && PracticePreviewObjects[0][0] == first && PracticePreviewObjects[0][1] == neighbour,
        "explicit root preview remains enabled after returning from configuration");
    PracticeHidePreview(0); PracticeShowTargets(0); MenuChoose(1); MenuChoose(4);
    EditorResult(EDIT_RESPONSE_FINAL, 40.0);
    Check(PracticeCountTargets(0) == 4 && TestLiveObjects == 4 && !PracticeEditObject[0],
        "duplicating adds a saved reference without hiding the original");
    MenuChoose(4); MenuChoose(5); MenuChoose(0);
    Check(PracticeCountTargets(0) == 3 && TestLiveObjects == 3,
        "deletion removes only the deleted reference");
    PracticeShowMain(0); MainChoose(SP_MAIN_START_PRACTICE);
    Check(PracticeIsBusy(0) && !TestLiveObjects && !TestLiveCP && !TestLiveLabels,
        "practice start cannot retain configuration references that interfere with shooting");

    ResetTest(); TestLabelFailAt = 3; PracticeShowTargets(0);
    Check(!PracticeSetupView[0] && !TestLiveObjects && !TestLiveLabels && !TestLiveCP,
        "partial automatic context allocation failure rolls back owned resources");
    TestLabelFailAt = 0; PracticeShowTargets(0); TestAdmin[0] = false; PracticeEditorTick();
    Check(!PracticeSetupView[0] && !TestLiveObjects && !TestLiveLabels && !TestLiveCP,
        "lost RCON permission clears automatic configuration view");
    TestAdmin[0] = true; PracticeShowTargets(0); TestState[0] = PLAYER_STATE_DRIVER; PracticeEditorTick();
    Check(!PracticeSetupView[0] && !TestLiveObjects, "vehicle entry clears automatic configuration view");
    TestState[0] = PLAYER_STATE_ONFOOT; PracticeShowTargets(0); TestDisconnect(0); PracticeResetUI(0);
    Check(!TestLiveObjects && !TestLiveLabels && !TestLiveCP, "disconnect removes every automatic reference");
}

stock TestStartPointPlacement()
{
    ResetTest(0); PracticePoints[0][0][PracticePointSet] = false;
    TestPosition[0][0] = 1482.5; TestPosition[0][1] = -1720.25; TestPosition[0][2] = 13.75;
    TestWorld[0] = 7; TestInterior[0] = 3;
    TestCommand_practicaconfig(0, "1"); AddStart();
    Check(PracticePoints[0][0][PracticePointSet] && PracticePoints[0][0][PracticePointX] == 1482.5
        && PracticePoints[0][0][PracticePointY] == -1720.25 && PracticePoints[0][0][PracticePointZ] == 13.75
        && PracticePoints[0][0][PracticePointAngle] == 90.0
        && PracticePoints[0][0][PracticePointWorld] == 7 && PracticePoints[0][0][PracticePointInterior] == 3,
        "first start captures real city coordinates without requiring proximity to an unset origin");
    Check(PracticeMenuPage[0] == SP_MENU_START && !PracticeHasDraft(0) && !PracticeConfigurationEditing(0),
        "first start returns to the point menu and releases draft ownership");
    Check(PracticeLoadConfiguration() && PracticePoints[0][0][PracticePointX] == 1482.5
        && PracticePoints[0][0][PracticePointWorld] == 7 && !PracticeCountTargets(0),
        "distant first start is saved and survives reload");

    ResetTest(3); TestPosition[0][0] = 1500.0; TestPosition[0][1] = -1800.0; TestPosition[0][2] = 13.0;
    TestCommand_practicaconfig(0, "1"); MarkExistingStart();
    Check(PracticePoints[0][0][PracticePointX] == 1500.0 && PracticePoints[0][0][PracticePointY] == -1800.0
        && PracticeCountTargets(0) == 3 && PracticePoints[0][1][PracticePointX] == 8.0,
        "same-dimension start relocation beyond 100 meters preserves saved targets");
    Check(!PracticeConfigurationEditing(0) && PracticeMenuPage[0] == SP_MENU_START,
        "distant relocation releases configuration ownership");

    ResetTest(3); TestPosition[0][0] = 2000.0; TestWorld[0] = 8; TestInterior[0] = 2;
    TestCommand_practicaconfig(0, "1"); MarkExistingStart();
    Check(PracticeMenuPage[0] == SP_MENU_START_CONFIRM && PracticeCountTargets(0) == 3
        && PracticePoints[0][0][PracticePointWorld] == 0, "distant dimension change retains old layout until confirmation");
    PracticeEditorTick();
    Check(PracticeHasDraft(0) && PracticeMenuPage[0] == SP_MENU_START_CONFIRM,
        "editor maintenance preserves a start confirmation far from the old station");
    MenuChoose(0);
    Check(PracticePoints[0][0][PracticePointX] == 2000.0 && PracticePoints[0][0][PracticePointWorld] == 8
        && PracticePoints[0][0][PracticePointInterior] == 2 && !PracticeCountTargets(0)
        && !PracticeConfigurationEditing(0), "distant dimension confirmation commits the new start and clears old targets");
    Check(PracticeLoadConfiguration() && PracticePoints[0][0][PracticePointX] == 2000.0
        && PracticePoints[0][0][PracticePointWorld] == 8, "confirmed distant dimension change is persisted");

    ResetTest(3); TestPosition[0][0] = 2000.0; TestWorld[0] = 8;
    TestCommand_practicaconfig(0, "1"); MarkExistingStart();
    PracticeEditorTick(); MenuBack();
    Check(PracticePoints[0][0][PracticePointX] == 0.0 && PracticePoints[0][0][PracticePointWorld] == 0
        && PracticeCountTargets(0) == 3 && !PracticeConfigurationEditing(0),
        "cancelling distant start confirmation preserves the original station");
}
