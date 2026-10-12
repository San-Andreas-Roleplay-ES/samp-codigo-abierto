#if defined SP_PRESENTATION_INCLUDED
    #endinput
#endif
#define SP_PRESENTATION_INCLUDED

#define PRACTICE_CUE_RADIUS (10.0)
#define PRACTICE_SOUND_COUNTDOWN (1056)
#define PRACTICE_SOUND_TARGET (1057)
#define PRACTICE_SOUND_END (1139)
// Object materials use ARGB, unlike chat's RGBA colours.
#define PRACTICE_SHOOT_MATERIAL (0xFFB83D3D)
#define PRACTICE_NO_SHOOT_MATERIAL (0xFF2465A8)

// Result lines are already formatted. Send them directly so a literal percent
// sign is never interpreted again by the gamemode's variadic chat wrapper.
#if !defined PracticeSendResultLine
    native PracticeSendResultLine(playerid, colour, const message[]) = SendClientMessage;
#endif

new PlayerText:PracticeCountdownText[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};

stock PracticeHideCountdown(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 1;
    if (PracticeCountdownText[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        if (IsPlayerConnected(playerid)) PlayerTextDrawDestroy(playerid, PracticeCountdownText[playerid]);
        PracticeCountdownText[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
    return 1;
}

stock bool:PracticeCanObserve(zone, playerid)
{
    return IsPlayerConnected(playerid) && !IsPlayerNPC(playerid)
        && GetPlayerVirtualWorld(playerid) == PracticeSessionOrigin[zone][PracticePointWorld]
        && GetPlayerInterior(playerid) == PracticeSessionOrigin[zone][PracticePointInterior]
        && IsPlayerInRangeOfPoint(playerid, PRACTICE_CUE_RADIUS, PracticeSessionOrigin[zone][PracticePointX],
            PracticeSessionOrigin[zone][PracticePointY], PracticeSessionOrigin[zone][PracticePointZ]);
}

stock PracticePlayCue(zone, soundid)
{
    // These are built-in frontend tones. Filter recipients explicitly; do not
    // assume 3D attenuation or that a virtual world filters sound automatically.
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (PracticeCanObserve(zone, playerid)) PlayerPlaySound(playerid, soundid, 0.0, 0.0, 0.0);
    return 1;
}

stock PracticeAmbientMessage(zone, const message[])
{
    // Anchor environmental messages to the station even if the participant has
    // disconnected, walked away or changed dimension. No player is impersonated.
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (PracticeCanObserve(zone, playerid)) SendClientMessage(playerid, COLOR_PURPLE, message);
    return 1;
}

stock PracticePresentCountdown(zone)
{
    new playerid = PracticeSession[zone][PracticePlayer], text[32];
    if (IsPlayerConnected(playerid))
    {
        // Plain GXT-safe text; only the participant sees this small, owned overlay.
        format(text, sizeof(text), "Comienza en %d", PracticeSession[zone][PracticeLastCountdown]);
        if (PracticeCountdownText[playerid] == PlayerText:INVALID_TEXT_DRAW)
        {
            PracticeCountdownText[playerid] = CreatePlayerTextDraw(playerid, 320.0, 360.0, text);
            if (PracticeCountdownText[playerid] != PlayerText:INVALID_TEXT_DRAW)
            {
                PlayerTextDrawAlignment(playerid, PracticeCountdownText[playerid], 2);
                PlayerTextDrawLetterSize(playerid, PracticeCountdownText[playerid], 0.25, 1.2);
                PlayerTextDrawFont(playerid, PracticeCountdownText[playerid], 1);
                PlayerTextDrawColor(playerid, PracticeCountdownText[playerid], 0xFFFFFFDD);
                PlayerTextDrawSetOutline(playerid, PracticeCountdownText[playerid], 1);
                PlayerTextDrawShow(playerid, PracticeCountdownText[playerid]);
            }
        }
        else PlayerTextDrawSetString(playerid, PracticeCountdownText[playerid], text);
    }
    return PracticePlayCue(zone, PRACTICE_SOUND_COUNTDOWN);
}

stock PracticePresentStart(zone, playerid)
{
    new name[MAX_PLAYER_NAME], message[144];
    GetPlayerRoleplayName(playerid, name, sizeof(name));
    format(message, sizeof(message), "* %s se prepara para comenzar la práctica de tiro.", name);
    PracticeAmbientMessage(zone, message);
    return PracticePresentCountdown(zone);
}

stock PracticePresentTarget(zone)
{
    return PracticePlayCue(zone, PRACTICE_SOUND_TARGET);
}

stock PracticePresentEnd(zone, bool:completed, playerid, bool:abandoned)
{
    PracticePlayCue(zone, PRACTICE_SOUND_END);
    if (abandoned)
    {
        new name[MAX_PLAYER_NAME], message[144];
        GetPlayerRoleplayName(playerid, name, sizeof(name));
        format(message, sizeof(message), "* %s abandona el puesto de tiro.", name);
        PracticeAmbientMessage(zone, message);
    }
    else if (completed)
        PracticeAmbientMessage(zone, "* La señal del puesto indica el final del ejercicio. (( Puesto de tiro ))");
    else
        PracticeAmbientMessage(zone, "* La señal del puesto indica que el ejercicio se ha interrumpido. (( Puesto de tiro ))");
    return 1;
}

stock PracticeApplyTargetAppearance(STREAMER_TAG_OBJECT:objectid, bool:noShoot, modelid = PRACTICE_TARGET_MODEL)
{
    // Preserve the native figures. Only the legacy marked box gets a text material.
    if (modelid != PRACTICE_TARGET_MODEL) return 1;
    return SetDynamicObjectMaterialText(objectid, 0, noShoot ? ("NO DISPARAR") : ("DISPARAR"),
        OBJECT_MATERIAL_SIZE_512x256, "Arial", 32, 1, 0xFFFFFFFF,
        noShoot ? PRACTICE_NO_SHOOT_MATERIAL : PRACTICE_SHOOT_MATERIAL, OBJECT_MATERIAL_TEXT_ALIGN_CENTER);
}

stock PracticeReportResult(playerid, bool:completed, const result[E_PRACTICE_SESSION], const reason[])
{
    new message[144];
    PracticeSendResultLine(playerid, PRACTICE_SECTION_COLOUR, PRACTICE_RESULT_BORDER);
    PracticeSendResultLine(playerid, PRACTICE_SECTION_COLOUR, "CAMPO DE TIRO");
    if (!completed)
    {
        format(message, sizeof(message), "Incompleta: %s", reason);
        PracticeSendResultLine(playerid, COLOR_WHITE, message);
        return PracticeSendResultLine(playerid, PRACTICE_SECTION_COLOUR, PRACTICE_RESULT_BORDER);
    }
    format(message, sizeof(message), "Puntos mínimos: %d%sErrores máximos: %d",
        result[PracticeRequiredPoints], PRACTICE_CHAT_SEPARATOR, result[PracticeAllowedErrors]);
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    format(message, sizeof(message), "Aciertos: %d/%d%sSin impactar a tiempo: %d",
        result[PracticeHits], result[PracticeShootTotal], PRACTICE_CHAT_SEPARATOR, result[PracticeExpired]);
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    format(message, sizeof(message), "Protegidos respetados: %d/%d%sErrores de identificación: %d",
        result[PracticeCorrectIgnores], result[PracticeNoShootTotal], PRACTICE_CHAT_SEPARATOR, result[PracticeIdentificationErrors]);
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    if (result[PracticeShots])
        format(message, sizeof(message), "Disparos: %d%sPrecisión: %.1f%c",
            result[PracticeShots], PRACTICE_CHAT_SEPARATOR, 100.0 * float(result[PracticeHits]) / float(result[PracticeShots]), '%');
    else
        format(message, sizeof(message), "Disparos: 0%sPrecisión: no disponible", PRACTICE_CHAT_SEPARATOR);
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    new bool:passed = PracticeResultPassed(result);
    format(message, sizeof(message), "Puntuación neta: %d%sVeredicto: {%06x}%s{FFFFFF}",
        PracticeResultPoints(result), PRACTICE_CHAT_SEPARATOR, (passed ? COLOR_LIGHTGREEN : COLOR_RED) >>> 8,
        passed ? ("Aprobado") : ("Reprobado"));
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    return PracticeSendResultLine(playerid, PRACTICE_SECTION_COLOUR, PRACTICE_RESULT_BORDER);
}

stock PracticeShowAllowedWeapons(zone, playerid)
{
    new message[144] = "Armas permitidas: ", name[32], count;
    for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
    {
        if (!PracticeWeaponAllowed(weaponid, PracticeRules[zone][PracticeWeaponMask])) continue;
        PracticeWeaponName(weaponid, name);
        // Leave room for punctuation, even if every weapon is enabled.
        if (strlen(message) + strlen(name) + 3 >= sizeof(message))
        {
            SendClientMessage(playerid, COLOR_GRAD1, message);
            format(message, sizeof(message), "Armas permitidas: ");
            count = 0;
        }
        if (count++) strcat(message, ", ", sizeof(message));
        strcat(message, name, sizeof(message));
    }
    return SendClientMessage(playerid, COLOR_GRAD1, message);
}

stock PracticeShowPlayerHelp(zone, playerid)
{
    SendClientMessage(playerid, COLOR_GRAD1, "En el puesto, equipa un arma permitida y usa /practica. Puedes alternar entre las permitidas. Para salir: /practica cancelar.");
    SendClientMessage(playerid, COLOR_GRAD1, "Blanco de tiro: un impacto suma 1. Blanco protegido: cualquier disparo, incluso al aire, resta 1 y cuenta un error de identificación.");
    SendClientMessage(playerid, COLOR_GRAD1, "Procura un tiro por blanco de tiro. Fallar o disparar de más reduce la precisión: aciertos divididos por todos los disparos.");
    SendClientMessage(playerid, COLOR_GRAD1, "Cajas: roja DISPARAR, azul NO DISPARAR. El instructor define el comportamiento de cada silueta.");
    PracticeShowAllowedWeapons(zone, playerid);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Preparación: %ds. Tiempo por blanco: %.1fs. Intervalo: %.1fs. La cuenta regresiva se muestra en pantalla.",
        PracticeRules[zone][PracticePreparation], float(PracticeRules[zone][PracticeExposure]) / 1000.0, float(PracticeRules[zone][PracticeInterval]) / 1000.0);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Para aprobar: mínimo %d puntos netos y máximo %d errores de identificación. Permanece a menos de 3 metros del punto desde el que iniciaste.",
        PracticeRules[zone][PracticePassPoints], PracticeRules[zone][PracticeMaxErrors]);
    return 1;
}
