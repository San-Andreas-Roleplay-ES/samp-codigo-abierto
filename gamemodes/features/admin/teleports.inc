CMD:cj(playerid, params[])
{
    SetPlayerPos(playerid, 2490.30, -1666.67, 13.34);
    return 1;
}

CMD:idlewood(playerid, params[])
{
    SetPlayerPos(playerid, 1953.74, -1759.88, 13.54);
    return 1;
}

CMD:ls(playerid, params[])
{
    SetPlayerPos(playerid, 1538.16, -1675.73, 13.54);
    return 1;
}

CMD:lv(playerid, params[])
{
    SetPlayerPos(playerid, 2169.56, 1676.70, 10.82);
    return 1;
}

CMD:area51(playerid, params[])
{

    SetPlayerPos(playerid, 96.47, 1949.38, 17.64);
    return 1;
}

alias:ap("angelpine")
CMD:ap(playerid, params[])
{
    SetPlayerPos(playerid, -2039.40, -2382.60, 30.62);
    return 1;
}

// Development shortcut to the shooting-practice test location.
CMD:irpruebas(playerid, params[])
{
    if (!IsPlayerAdmin(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "Necesitas ser administrador RCON para ir a la zona de pruebas.");
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
        return SendClientMessage(playerid, COLOR_FADE3, "Debes estar vivo y a pie para ir a la zona de pruebas.");

    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerPos(playerid, 802.8580, 1674.5205, 5.2813);
    SetPlayerFacingAngle(playerid, 181.2324);
    SetCameraBehindPlayer(playerid);
    SendClientMessage(playerid, COLOR_LIGHTGREEN, "Llegaste a la zona de pruebas del campo de tiro.");
    return 1;
}
