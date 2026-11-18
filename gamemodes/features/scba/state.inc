enum E_SCBA_AIR_BAND
{
    SCBA_AIR_NORMAL,
    SCBA_AIR_LOW,
    SCBA_AIR_CRITICAL,
    SCBA_AIR_EMPTY
}
enum E_SCBA_SOURCE
{
    SCBA_SOURCE_NONE,
    SCBA_SOURCE_POINT,
    SCBA_SOURCE_VEHICLE
}
enum E_SCBA_PLAYER
{
    bool:ScbaEquipped,
    bool:ScbaMaskOn,
    bool:ScbaBreathingIndoors,
    bool:ScbaAssetsReady,
    ScbaAirRemainingMs,
    ScbaLastAirUpdate,
    ScbaNextDamageAt,
    ScbaLastAlarmAt,
    ScbaLastStreamAt,
    bool:ScbaHasStreamTime,
    bool:ScbaStreamIsAlarm,
    ScbaLastDistressAt,
    ScbaReportedSymptom,
    ScbaDizzinessLevel,
    bool:ScbaHasAlarmTime,
    bool:ScbaHasDistressTime,
    E_SCBA_AIR_BAND:ScbaReportedBand,
    ScbaAttachmentSkin,
    E_SCBA_SOURCE:ScbaSupplySource,
    ScbaSupplyIndex,
    ScbaSupplyVehicleId,
    ScbaSupplyVehicleModel,
    ScbaSupplyStartedAt,
    bool:ScbaTruckControlsLocked,
    bool:ScbaServiceAnimSeen,
    ScbaLastAnimAttemptAt,
    bool:ScbaHostControlsLocked,
    Float:ScbaSupplyStartX,
    Float:ScbaSupplyStartY,
    Float:ScbaSupplyStartZ,
    ScbaLastHudUpdate,
    ScbaNeedleFrame,
    E_SCBA_GAUGE_STATUS:ScbaGaugeStatus
}
new ScbaPlayer[MAX_PLAYERS][E_SCBA_PLAYER];
new ScbaAttachmentSlots[MAX_PLAYERS][SCBA_PART_COUNT];
new PlayerText:ScbaGauge[MAX_PLAYERS][E_SCBA_GAUGE_LAYER];
new bool:ScbaGaugeVisible[MAX_PLAYERS];
new bool:ScbaModelsRegistered;
new ScbaPulseTimer;

stock ScbaElapsed(now, previous)
{
    return now - previous;
}

stock ScbaAirPercent(playerid)
{
    const millisecondsPerPercent = SCBA_FULL_AIR_MS / 100;
    return (ScbaPlayer[playerid][ScbaAirRemainingMs] + millisecondsPerPercent - 1) / millisecondsPerPercent;
}

stock E_SCBA_AIR_BAND:ScbaAirBand(playerid)
{
    new percent = ScbaAirPercent(playerid);
    if (percent <= 0) return SCBA_AIR_EMPTY;
    if (percent <= SCBA_CRITICAL_AIR_PERCENT) return SCBA_AIR_CRITICAL;
    if (percent <= SCBA_LOW_AIR_PERCENT) return SCBA_AIR_LOW;
    return SCBA_AIR_NORMAL;
}

stock ScbaSecondsRemaining(playerid)
{
    return (ScbaPlayer[playerid][ScbaAirRemainingMs] + SCBA_SECOND_MS - 1) / SCBA_SECOND_MS;
}

stock ScbaNeedleStep(playerid)
{
    return floatround(float(ScbaPlayer[playerid][ScbaAirRemainingMs])
        * float(SCBA_GAUGE_NEEDLE_STEPS) / float(SCBA_FULL_AIR_MS));
}

stock bool:ScbaPlayerAlive(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return false;
    new playerState = GetPlayerState(playerid), Float:health;
    if (playerState != PLAYER_STATE_ONFOOT && playerState != PLAYER_STATE_DRIVER && playerState != PLAYER_STATE_PASSENGER) return false;
    GetPlayerHealth(playerid, health);
    return health > 0.0;
}

stock ScbaResetPlayer(playerid)
{
    for (new E_SCBA_PLAYER:field = E_SCBA_PLAYER:0; field < E_SCBA_PLAYER; field++) ScbaPlayer[playerid][field] = 0;
    ScbaPlayer[playerid][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS;
    ScbaPlayer[playerid][ScbaSupplyIndex] = SCBA_EMPTY_SLOT;
    ScbaPlayer[playerid][ScbaNeedleFrame] = SCBA_EMPTY_SLOT;
    ScbaPlayer[playerid][ScbaReportedSymptom] = SCBA_EMPTY_SLOT;
    ScbaPlayer[playerid][ScbaAttachmentSkin] = SCBA_EMPTY_SLOT;
    for (new E_SCBA_PART:part = SCBA_BACKPACK; part < SCBA_PART_COUNT; part++) ScbaAttachmentSlots[playerid][part] = SCBA_EMPTY_SLOT;
    for (new E_SCBA_GAUGE_LAYER:layer; layer < E_SCBA_GAUGE_LAYER; layer++)
        ScbaGauge[playerid][layer] = PlayerText:INVALID_TEXT_DRAW;
    ScbaGaugeVisible[playerid] = false;
    return 1;
}
