stock bool:ScbaParseAction(const params[], action[], size)
{
    new position = 0, length = 0;
    while (params[position] == ' ' || params[position] == '\t') position++;
    while (params[position] && params[position] != ' ' && params[position] != '\t')
    {
        if (length >= size - 1) return false;
        action[length++] = params[position++];
    }
    action[length] = EOS;
    while (params[position] == ' ' || params[position] == '\t') position++;
    return params[position] == EOS;
}

stock ScbaShowHelp(playerid)
{
    return SendClientMessage(playerid, SCBA_COLOUR_MUTED,
        "Uso: /scba equipar, /scba mascara, /scba recargar, /scba cancelar o /scba guardar.");
}

stock ScbaHandleCommand(playerid, const params[])
{
    if (!ScbaHostCanUse(playerid))
        return SendClientMessage(playerid, SCBA_COLOUR_MUTED, "No tienes autorización para utilizar el equipo de respiración.");
    new action[SCBA_ACTION_LENGTH];
    if (!ScbaParseAction(params, action, sizeof(action)) || !action[0])
        return ScbaShowHelp(playerid);
    ScbaTickPlayer(playerid, GetTickCount());
    if (!ScbaPlayerAlive(playerid))
        return SendClientMessage(playerid, SCBA_COLOUR_MUTED, "Debes estar vivo para utilizar el equipo.");
    if (!strcmp(action, "cancelar", true))
    {
        if (ScbaPlayer[playerid][ScbaSupplySource] == SCBA_SOURCE_NONE)
            return SendClientMessage(playerid, SCBA_COLOUR_MUTED, "No hay una recarga en curso.");
        return ScbaCancelSupply(playerid, true);
    }
    if (ScbaPlayer[playerid][ScbaSupplySource] != SCBA_SOURCE_NONE)
        return SendClientMessage(playerid, SCBA_COLOUR_MUTED, "Recarga en curso. Usa /scba cancelar para interrumpirla.");
    if (!strcmp(action, "equipar", true)) ScbaEquip(playerid);
    else if (!strcmp(action, "mascara", true)) ScbaToggleMask(playerid, GetTickCount());
    else if (!strcmp(action, "recargar", true)) ScbaBeginSupply(playerid, GetTickCount());
    else if (!strcmp(action, "guardar", true)) ScbaStore(playerid);
    else ScbaShowHelp(playerid);
    return 1;
}

CMD:scba(playerid, const params[])
{
    return ScbaHandleCommand(playerid, params);
}
