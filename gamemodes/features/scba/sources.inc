stock bool:ScbaPointAvailable(playerid, index, E_SCBA_ACTION:action)
{
    if (index < 0 || index >= sizeof(ScbaSupplyPoints) || !ScbaSupplyPoints[index][ScbaPointEnabled]) return false;
    if (!(ScbaSupplyPoints[index][ScbaPointActions] & _:action) || ScbaSupplyPoints[index][ScbaPointRadius] <= 0.0) return false;
    if (GetPlayerInterior(playerid) != ScbaSupplyPoints[index][ScbaPointInterior]
        || GetPlayerVirtualWorld(playerid) != ScbaSupplyPoints[index][ScbaPointWorld]) return false;
    return IsPlayerInRangeOfPoint(playerid, ScbaSupplyPoints[index][ScbaPointRadius],
        ScbaSupplyPoints[index][ScbaPointX], ScbaSupplyPoints[index][ScbaPointY], ScbaSupplyPoints[index][ScbaPointZ]) != 0;
}

stock bool:ScbaVehicleAvailable(playerid, index, E_SCBA_ACTION:action)
{
    if (!ScbaHostSupplyVehicle(index) || !(ScbaSupplyVehicles[index][ScbaVehicleActions] & _:action)
        || ScbaSupplyVehicles[index][ScbaVehicleRadius] <= 0.0) return false;
    new vehicleid = ScbaSupplyVehicles[index][ScbaVehicleId];
    if (GetPlayerInterior(playerid) != ScbaSupplyVehicles[index][ScbaVehicleInterior]
        || GetPlayerVirtualWorld(playerid) != GetVehicleVirtualWorld(vehicleid)) return false;
    new Float:x, Float:y, Float:z, Float:vx, Float:vy, Float:vz;
    GetVehiclePos(vehicleid, x, y, z);
    GetVehicleVelocity(vehicleid, vx, vy, vz);
    if (vx * vx + vy * vy + vz * vz > SCBA_VEHICLE_MAX_SPEED * SCBA_VEHICLE_MAX_SPEED) return false;
    return IsPlayerInRangeOfPoint(playerid, ScbaSupplyVehicles[index][ScbaVehicleRadius], x, y, z) != 0;
}

stock bool:ScbaFindSource(playerid, E_SCBA_ACTION:action, &E_SCBA_SOURCE:source, &index)
{
    source = SCBA_SOURCE_NONE;
    index = SCBA_EMPTY_SLOT;
    for (new point = 0; point < sizeof(ScbaSupplyPoints); point++)
        if (ScbaPointAvailable(playerid, point, action))
        {
            source = SCBA_SOURCE_POINT;
            index = point;
            return true;
        }
    for (new vehicle = 0; vehicle < sizeof(ScbaSupplyVehicles); vehicle++)
        if (ScbaVehicleAvailable(playerid, vehicle, action))
        {
            source = SCBA_SOURCE_VEHICLE;
            index = vehicle;
            return true;
        }
    return false;
}

stock bool:ScbaServiceSourceValid(playerid)
{
    if (!IsPlayerInRangeOfPoint(playerid, SCBA_SERVICE_MOVEMENT_RADIUS,
        ScbaPlayer[playerid][ScbaSupplyStartX], ScbaPlayer[playerid][ScbaSupplyStartY],
        ScbaPlayer[playerid][ScbaSupplyStartZ])) return false;
    new index = ScbaPlayer[playerid][ScbaSupplyIndex];
    switch (ScbaPlayer[playerid][ScbaSupplySource])
    {
        case SCBA_SOURCE_POINT: return ScbaPointAvailable(playerid, index, SCBA_REFILL);
        case SCBA_SOURCE_VEHICLE:
        {
            if (!ScbaVehicleAvailable(playerid, index, SCBA_REFILL)) return false;
            return ScbaSupplyVehicles[index][ScbaVehicleId] == ScbaPlayer[playerid][ScbaSupplyVehicleId]
                && GetVehicleModel(ScbaPlayer[playerid][ScbaSupplyVehicleId])
                    == ScbaPlayer[playerid][ScbaSupplyVehicleModel];
        }
    }
    return false;
}
