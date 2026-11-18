#include <YSI_Coding\y_hooks>

// YSI's generated symbol exceeds Pawn's 31-character limit. The hook needs the full native name.
#pragma warning push
#pragma warning disable 200
hook native TogglePlayerControllable(playerid, toggle)
{
    new effective = ScbaEffectiveControls(playerid, toggle);
    return continue(playerid, effective);
}
#pragma warning pop

hook native SetPlayerDrunkLevel(playerid, level)
{
    ScbaYieldDizziness(playerid);
    return continue(playerid, level);
}

hook OnGameModeInit()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) ScbaResetPlayer(playerid);
    ScbaRegisterModels();
    ScbaPulseTimer = SetTimer("ScbaPulse", SCBA_PULSE_INTERVAL_MS, true);
    return 1;
}

hook OnGameModeExit()
{
    if (ScbaPulseTimer) KillTimer(ScbaPulseTimer);
    ScbaPulseTimer = 0;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (IsPlayerConnected(playerid)) ScbaCleanup(playerid);
    return 1;
}

hook OnPlayerConnect(playerid)
{
    ScbaResetPlayer(playerid);
    return 1;
}

DEFINE_HOOK_REPLACEMENT(Downloading, DLoad);
hook OnPlayerFinishedDLoad(playerid, virtualworld)
{
    #pragma unused virtualworld
    ScbaPlayer[playerid][ScbaAssetsReady] = true;
    return 1;
}

DEFINE_HOOK_REPLACEMENT(Attached, Atch);
hook OnPlayerEditAtchObject(playerid, response, index, modelid, boneid, Float:fOffsetX, Float:fOffsetY, Float:fOffsetZ, Float:fRotX, Float:fRotY, Float:fRotZ, Float:fScaleX, Float:fScaleY, Float:fScaleZ)
{
    #pragma unused fScaleX, fScaleY, fScaleZ
    new pose[E_SCBA_POSE];
    pose[ScbaOffsetX] = fOffsetX;
    pose[ScbaOffsetY] = fOffsetY;
    pose[ScbaOffsetZ] = fOffsetZ;
    pose[ScbaRotationX] = fRotX;
    pose[ScbaRotationY] = fRotY;
    pose[ScbaRotationZ] = fRotZ;
    ScbaFinishFitEdit(playerid, response, index, modelid, boneid, pose);
    return 1;
}

hook OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
    if (ScbaPlayer[playerid][ScbaMaskOn]) ScbaUpdateBreathing(playerid, GetTickCount(), newinteriorid);
    return 1;
}

hook OnPlayerSpawn(playerid)
{
    ScbaCleanup(playerid);
    return 1;
}

hook OnPlayerDeath(playerid, killerid, reason)
{
    ScbaCleanup(playerid);
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    ScbaCleanup(playerid);
    ScbaResetPlayer(playerid);
    return 1;
}

forward ScbaPulse();
public ScbaPulse()
{
    new now = GetTickCount();
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (IsPlayerConnected(playerid)) ScbaTickPlayer(playerid, now);
    return 1;
}
