#if defined SP_HOOKS_INCLUDED
    #endinput
#endif
#define SP_HOOKS_INCLUDED

#include <YSI_Coding\y_hooks>

new PracticePulseTimer;
forward PracticePulse();
public PracticePulse()
{
    PracticeRepairPreviews();
    new now = GetTickCount();
    PracticeCatalogueTick(now);
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
        if (PracticeIsBusy(zone)) PracticeTick(zone, now);
    return 1;
}

hook OnGameModeInit()
{
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++) PracticeCleanup(zone);
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        PracticeResetUI(playerid);
        PracticeSelectedZone[playerid] = PracticePlayerZone[playerid] = -1;
        PracticeSelectedZoneID[playerid] = 0;
    }
    PracticeInitializeRegistry();
    PracticeRepositoryBootstrap();
    // One gamemode-owned pulse polls real deadlines. SA-MP timer delays never
    // shorten countdown pauses, exposure or decimal intervals.
    PracticePulseTimer = SetTimer("PracticePulse", 50, true);
    return 1;
}

hook OnGameModeExit()
{
    if (PracticePulseTimer) KillTimer(PracticePulseTimer);
    PracticePulseTimer = 0;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) PracticeResetUI(playerid);
    PracticeResetDiscovery();
    PracticeIntegrationShutdown();
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++) PracticeCleanup(zone);
    return 1;
}

// Editor validity checks retain the shared 500 ms maintenance cycle.
hook Task500()
{
    PracticeEditorTick();
    PracticeDiscoveryTick();
    return 1;
}

// YSI expands Dyn to Dynamic. Preview hits never enter the scoring callback.
hook OnPlayerShootDynObject(playerid, weaponid, objectid, Float:x, Float:y, Float:z)
{
    if (PracticeProtectPreviewShot(playerid, objectid)) return ~0;
    return 1;
}

hook OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, Float:x, Float:y, Float:z)
{
    // Count and score once in the same callback. Streamer's dynamic-object
    // callback is deliberately not also counted (it fires for the same shot).
    PracticeHandleShot(playerid, weaponid, hittype, hitid, GetTickCount());
    return 1;
}

hook OnPlayerUpdate(playerid)
{
    new zone = PracticeGetPlayerZone(playerid);
    if (zone >= 0) PracticeParticipantValid(zone);
    return 1;
}

hook OnPlayerDeath(playerid, killerid, reason)
{
    PracticeResetUI(playerid);
    new zone = PracticeGetPlayerZone(playerid);
    if (zone >= 0) PracticeEnd(zone, false, "el participante murió.");
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    PracticeResetHint(playerid);
    PracticeResetUI(playerid);
    new zone = PracticeGetPlayerZone(playerid);
    if (zone >= 0) PracticeEnd(zone, false, "desconexión", false);
    PracticeCatalogueCancelEchoes[playerid] = 0;
    PracticeSelectedZone[playerid] = PracticePlayerZone[playerid] = -1;
    PracticeSelectedZoneID[playerid] = 0;
    return 1;
}

hook OnPlayerStateChange(playerid, newstate, oldstate)
{
    if (newstate != PLAYER_STATE_ONFOOT) PracticeResetUI(playerid);
    new zone = PracticeGetPlayerZone(playerid);
    if (zone >= 0 && newstate != PLAYER_STATE_ONFOOT)
        PracticeEnd(zone, false, "debes permanecer a pie.");
    return 1;
}

hook OnPlayerConnect(playerid)
{
    PracticeResetHint(playerid);
    new zone = PracticeGetPlayerZone(playerid);
    if (zone >= 0) PracticeCleanup(zone);
    PracticeHideCountdown(playerid);
    PracticeResetUI(playerid);
    PracticeCatalogueCancelEchoes[playerid] = 0;
    PracticeSelectedZone[playerid] = PracticePlayerZone[playerid] = -1;
    PracticeSelectedZoneID[playerid] = 0;
    return 1;
}

// YSI expands Dyn to Dynamic; the shorter name leaves room for its unique suffix.
hook OnPlayerEditDynObject(playerid, objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
    PracticeHandleObjectEdit(playerid, objectid, response, x, y, z, rx, ry, rz);
    return 1;
}

hook OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    if (PracticeHandleCatalogueClick(playerid, playertextid)) return ~1;
    return 1;
}

hook OnPlayerClickTextDraw(playerid, Text:clickedid)
{
    if (clickedid == Text:INVALID_TEXT_DRAW && PracticeHandleCatalogueEscape(playerid)) return ~1;
    return 1;
}
