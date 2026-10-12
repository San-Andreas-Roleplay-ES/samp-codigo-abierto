stock TestIntegrationPorts()
{
    ResetTest();
    Check(PracticeHostIsAdministrator(0) && PracticeIsAdministrator(0), "host administration port keeps RCON policy");
    TestAdmin[0] = false;
    Check(!PracticeIsAdministrator(0) && PracticeCanUseStation(0, 0),
        "public practice through host port does not grant configuration authority");
    TestNPC[0] = true;
    Check(!PracticeCanUseStation(0, 0), "host practice port excludes NPCs");
    TestNPC[0] = false;
    new subject[PRACTICE_SUBJECT_KEY_SIZE] = "fake";
    Check(!PracticeHostResolveSubject(0, subject) && !subject[0],
        "unimplemented stable identity resolver clears output and fails instead of inventing an ID");

    ResetTest();
    new secondStart = TestSecondStart(0, 6.0);
    TestPosition[0][1] = 6.0;
    PracticePoints[0][1][PracticePointModel] = 1543;
    PracticePoints[0][2][PracticePointNoShoot] = true;
    PracticeSaveAndNotify(0);
    Check(TestRepositorySaves == 1 && PracticeRepositoryLoad() && PracticeCountStarts(0) == 2,
        "confirmed configuration save calls the repository port and reload keeps multiple posts");
    PracticePoints[0][0][PracticePointName][0] = 'X';
    Check(PracticeRepositoryBootstrap() && !PracticePoints[0][0][PracticePointName][0],
        "bootstrap delegates to validated file loading instead of a fake SQL success");
    PracticeStart(0, 0);
    Check(TestIntegrationStarts == 1 && TestIntegrationStartSlot == secondStart
        && PracticeSession[0][PracticePurpose] == SP_PURPOSE_TRAINING && PracticeIsBusy(0),
        "before-start port receives the chosen post and the frozen training purpose");
    new recorded[E_PRACTICE_SESSION];
    recorded = PracticeSession[0]; recorded[PracticePurpose] = SP_PURPOSE_EVALUATION;
    Check(!PracticeIntegrationBeforeStart(0, recorded, PracticeSessionOrigin[0], secondStart,
        PracticeSequenceSlots[0], PracticeSequenceModels[0], PracticeSequenceNoShoot[0]),
        "recorded evaluation cannot start with the placeholder identity and file backend");
    Check(PracticeIntegrationOnEnd(0, false, "test", recorded, PracticeSessionOrigin[0], secondStart,
        PracticeSequenceSlots[0], PracticeSequenceModels[0], PracticeSequenceNoShoot[0]) == SP_HISTORY_UNAVAILABLE,
        "unimplemented evaluation close reports unavailable rather than claiming a save");
    TestNow += 3000; PracticeTick(0, TestNow); Shoot(true);
    PracticeEnd(0, false, "test interruption");
    Check(TestIntegrationEnds == 1 && TestIntegrationEndSlot == secondStart
        && TestIntegrationOrigin[PracticePointY] == 6.0 && TestIntegrationFinal[PracticeHits] == 1
        && TestIntegrationFinal[PracticePurpose] == SP_PURPOSE_TRAINING
        && PracticeSessionStartSlot[0] == -1 && !PracticeIsBusy(0),
        "end port receives final counters and selected origin before runtime cleanup");
    new bool:planValid = true;
    for (new i = 0; i < TestIntegrationFinal[PracticeTotal]; i++)
        planValid = planValid && TestIntegrationSlots[i] >= 1 && PracticeModelIndex(TestIntegrationModels[i]) >= 0;
    Check(planValid && TestRepositorySaves == 1 && PracticeIntegrationOnEnd(0, false, "test",
        TestIntegrationFinal, TestIntegrationOrigin, secondStart,
        TestIntegrationSlots, TestIntegrationModels, TestIntegrationTypes) == SP_HISTORY_NOT_RECORDED,
        "training carries the frozen sequence to the port and never creates a history save");

    ResetTest(); TestIntegrationReject = true; PracticeStart(0, 0);
    Check(TestIntegrationStarts == 1 && !PracticeIsBusy(0) && PracticePlayerZone[0] == -1
        && !TestSoundCount[0] && !TestTDCount[0] && !TestLiveObjects,
        "a rejected before-start port releases ownership without a countdown or target");

    ResetTest(); PracticeStart(0, 0); TestDisconnect(0);
    PracticeEnd(0, false, "disconnect", false);
    Check(TestIntegrationEnds == 1 && !TestResult && !PracticeIsBusy(0),
        "disconnect still supplies the final snapshot when participant notification is disabled");
    ResetTest(); ShowFirstTarget(); new sounds = TestSoundCount[0], messages = TestMessages[0];
    PracticeIntegrationShutdown();
    Check(PracticeIsBusy(0) && TestSoundCount[0] == sounds && TestMessages[0] == messages
        && !TestRepositorySaves, "shutdown integration port is silent and does not persist training history");
    PracticeCleanup(0);
}
