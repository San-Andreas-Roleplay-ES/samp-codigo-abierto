#define _qfa_included

// Variables locales por jugador
new LastKiller[MAX_PLAYERS];
new LastKillerHour[MAX_PLAYERS];
new LastKillerMinute[MAX_PLAYERS];
new LastKillerWeapon[MAX_PLAYERS];

// Registrar el asesino cuando el jugador muere
hook OnPlayerDeath(playerid, killerid, reason)
{
    if (killerid != INVALID_PLAYER_ID)
    {
        LastKiller[playerid] = killerid;

        new hour, minute, second;
        gettime(hour, minute, second);

        LastKillerHour[playerid] = hour;
        LastKillerMinute[playerid] = minute;
        LastKillerWeapon[playerid] = reason;
    }
    else
    {
        LastKiller[playerid] = INVALID_PLAYER_ID;
    }
    return 1;
}

// Limpiar datos al conectar
hook OnPlayerConnect(playerid)
{
    LastKiller[playerid] = INVALID_PLAYER_ID;
    LastKillerHour[playerid] = 0;
    LastKillerMinute[playerid] = 0;
    LastKillerWeapon[playerid] = 0;
    return 1;
}


CMD:qfa(playerid, params[])
{
    new targetid;
    if (sscanf(params, "d", targetid))
        targetid = playerid;

    if (targetid != playerid && !IsPlayerAdmin(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No tienes permiso para ver el QFA de otro jugador.");

    if (!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_FADE3, "Ese jugador no está conectado.");

    if (LastKiller[targetid] == INVALID_PLAYER_ID)
        return SendClientMessage(playerid, COLOR_FADE3, "Este jugador no ha sido asesinado recientemente.");

    new nameKiller[MAX_PLAYER_NAME];
    GetPlayerName(LastKiller[targetid], nameKiller, sizeof(nameKiller));

    new arma[32];
    GetWeaponName(LastKillerWeapon[targetid], arma, sizeof(arma));

    new msg[144];
    format(msg, sizeof(msg), "Último asesino: %s | Hora: %02d:%02d | Arma: %s",
        nameKiller, LastKillerHour[targetid], LastKillerMinute[targetid], arma);

    SendClientMessage(playerid, COLOR_GRAD2, msg);
    return 1;
}