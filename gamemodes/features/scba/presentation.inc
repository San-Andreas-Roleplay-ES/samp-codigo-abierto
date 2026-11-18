// Already formatted lines must bypass the gamemode's variadic chat wrapper.
#if !defined ScbaSendChatText
    native ScbaSendChatText(playerid, colour, const text[]) = SendClientMessage;
#endif

stock ScbaBroadcast(playerid, const environment[], soundid = 0, const clip[] = "", bool:alarm = false)
{
    new Float:x, Float:y, Float:z, message[144];
    new world = GetPlayerVirtualWorld(playerid), interior = GetPlayerInterior(playerid);
    GetPlayerPos(playerid, x, y, z);
    if (environment[0]) format(message, sizeof(message), "* %s", environment);
    new url[220], now = GetTickCount();
    if (clip[0] && ScbaAudioBaseUrl[0]) format(url, sizeof(url), "%s/%s", ScbaAudioBaseUrl, clip);
    for (new listener = 0; listener < MAX_PLAYERS; listener++)
    {
        if (!IsPlayerConnected(listener) || IsPlayerNPC(listener)) continue;
        if (GetPlayerVirtualWorld(listener) != world || GetPlayerInterior(listener) != interior) continue;
        if (!IsPlayerInRangeOfPoint(listener, SCBA_EVENT_RADIUS, x, y, z)) continue;
        if (message[0]) ScbaSendChatText(listener, SCBA_COLOUR_ROLEPLAY, message);
        if (url[0])
        {
            // A reserve alarm can interrupt a fitting sound on the single client stream.
            if (ScbaPlayer[listener][ScbaHasStreamTime]
                && (!alarm || ScbaPlayer[listener][ScbaStreamIsAlarm])
                && ScbaElapsed(now, ScbaPlayer[listener][ScbaLastStreamAt]) < SCBA_AUDIO_CLIP_WINDOW_MS) continue;
            if (PlayAudioStreamForPlayer(listener, url, x, y, z, SCBA_EVENT_RADIUS, 1))
            {
                ScbaPlayer[listener][ScbaHasStreamTime] = true;
                ScbaPlayer[listener][ScbaStreamIsAlarm] = alarm;
                ScbaPlayer[listener][ScbaLastStreamAt] = now;
            }
        }
        else if (soundid > 0) PlayerPlaySound(listener, soundid, x, y, z);
    }
    return 1;
}

stock ScbaSendMe(playerid, const action[])
{
    new name[MAX_PLAYER_NAME], message[144];
    GetPlayerRoleplayName(playerid, name, sizeof(name));
    format(message, sizeof(message), "%s %s", name, action);
    return ScbaBroadcast(playerid, message);
}

stock E_SCBA_GAUGE_STATUS:ScbaCurrentGaugeStatus(playerid)
{
    if (ScbaPlayer[playerid][ScbaSupplySource] != SCBA_SOURCE_NONE) return SCBA_STATUS_FILLING;
    if (!ScbaPlayer[playerid][ScbaMaskOn]) return SCBA_STATUS_DISCONNECTED;
    switch (ScbaAirBand(playerid))
    {
        case SCBA_AIR_LOW: return SCBA_STATUS_LOW;
        case SCBA_AIR_CRITICAL: return SCBA_STATUS_CRITICAL;
        case SCBA_AIR_EMPTY: return SCBA_STATUS_EMPTY;
    }
    return SCBA_STATUS_NORMAL;
}

stock ScbaStatusTexture(E_SCBA_GAUGE_STATUS:status, texture[], size)
{
    return format(texture, size, "mdl%d:%s", ScbaModels[SCBA_MASK][ScbaCustomModel], ScbaStatusArt[status][ScbaStatusSprite]);
}

stock PlayerText:ScbaHudSprite(playerid, const texture[], Float:x, Float:y, Float:width, Float:height, colour = 0xFFFFFFFF)
{
    new PlayerText:draw = CreatePlayerTextDraw(playerid, x, y, texture);
    if (draw == PlayerText:INVALID_TEXT_DRAW) return draw;
    PlayerTextDrawFont(playerid, draw, 4);
    PlayerTextDrawTextSize(playerid, draw, width, height);
    PlayerTextDrawColor(playerid, draw, colour);
    PlayerTextDrawShow(playerid, draw);
    return draw;
}

stock PlayerText:ScbaHudLabel(playerid, Float:x, Float:y, const text[], colour, Float:scale, font = 1)
{
    new PlayerText:draw = CreatePlayerTextDraw(playerid, SCBA_GAUGE_TEXT_X(x), SCBA_GAUGE_TEXT_Y(y), text);
    if (draw == PlayerText:INVALID_TEXT_DRAW) return draw;
    PlayerTextDrawFont(playerid, draw, font);
    PlayerTextDrawLetterSize(playerid, draw, SCBA_GAUGE_FONT_WIDTH * scale, SCBA_GAUGE_FONT_HEIGHT * scale);
    PlayerTextDrawAlignment(playerid, draw, 2);
    PlayerTextDrawColor(playerid, draw, colour);
    PlayerTextDrawBackgroundColor(playerid, draw, SCBA_GAUGE_INK);
    PlayerTextDrawSetOutline(playerid, draw, 1);
    PlayerTextDrawSetShadow(playerid, draw, 0);
    PlayerTextDrawSetProportional(playerid, draw, 1);
    PlayerTextDrawShow(playerid, draw);
    return draw;
}

stock ScbaHideGauge(playerid)
{
    for (new E_SCBA_GAUGE_LAYER:layer; layer < E_SCBA_GAUGE_LAYER; layer++)
    {
        if (ScbaGauge[playerid][layer] != PlayerText:INVALID_TEXT_DRAW)
            PlayerTextDrawDestroy(playerid, ScbaGauge[playerid][layer]);
        ScbaGauge[playerid][layer] = PlayerText:INVALID_TEXT_DRAW;
    }
    ScbaGaugeVisible[playerid] = false;
    ScbaPlayer[playerid][ScbaNeedleFrame] = SCBA_EMPTY_SLOT;
    return 1;
}

stock ScbaNeedleTexture(frame, texture[], size)
{
    return format(texture, size, "mdl%d:air_n%02d", ScbaModels[SCBA_MASK][ScbaCustomModel], frame);
}

stock bool:ScbaUpdateGauge(playerid, now)
{
    if (!ScbaGaugeVisible[playerid]) return false;
    new frame = ScbaNeedleStep(playerid), text[32];
    if (frame != ScbaPlayer[playerid][ScbaNeedleFrame])
    {
        ScbaNeedleTexture(frame, text, sizeof(text));
        PlayerTextDrawSetString(playerid, ScbaGauge[playerid][SCBA_DIAL_NEEDLE], text);
        PlayerTextDrawShow(playerid, ScbaGauge[playerid][SCBA_DIAL_NEEDLE]);
        ScbaPlayer[playerid][ScbaNeedleFrame] = frame;
    }
    new seconds = ScbaSecondsRemaining(playerid);
    format(text, sizeof(text), "%d%%", ScbaAirPercent(playerid));
    PlayerTextDrawSetString(playerid, ScbaGauge[playerid][SCBA_AIR_READING], text);
    format(text, sizeof(text), "%02d:%02d", seconds / 60, seconds % 60);
    PlayerTextDrawSetString(playerid, ScbaGauge[playerid][SCBA_TIME_READING], text);
    new E_SCBA_GAUGE_STATUS:status = ScbaCurrentGaugeStatus(playerid);
    if (status != ScbaPlayer[playerid][ScbaGaugeStatus])
    {
        ScbaStatusTexture(status, text, sizeof(text));
        PlayerTextDrawSetString(playerid, ScbaGauge[playerid][SCBA_BAND_READING], text);
        PlayerTextDrawColor(playerid, ScbaGauge[playerid][SCBA_BAND_READING], ScbaStatusArt[status][ScbaStatusColour]);
        PlayerTextDrawShow(playerid, ScbaGauge[playerid][SCBA_BAND_READING]);
        ScbaPlayer[playerid][ScbaGaugeStatus] = status;
    }
    ScbaPlayer[playerid][ScbaLastHudUpdate] = now;
    return true;
}

stock bool:ScbaShowGauge(playerid, now)
{
    ScbaHideGauge(playerid);
    new texture[32], frame = ScbaNeedleStep(playerid);
    format(texture, sizeof(texture), "mdl%d:air_dial", ScbaModels[SCBA_MASK][ScbaCustomModel]);
    ScbaGauge[playerid][SCBA_DIAL_FACE] = ScbaHudSprite(playerid, texture,
        SCBA_GAUGE_LEFT, SCBA_GAUGE_TOP, SCBA_GAUGE_WIDTH, SCBA_GAUGE_HEIGHT);
    ScbaNeedleTexture(frame, texture, sizeof(texture));
    ScbaGauge[playerid][SCBA_DIAL_NEEDLE] = ScbaHudSprite(playerid, texture,
        SCBA_GAUGE_TEXT_X(SCBA_GAUGE_DIAL_X - SCBA_GAUGE_DIAL_SIZE / 2.0),
        SCBA_GAUGE_TEXT_Y(SCBA_GAUGE_DIAL_Y - SCBA_GAUGE_DIAL_SIZE / 2.0),
        SCBA_GAUGE_WIDTH * SCBA_GAUGE_DIAL_SIZE / SCBA_GAUGE_ART_SIZE,
        SCBA_GAUGE_HEIGHT * SCBA_GAUGE_DIAL_SIZE / SCBA_GAUGE_ART_SIZE);
    ScbaPlayer[playerid][ScbaNeedleFrame] = frame;
    ScbaGauge[playerid][SCBA_AIR_CAPTION] = ScbaHudLabel(playerid, 79.0, 173.0, "AIRE", SCBA_COLOUR_MUTED, 0.67);
    ScbaGauge[playerid][SCBA_TIME_CAPTION] = ScbaHudLabel(playerid, 177.0, 173.0, "TIEMPO", SCBA_COLOUR_MUTED, 0.67);
    ScbaGauge[playerid][SCBA_AIR_READING] = ScbaHudLabel(playerid, 79.0, 186.0, "_", SCBA_COLOUR_TEXT, 1.0, SCBA_GAUGE_DIGITS);
    ScbaGauge[playerid][SCBA_TIME_READING] = ScbaHudLabel(playerid, 177.0, 186.0, "_", SCBA_COLOUR_TEXT, 1.0, SCBA_GAUGE_DIGITS);
    new E_SCBA_GAUGE_STATUS:status = ScbaCurrentGaugeStatus(playerid);
    ScbaStatusTexture(status, texture, sizeof(texture));
    ScbaGauge[playerid][SCBA_BAND_READING] = ScbaHudSprite(playerid, texture,
        SCBA_GAUGE_TEXT_X(SCBA_GAUGE_STATUS_LEFT), SCBA_GAUGE_TEXT_Y(SCBA_GAUGE_STATUS_TOP),
        SCBA_GAUGE_WIDTH * SCBA_GAUGE_STATUS_WIDTH / SCBA_GAUGE_ART_SIZE,
        SCBA_GAUGE_HEIGHT * SCBA_GAUGE_STATUS_HEIGHT / SCBA_GAUGE_ART_SIZE, ScbaStatusArt[status][ScbaStatusColour]);
    for (new E_SCBA_GAUGE_LAYER:layer; layer < E_SCBA_GAUGE_LAYER; layer++)
        if (ScbaGauge[playerid][layer] == PlayerText:INVALID_TEXT_DRAW)
        {
            ScbaHideGauge(playerid);
            return false;
        }
    ScbaPlayer[playerid][ScbaGaugeStatus] = status;
    ScbaGaugeVisible[playerid] = true;
    return ScbaUpdateGauge(playerid, now);
}

stock ScbaNotifyReserve(playerid, now)
{
    new E_SCBA_AIR_BAND:band = ScbaAirBand(playerid);
    if (band > ScbaPlayer[playerid][ScbaReportedBand])
    {
        if (band == SCBA_AIR_LOW)
            ScbaBroadcast(playerid, "Se escucha la alarma de reserva de un equipo de respiración.");
        else if (band == SCBA_AIR_CRITICAL)
            ScbaBroadcast(playerid, "Se escucha con insistencia la alarma de reserva de un equipo de respiración.");
        ScbaPlayer[playerid][ScbaReportedBand] = band;
    }
    if (band != SCBA_AIR_LOW && band != SCBA_AIR_CRITICAL) return 1;
    if (!ScbaPlayer[playerid][ScbaHasAlarmTime]
        || ScbaElapsed(now, ScbaPlayer[playerid][ScbaLastAlarmAt]) >= SCBA_ALARM_INTERVAL_MS)
    {
        if (band == SCBA_AIR_LOW)
            ScbaBroadcast(playerid, "", SCBA_FALLBACK_RESERVE_SOUND, SCBA_RESERVE_CLIP, true);
        else
            ScbaBroadcast(playerid, "", SCBA_FALLBACK_CRITICAL_SOUND, SCBA_CRITICAL_CLIP, true);
        ScbaPlayer[playerid][ScbaLastAlarmAt] = now;
        ScbaPlayer[playerid][ScbaHasAlarmTime] = true;
    }
    return 1;
}
