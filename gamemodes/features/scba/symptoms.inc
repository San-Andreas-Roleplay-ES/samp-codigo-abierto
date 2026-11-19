stock ScbaYieldDizziness(playerid)
{
    if (playerid >= 0 && playerid < MAX_PLAYERS && ScbaPlayer[playerid][ScbaDizzinessLevel] > 0)
        ScbaPlayer[playerid][ScbaDizzinessLevel] = SCBA_EXTERNAL_DIZZINESS;
    return 1;
}

stock ScbaSetDizziness(playerid, level)
{
    new applied = ScbaPlayer[playerid][ScbaDizzinessLevel];
    if ((!level && !applied) || applied == SCBA_EXTERNAL_DIZZINESS || !IsPlayerConnected(playerid)) return 1;
    if (GetPlayerDrunkLevel(playerid) > applied)
    {
        ScbaPlayer[playerid][ScbaDizzinessLevel] = SCBA_EXTERNAL_DIZZINESS;
        return 1;
    }
    SetPlayerDrunkLevel(playerid, level);
    ScbaPlayer[playerid][ScbaDizzinessLevel] = level;
    return 1;
}

stock ScbaClearSymptoms(playerid)
{
    if (ScbaPlayer[playerid][ScbaDizzinessLevel] > 0) ScbaSetDizziness(playerid, 0);
    ScbaPlayer[playerid][ScbaDizzinessLevel] = 0;
    ScbaPlayer[playerid][ScbaReportedSymptom] = SCBA_EMPTY_SLOT;
    ScbaPlayer[playerid][ScbaHasDistressTime] = false;
    return 1;
}

stock ScbaReportSymptoms(playerid, Float:health, now)
{
    new stage = 0;
    while (stage < sizeof(ScbaSymptoms) - 1 && health <= ScbaSymptoms[stage][ScbaHealthAbove]) stage++;
    if (!ScbaPlayer[playerid][ScbaHasDistressTime])
    {
        ScbaBroadcast(playerid, "Se escuchan jadeos entrecortados.", SCBA_DISTRESS_SOUND);
        ScbaPlayer[playerid][ScbaHasDistressTime] = true;
        ScbaPlayer[playerid][ScbaLastDistressAt] = now;
    }
    else if (ScbaElapsed(now, ScbaPlayer[playerid][ScbaLastDistressAt]) >= ScbaSymptoms[stage][ScbaGaspIntervalMs])
    {
        ScbaBroadcast(playerid, "", SCBA_DISTRESS_SOUND);
        ScbaPlayer[playerid][ScbaLastDistressAt] = now;
    }

    if (ScbaPlayer[playerid][ScbaReportedSymptom] == SCBA_EMPTY_SLOT)
    {
        SendClientMessage(playerid, SCBA_COLOUR_TEXT, ScbaSymptoms[0][ScbaSymptomMessage]);
        ScbaPlayer[playerid][ScbaReportedSymptom] = 0;
        return 1;
    }
    if (stage > ScbaPlayer[playerid][ScbaReportedSymptom])
    {
        SendClientMessage(playerid, SCBA_COLOUR_TEXT, ScbaSymptoms[stage][ScbaSymptomMessage]);
        ScbaPlayer[playerid][ScbaReportedSymptom] = stage;
    }
    if (GetPlayerState(playerid) == PLAYER_STATE_ONFOOT)
    {
        new level = ScbaSymptoms[stage][ScbaSwayOnset];
        if (stage > 0 && level != ScbaSymptoms[stage][ScbaSwayLevel])
        {
            new Float:upper = ScbaSymptoms[stage - 1][ScbaHealthAbove];
            new Float:progress = (upper - health) / (upper - ScbaSymptoms[stage][ScbaHealthAbove]);
            level += floatround(progress * (ScbaSymptoms[stage][ScbaSwayLevel] - level));
        }
        ScbaSetDizziness(playerid, level);
    }
    return 1;
}
