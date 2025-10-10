// SAN ANDREAS ROLEPLAY - Vehicle Part Position Viewer
// Tool para visualizar posiciones de partes de cualquier vehículo

#include <YSI_Coding\y_hooks>

#define MAX_VEHICLE_LABELS 15
#define MAX_VEHICLES_LABELS 2000

// Sistema global de debug de vehículos
new bool:VehicleDebugActive = false;
new Text3D:VehicleLabels[MAX_VEHICLES_LABELS];
new VehicleLabelCount = 0;

stock ResetVehicleDebugSystem()
{
    VehicleDebugActive = false;
    for(new i = 0; i < VehicleLabelCount; i++)
    {
        if(VehicleLabels[i] != Text3D:INVALID_3DTEXT_ID)
        {
            Delete3DTextLabel(VehicleLabels[i]);
            VehicleLabels[i] = Text3D:INVALID_3DTEXT_ID;
        }
    }
    VehicleLabelCount = 0;
}

stock bool:IsValidVehiclePart(vehicleid, part)
{
    new Float:pos[3], Float:vehiclePos[3];
    
    // Obtener posición del vehículo para comparar
    GetVehiclePos(vehicleid, vehiclePos[0], vehiclePos[1], vehiclePos[2]);
    
    // Obtener posición de la parte
    GetVehiclePartPos(vehicleid, part, pos[0], pos[1], pos[2]);
    
    // Si la posición de la parte es igual a la posición del vehículo o (0,0,0), 
    // significa que la parte no existe
    if((pos[0] == vehiclePos[0] && pos[1] == vehiclePos[1] && pos[2] == vehiclePos[2]) ||
       (pos[0] == 0.0 && pos[1] == 0.0 && pos[2] == 0.0))
    {
        return false;
    }
    
    return true;
}

stock CreateVehiclePartLabel(vehicleid, part, const text[], color)
{
    if(VehicleLabelCount >= MAX_VEHICLES_LABELS) return;
    
    // Validar que la parte existe en este vehículo
    if(!IsValidVehiclePart(vehicleid, part)) return;
    
    new Float:pos[3];
    GetVehiclePartPos(vehicleid, part, pos[0], pos[1], pos[2]);
    
    new Text3D:label = Create3DTextLabel(text, color, pos[0], pos[1], pos[2], 10.0, 0);
    Attach3DTextLabelToVehicle(label, vehicleid, pos[0], pos[1], pos[2]);
    
    VehicleLabels[VehicleLabelCount] = label;
    VehicleLabelCount++;
}

stock ShowAllVehiclePositions()
{
    if(VehicleDebugActive)
    {
        return 0; // Ya está activo
    }
    
    VehicleDebugActive = true;
    VehicleLabelCount = 0;
    
    // Iterar por todos los vehículos del servidor
    for(new vehicleid = 1; vehicleid < MAX_VEHICLES; vehicleid++)
    {
        if(!IsValidVehicle(vehicleid)) continue;
        
        // Abrir capó y maletero para mejor visualización
        new engine, lights, alarm, doors, bonnet, boot, objective;
        GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
        SetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, 1, 1, objective);
        
        // Crear labels para todas las partes del vehículo
        CreateVehiclePartLabel(vehicleid, VEH_PART_RFTIRE, "RF Tire", 0xFFFF00FF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_LFTIRE, "LF Tire", 0xFFFF00FF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_RRTIRE, "RR Tire", 0xFFFF00FF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_LRTIRE, "LR Tire", 0xFFFF00FF);
        
        CreateVehiclePartLabel(vehicleid, VEH_PART_RFSEAT, "RF Seat", 0x00FF00FF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_LFSEAT, "LF Seat", 0x00FF00FF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_RRSEAT, "RR Seat", 0x00FFFFFF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_LRSEAT, "LR Seat", 0x00FFFFFF);
        
        CreateVehiclePartLabel(vehicleid, VEH_PART_HOOD, "Hood", 0xFF0000FF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_TRUNK, "Trunk", 0xFF0000FF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_PCAP, "P.Cap", 0x0000FFFF);
        
        CreateVehiclePartLabel(vehicleid, VEH_PART_FTIRE, "F Tire", 0xFFFFFFFF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_RTIRE, "R Tire", 0xFFFFFFFF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_RMTIRE, "RM Tire", 0xFFFFFFFF);
        CreateVehiclePartLabel(vehicleid, VEH_PART_LMTIRE, "LM Tire", 0xFFFFFFFF);
    }
    
    return 1;
}

stock ShowVehiclePositions(playerid)
{
    if(VehicleDebugActive)
    {
        SendClientMessage(playerid, 0xFF0000FF, "El sistema de debug de vehículos ya está activo.");
        return 1;
    }
    
    new result = ShowAllVehiclePositions();
    if(result)
    {
        new string[128];
        format(string, sizeof(string), "? Debug activado: %d labels creadas en todos los vehículos del servidor.", VehicleLabelCount);
        SendClientMessage(playerid, 0x00FF00FF, string);
        SendClientMessage(playerid, 0xFFFFFFFF, "Usa /hidepositions para ocultar las posiciones.");
    }
    else
    {
        SendClientMessage(playerid, 0xFF0000FF, "Error al activar el sistema de debug.");
    }
    
    return 1;
}

stock HideVehiclePositions(playerid)
{
    if(!VehicleDebugActive)
    {
        SendClientMessage(playerid, 0xFF0000FF, "El sistema de debug no está activo.");
        return 0;
    }
    
    // Restaurar parámetros de todos los vehículos (cerrar capós y maleteros)
    for(new vehicleid = 1; vehicleid < MAX_VEHICLES; vehicleid++)
    {
        if(!IsValidVehicle(vehicleid)) continue;
        
        new engine, lights, alarm, doors, bonnet, boot, objective;
        GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
        SetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, 0, 0, objective); // Cerrar capó y maletero
    }
    
    // Resetear el sistema
    ResetVehicleDebugSystem();
    
    SendClientMessage(playerid, 0x00FF00FF, "? Sistema de debug de vehículos desactivado.");
    return 1;
}

CMD:showpositions(playerid, params[])
{
    // Comando de administrador para debug de vehículos
    return ShowVehiclePositions(playerid);
}

CMD:hidepositions(playerid, params[])
{
    // Comando de administrador para debug de vehículos
    return HideVehiclePositions(playerid);
}

CMD:vehinfo(playerid, params[])
{
    // Comando de administrador para obtener información del vehículo actual
    if(!IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, 0xFF0000FF, "Debes estar en un vehículo para ver su información.");
        return 1;
    }
    
    new vehicleid = GetPlayerVehicleID(playerid);
    new model = GetVehicleModel(vehicleid);
    new Float:health, Float:pos[3], Float:angle;
    new string[256];
    
    GetVehicleHealth(vehicleid, health);
    GetVehiclePos(vehicleid, pos[0], pos[1], pos[2]);
    GetVehicleZAngle(vehicleid, angle);
    
    format(string, sizeof(string), "=== INFORMACIÓN DEL VEHÍCULO ===");
    SendClientMessage(playerid, 0x00FF00FF, string);
    
    format(string, sizeof(string), "ID: %d | Modelo: %d | Vida: %.1f", vehicleid, model, health);
    SendClientMessage(playerid, 0xFFFFFFFF, string);
    
    format(string, sizeof(string), "Posición: %.2f, %.2f, %.2f | Ángulo: %.2f", pos[0], pos[1], pos[2], angle);
    SendClientMessage(playerid, 0xFFFFFFFF, string);
    
    format(string, sizeof(string), "Usa /showpositions para ver las partes del vehículo");
    SendClientMessage(playerid, 0xFFFF00FF, string);
    
    return 1;
}

// Hooks para manejo automático del sistema
hook OnGameModeInit()
{
    VehicleDebugActive = false;
    VehicleLabelCount = 0;
    for(new i = 0; i < MAX_VEHICLES_LABELS; i++)
    {
        VehicleLabels[i] = Text3D:INVALID_3DTEXT_ID;
    }
    return 1;
}

hook OnGameModeExit()
{
    ResetVehicleDebugSystem();
    return 1;
}