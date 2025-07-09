alias:crearvehiculo("crearveh")
CMD:crearvehiculo(playerid, params[])
{
    new vehiclemodelid, color1, color2, sirena;
    if(sscanf(params, "dddD(0)", vehiclemodelid, color1, color2, sirena))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /crearveh(iculo) [vehicleid] [color1] [color2] [sirena (opcional)]");

    if(vehiclemodelid < 400 || vehiclemodelid > 611 ||
        vehiclemodelid == 449 || vehiclemodelid == 537 || vehiclemodelid == 538 ||
        vehiclemodelid == 569 || vehiclemodelid == 570 || vehiclemodelid == 590)
    {
        return SendClientMessage(playerid, COLOR_FADE3, "Modelo de vehículo inválido.");
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    new sampvehicleid = CreateVehicle(vehiclemodelid, x, y, z, a, color1, color2, sirena == 1 ? 0 : -1);
    PutPlayerInVehicle(playerid, sampvehicleid, 0);

    SendClientMessage(playerid, COLOR_GRAD2, va_return("Vehículo creado con: Model ID: %d | Samp ID: %d ", vehiclemodelid, sampvehicleid));
    return 1;
}

alias:borrarvehiculo("borrarveh")
CMD:borrarvehiculo(playerid, params[])
{
    new vehicleid;
    if(sscanf(params, "d", vehicleid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /borrarveh(iculo) [vehicleid]");

    if(vehicleid < 1 || vehicleid > MAX_VEHICLES)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de vehículo inválido.");

    if(!IsValidVehicle(vehicleid))
        return SendClientMessage(playerid, COLOR_FADE3, "El vehículo no existe.");

    DestroyVehicle(vehicleid);
    SendClientMessage(playerid, COLOR_GRAD2, va_return("Vehículo ID %d eliminado.", vehicleid));
    return 1;
}

alias:irvehiculo("irveh")
CMD:irvehiculo(playerid, params[])
{
    new vehicleid;
    if(sscanf(params, "d", vehicleid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /irveh(iculo) [vehicleid]");

    if(vehicleid < 1 || vehicleid > MAX_VEHICLES)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de vehículo inválido.");

    if(!IsValidVehicle(vehicleid))
        return SendClientMessage(playerid, COLOR_FADE3, "El vehículo no existe.");

    new Float:x, Float:y, Float:z, Float:angle;
    GetVehiclePos(vehicleid, x, y, z);
    GetVehicleZAngle(vehicleid, angle);
    
    // Calcular posición de la puerta del conductor
    new Float:doorX = x + (1.5 * floatsin(-angle, degrees));
    new Float:doorY = y + (1.5 * floatcos(-angle, degrees));
    
    SetPlayerPos(playerid, doorX, doorY, z);
    SetPlayerFacingAngle(playerid, angle - 90.0);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, GetVehicleVirtualWorld(vehicleid));

    SendClientMessage(playerid, COLOR_GRAD2, va_return("Teletransportado a la puerta del vehículo ID %d.", vehicleid));
    return 1;
}

alias:traervehiculo("traerveh")
CMD:traervehiculo(playerid, params[])
{
    new vehicleid;
    if(sscanf(params, "d", vehicleid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /traerveh(iculo) [vehicleid]");

    if(vehicleid < 1 || vehicleid > MAX_VEHICLES)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de vehículo inválido.");

    if(!IsValidVehicle(vehicleid))
        return SendClientMessage(playerid, COLOR_FADE3, "El vehículo no existe.");

    new Float:x, Float:y, Float:z, Float:playerAngle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, playerAngle);

    new Float:vehicleX = x + (3.0 * floatsin(-playerAngle, degrees));
    new Float:vehicleY = y + (3.0 * floatcos(-playerAngle, degrees));
    
    new Float:vehicleAngle = playerAngle + 90.0;
    if(vehicleAngle > 360.0) vehicleAngle -= 360.0;

    SetVehiclePos(vehicleid, vehicleX, vehicleY, z);
    SetVehicleZAngle(vehicleid, vehicleAngle);
    SetVehicleVirtualWorld(vehicleid, GetPlayerVirtualWorld(playerid));
    LinkVehicleToInterior(vehicleid, GetPlayerInterior(playerid));

    // SendClientMessage(playerid, COLOR_GRAD2, va_return("Vehículo ID %d traído.", vehicleid));
    return 1;
}

alias:repararvehiculo("repararveh")
CMD:repararvehiculo(playerid, params[])
{
    new vehicleid;
    if(sscanf(params, "d", vehicleid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /repararveh(iculo) [vehicleid]");

    if(vehicleid < 1 || vehicleid > MAX_VEHICLES)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de vehículo inválido.");

    if(!IsValidVehicle(vehicleid))
        return SendClientMessage(playerid, COLOR_FADE3, "El vehículo no existe.");

    RepairVehicle(vehicleid);
    SetVehicleHealth(vehicleid, 1000.0);
    
    new panels, doors, lights, tires;
    GetVehicleDamageStatus(vehicleid, panels, doors, lights, tires);
    UpdateVehicleDamageStatus(vehicleid, 0, 0, 0, 0);

    // SendClientMessage(playerid, COLOR_GRAD2, va_return("Vehículo ID %d reparado completamente.", vehicleid));
    return 1;
}

alias:dardlvehiculo("dardlveh")
CMD:dardlvehiculo(playerid, params[])
{
    new vehicleid, Float:dl;
    if(sscanf(params, "df", vehicleid, dl))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /dardlveh(iculo) [vehicleid] [dl]");

    if(vehicleid < 1 || vehicleid > MAX_VEHICLES)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de vehículo inválido.");

    if(!IsValidVehicle(vehicleid))
        return SendClientMessage(playerid, COLOR_FADE3, "El vehículo no existe.");

    SetVehicleHealth(vehicleid, dl);
    
    //SendClientMessage(playerid, COLOR_GRAD2, va_return("Daño del vehículo ID %d establecido a %.1f.", vehicleid, dl));
    
    return 1;
}

alias:irvehiculointerior("irvehin")
CMD:irvehiculodentro(playerid, params[])
{
    new vehicleid;
    if(sscanf(params, "d", vehicleid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /irvehin [vehicleid]");

    if(vehicleid < 1 || vehicleid > MAX_VEHICLES)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de vehículo inválido.");

    if(!IsValidVehicle(vehicleid))
        return SendClientMessage(playerid, COLOR_FADE3, "El vehículo no existe.");

    // Verificar si el vehículo está ocupado
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(IsPlayerConnected(i) && IsPlayerInVehicle(i, vehicleid))
        {
            return SendClientMessage(playerid, COLOR_FADE3, "El vehículo está ocupado por otro jugador.");
        }
    }

    PutPlayerInVehicle(playerid, vehicleid, 0);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, GetVehicleVirtualWorld(vehicleid));

    // SendClientMessage(playerid, COLOR_GRAD2, va_return("Teletransportado dentro del vehículo ID %d.", vehicleid));
    
    return 1;
}

alias:traervehiculointerior("traervehin")
CMD:traervehiculointerior(playerid, params[])
{
    new vehicleid;
    if(sscanf(params, "d", vehicleid))
        return SendClientMessage(playerid, COLOR_GRAD1, "USO: /traervehin [vehicleid]");

    if(vehicleid < 1 || vehicleid > MAX_VEHICLES)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de vehículo inválido.");

    if(!IsValidVehicle(vehicleid))
        return SendClientMessage(playerid, COLOR_FADE3, "El vehículo no existe.");

    // Verificar si el vehículo está ocupado
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(IsPlayerConnected(i) && IsPlayerInVehicle(i, vehicleid))
        {
            return SendClientMessage(playerid, COLOR_FADE3, "El vehículo está ocupado por otro jugador.");
        }
    }

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    SetVehiclePos(vehicleid, x, y, z);
    SetVehicleZAngle(vehicleid, a);
    SetVehicleVirtualWorld(vehicleid, GetPlayerVirtualWorld(playerid));
    LinkVehicleToInterior(vehicleid, GetPlayerInterior(playerid));

    PutPlayerInVehicle(playerid, vehicleid, 0);

    SendClientMessage(playerid, COLOR_GRAD2, va_return("Vehículo ID %d traído y montado.", vehicleid));
    return 1;
}