// features/player/rapel.inc

#if defined _INC_RAPEL
    #endinput
#endif
#define _INC_RAPEL

// Constantes
#define MAX_RAPEL_HEIGHT_HELI 100.0 // Altura máxima para rapel desde helicóptero (150 metros)
#define MAX_RAPEL_HEIGHT_ROPE 30.0  // Altura máxima para cuerda estática (80 metros)
#define MIN_RAPEL_HEIGHT 3.0        // Altura mínima para usar rapel
#define RAPEL_SPEED 0.8             // Velocidad de descenso por segundo
#define MAX_HELI_SPEED 15.0          // Velocidad máxima del helicóptero en km/h para usar rapel

// Enums para organizar datos
enum E_RAPEL_DATA
{
    bool:isRapeling,
    ropeObject[10], // Array para múltiples segmentos de cuerda
    ropeSegments,
    rapelTimer,
    animTimer, 
    Float:targetZ,
    rapelVehicle,
    bool:usingStaticRope,
    staticRopeObject[10], // Array para múltiples segmentos
    staticRopeSegments,
    Float:staticRopeX,
    Float:staticRopeY,
    Float:staticRopeZ,
    Float:frozenVehiclePos[3], // Posición congelada del helicóptero
    bool:vehicleFrozen
}

new PlayerRapelData[MAX_PLAYERS][E_RAPEL_DATA];


stock bool:IsPlayerInHelicopter(playerid)
{
    if(!IsPlayerInAnyVehicle(playerid)) return false;
    
    new vehicleid = GetPlayerVehicleID(playerid);
    new modelid = GetVehicleModel(vehicleid);
    
    // IDs de helicópteros en SA-MP 
    //COLOCAR QUE SOLO HELI DE FACC LEO/FD
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

stock Float:GetVehicleSpeed(vehicleid)
{
    new Float:vx, Float:vy, Float:vz;
    GetVehicleVelocity(vehicleid, vx, vy, vz);
    
    // Calcular velocidad en km/h
    return floatsqroot(vx * vx + vy * vy + vz * vz) * 180.0;
}

stock bool:IsHelicopterStable(vehicleid)
{
    new Float:speed = GetVehicleSpeed(vehicleid);
    return (speed <= MAX_HELI_SPEED);
}

stock CreateRopeSegments(playerid, Float:startX, Float:startY, Float:startZ, Float:endZ, bool:isStatic = false)
{
    // Calcular cuántos segmentos se necesitan para que visualmente la cuerda toque el suelo.
    new Float:totalHeight = startZ - endZ;
    new segments = floatround(totalHeight / 3.0) + 1; // +1 para asegurar que llegue al suelo
    
    if(segments > 10) segments = 40; // Máximo 40 segmentos
    if(segments < 1) segments = 1;
    
    new Float:segmentHeight = totalHeight / float(segments);
    

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
        
        // Mantener el helicóptero congelado en su posición
        if(PlayerRapelData[playerid][vehicleFrozen])
        {
            SetVehiclePos(vehicleid, 
                PlayerRapelData[playerid][frozenVehiclePos][0],
                PlayerRapelData[playerid][frozenVehiclePos][1],
                PlayerRapelData[playerid][frozenVehiclePos][2]
            );
            SetVehicleVelocity(vehicleid, 0.0, 0.0, 0.0);
        }
        
        new Float:vx = PlayerRapelData[playerid][frozenVehiclePos][0];
        new Float:vy = PlayerRapelData[playerid][frozenVehiclePos][1];
        new Float:vz = PlayerRapelData[playerid][frozenVehiclePos][2];
        
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
        }
        else
        {
            StopRapel(playerid);
            SendClientMessage(playerid, 0x2ECC71FF, "Has llegado al suelo.");
        }
    }
}

// Función para aplicar animación de rapel de forma suave
forward ApplyRapelAnimation(playerid);
public ApplyRapelAnimation(playerid)
{
    if(!PlayerRapelData[playerid][isRapeling]) 
    {
        // Si ya no está en rapel, matar el timer
        if(PlayerRapelData[playerid][animTimer] != -1)
        {
            KillTimer(PlayerRapelData[playerid][animTimer]);
            PlayerRapelData[playerid][animTimer] = -1;
        }
        return;
    }
    
    // Aplicar animación de rapel con loop
    ApplyAnimation(playerid, "ped", "abseil", 4.0, 1, 0, 0, 0, 0, 1);
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
    
  
    if(PlayerRapelData[playerid][animTimer] != -1)
    {
        KillTimer(PlayerRapelData[playerid][animTimer]);
        PlayerRapelData[playerid][animTimer] = -1;
    }
    
    // Descongelar helicóptero si fue congelado
    if(PlayerRapelData[playerid][vehicleFrozen] && PlayerRapelData[playerid][rapelVehicle] != INVALID_VEHICLE_ID)
    {
        PlayerRapelData[playerid][vehicleFrozen] = false;
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

// Función para cancelar rapel
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
        SendClientMessage(playerid, 0xF1C40FFF, "Uso: /rapel descender [id]");
        return 1;
    }
    
    if(!strcmp(option, "descender", true))
    {
        // Verificar que sea el piloto
        if(!IsPlayerHelicopterPilot(playerid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Debes ser el piloto del helicóptero para usar el rapel.");
            return 1;
        }
        
        // Verificar que se haya especificado un ID
        if(targetid == -1)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Debes especificar el ID del jugador a descender.");
            SendClientMessage(playerid, 0xF1C40FFF, "Uso: /rapel descender [id]");
            return 1;
        }
        
        // Verificar que el jugador exista
        if(!IsPlayerConnected(targetid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador no está conectado.");
            return 1;
        }
        
        // Verificar que el objetivo esté en el helicóptero
        new vehicleid = GetPlayerVehicleID(playerid);
        if(!IsPlayerInVehicle(targetid, vehicleid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador no está en tu helicóptero.");
            return 1;
        }
        
        // Verificar que el objetivo no sea el piloto
        if(targetid == playerid)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El piloto no puede descender en rapel.");
            return 1;
        }
        
        // Verificar que el objetivo no esté ya en rapel
        if(PlayerRapelData[targetid][isRapeling])
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador ya está usando el rapel.");
            return 1;
        }
        
        // Verificar estabilidad del helicóptero
        if(!IsHelicopterStable(vehicleid))
        {
            new Float:speed = GetVehicleSpeed(vehicleid);
            SendClientMessage(playerid, 0xE74C3CFF, "El helicóptero está moviéndose demasiado rápido para usar el rapel.");
            new msg[128];
            format(msg, sizeof(msg), "Velocidad actual: %.1f km/h - Máximo permitido: %.1f km/h", speed, MAX_HELI_SPEED); // DEBUG
            SendClientMessage(playerid, 0xF1C40FFF, msg);
            return 1;
        }
        
        // Verificar altura
        new Float:x, Float:y, Float:z;
        GetPlayerPos(playerid, x, y, z);
        new Float:groundZ = GetGroundZ(x, y, z);
        new Float:height = z - groundZ;
        
        if(height > MAX_RAPEL_HEIGHT_HELI)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El helicóptero está demasiado alto para usar el rapel.");
            new msg[128];
            format(msg, sizeof(msg), "Altura actual: %.1f metros - Máximo: %.1f metros", height, MAX_RAPEL_HEIGHT_HELI);
            SendClientMessage(playerid, 0xF1C40FFF, msg);
            return 1;
        }
        
        if(height < MIN_RAPEL_HEIGHT)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El helicóptero está demasiado bajo para usar el rapel.");
            return 1;
        }
        
        // Congelar helicóptero
        new Float:vx, Float:vy, Float:vz;
        GetVehiclePos(vehicleid, vx, vy, vz);
        
        PlayerRapelData[targetid][frozenVehiclePos][0] = vx;
        PlayerRapelData[targetid][frozenVehiclePos][1] = vy;
        PlayerRapelData[targetid][frozenVehiclePos][2] = vz;
        PlayerRapelData[targetid][vehicleFrozen] = true;
        
        // Remover jugador del helicóptero
        RemovePlayerFromVehicle(targetid);
        
        SetPlayerPos(targetid, vx, vy, vz - 2.0);
        SetPlayerVirtualWorld(targetid, GetPlayerVirtualWorld(targetid));
        SetPlayerInterior(targetid, GetPlayerInterior(targetid));
        
        // Crear segmentos de cuerda desde el helicóptero hasta el suelo
        CreateRopeSegments(targetid, vx, vy, vz - 1.0, groundZ, false);
        
   
        PlayerRapelData[targetid][isRapeling] = true;
        PlayerRapelData[targetid][rapelVehicle] = vehicleid;
        PlayerRapelData[targetid][targetZ] = groundZ + 1.0;
        PlayerRapelData[targetid][usingStaticRope] = false;
        
        TogglePlayerControllable(targetid, 0);
        
        // Aplicar animación inicial
        ApplyAnimation(targetid, "ped", "abseil", 4.0, 1, 0, 0, 0, 0, 1);
        
        // Timer para actualizar posición (cada 100ms)
        PlayerRapelData[targetid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", targetid);
        
        // Timer para mantener la animación (cada 4 segundos)
        PlayerRapelData[targetid][animTimer] = SetTimerEx("ApplyRapelAnimation", 4000, true, "i", targetid);
        
        SendClientMessage(playerid, 0x2ECC71FF, "Iniciando descenso en rapel del jugador...");
        SendClientMessage(targetid, 0x2ECC71FF, "Comenzando descenso en rapel...");
        SendClientMessage(targetid, 0xF1C40FFF, "Usa /pararrapel para cancelar");
    }
    else
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Opción inválida. Usa: descender");
    }
    
    return 1;
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
    
    if(height > MAX_RAPEL_HEIGHT_ROPE)
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Estás demasiado alto para usar una cuerda.");
        new msg[128];
        format(msg, sizeof(msg), "Altura actual: %.1f metros - Máximo: %.1f metros", height, MAX_RAPEL_HEIGHT_ROPE);
        SendClientMessage(playerid, 0xF1C40FFF, msg);
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
    PlayerRapelData[playerid][vehicleFrozen] = false;
    
    TogglePlayerControllable(playerid, 0);
    
    // Aplicar animación inicial
    ApplyAnimation(playerid, "ped", "abseil", 4.0, 1, 0, 0, 0, 0, 1);
    
    // Timer para actualizar posición (cada 100ms)
    PlayerRapelData[playerid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", playerid);
    
    // Timer para mantener la animación (cada 4 segundos)
    PlayerRapelData[playerid][animTimer] = SetTimerEx("ApplyRapelAnimation", 4000, true, "i", playerid);
    
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
        PlayerRapelData[i][animTimer] = -1;
        PlayerRapelData[i][rapelVehicle] = INVALID_VEHICLE_ID;
        PlayerRapelData[i][usingStaticRope] = false;
        PlayerRapelData[i][isRapeling] = false;
        PlayerRapelData[i][vehicleFrozen] = false;
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