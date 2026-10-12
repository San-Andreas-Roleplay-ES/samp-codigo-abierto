// Closing the modal preserves the current section and owned setup context.
stock CloseConfiguration(playerid = 0)
{
    switch (PracticeMenuPage[playerid])
    {
        case SP_MENU_TARGETS: return MenuChoose(PracticeMenuRowCount[playerid] - 1, playerid);
        case SP_MENU_STARTS: return MenuChoose(PracticeStartRowCount[playerid] - 1, playerid);
        case SP_MENU_START: return MenuChoose(3, playerid);
        case SP_MENU_DRAFT: return MenuChoose(PracticeDraftNew[playerid] ? 4 : 6, playerid);
    }
    return 0;
}

stock TestConfigurationPause()
{
    ResetTest(); PracticeShowTargets(0);
    new objectid = PracticePreviewObjects[0][0], cp = PracticePreviewCheckpoint[0][0], revision = PracticeRevision[0];
    Check(strfind(TestDialogBody[0], "Cerrar menú") >= 0 && strfind(TestDialogBody[0], "Continuar con /practicaconfig") >= 0,
        "target list offers neutral close and continuation labels");
    CloseConfiguration();
    Check(PracticeMenuPage[0] == SP_MENU_NONE && PracticePausedMenu[0] == SP_MENU_TARGETS
        && PracticeSetupView[0] && PracticePreviewObjects[0][0] == objectid && PracticePreviewCheckpoint[0][0] == cp
        && !PracticeConfigurationEditing(0) && PracticeRevision[0] == revision,
        "close hides only the target modal without taking a writer lock or changing saved data");
    Check(!PracticeHandleMenu(0, true, 0, "") && !PracticeHasDraft(0), "late dialog responses cannot add while the menu is closed");
    TestPosition[0][0] = 20.0; PracticeEditorTick(); TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_TARGETS && PracticePausedMenu[0] == SP_MENU_NONE
        && PracticePreviewObjects[0][0] == objectid, "bare config command resumes the target list after walking");
    MenuChoose(0);
    Check(PracticeHasDraft(0) && PracticeCatalogueOpen[0] && PracticeDraft[0][PracticePointX] == 16.5,
        "adding a target after resumption places its draft ahead of the new player position");
    SelectModel(1583); MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, 26.0);
    Check(PracticePoints[0][4][PracticePointX] == 26.0 && PracticeCountTargets(0) == 4,
        "resumed target creation still follows visual confirmation and persistence");

    ResetTest(); PracticeShowStarts(0); CloseConfiguration(); TestPosition[0][1] = 6.0;
    PracticeEditorTick(); TestCommand_practicaconfig(0, "1"); MenuChoose(0);
    new second = PracticeStartSlot(1);
    Check(PracticeCountStarts(0) == 2 && PracticePoints[0][second][PracticePointY] == 6.0,
        "point list resumes with explicit matching ID and captures the new location when Add is selected");
    MenuChoose(3); TestPosition[0][1] = 7.0; TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_START && PracticeSelectedStart[0] == second,
        "selected point menu resumes the same point after walking");
    MenuChoose(1);
    Check(PracticePoints[0][second][PracticePointY] == 7.0 && PracticePoints[0][0][PracticePointY] == 0.0,
        "point relocation after closing changes only the selected post");

    ResetTest(); OpenTarget(1); MenuChoose(0); MenuInput("Pendiente");
    CloseConfiguration();
    Check(PracticeHasDraft(0) && PracticePausedMenu[0] == SP_MENU_DRAFT
        && !PracticePoints[0][1][PracticePointName][0] && PracticeConfigurationEditing(0)
        && !PracticeCanStart(0, 1), "paused target draft preserves pending changes and exclusive ownership");
    TestPosition[0][0] = 4.0; TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_DRAFT && !strcmp(PracticeDraft[0][PracticePointName], "Pendiente")
        && strfind(TestDialogBody[0], "Guardar cambios") > strfind(TestDialogBody[0], "Cerrar menú"),
        "draft resumes with pending edits and Save remains the last row");
    MenuChoose(7);
    Check(!strcmp(PracticePoints[0][1][PracticePointName], "Pendiente") && !PracticeHasDraft(0),
        "resumed existing draft saves only on explicit confirmation");
    ResetTest(0); PracticeBeginDraft(0, 1, true); PracticeDraftWizard[0] = false; PracticeShowDraftMenu(0);
    CloseConfiguration(); TestPosition[0][0] = 1.0; TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_DRAFT && PracticeDraftNew[0] && !PracticeCountTargets(0),
        "unsaved new draft can also close and resume without publishing a target");
    MenuChoose(5);
    Check(PracticeCountTargets(0) == 1 && !PracticeHasDraft(0), "new resumed draft keeps Save as its final action");

    ResetTest(); PracticeShowTargets(0); CloseConfiguration(); PracticeShowRules(1); MenuChoose(6, 1);
    TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_TARGETS && PracticeMenuRevision[0] == PracticeRevision[0]
        && PracticeRules[0][PracticeRandomOrder], "resuming a list refreshes revisions changed by another administrator");
    ResetTest(); PracticeShowStarts(0); MenuChoose(1); CloseConfiguration();
    PracticeShowStarts(1); MenuChoose(1, 1); MenuChoose(2, 1); MenuChoose(0, 1);
    TestPosition[1][1] = 6.0; AddStart(1); TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_STARTS && PracticeSelectedStart[0] == -1,
        "stale point context returns to the refreshed list instead of selecting a replaced slot");

    ResetTest(); OpenTarget(1); MenuChoose(0); MenuInput("Discard me"); CloseConfiguration();
    new zone = TestAddZone("Other", 200.0); TestCommand_practicaconfig(0, "2");
    Check(PracticeSelectedZone[0] == zone && PracticeMenuPage[0] == SP_MENU_MAIN
        && PracticePausedMenu[0] == SP_MENU_NONE && !PracticeConfigurationEditing(0)
        && !PracticePoints[0][1][PracticePointName][0] && !TestLiveObjects,
        "switching zones clears the suspended view and discards the old draft without saving");
    ResetTest(); PracticeShowStarts(0); CloseConfiguration(); TestCommand_practicaconfig(0, "invalid");
    Check(PracticePausedMenu[0] == SP_MENU_STARTS, "invalid command arguments cannot alter suspended configuration");
    TestAdmin[0] = false; PracticeEditorTick();
    Check(PracticePausedMenu[0] == SP_MENU_NONE && !PracticeSetupView[0] && !TestLiveObjects,
        "revoked RCON access clears suspended context and views");
    ResetTest(0); PracticePoints[0][0][PracticePointSet] = false; PracticeShowStarts(0); CloseConfiguration();
    TestHealth[0] = 0.0; PracticeEditorTick();
    Check(PracticePausedMenu[0] == SP_MENU_NONE, "death clears suspended context even when no preview items exist");

    ResetTest(); OpenTarget(1); CloseConfiguration(); TestPosition[0][0] = 101.0; PracticeEditorTick();
    Check(!PracticeHasDraft(0) && !PracticeConfigurationEditing(0) && PracticePausedMenu[0] == SP_MENU_NONE
        && !TestLiveObjects, "leaving the valid editing range releases a suspended draft and its view");
    ResetTest(); PracticeShowTargets(0); CloseConfiguration(); PracticeResetUI(0);
    Check(PracticePausedMenu[0] == SP_MENU_NONE && !PracticeSetupView[0] && !TestLiveObjects,
        "lifecycle cleanup clears closed menu context for player ID reuse");
}
