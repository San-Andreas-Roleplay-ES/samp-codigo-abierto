#include <YSI_Coding\y_hooks>

#define _qfa_included

// Aca se definimos la estructura para guardar datos del último asesino en un ENUM.
enum E_KILLER_DATA
{
    KillerName[MAX_PLAYER_NAME],
    KillerHour,
    KillerMinute,
    KillerWeapon
}

// Arreglo que guarda la información por jugador
new PlayerKillerData[MAX_PLAYERS][E_KILLER_DATA];

// Registrar el asesino cuando el jugador muere
hook OnPlayerDeath(playerid, killerid, reason)
{
    if (killerid != INVALID_PLAYER_ID)
    {
        new hour, minute, second;
        gettime(hour, minute, second);

        GetPlayerName(killerid, PlayerKillerData[playerid][KillerName], MAX_PLAYER_NAME);
        PlayerKillerData[playerid][KillerHour] = hour;
        PlayerKillerData[playerid][KillerMinute] = minute;

        //Se guarda el arma como ID 
        PlayerKillerData[playerid][KillerWeapon] = reason;

    }
    else
    {
        PlayerKillerData[playerid][KillerName][0] = '\0'; // cadena vacía
    }
    return 1;
}

// Limpiar datos al conectar
hook OnPlayerConnect(playerid)
{
    PlayerKillerData[playerid][KillerName][0] = '\0';
    PlayerKillerData[playerid][KillerHour] = 0;
    PlayerKillerData[playerid][KillerMinute] = 0;
    PlayerKillerData[playerid][KillerWeapon] = 0;
    return 1;
}

// Comando /qfa
CMD:qfa(playerid, params[])
{
    new targetid;
    if (sscanf(params, "d", targetid))
        targetid = playerid;

    if (targetid != playerid && !IsPlayerAdmin(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No tienes permiso para ver el QFA de otro jugador.");

    if (!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_FADE3, "Ese jugador no está conectado.");

    if (isnull(PlayerKillerData[targetid][KillerName]))
        return SendClientMessage(playerid, COLOR_FADE3, "Este jugador no ha sido asesinado recientemente.");

    new arma[32];

    //se extra e id del arma y se pasa a String en la variable arma.
    GetWeaponName(PlayerKillerData[targetid][KillerWeapon], arma, sizeof(arma));

    new msg[144];
    format(msg, sizeof(msg), "Último asesino: %s | Hora: %02d:%02d | Arma: %s",
        PlayerKillerData[targetid][KillerName],
        PlayerKillerData[targetid][KillerHour],
        PlayerKillerData[targetid][KillerMinute],
        arma);

    SendClientMessage(playerid, COLOR_GRAD2, msg);
    return 1;
}