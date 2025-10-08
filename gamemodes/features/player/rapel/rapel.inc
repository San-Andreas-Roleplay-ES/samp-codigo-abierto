// features/player/rapel.inc

#if defined _INC_RAPEL
    #endinput
#endif
#define _INC_RAPEL

// Constantes
#define MAX_RAPEL_HEIGHT 30.0 // Altura máxima para usar rapel (30 metros)
#define MIN_RAPEL_HEIGHT 3.0  // Altura mínima para usar rapel
#define RAPEL_SPEED 0.5      // Velocidad de descenso/ascenso por segundo (más lento)

// Enums para organizar datos
enum E_RAPEL_DATA
{
    bool:isRapeling,
    ropeObject[10], // Array para múltiples segmentos de cuerda
    ropeSegments,
    rapelTimer,
    Float:targetZ,
    rapelVehicle,
    bool:usingStaticRope,
    staticRopeObject[10], // Array para múltiples segmentos
    staticRopeSegments,
    Float:staticRopeX,
    Float:staticRopeY,
    Float:staticRopeZ
}

new PlayerRapelData[MAX_PLAYERS][E_RAPEL_DATA];

// Funciones auxiliares
stock bool:IsPlayerInHelicopter(playerid)
{
    if(!IsPlayerInAnyVehicle(playerid)) return false;
    
    new vehicleid = GetPlayerVehicleID(playerid);
    new modelid = GetVehicleModel(vehicleid);
    
    // IDs de helicópteros en SA-MP
    switch(modelid)
    {
        case 417, 425, 447, 469, 487, 488, 497, 548, 563: return true;
    }
    return false;
}

stock bool:IsPlayerHelicopterPilot(playerid)
{
    if(!IsPlayerInHelicopter(playerid)) return false;
    
    new seat = GetPlayerVehicleSeat(playerid);
    return (seat == 0); // Asiento del piloto
}

stock CreateRopeSegments(playerid, Float:startX, Float:startY, Float:startZ, Float:endZ, bool:isStatic = false)
{
    // Calcular cuántos segmentos necesitamos (cada segmento de cuerda mide aproximadamente 3 metros)
    new Float:totalHeight = startZ - endZ;
    new segments = floatround(totalHeight / 3.0) + 1; // +1 para asegurar que llegue al suelo
    
    if(segments > 10) segments = 10; // Máximo 10 segmentos
    if(segments < 1) segments = 1;
    
    new Float:segmentHeight = totalHeight / float(segments);
    
    // Crear cada segmento de cuerda
    for(new i = 0; i < segments; i++)
    {
        new Float:segZ = startZ - (float(i) * segmentHeight);
        new objectid = CreateDynamicObject(19089, startX, startY, segZ, 0.0, 0.0, 0.0);
        
        if(isStatic)
        {
            PlayerRapelData[playerid][staticRopeObject][i] = objectid;
        }
        else
        {
            PlayerRapelData[playerid][ropeObject][i] = objectid;
        }
    }
    
    if(isStatic)
    {
        PlayerRapelData[playerid][staticRopeSegments] = segments;
    }
    else
    {
        PlayerRapelData[playerid][ropeSegments] = segments;
    }
    
    return segments;
}

stock Float:GetGroundZ(Float:x, Float:y, Float:z)
{
    new Float:groundZ;
    
    // Usar ColAndreas RayCast desde la posición actual hacia abajo
    new Float:hit_x, Float:hit_y, Float:hit_z;
    if(CA_RayCastLine(x, y, z + 5.0, x, y, z - 500.0, hit_x, hit_y, hit_z))
    {
        groundZ = hit_z;
    }
    else
    {
        // Si ColAndreas no encuentra nada, usar altura relativa
        groundZ = z - 50.0;
    }
    
    return groundZ;
}

stock GetVehicleOccupant(vehicleid, seat)
{
    foreach(new i : Player)
    {
        if(IsPlayerInVehicle(i, vehicleid) && GetPlayerVehicleSeat(i) == seat)
        {
            return i;
        }
    }
    return INVALID_PLAYER_ID;
}

// Función para actualizar la posición durante el rapel
forward RapelUpdate(playerid);
public RapelUpdate(playerid)
{
    // IMPORTANTE: Solo ejecutar si está activamente en rapel
    if(!PlayerRapelData[playerid][isRapeling]) 
    {
        return;
    }
    
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    if(PlayerRapelData[playerid][usingStaticRope])
    {
        // Rapel con cuerda estática
        if(z > PlayerRapelData[playerid][targetZ])
        {
            z -= RAPEL_SPEED;
            if(z < PlayerRapelData[playerid][targetZ]) z = PlayerRapelData[playerid][targetZ];
            
            SetPlayerPos(playerid, x, y, z);
            ApplyAnimation(playerid, "ped", "abseil", 4.1, 1, 0, 0, 1, 0);
        }
        else
        {
            // Terminó el descenso
            StopRapel(playerid);
            SendClientMessage(playerid, 0x2ECC71FF, "Has llegado al suelo.");
        }
    }
    else if(PlayerRapelData[playerid][rapelVehicle] != INVALID_VEHICLE_ID)
    {
        // Rapel desde helicóptero
        new vehicleid = PlayerRapelData[playerid][rapelVehicle];
        
        if(!IsValidVehicle(vehicleid))
        {
            StopRapel(playerid);
            SendClientMessage(playerid, 0xE74C3CFF, "El helicóptero ya no existe.");
            return;
        }
        
        new Float:vx, Float:vy, Float:vz;
        GetVehiclePos(vehicleid, vx, vy, vz);
        
        // Actualizar posición de todos los segmentos de cuerda
        if(PlayerRapelData[playerid][ropeSegments] > 0)
        {
            new Float:totalHeight = vz - PlayerRapelData[playerid][targetZ];
            new Float:segmentHeight = totalHeight / float(PlayerRapelData[playerid][ropeSegments]);
            
            for(new i = 0; i < PlayerRapelData[playerid][ropeSegments]; i++)
            {
                if(IsValidDynamicObject(PlayerRapelData[playerid][ropeObject][i]))
                {
                    new Float:segZ = vz - 1.0 - (float(i) * segmentHeight);
                    SetDynamicObjectPos(PlayerRapelData[playerid][ropeObject][i], vx, vy, segZ);
                }
            }
        }
        
        if(z > PlayerRapelData[playerid][targetZ])
        {
            z -= RAPEL_SPEED;
            if(z < PlayerRapelData[playerid][targetZ]) z = PlayerRapelData[playerid][targetZ];
            
            // Mantener al jugador debajo del helicóptero
            SetPlayerPos(playerid, vx, vy, z);
            ApplyAnimation(playerid, "ped", "abseil", 4.1, 1, 0, 0, 1, 0);
        }
        else
        {
            StopRapel(playerid);
            SendClientMessage(playerid, 0x2ECC71FF, "Has llegado al suelo.");
        }
    }
}

// Función para detener el rapel
stock StopRapel(playerid)
{
    PlayerRapelData[playerid][isRapeling] = false;
    
    if(PlayerRapelData[playerid][rapelTimer] != -1)
    {
        KillTimer(PlayerRapelData[playerid][rapelTimer]);
        PlayerRapelData[playerid][rapelTimer] = -1;
    }
    
    // Destruir todos los segmentos de cuerda del helicóptero
    for(new i = 0; i < PlayerRapelData[playerid][ropeSegments]; i++)
    {
        if(IsValidDynamicObject(PlayerRapelData[playerid][ropeObject][i]))
        {
            DestroyDynamicObject(PlayerRapelData[playerid][ropeObject][i]);
        }
        PlayerRapelData[playerid][ropeObject][i] = -1;
    }
    PlayerRapelData[playerid][ropeSegments] = 0;
    
    // Destruir todos los segmentos de cuerda estática
    for(new i = 0; i < PlayerRapelData[playerid][staticRopeSegments]; i++)
    {
        if(IsValidDynamicObject(PlayerRapelData[playerid][staticRopeObject][i]))
        {
            DestroyDynamicObject(PlayerRapelData[playerid][staticRopeObject][i]);
        }
        PlayerRapelData[playerid][staticRopeObject][i] = -1;
    }
    PlayerRapelData[playerid][staticRopeSegments] = 0;
    
    PlayerRapelData[playerid][rapelVehicle] = INVALID_VEHICLE_ID;
    PlayerRapelData[playerid][usingStaticRope] = false;
    ClearAnimations(playerid);
    TogglePlayerControllable(playerid, 1);
}

// Función para cancelar rapel con tecla
CMD:pararrapel(playerid, params[])
{
    if(!PlayerRapelData[playerid][isRapeling])
    {
        SendClientMessage(playerid, 0xE74C3CFF, "No estás usando el rapel.");
        return 1;
    }
    
    StopRapel(playerid);
    SendClientMessage(playerid, 0xF1C40FFF, "Has cancelado el rapel.");
    return 1;
}

// Comandos
CMD:rapel(playerid, params[])
{
    new option[32], targetid;
    
    if(sscanf(params, "s[32]D(-1)", option, targetid))
    {
        SendClientMessage(playerid, 0xF1C40FFF, "Uso: /rapel [descender/ascender] [id (solo para ascender)]");
        return 1;
    }
    
    if(!strcmp(option, "descender", true))
    {
        // Verificar que no esté ya en rapel
        if(PlayerRapelData[playerid][isRapeling])
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ya estás usando el rapel.");
            return 1;
        }
        
        // Verificar que esté en un helicóptero
        if(!IsPlayerInHelicopter(playerid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Debes estar en un helicóptero para usar el rapel.");
            return 1;
        }
        
        // Verificar que no sea el piloto
        if(IsPlayerHelicopterPilot(playerid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El piloto no puede usar el rapel.");
            return 1;
        }
        
        // Verificar altura
        new Float:x, Float:y, Float:z;
        GetPlayerPos(playerid, x, y, z);
        new Float:groundZ = GetGroundZ(x, y, z);
        new Float:height = z - groundZ;
        
        if(height > MAX_RAPEL_HEIGHT)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El helicóptero está demasiado alto para usar el rapel.");
            return 1;
        }
        
        if(height < MIN_RAPEL_HEIGHT)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El helicóptero está demasiado bajo para usar el rapel.");
            return 1;
        }
        
        // Iniciar rapel
        new vehicleid = GetPlayerVehicleID(playerid);
        RemovePlayerFromVehicle(playerid);
        
        new Float:vx, Float:vy, Float:vz;
        GetVehiclePos(vehicleid, vx, vy, vz);
        
        SetPlayerPos(playerid, vx, vy, vz - 2.0);
        SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(playerid));
        SetPlayerInterior(playerid, GetPlayerInterior(playerid));
        
        // Crear segmentos de cuerda desde el helicóptero hasta el suelo
        CreateRopeSegments(playerid, vx, vy, vz - 1.0, groundZ, false);
        
        // Configurar rapel
        PlayerRapelData[playerid][isRapeling] = true;
        PlayerRapelData[playerid][rapelVehicle] = vehicleid;
        PlayerRapelData[playerid][targetZ] = groundZ + 1.0;
        PlayerRapelData[playerid][usingStaticRope] = false;
        
        TogglePlayerControllable(playerid, 0);
        ApplyAnimation(playerid, "ped", "abseil", 4.1, 1, 0, 0, 1, 0);

        PlayerRapelData[playerid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", playerid);
        
        SendClientMessage(playerid, 0x2ECC71FF, "Comenzando descenso en rapel...");
        SendClientMessage(playerid, 0xF1C40FFF, "Usa /pararrapel para cancelar");
    }
    else if(!strcmp(option, "ascender", true))
    {
        // Verificar que sea el piloto
        if(!IsPlayerHelicopterPilot(playerid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Debes ser el piloto del helicóptero para ascender a alguien.");
            return 1;
        }
        
        if(targetid == -1)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Debes especificar el ID del jugador a ascender.");
            SendClientMessage(playerid, 0xF1C40FFF, "Uso: /rapel ascender [id]");
            return 1;
        }
        
        if(!IsPlayerConnected(targetid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador no está conectado.");
            return 1;
        }
        
        if(PlayerRapelData[targetid][isRapeling])
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador ya está usando el rapel.");
            return 1;
        }
        
        if(IsPlayerInAnyVehicle(targetid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador está en un vehículo.");
            return 1;
        }
        
        // Verificar distancia
        new Float:px, Float:py, Float:pz, Float:tx, Float:ty, Float:tz;
        GetPlayerPos(playerid, px, py, pz);
        GetPlayerPos(targetid, tx, ty, tz);
        
        new Float:distance = floatsqroot(floatpower(px - tx, 2.0) + floatpower(py - ty, 2.0));
        
        if(distance > 10.0)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El jugador está demasiado lejos del helicóptero.");
            return 1;
        }
        
        new Float:heightDiff = pz - tz;
        if(heightDiff < MIN_RAPEL_HEIGHT || heightDiff > MAX_RAPEL_HEIGHT)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "La altura no es adecuada para el ascenso.");
            return 1;
        }
        
        // Iniciar ascenso
        new vehicleid = GetPlayerVehicleID(playerid);
        
        PlayerRapelData[targetid][isRapeling] = true;
        PlayerRapelData[targetid][rapelVehicle] = vehicleid;
        PlayerRapelData[targetid][targetZ] = pz - 2.0;
        PlayerRapelData[targetid][usingStaticRope] = false;
        
        // Crear segmentos de cuerda desde el helicóptero hasta el jugador
        CreateRopeSegments(targetid, px, py, pz - 1.0, tz, false);
        
        TogglePlayerControllable(targetid, 0);
        ApplyAnimation(targetid, "ped", "abseil", 4.1, 1, 0, 0, 1, 0);
        
        PlayerRapelData[targetid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", targetid);
        
        SendClientMessage(playerid, 0x2ECC71FF, "Ascendiendo jugador al helicóptero...");
        SendClientMessage(targetid, 0x2ECC71FF, "Estás siendo ascendido al helicóptero...");
        
        // Cuando llegue arriba, meterlo en el helicóptero
        SetTimerEx("PutPlayerInHeliAfterRapel", floatround((heightDiff / RAPEL_SPEED) * 100.0), false, "ii", targetid, vehicleid);
    }
    else
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Opción inválida. Usa: descender o ascender");
    }
    
    return 1;
}

forward PutPlayerInHeliAfterRapel(playerid, vehicleid);
public PutPlayerInHeliAfterRapel(playerid, vehicleid)
{
    if(!PlayerRapelData[playerid][isRapeling]) return;
    
    StopRapel(playerid);
    
    if(IsValidVehicle(vehicleid))
    {
        // Buscar asiento libre
        for(new seat = 1; seat < 4; seat++)
        {
            new occupant = GetVehicleOccupant(vehicleid, seat);
            if(occupant == INVALID_PLAYER_ID)
            {
                PutPlayerInVehicle(playerid, vehicleid, seat);
                SendClientMessage(playerid, 0x2ECC71FF, "Has sido subido al helicóptero.");
                return;
            }
        }
        SendClientMessage(playerid, 0xE74C3CFF, "No hay asientos libres en el helicóptero.");
    }
}

CMD:cuerda(playerid, params[])
{
    if(PlayerRapelData[playerid][isRapeling])
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Ya estás usando una cuerda.");
        return 1;
    }
    
    if(IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, 0xE74C3CFF, "No puedes usar una cuerda dentro de un vehículo.");
        return 1;
    }
    
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    new Float:groundZ = GetGroundZ(x, y, z);
    new Float:height = z - groundZ;
    
    if(height < MIN_RAPEL_HEIGHT)
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Estás demasiado cerca del suelo para usar una cuerda.");
        return 1;
    }
    
    if(height > MAX_RAPEL_HEIGHT)
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Estás demasiado alto para usar una cuerda.");
        return 1;
    }
    
    // Crear cuerda estática con múltiples segmentos
    CreateRopeSegments(playerid, x, y, z, groundZ, true);
    PlayerRapelData[playerid][staticRopeX] = x;
    PlayerRapelData[playerid][staticRopeY] = y;
    PlayerRapelData[playerid][staticRopeZ] = z;
    
    // Iniciar descenso
    PlayerRapelData[playerid][isRapeling] = true;
    PlayerRapelData[playerid][targetZ] = groundZ + 1.0;
    PlayerRapelData[playerid][usingStaticRope] = true;
    PlayerRapelData[playerid][rapelVehicle] = INVALID_VEHICLE_ID;
    
    TogglePlayerControllable(playerid, 0);
    ApplyAnimation(playerid, "ped", "abseil", 4.1, 1, 0, 0, 1, 0);
    
    PlayerRapelData[playerid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", playerid);
    
    SendClientMessage(playerid, 0x2ECC71FF, "Descendiendo por la cuerda...");
    SendClientMessage(playerid, 0xF1C40FFF, "Usa /pararrapel para cancelar");
    return 1;
}

// Inicialización
stock RapelSystem_Init()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        for(new j = 0; j < 10; j++)
        {
            PlayerRapelData[i][ropeObject][j] = -1;
            PlayerRapelData[i][staticRopeObject][j] = -1;
        }
        PlayerRapelData[i][ropeSegments] = 0;
        PlayerRapelData[i][staticRopeSegments] = 0;
        PlayerRapelData[i][rapelTimer] = -1;
        PlayerRapelData[i][rapelVehicle] = INVALID_VEHICLE_ID;
        PlayerRapelData[i][usingStaticRope] = false;
        PlayerRapelData[i][isRapeling] = false;
    }
}

// Llamar al disconnect
stock RapelSystem_OnPlayerDisconnect(playerid)
{
    if(PlayerRapelData[playerid][isRapeling])
    {
        StopRapel(playerid);
    }
}

