// features/player/rapel.inc

#if defined _INC_RAPEL
    #endinput
#endif
#define _INC_RAPEL

#include <YSI_Coding\y_hooks>

// Constantes
#define MAX_RAPEL_HEIGHT_HELI 100.0 // Altura maxima para rapel desde helicoptero (100 metros)
#define MAX_RAPEL_HEIGHT_ROPE 30.0  // Altura maxima para cuerda estatica (30 metros)
#define MIN_RAPEL_HEIGHT 3.0        // Altura minima para usar rapel
#define RAPEL_SPEED 2.0             // Velocidad de descenso/ascenso en metros por segundo
#define MAX_HELI_SPEED 15.0          // Velocidad maxima del helicoptero en km/h para usar rapel
#define MAX_RAPEL_PICKUP_RADIUS 8.0  // Radio horizontal maximo para recoger a alguien del suelo

// Enums para organizar datos
enum E_RAPEL_DATA
{
    bool:isRapeling,
    bool:isAscending, // true = subiendo hacia el helicoptero, false = bajando
    ropeObject[10], // Array para multiples segmentos de cuerda
    ropeSegments,
    rapelTimer,
    animTimer,
    Float:targetZ, // Altura donde el jugador se detiene (suelo al bajar, helicoptero al subir)
    Float:ropeAnchorZ, // Extremo fijo de la cuerda usado solo para dibujar los segmentos
    ascendTicksLeft, // Cuenta regresiva de seguridad (ticks de 100ms) para forzar la llegada al subir
    rapelVehicle,
    bool:usingStaticRope,
    staticRopeObject[10], // Array para multiples segmentos
    staticRopeSegments,
    Float:staticRopeX,
    Float:staticRopeY,
    Float:staticRopeZ,
    Float:frozenVehiclePos[3], // Posicion congelada del helicoptero
    bool:vehicleFrozen
}

new PlayerRapelData[MAX_PLAYERS][E_RAPEL_DATA];


stock bool:IsPlayerInHelicopter(playerid)
{
    if(!IsPlayerInAnyVehicle(playerid)) return false;

    new vehicleid = GetPlayerVehicleID(playerid);
    new modelid = GetVehicleModel(vehicleid);

    // IDs de helicopteros en SA-MP
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

// Calcula cuantos ticks de 100ms deberia tardar en recorrer "distance" metros
// a RAPEL_SPEED, mas un margen de seguridad de ~1 segundo (10 ticks).
stock CalculateAscendTicks(Float:distance)
{
    new ticks = floatround(distance / (RAPEL_SPEED / 10.0), floatround_ceil) + 10;
    if(ticks < 10) ticks = 10;
    return ticks;
}

stock CreateRopeSegments(playerid, Float:startX, Float:startY, Float:startZ, Float:endZ, bool:isStatic = false)
{
    // Calcular cuantos segmentos se necesitan para que visualmente la cuerda toque el suelo.
    new Float:totalHeight = startZ - endZ;
    new segments = floatround(totalHeight / 3.0) + 1; // +1 para asegurar que llegue al suelo

    if(segments > 10) segments = 10; // Maximo 10 segmentos (tamano real de ropeObject/staticRopeObject)
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

    // Usar ColAndreas RayCast desde la posicion actual hacia abajo
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

// Funcion para actualizar la posicion durante el rapel
forward RapelUpdate(playerid);
public RapelUpdate(playerid)
{
    // IMPORTANTE: Solo ejecutar si esta activamente en rapel
    if(!PlayerRapelData[playerid][isRapeling])
    {
        return;
    }

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    if(PlayerRapelData[playerid][usingStaticRope])
    {
        // Rapel con cuerda estatica
        if(z > PlayerRapelData[playerid][targetZ])
        {
            z -= RAPEL_SPEED / 10.0; // Timer corre cada 100ms, RAPEL_SPEED es por segundo
            if(z < PlayerRapelData[playerid][targetZ]) z = PlayerRapelData[playerid][targetZ];

            SetPlayerPos(playerid, x, y, z);
            SetPlayerVelocity(playerid, 0.0, 0.0, 0.0); // Anular caida por gravedad entre ticks
        }
        else
        {
            // Termino el descenso
            StopRapel(playerid);
            SendClientMessage(playerid, 0x2ECC71FF, "Has llegado al suelo.");
        }
    }
    else if(PlayerRapelData[playerid][rapelVehicle] != INVALID_VEHICLE_ID)
    {
        // Rapel desde helicoptero
        new vehicleid = PlayerRapelData[playerid][rapelVehicle];

        if(!IsValidVehicle(vehicleid))
        {
            StopRapel(playerid);
            SendClientMessage(playerid, 0xE74C3CFF, "El helicoptero ya no existe.");
            return;
        }

        // Mantener el helicoptero congelado en su posicion
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

        // Actualizar posicion de todos los segmentos de cuerda (siempre ancladas entre el helicoptero y el punto fijo de la cuerda)
        if(PlayerRapelData[playerid][ropeSegments] > 0)
        {
            new Float:totalHeight = vz - PlayerRapelData[playerid][ropeAnchorZ];
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

        if(PlayerRapelData[playerid][isAscending])
        {
            // Cuenta regresiva de seguridad: si por lo que sea nunca se detecta
            // que llego (drift de fisica, gravedad, lo que sea), forzamos la
            // llegada de todos modos en cuanto se acaba el margen.
            PlayerRapelData[playerid][ascendTicksLeft]--;

            if(z < PlayerRapelData[playerid][targetZ] && PlayerRapelData[playerid][ascendTicksLeft] > 0)
            {
                z += RAPEL_SPEED / 10.0; // Timer corre cada 100ms, RAPEL_SPEED es por segundo
                if(z > PlayerRapelData[playerid][targetZ]) z = PlayerRapelData[playerid][targetZ];

                // Mantener al jugador debajo del helicoptero mientras sube
                SetPlayerPos(playerid, vx, vy, z);
                SetPlayerVelocity(playerid, 0.0, 0.0, 0.0); // Anular caida por gravedad entre ticks
            }
            else
            {
                FinishAscend(playerid, vehicleid);
            }
        }
        else
        {
            if(z > PlayerRapelData[playerid][targetZ])
            {
                z -= RAPEL_SPEED / 10.0; // Timer corre cada 100ms, RAPEL_SPEED es por segundo
                if(z < PlayerRapelData[playerid][targetZ]) z = PlayerRapelData[playerid][targetZ];

                // Mantener al jugador debajo del helicoptero
                SetPlayerPos(playerid, vx, vy, z);
                SetPlayerVelocity(playerid, 0.0, 0.0, 0.0); // Anular caida por gravedad entre ticks
            }
            else
            {
                StopRapel(playerid);
                SendClientMessage(playerid, 0x2ECC71FF, "Has llegado al suelo.");
            }
        }
    }
}

// Funcion para aplicar animacion de rapel de forma suave
forward ApplyRapelAnimation(playerid);
public ApplyRapelAnimation(playerid)
{
    if(!PlayerRapelData[playerid][isRapeling])
    {
        // Si ya no esta en rapel, matar el timer
        if(PlayerRapelData[playerid][animTimer] != -1)
        {
            KillTimer(PlayerRapelData[playerid][animTimer]);
            PlayerRapelData[playerid][animTimer] = -1;
        }
        return;
    }

    // Aplicar animacion de rapel con loop
    ApplyAnimation(playerid, "ped", "abseil", 4.0, 1, 0, 0, 0, 0, 1);
}

// Funcion para detener el rapel
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

    // Descongelar helicoptero si fue congelado
    if(PlayerRapelData[playerid][vehicleFrozen] && PlayerRapelData[playerid][rapelVehicle] != INVALID_VEHICLE_ID)
    {
        PlayerRapelData[playerid][vehicleFrozen] = false;
    }

    // Destruir todos los segmentos de cuerda del helicoptero
    for(new i = 0; i < PlayerRapelData[playerid][ropeSegments]; i++)
    {
        if(IsValidDynamicObject(PlayerRapelData[playerid][ropeObject][i]))
        {
            DestroyDynamicObject(PlayerRapelData[playerid][ropeObject][i]);
        }
        PlayerRapelData[playerid][ropeObject][i] = -1;
    }
    PlayerRapelData[playerid][ropeSegments] = 0;

    // Destruir todos los segmentos de cuerda estatica
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
    PlayerRapelData[playerid][isAscending] = false;
    ClearAnimations(playerid);
    TogglePlayerControllable(playerid, 1);
}

// Devuelve si algun jugador conectado ya ocupa ese asiento del vehiculo.
// No se puede usar IsPlayerInVehicle justo despues de PutPlayerInVehicle para
// esto: el estado del jugador que llama a PutPlayerInVehicle tarda ~1-2s en
// pasar a DRIVER/PASSENGER (animacion de entrar), asi que solo sirve para
// verificar a OTROS jugadores que ya estaban sentados de antes.
stock bool:IsVehicleSeatTaken(vehicleid, seat)
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(IsPlayerConnected(i) && GetPlayerVehicleID(i) == vehicleid && GetPlayerVehicleSeat(i) == seat)
        {
            return true;
        }
    }
    return false;
}

// Termina el ascenso: sienta al jugador de vuelta en el helicoptero
stock FinishAscend(playerid, vehicleid)
{
    new bool:seated = false;
    new seatToUse = -1;

    if(IsValidVehicle(vehicleid))
    {
        for(new seat = 1; seat < 4; seat++)
        {
            if(!IsVehicleSeatTaken(vehicleid, seat))
            {
                seatToUse = seat;
                break;
            }
        }
    }

    // IMPORTANTE: limpiar todo (timers, cuerda, animacion, control) ANTES de
    // llamar a PutPlayerInVehicle, nunca despues. StopRapel llama a
    // ClearAnimations, y si eso corre DESPUES de PutPlayerInVehicle corta la
    // animacion de "entrar al vehiculo" a mitad de camino: el cliente
    // revierte la asignacion y el jugador queda afuera (confirmado con logs:
    // GetPlayerVehicleID volvia a 0 dos segundos despues).
    StopRapel(playerid);

    if(seatToUse != -1)
    {
        PutPlayerInVehicle(playerid, vehicleid, seatToUse);
        seated = true;
    }

    if(seated)
    {
        SendClientMessage(playerid, 0x2ECC71FF, "Has subido al helicoptero.");
    }
    else
    {
        SendClientMessage(playerid, 0xE74C3CFF, "No hay asientos disponibles en el helicoptero.");
    }
}

// Funcion para cancelar rapel
CMD:pararrapel(playerid, params[])
{
    if(!PlayerRapelData[playerid][isRapeling])
    {
        SendClientMessage(playerid, 0xE74C3CFF, "No estas usando el rapel.");
        return 1;
    }

    StopRapel(playerid);
    SendClientMessage(playerid, 0xF1C40FFF, "Has cancelado el rapel.");
    return 1;
}

// Comandos
CMD:rapel(playerid, params[])
{
    // Verificar que sea el piloto de un helicoptero ANTES que nada, incluso
    // antes de validar los argumentos, para dar siempre el mensaje correcto.
    if(!IsPlayerHelicopterPilot(playerid))
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Debes subir al asiento del piloto de un helicoptero para usar /rapel.");
        return 1;
    }

    new option[32], targetid;

    if(sscanf(params, "s[32]D(-1)", option, targetid))
    {
        SendClientMessage(playerid, 0xF1C40FFF, "Uso: /rapel descender [id] o /rapel subir [id]");
        return 1;
    }

    if(!strcmp(option, "descender", true))
    {
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
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador no esta conectado.");
            return 1;
        }

        // Verificar que el objetivo este en el helicoptero
        new vehicleid = GetPlayerVehicleID(playerid);
        if(!IsPlayerInVehicle(targetid, vehicleid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador no esta en tu helicoptero.");
            return 1;
        }

        // Verificar que el objetivo no sea el piloto
        if(targetid == playerid)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El piloto no puede descender en rapel.");
            return 1;
        }

        // Verificar que el objetivo no este ya en rapel
        if(PlayerRapelData[targetid][isRapeling])
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador ya esta usando el rapel.");
            return 1;
        }

        // Verificar estabilidad del helicoptero
        if(!IsHelicopterStable(vehicleid))
        {
            new Float:speed = GetVehicleSpeed(vehicleid);
            SendClientMessage(playerid, 0xE74C3CFF, "El helicoptero esta moviendose demasiado rapido para usar el rapel.");
            new msg[128];
            format(msg, sizeof(msg), "Velocidad actual: %.1f km/h - Maximo permitido: %.1f km/h", speed, MAX_HELI_SPEED); // DEBUG
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
            SendClientMessage(playerid, 0xE74C3CFF, "El helicoptero esta demasiado alto para usar el rapel.");
            new msg[128];
            format(msg, sizeof(msg), "Altura actual: %.1f metros - Maximo: %.1f metros", height, MAX_RAPEL_HEIGHT_HELI);
            SendClientMessage(playerid, 0xF1C40FFF, msg);
            return 1;
        }

        if(height < MIN_RAPEL_HEIGHT)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El helicoptero esta demasiado bajo para usar el rapel.");
            return 1;
        }

        // Congelar helicoptero
        new Float:vx, Float:vy, Float:vz;
        GetVehiclePos(vehicleid, vx, vy, vz);

        PlayerRapelData[targetid][frozenVehiclePos][0] = vx;
        PlayerRapelData[targetid][frozenVehiclePos][1] = vy;
        PlayerRapelData[targetid][frozenVehiclePos][2] = vz;
        PlayerRapelData[targetid][vehicleFrozen] = true;

        // Remover jugador del helicoptero
        RemovePlayerFromVehicle(targetid);

        SetPlayerPos(targetid, vx, vy, vz - 2.0);
        SetPlayerVirtualWorld(targetid, GetPlayerVirtualWorld(targetid));
        SetPlayerInterior(targetid, GetPlayerInterior(targetid));

        // Crear segmentos de cuerda desde el helicoptero hasta el suelo
        CreateRopeSegments(targetid, vx, vy, vz - 1.0, groundZ, false);


        PlayerRapelData[targetid][isRapeling] = true;
        PlayerRapelData[targetid][isAscending] = false;
        PlayerRapelData[targetid][rapelVehicle] = vehicleid;
        PlayerRapelData[targetid][targetZ] = groundZ + 1.0;
        PlayerRapelData[targetid][ropeAnchorZ] = groundZ;
        PlayerRapelData[targetid][usingStaticRope] = false;

        TogglePlayerControllable(targetid, 0);

        // Aplicar animacion inicial
        ApplyAnimation(targetid, "ped", "abseil", 4.0, 1, 0, 0, 0, 0, 1);

        // Timer para actualizar posicion (cada 100ms)
        PlayerRapelData[targetid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", targetid);

        // Timer para mantener la animacion (cada 4 segundos)
        PlayerRapelData[targetid][animTimer] = SetTimerEx("ApplyRapelAnimation", 4000, true, "i", targetid);

        SendClientMessage(playerid, 0x2ECC71FF, "Iniciando descenso en rapel del jugador...");
        SendClientMessage(targetid, 0x2ECC71FF, "Comenzando descenso en rapel...");
        SendClientMessage(targetid, 0xF1C40FFF, "Usa /pararrapel para cancelar");
    }
    else if(!strcmp(option, "subir", true))
    {
        // Verificar que se haya especificado un ID
        if(targetid == -1)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Debes especificar el ID del jugador a subir.");
            SendClientMessage(playerid, 0xF1C40FFF, "Uso: /rapel subir [id]");
            return 1;
        }

        // Verificar que el jugador exista
        if(!IsPlayerConnected(targetid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador no esta conectado.");
            return 1;
        }

        // Verificar que el objetivo no sea el piloto
        if(targetid == playerid)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El piloto no puede subirse a si mismo en rapel.");
            return 1;
        }

        new vehicleid = GetPlayerVehicleID(playerid);

        // Verificar que el objetivo no este ya dentro del helicoptero
        if(IsPlayerInVehicle(targetid, vehicleid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador ya esta dentro de tu helicoptero.");
            return 1;
        }

        // Verificar estabilidad del helicoptero
        if(!IsHelicopterStable(vehicleid))
        {
            new Float:speed = GetVehicleSpeed(vehicleid);
            SendClientMessage(playerid, 0xE74C3CFF, "El helicoptero esta moviendose demasiado rapido para usar el rapel.");
            new msg[128];
            format(msg, sizeof(msg), "Velocidad actual: %.1f km/h - Maximo permitido: %.1f km/h", speed, MAX_HELI_SPEED);
            SendClientMessage(playerid, 0xF1C40FFF, msg);
            return 1;
        }

        new Float:vx, Float:vy, Float:vz;
        GetVehiclePos(vehicleid, vx, vy, vz);

        // Caso 1: el jugador ya esta bajando en rapel desde este mismo helicoptero -> se revierte y se le sube de vuelta
        if(PlayerRapelData[targetid][isRapeling] && !PlayerRapelData[targetid][usingStaticRope] && PlayerRapelData[targetid][rapelVehicle] == vehicleid)
        {
            new Float:tCurX, Float:tCurY, Float:tCurZ;
            GetPlayerPos(targetid, tCurX, tCurY, tCurZ);

            PlayerRapelData[targetid][isAscending] = true;
            PlayerRapelData[targetid][targetZ] = vz - 1.0;
            PlayerRapelData[targetid][ascendTicksLeft] = CalculateAscendTicks((vz - 1.0) - tCurZ);

            PlayerRapelData[targetid][frozenVehiclePos][0] = vx;
            PlayerRapelData[targetid][frozenVehiclePos][1] = vy;
            PlayerRapelData[targetid][frozenVehiclePos][2] = vz;
            PlayerRapelData[targetid][vehicleFrozen] = true;

            SendClientMessage(playerid, 0x2ECC71FF, "Subiendo al jugador de vuelta al helicoptero...");
            SendClientMessage(targetid, 0x2ECC71FF, "Te estan subiendo de vuelta al helicoptero...");
            return 1;
        }

        // El jugador esta usando rapel en otro lado (otro helicoptero o cuerda estatica)
        if(PlayerRapelData[targetid][isRapeling])
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador ya esta usando el rapel en otro lado.");
            return 1;
        }

        // Caso 2: el jugador esta en el suelo, cerca y debajo del helicoptero -> se le sube por primera vez
        if(IsPlayerInAnyVehicle(targetid))
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador esta dentro de un vehiculo.");
            return 1;
        }

        new Float:tx, Float:ty, Float:tz;
        GetPlayerPos(targetid, tx, ty, tz);

        new Float:horizontalDist = floatsqroot((vx - tx) * (vx - tx) + (vy - ty) * (vy - ty));
        if(horizontalDist > MAX_RAPEL_PICKUP_RADIUS)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "Ese jugador esta demasiado lejos del helicoptero.");
            return 1;
        }

        new Float:height = vz - tz;

        if(height > MAX_RAPEL_HEIGHT_HELI)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El helicoptero esta demasiado alto para recoger a ese jugador.");
            return 1;
        }

        if(height < MIN_RAPEL_HEIGHT)
        {
            SendClientMessage(playerid, 0xE74C3CFF, "El helicoptero esta demasiado bajo para usar el rapel.");
            return 1;
        }

        PlayerRapelData[targetid][frozenVehiclePos][0] = vx;
        PlayerRapelData[targetid][frozenVehiclePos][1] = vy;
        PlayerRapelData[targetid][frozenVehiclePos][2] = vz;
        PlayerRapelData[targetid][vehicleFrozen] = true;

        // Crear segmentos de cuerda desde el helicoptero hasta el jugador
        CreateRopeSegments(targetid, vx, vy, vz - 1.0, tz, false);

        PlayerRapelData[targetid][isRapeling] = true;
        PlayerRapelData[targetid][isAscending] = true;
        PlayerRapelData[targetid][rapelVehicle] = vehicleid;
        PlayerRapelData[targetid][targetZ] = vz - 1.0;
        PlayerRapelData[targetid][ropeAnchorZ] = tz;
        PlayerRapelData[targetid][ascendTicksLeft] = CalculateAscendTicks((vz - 1.0) - tz);
        PlayerRapelData[targetid][usingStaticRope] = false;

        TogglePlayerControllable(targetid, 0);

        // Aplicar animacion inicial
        ApplyAnimation(targetid, "ped", "abseil", 4.0, 1, 0, 0, 0, 0, 1);

        // Timer para actualizar posicion (cada 100ms)
        PlayerRapelData[targetid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", targetid);

        // Timer para mantener la animacion (cada 4 segundos)
        PlayerRapelData[targetid][animTimer] = SetTimerEx("ApplyRapelAnimation", 4000, true, "i", targetid);

        SendClientMessage(playerid, 0x2ECC71FF, "Subiendo al jugador hacia el helicoptero...");
        SendClientMessage(targetid, 0x2ECC71FF, "Te estan subiendo hacia el helicoptero...");
        SendClientMessage(targetid, 0xF1C40FFF, "Usa /pararrapel para cancelar");
    }
    else
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Opcion invalida. Usa: descender o subir");
    }

    return 1;
}

CMD:cuerda(playerid, params[])
{
    if(PlayerRapelData[playerid][isRapeling])
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Ya estas usando una cuerda.");
        return 1;
    }

    if(IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, 0xE74C3CFF, "No puedes usar una cuerda dentro de un vehiculo.");
        return 1;
    }

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    new Float:groundZ = GetGroundZ(x, y, z);
    new Float:height = z - groundZ;

    if(height < MIN_RAPEL_HEIGHT)
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Estas demasiado cerca del suelo para usar una cuerda.");
        return 1;
    }

    if(height > MAX_RAPEL_HEIGHT_ROPE)
    {
        SendClientMessage(playerid, 0xE74C3CFF, "Estas demasiado alto para usar una cuerda.");
        new msg[128];
        format(msg, sizeof(msg), "Altura actual: %.1f metros - Maximo: %.1f metros", height, MAX_RAPEL_HEIGHT_ROPE);
        SendClientMessage(playerid, 0xF1C40FFF, msg);
        return 1;
    }

    // Crear cuerda estatica con multiples segmentos
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

    // Aplicar animacion inicial
    ApplyAnimation(playerid, "ped", "abseil", 4.0, 1, 0, 0, 0, 0, 1);

    // Timer para actualizar posicion (cada 100ms)
    PlayerRapelData[playerid][rapelTimer] = SetTimerEx("RapelUpdate", 100, true, "i", playerid);

    // Timer para mantener la animacion (cada 4 segundos)
    PlayerRapelData[playerid][animTimer] = SetTimerEx("ApplyRapelAnimation", 4000, true, "i", playerid);

    SendClientMessage(playerid, 0x2ECC71FF, "Descendiendo por la cuerda...");
    SendClientMessage(playerid, 0xF1C40FFF, "Usa /pararrapel para cancelar");
    return 1;
}

// Inicializacion
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
        PlayerRapelData[i][isAscending] = false;
        PlayerRapelData[i][vehicleFrozen] = false;
    }
}

hook OnPlayerDisconnect(playerid, reason)
{
    if(PlayerRapelData[playerid][isRapeling])
    {
        StopRapel(playerid);
    }
    return 1;
}
