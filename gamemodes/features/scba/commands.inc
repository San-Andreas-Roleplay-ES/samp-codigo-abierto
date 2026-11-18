stock bool:ScbaReadWord(const params[], &position, action[], size)
{
    new length = 0;
    while (params[position] == ' ' || params[position] == '\t') position++;
    while (params[position] && params[position] != ' ' && params[position] != '\t')
    {
        if (length >= size - 1) return false;
        action[length++] = params[position++];
    }
    action[length] = EOS;
    while (params[position] == ' ' || params[position] == '\t') position++;
    return true;
}

stock ScbaShowHelp(playerid)
{
    return SendClientMessage(playerid, SCBA_COLOUR_MUTED,
        "Uso: /scba equipar, /scba mascara, /scba aire, /scba ajustar, /scba recargar, /scba cancelar o /scba guardar.");
}

stock ScbaHandleCommand(playerid, const params[])
{
    if (!ScbaHostCanUse(playerid))
        return SendClientMessage(playerid, SCBA_COLOUR_MUTED, "No tienes autorización para utilizar el equipo de respiración.");
    new action[SCBA_ACTION_LENGTH], position;
    if (!ScbaReadWord(params, position, action, sizeof(action)) || !action[0]) return ScbaShowHelp(playerid);
    new bool:adjusting = !strcmp(action, "ajustar", true) || !strcmp(action, "ajusta", true);
    new E_SCBA_PART:part = SCBA_PART_COUNT;
    if (adjusting)
    {
        new piece[SCBA_ACTION_LENGTH];
        if (ScbaReadWord(params, position, piece, sizeof(piece)) && !params[position] && piece[0])
        {
            if (!strcmp(piece, "equipo", true)) part = SCBA_BACKPACK;
            else if (!strcmp(piece, "mascara", true)) part = SCBA_MASK;
        }
    }
    else if (params[position]) return ScbaShowHelp(playerid);
    new now = GetTickCount();
    ScbaTickPlayer(playerid, now);
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
    if (adjusting) ScbaBeginFitEdit(playerid, part);
    else if (!strcmp(action, "equipar", true)) ScbaEquip(playerid);
    else if (!strcmp(action, "mascara", true)) ScbaToggleMask(playerid, now);
    else if (!strcmp(action, "aire", true)) ScbaToggleAir(playerid, now);
    else if (!strcmp(action, "recargar", true)) ScbaBeginSupply(playerid, now);
    else if (!strcmp(action, "guardar", true)) ScbaStore(playerid);
    else ScbaShowHelp(playerid);
    return 1;
}

CMD:scba(playerid, const params[])
{
    return ScbaHandleCommand(playerid, params);
}
