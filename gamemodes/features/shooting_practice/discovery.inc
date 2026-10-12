#if defined SP_DISCOVERY_INCLUDED
    #endinput
#endif
#define SP_DISCOVERY_INCLUDED

// Public decorative pickups. Type 1 disables the weapon's built-in collection script.
// No 3D labels or pickup actions: participation still goes through /practica.
new STREAMER_TAG_PICKUP:PracticeStartPickups[PRACTICE_MAX_ZONES][PRACTICE_MAX_STARTS];
enum E_PRACTICE_INDICATOR {
    Float:PracticeIndicatorX, Float:PracticeIndicatorY, Float:PracticeIndicatorZ,
    PracticeIndicatorInterior, PracticeIndicatorWorld
}
new PracticeIndicatorCache[PRACTICE_MAX_ZONES][PRACTICE_MAX_STARTS][E_PRACTICE_INDICATOR];
new PracticeHintZoneID[MAX_PLAYERS], PracticeHintStartSlot[MAX_PLAYERS] = {-1, ...};

stock PracticeResetHint(playerid)
{
    PracticeHintZoneID[playerid] = 0;
    PracticeHintStartSlot[playerid] = -1;
    return 1;
}

stock PracticeRemoveIndicator(zone, index)
{
    new STREAMER_TAG_PICKUP:pickupid = PracticeStartPickups[zone][index];
    PracticeStartPickups[zone][index] = INVALID_STREAMER_ID;
    if (pickupid != INVALID_STREAMER_ID && IsValidDynamicPickup(pickupid)) DestroyDynamicPickup(pickupid);
    return 1;
}

stock PracticeHideIndicators(zone)
{
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++) PracticeRemoveIndicator(zone, index);
    return 1;
}

stock PracticeResetDiscovery()
{
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++) PracticeHideIndicators(zone);
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) PracticeResetHint(playerid);
    return 1;
}

stock bool:PracticeDiscoveryAvailable(zone)
{
    return PracticeZoneExists(zone) && PracticeZoneEnabled[zone] && !PracticeZoneArchived[zone];
}

stock PracticeSynchronizeIndicators(zone)
{
    if (zone < 0 || zone >= PRACTICE_MAX_ZONES) return 0;
    if (!PracticeDiscoveryAvailable(zone) || !PracticeZoneIndicators[zone]) return PracticeHideIndicators(zone);
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
    {
        new slot = PracticeStartSlot(index);
        if (!PracticePoints[zone][slot][PracticePointSet])
        { PracticeRemoveIndicator(zone, index); continue; }
        if (IsValidDynamicPickup(PracticeStartPickups[zone][index])
            && PracticeIndicatorCache[zone][index][PracticeIndicatorX] == PracticePoints[zone][slot][PracticePointX]
            && PracticeIndicatorCache[zone][index][PracticeIndicatorY] == PracticePoints[zone][slot][PracticePointY]
            && PracticeIndicatorCache[zone][index][PracticeIndicatorZ] == PracticePoints[zone][slot][PracticePointZ]
            && PracticeIndicatorCache[zone][index][PracticeIndicatorInterior] == PracticePoints[zone][slot][PracticePointInterior]
            && PracticeIndicatorCache[zone][index][PracticeIndicatorWorld] == PracticePoints[zone][slot][PracticePointWorld]) continue;
        PracticeRemoveIndicator(zone, index);
        new STREAMER_TAG_PICKUP:pickupid = CreateDynamicPickup(PRACTICE_START_PICKUP_MODEL, PRACTICE_START_PICKUP_TYPE,
            PracticePoints[zone][slot][PracticePointX], PracticePoints[zone][slot][PracticePointY],
            PracticePoints[zone][slot][PracticePointZ] + PRACTICE_START_PICKUP_Z,
            PracticePoints[zone][slot][PracticePointWorld], PracticePoints[zone][slot][PracticePointInterior]);
        if (pickupid == INVALID_STREAMER_ID || !IsValidDynamicPickup(pickupid))
        {
            // Roll back partial allocation. The next maintenance tick can retry.
            PracticeStartPickups[zone][index] = pickupid;
            PracticeHideIndicators(zone);
            return 0;
        }
        PracticeStartPickups[zone][index] = pickupid;
        PracticeIndicatorCache[zone][index][PracticeIndicatorX] = PracticePoints[zone][slot][PracticePointX];
        PracticeIndicatorCache[zone][index][PracticeIndicatorY] = PracticePoints[zone][slot][PracticePointY];
        PracticeIndicatorCache[zone][index][PracticeIndicatorZ] = PracticePoints[zone][slot][PracticePointZ];
        PracticeIndicatorCache[zone][index][PracticeIndicatorInterior] = PracticePoints[zone][slot][PracticePointInterior];
        PracticeIndicatorCache[zone][index][PracticeIndicatorWorld] = PracticePoints[zone][slot][PracticePointWorld];
    }
    return 1;
}

stock PracticeDiscoverPlayer(playerid)
{
    new Float:health;
    if (!IsPlayerConnected(playerid) || IsPlayerNPC(playerid) || GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
    { PracticeResetHint(playerid); return 1; }
    GetPlayerHealth(playerid, health);
    if (health <= 0.0) { PracticeResetHint(playerid); return 1; }
    new active = PracticeGetPlayerZone(playerid);
    if (active >= 0)
    {
        PracticeHintZoneID[playerid] = PracticeZoneID[active];
        PracticeHintStartSlot[playerid] = PracticeSessionStartSlot[active];
        return 1;
    }
    if (PracticeHasDraft(playerid) || PracticeMenuPage[playerid] != SP_MENU_NONE) return 1;
    new previous = PracticeFindZone(PracticeHintZoneID[playerid]);
    // Hysteresis: enter at the command's 1.5 m radius, leave beyond 3 m.
    // Nearby/overlapping posts of the same field do not flood the chat.
    new slot = PracticeHintStartSlot[playerid];
    if (PracticeDiscoveryAvailable(previous) && PracticeIsStartSlot(slot)
        && PracticePoints[previous][slot][PracticePointSet]
        && GetPlayerInterior(playerid) == PracticePoints[previous][slot][PracticePointInterior]
        && GetPlayerVirtualWorld(playerid) == PracticePoints[previous][slot][PracticePointWorld]
        && IsPlayerInRangeOfPoint(playerid, PRACTICE_ACTIVE_RADIUS, PracticePoints[previous][slot][PracticePointX],
            PracticePoints[previous][slot][PracticePointY], PracticePoints[previous][slot][PracticePointZ])) return 1;
    PracticeResetHint(playerid);
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        if (!PracticeDiscoveryAvailable(zone) || !PracticeCanUseStation(zone, playerid)) continue;
        new nearest = PracticeNearestStart(zone, playerid, PRACTICE_START_RADIUS);
        if (nearest < 0) continue;
        new reason[144];
        if (!PracticeConfigurationReady(zone, reason)) continue;
        PracticeHintZoneID[playerid] = PracticeZoneID[zone];
        PracticeHintStartSlot[playerid] = nearest;
        SendClientMessage(playerid, COLOR_WHITE, "{A8C4D9}Campo de tiro.{FFFFFF} Usa /practica para iniciar una práctica.");
        break;
    }
    return 1;
}

stock PracticeDiscoveryTick()
{
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++) PracticeSynchronizeIndicators(zone);
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) PracticeDiscoverPlayer(playerid);
    return 1;
}
