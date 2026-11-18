stock bool:ScbaHostCanUse(playerid)
{
    // TODO(SCBA-LSFD): check faction membership here once the host provides it.
    return playerid >= 0 && playerid < MAX_PLAYERS && IsPlayerConnected(playerid) && !IsPlayerNPC(playerid);
}

#if !defined ScbaHostSaveFit
stock bool:ScbaHostSaveFit(playerid, E_SCBA_PART:part, const pose[E_SCBA_POSE])
{
    // TODO(SCBA-STATS): write this piece to the selected character's stats.
    #pragma unused playerid, part, pose
    return true;
}
#endif

stock bool:ScbaHostIsMapping(playerid)
{
    #if defined MAX_ADMIN_OBJECTS
        return playerEditingObjectID[playerid] != -1;
    #else
        #pragma unused playerid
        return false;
    #endif
}

stock bool:ScbaHostIsIndoors(playerid, interiorid)
{
    // Servers with mapped buildings in interior 0 can consult their property state here.
    #pragma unused playerid
    return interiorid > 0;
}

stock bool:ScbaHostSupplyVehicle(index)
{
    if (index < 0 || index >= sizeof(ScbaSupplyVehicles) || !ScbaSupplyVehicles[index][ScbaVehicleEnabled]) return false;
    new vehicleid = ScbaSupplyVehicles[index][ScbaVehicleId];
    new expectedModel = ScbaSupplyVehicles[index][ScbaVehicleExpectedModel], Float:health;
    if (vehicleid <= 0 || vehicleid >= MAX_VEHICLES || expectedModel < 400 || expectedModel > 611) return false;
    if (GetVehicleModel(vehicleid) != expectedModel) return false;
    GetVehicleHealth(vehicleid, health);
    return health > 0.0;
}

stock ScbaEffectiveControls(playerid, requested)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return requested;
    ScbaPlayer[playerid][ScbaHostControlsLocked] = requested == 0;
    return ScbaPlayer[playerid][ScbaTruckControlsLocked] ? 0 : requested;
}

stock ScbaSetTruckControlLock(playerid, bool:locked)
{
    if (ScbaPlayer[playerid][ScbaTruckControlsLocked] == locked) return 1;
    ScbaPlayer[playerid][ScbaTruckControlsLocked] = locked;
    if (IsPlayerConnected(playerid))
        TogglePlayerControllable(playerid, !ScbaPlayer[playerid][ScbaHostControlsLocked]);
    return 1;
}

stock ScbaPreloadServiceAnimations(playerid)
{
    ApplyAnimation(playerid, SCBA_SERVICE_ANIM_LIBRARY, "null", SCBA_SERVICE_ANIM_SPEED, 0, 0, 0, 0, 0, 0);
    ApplyAnimation(playerid, SCBA_TRUCK_ANIM_LIBRARY, "null", SCBA_SERVICE_ANIM_SPEED, 0, 0, 0, 0, 0, 0);
    return 1;
}

stock ScbaPrepareServiceAnimation(playerid)
{
    if (ScbaPlayer[playerid][ScbaSupplySource] != SCBA_SOURCE_VEHICLE) return 1;
    new Float:x, Float:y, Float:z;
    GetVehiclePos(ScbaPlayer[playerid][ScbaSupplyVehicleId], x, y, z);
    SetPlayerFacingAngle(playerid, atan2(y - ScbaPlayer[playerid][ScbaSupplyStartY],
        x - ScbaPlayer[playerid][ScbaSupplyStartX]) - 90.0);
    return ScbaSetTruckControlLock(playerid, true);
}

stock ScbaPlayServiceAnimation(playerid, now)
{
    ScbaPlayer[playerid][ScbaLastAnimAttemptAt] = now;
    if (ScbaPlayer[playerid][ScbaSupplySource] == SCBA_SOURCE_VEHICLE)
        return ApplyAnimation(playerid, SCBA_TRUCK_ANIM_LIBRARY, SCBA_TRUCK_ANIM_NAME, SCBA_SERVICE_ANIM_SPEED,
            1, 0, 0, 0, 0, 1);
    return ApplyAnimation(playerid, SCBA_SERVICE_ANIM_LIBRARY, SCBA_SERVICE_ANIM_NAME, SCBA_SERVICE_ANIM_SPEED,
        1, 0, 0, 0, 0, 1);
}

stock bool:ScbaServiceAnimationPlaying(playerid)
{
    new index = GetPlayerAnimationIndex(playerid), library[32], name[32];
    if (index <= 0) return false;
    GetAnimationName(index, library, sizeof(library), name, sizeof(name));
    if (!library[0] || !name[0]) return false;
    if (ScbaPlayer[playerid][ScbaSupplySource] == SCBA_SOURCE_VEHICLE)
        return !strcmp(library, SCBA_TRUCK_ANIM_LIBRARY, true) && !strcmp(name, SCBA_TRUCK_ANIM_NAME, true);
    return !strcmp(library, SCBA_SERVICE_ANIM_LIBRARY, true) && !strcmp(name, SCBA_SERVICE_ANIM_NAME, true);
}

stock ScbaStopServiceAnimation(playerid)
{
    if (ScbaServiceAnimationPlaying(playerid)
        || (!ScbaPlayer[playerid][ScbaServiceAnimSeen] && GetPlayerAnimationIndex(playerid) == 0))
        ClearAnimations(playerid);
    ScbaPlayer[playerid][ScbaServiceAnimSeen] = false;
    ScbaSetTruckControlLock(playerid, false);
    return 1;
}

stock bool:ScbaModelFilesRegistered(E_SCBA_PART:part)
{
    new dff[80], txd[80];
    return FindModelFileNameFromCRC(ScbaModels[part][ScbaModelDffCrc], dff, sizeof(dff)) != 0
        && FindTextureFileNameFromCRC(ScbaModels[part][ScbaModelTxdCrc], txd, sizeof(txd)) != 0
        && !strcmp(dff, ScbaModels[part][ScbaModelDff]) && !strcmp(txd, ScbaModels[part][ScbaModelTxd]);
}

stock bool:ScbaRegisterModels()
{
    new bool:success = true;
    for (new E_SCBA_PART:part = SCBA_BACKPACK; part < SCBA_PART_COUNT; part++)
    {
        // 0.3.DL returns 0 for both the first successful registration and failure.
        AddSimpleModel(-1, ScbaModels[part][ScbaBaseModel], ScbaModels[part][ScbaCustomModel],
            ScbaModels[part][ScbaModelDff], ScbaModels[part][ScbaModelTxd]);
        if (!ScbaModelFilesRegistered(part)) success = false;
    }
    ScbaModelsRegistered = success;
    if (!success) print("[SCBA] No se pudieron registrar los modelos. Revisa models/scba y sus CRC.");
    return success;
}
