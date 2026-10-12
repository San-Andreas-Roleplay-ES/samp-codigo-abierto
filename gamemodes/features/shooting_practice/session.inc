#if defined SP_SESSION_INCLUDED
    #endinput
#endif
#define SP_SESSION_INCLUDED

enum E_PRACTICE_PHASE
{
    PRACTICE_IDLE,
    PRACTICE_COUNTDOWN,
    PRACTICE_TARGET,
    PRACTICE_INTERVAL
}

enum E_PRACTICE_SESSION
{
    E_PRACTICE_PHASE:PracticePhase,
    PracticePlayer,
    E_PRACTICE_PURPOSE:PracticePurpose,
    STREAMER_TAG_OBJECT:PracticeObject,
    PracticeSlot,
    PracticeTotal,
    PracticeShootTotal,
    PracticeNoShootTotal,
    PracticeRequiredPoints,
    PracticeAllowedErrors,
    PracticeAllowedWeapons,
    PracticeLastWeapon,
    bool:PracticeNoShoot,
    PracticePresented,
    PracticeShots,
    PracticeHits,
    PracticeExpired,
    PracticeCorrectIgnores,
    PracticeIdentificationErrors,
    PracticePhaseStarted,
    PracticePhaseDuration,
    PracticeLastCountdown,
    PracticeCountdownCueAt
}

forward bool:PracticeIntegrationBeforeStart(zone, const session[E_PRACTICE_SESSION],
    const origin[E_PRACTICE_POINT], startSlot, const slots[], const models[], const bool:noShoot[]);
forward E_PRACTICE_HISTORY_STATUS:PracticeIntegrationOnEnd(zone, bool:completed, const reason[],
    const session[E_PRACTICE_SESSION], const origin[E_PRACTICE_POINT], startSlot,
    const slots[], const models[], const bool:noShoot[]);

// One independent session/sequence per zone, with one active attempt per player.
// The player lookup routes shots and lifecycle cleanup to their owning zone.
new PracticeSession[PRACTICE_MAX_ZONES][E_PRACTICE_SESSION];
new PracticeSessionOrigin[PRACTICE_MAX_ZONES][E_PRACTICE_POINT];
new PracticeSessionStartSlot[PRACTICE_MAX_ZONES] = {-1, ...};
new PracticeSequenceSlots[PRACTICE_MAX_ZONES][PRACTICE_MAX_TARGETS];
new bool:PracticeSequenceNoShoot[PRACTICE_MAX_ZONES][PRACTICE_MAX_TARGETS];
new PracticeSequenceModels[PRACTICE_MAX_ZONES][PRACTICE_MAX_TARGETS];
new PracticePlayerZone[MAX_PLAYERS] = {-1, ...};

stock bool:PracticeIsBusy(zone)
{
    return zone >= 0 && zone < PRACTICE_MAX_ZONES && PracticeSession[zone][PracticePhase] != PRACTICE_IDLE;
}

// TODO(SP-ACCESS): allow open access or a matching group/faction
// whitelist entry using cached identity/membership supplied by the host gamemode.
// Restricted access with no matching entry or unresolved identity must fail.
// Keep this check synchronous: menu/start
// validation must not query the database. Practice access does not grant editing.
// The current standalone prototype deliberately allows every non-NPC player.
stock bool:PracticeCanUseStation(zone, playerid)
{
    return PracticeZoneExists(zone) && !PracticeZoneArchived[zone] && PracticeHostCanUseZone(zone, playerid);
}

stock PracticeGetPlayerZone(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return -1;
    new zone = PracticePlayerZone[playerid];
    if (PracticeIsBusy(zone) && PracticeSession[zone][PracticePlayer] == playerid) return zone;
    PracticePlayerZone[playerid] = -1;
    return -1;
}

stock PracticeRemoveTarget(zone)
{
    new STREAMER_TAG_OBJECT:objectid = PracticeSession[zone][PracticeObject];
    PracticeSession[zone][PracticeObject] = INVALID_STREAMER_ID;
    if (objectid != INVALID_STREAMER_ID && IsValidDynamicObject(objectid))
        DestroyDynamicObject(objectid);
    return 1;
}

stock PracticeCleanup(zone, bool:keepOrigin = false)
{
    if (zone < 0 || zone >= PRACTICE_MAX_ZONES) return 0;
    new playerid = PracticeSession[zone][PracticePlayer];
    if (playerid >= 0 && playerid < MAX_PLAYERS && PracticePlayerZone[playerid] == zone)
    {
        PracticeHideCountdown(playerid);
        PracticePlayerZone[playerid] = -1;
    }
    PracticeRemoveTarget(zone);
    new empty[E_PRACTICE_SESSION];
    PracticeSession[zone] = empty;
    PracticeSession[zone][PracticePlayer] = INVALID_PLAYER_ID;
    if (!keepOrigin)
    {
        new emptyOrigin[E_PRACTICE_POINT];
        PracticeSessionOrigin[zone] = emptyOrigin;
        PracticeSessionStartSlot[zone] = -1;
    }
    for (new index = 0; index < PRACTICE_MAX_TARGETS; index++)
    {
        PracticeSequenceSlots[zone][index] = 0;
        PracticeSequenceNoShoot[zone][index] = false;
        PracticeSequenceModels[zone][index] = 0;
    }
    return 1;
}

stock PracticeResultPoints(const result[E_PRACTICE_SESSION])
{
    return result[PracticeHits] - result[PracticeIdentificationErrors];
}

stock bool:PracticeResultPassed(const result[E_PRACTICE_SESSION])
{
    return PracticeResultPoints(result) >= result[PracticeRequiredPoints]
        && result[PracticeIdentificationErrors] <= result[PracticeAllowedErrors];
}

stock PracticeEnd(zone, bool:completed, const reason[] = "", bool:notify = true, bool:abandoned = false)
{
    if (!PracticeIsBusy(zone)) return 1;
    new result[E_PRACTICE_SESSION];
    result = PracticeSession[zone];
    // Supply the final snapshot, frozen origin and sequence before cleanup.
    // The active training port returns NOT_RECORDED, never a fake DB acknowledgement.
    PracticeIntegrationOnEnd(zone, completed, reason, result, PracticeSessionOrigin[zone],
        PracticeSessionStartSlot[zone], PracticeSequenceSlots[zone],
        PracticeSequenceModels[zone], PracticeSequenceNoShoot[zone]);
    // Keep the frozen origin through the final public cue, including abandonment.
    PracticeCleanup(zone, true);
    new playerid = result[PracticePlayer];
    PracticePresentEnd(zone, completed, playerid, abandoned);
    new emptyOrigin[E_PRACTICE_POINT];
    PracticeSessionOrigin[zone] = emptyOrigin;
    PracticeSessionStartSlot[zone] = -1;
    if (notify && IsPlayerConnected(playerid))
        PracticeReportResult(playerid, completed, result, reason);
    return 1;
}

stock PracticeWeaponSlot(weaponid)
{
    switch (weaponid)
    {
        case 22..24: return 2;
        case 25..27: return 3;
        case 28, 29, 32: return 4;
        case 30, 31: return 5;
        case 33, 34: return 6;
    }
    return 0;
}

stock bool:PracticeParticipantValid(zone)
{
    new playerid = PracticeSession[zone][PracticePlayer];
    if (!IsPlayerConnected(playerid))
    {
        PracticeEnd(zone, false, "desconexión", false);
        return false;
    }
    new Float:health;
    GetPlayerHealth(playerid, health);
    if (health <= 0.0 || GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
    {
        PracticeEnd(zone, false, "debes permanecer vivo y a pie.");
        return false;
    }
    if (GetPlayerInterior(playerid) != PracticeSessionOrigin[zone][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticeSessionOrigin[zone][PracticePointWorld]
        || !IsPlayerInRangeOfPoint(playerid, PRACTICE_ACTIVE_RADIUS, PracticeSessionOrigin[zone][PracticePointX],
            PracticeSessionOrigin[zone][PracticePointY], PracticeSessionOrigin[zone][PracticePointZ]))
    {
        PracticeEnd(zone, false, "Cancelada automáticamente por salir del puesto de tiro.", true, true);
        return false;
    }
    new heldWeapon = GetPlayerWeapon(playerid);
    if (PracticeWeaponAllowed(heldWeapon, PracticeSession[zone][PracticeAllowedWeapons]))
        PracticeSession[zone][PracticeLastWeapon] = heldWeapon;
    else
    {
        new weaponid, ammo, lastWeapon = PracticeSession[zone][PracticeLastWeapon];
        GetPlayerWeaponData(playerid, PracticeWeaponSlot(lastWeapon), weaponid, ammo);
        // SA-MP may switch to fists on exhaustion. Check the last permitted
        // weapon's slot, including when the player switched weapons mid-round.
        if (heldWeapon != 0 || ammo > 0 || (weaponid != 0 && weaponid != lastWeapon))
        {
            PracticeEnd(zone, false, "cambiaste a un arma no permitida.");
            return false;
        }
    }
    if (PracticeSession[zone][PracticePhase] == PRACTICE_TARGET
        && !IsValidDynamicObject(PracticeSession[zone][PracticeObject]))
    {
        PracticeEnd(zone, false, "el blanco activo ya no existe.");
        return false;
    }
    return true;
}

stock PracticeShowNextTarget(zone, now)
{
    new index = PracticeSession[zone][PracticePresented];
    if (index >= PracticeSession[zone][PracticeTotal])
        return PracticeEnd(zone, false, "no se encontró la siguiente posición.");
    new slot = PracticeSequenceSlots[zone][index];

    new playerid = PracticeSession[zone][PracticePlayer];
    new STREAMER_TAG_OBJECT:objectid = CreateDynamicObject(PracticeSequenceModels[zone][index],
        PracticePoints[zone][slot][PracticePointX], PracticePoints[zone][slot][PracticePointY],
        PracticePoints[zone][slot][PracticePointZ] + PRACTICE_TARGET_Z_OFFSET,
        PracticePoints[zone][slot][PracticePointRotX], PracticePoints[zone][slot][PracticePointRotY], PracticePoints[zone][slot][PracticePointAngle],
        PracticeSessionOrigin[zone][PracticePointWorld], PracticeSessionOrigin[zone][PracticePointInterior]);
    PracticeSession[zone][PracticeObject] = objectid;
    if (objectid == INVALID_STREAMER_ID || !IsValidDynamicObject(objectid))
        return PracticeEnd(zone, false, "no se pudo crear el blanco.");
    if (!PracticeApplyTargetAppearance(objectid, PracticeSequenceNoShoot[zone][index], PracticeSequenceModels[zone][index]))
        return PracticeEnd(zone, false, "no se pudo identificar visualmente el blanco.");

    Streamer_Update(playerid, STREAMER_TYPE_OBJECT);
    if (!Streamer_IsItemVisible(playerid, STREAMER_TYPE_OBJECT, objectid))
        return PracticeEnd(zone, false, "el blanco no pudo mostrarse. Revisa la posición y el límite de objetos.");

    PracticeHideCountdown(playerid);
    PracticeSession[zone][PracticeSlot] = slot;
    PracticeSession[zone][PracticeNoShoot] = PracticeSequenceNoShoot[zone][index];
    PracticeSession[zone][PracticePresented]++;
    PracticeSession[zone][PracticePhase] = PRACTICE_TARGET;
    PracticeSession[zone][PracticePhaseStarted] = now;
    PracticeSession[zone][PracticePhaseDuration] = PracticeRules[zone][PracticeExposure];
    // Sound only after creation, markings and streaming have succeeded. The first
    // appearance is also the start signal, with no duplicate cue or type hint.
    PracticePresentTarget(zone);
    return 1;
}

stock PracticeResolveTarget(zone, bool:byShot, now)
{
    if (PracticeSession[zone][PracticePhase] != PRACTICE_TARGET) return 1;
    // Retire the phase before destroying the object, so it can resolve only once.
    PracticeSession[zone][PracticePhase] = PRACTICE_INTERVAL;
    PracticeRemoveTarget(zone);
    if (PracticeSession[zone][PracticeNoShoot])
    {
        if (byShot) PracticeSession[zone][PracticeIdentificationErrors]++;
        else PracticeSession[zone][PracticeCorrectIgnores]++;
    }
    else if (byShot)
    {
        PracticeSession[zone][PracticeHits]++;
    }
    else
    {
        PracticeSession[zone][PracticeExpired]++;
    }
    if (PracticeSession[zone][PracticePresented] == PracticeSession[zone][PracticeTotal])
        return PracticeEnd(zone, true);
    PracticeSession[zone][PracticePhaseStarted] = now;
    PracticeSession[zone][PracticePhaseDuration] = PracticeRules[zone][PracticeInterval];
    return 1;
}

stock PracticeTick(zone, now)
{
    if (!PracticeIsBusy(zone) || !PracticeParticipantValid(zone)) return 1;
    // Subtraction remains valid across GetTickCount's signed wraparound.
    new elapsed = now - PracticeSession[zone][PracticePhaseStarted];
    if (PracticeSession[zone][PracticePhase] == PRACTICE_COUNTDOWN
        && now - PracticeSession[zone][PracticeCountdownCueAt] < 1000) return 1;
    if (elapsed >= PracticeSession[zone][PracticePhaseDuration])
    {
        switch (PracticeSession[zone][PracticePhase])
        {
            case PRACTICE_COUNTDOWN, PRACTICE_INTERVAL: PracticeShowNextTarget(zone, now);
            case PRACTICE_TARGET: PracticeResolveTarget(zone, false, now);
        }
    }
    else if (PracticeSession[zone][PracticePhase] == PRACTICE_COUNTDOWN)
    {
        new remaining = (PracticeSession[zone][PracticePhaseDuration] - elapsed + 999) / 1000;
        if (remaining != PracticeSession[zone][PracticeLastCountdown])
        {
            PracticeSession[zone][PracticeLastCountdown] = remaining;
            // Keep a full second between actual cues, including the start signal.
            // Delayed ticks show the current value without replaying missed beeps.
            PracticeSession[zone][PracticeCountdownCueAt] = now;
            PracticePresentCountdown(zone);
        }
    }
    return 1;
}

stock PracticeHandleShot(playerid, weaponid, hittype, hitid, now)
{
    new zone = PracticeGetPlayerZone(playerid);
    if (zone < 0) return 1;
    if (!PracticeIsBusy(zone) || playerid != PracticeSession[zone][PracticePlayer]) return 1;
    if (!PracticeParticipantValid(zone)) return 1;
    if (!PracticeWeaponAllowed(weaponid, PracticeSession[zone][PracticeAllowedWeapons]))
        return PracticeEnd(zone, false, "disparaste con un arma no permitida.");
    if (PracticeSession[zone][PracticePhase] == PRACTICE_COUNTDOWN) return 1;

    // Enforce the deadline even between timer ticks. Never create a new target
    // from a shot callback: a late hit must not be credited to a reused object ID.
    if (PracticeSession[zone][PracticePhase] == PRACTICE_TARGET
        && now - PracticeSession[zone][PracticePhaseStarted] >= PracticeSession[zone][PracticePhaseDuration])
        PracticeResolveTarget(zone, false, now);
    if (!PracticeIsBusy(zone)) return 1;

    PracticeSession[zone][PracticeShots]++;
    if (PracticeSession[zone][PracticePhase] == PRACTICE_TARGET)
    {
        // A no-shoot presentation tests trigger restraint: the first discharge
        // resolves it as an error even if it hits air, scenery or another object.
        if (PracticeSession[zone][PracticeNoShoot]) PracticeResolveTarget(zone, true, now);
        else if (hittype == BULLET_HIT_TYPE_PLAYER_OBJECT
            && Streamer_GetItemStreamerID(playerid, STREAMER_TYPE_OBJECT, hitid) == PracticeSession[zone][PracticeObject])
            PracticeResolveTarget(zone, true, now);
    }
    return 1;
}

stock bool:PracticeStartReady(zone, playerid, reason[], size = sizeof(reason))
{
    if (!PracticeZoneExists(zone) || PracticeZoneArchived[zone]) format(reason, size, "La zona no existe o está archivada.");
    else if (!PracticeZoneEnabled[zone]) format(reason, size, "Esta zona está desactivada.");
    else if (!IsPlayerConnected(playerid)) format(reason, size, "El participante no está conectado.");
    else if (PracticeGetPlayerZone(playerid) >= 0) format(reason, size, "Ya tienes una práctica activa. Usa /practica cancelar.");
    else if (PracticeHasDraft(playerid)) format(reason, size, "Guarda o descarta tu edición antes de iniciar una práctica.");
    else if (PracticeConfigurationEditing(zone)) format(reason, size, "Espera a que el administrador termine de editar el puesto.");
    else if (PracticeIsBusy(zone))
    {
        if (PracticeSession[zone][PracticePlayer] == playerid)
            format(reason, size, "Ya tienes una práctica activa. Usa /practica cancelar.");
        else format(reason, size, "Hay una práctica activa en este campo de tiro. Espera a que termine.");
    }
    else if (!PracticeConfigurationReady(zone, reason, size)) return false;
    else if (!PracticeCanUseStation(zone, playerid)) format(reason, size, "No tienes acceso a este puesto de tiro.");
    else
    {
        new Float:health;
        GetPlayerHealth(playerid, health);
        if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT || health <= 0.0)
            format(reason, size, "Debes estar vivo y a pie para iniciar.");
        else if (PracticeNearestStart(zone, playerid, PRACTICE_START_RADIUS) < 0)
            format(reason, size, "Acércate a uno de los puntos de inicio del campo de tiro.");
        else if (!PracticeWeaponAllowed(GetPlayerWeapon(playerid), PracticeRules[zone][PracticeWeaponMask]))
            format(reason, size, "Equipa una de las armas permitidas antes de iniciar.");
        else { reason[0] = EOS; return true; }
    }
    return false;
}

stock bool:PracticeCanStart(zone, playerid)
{
    new reason[144];
    return PracticeStartReady(zone, playerid, reason);
}

stock PracticeStart(zone, playerid)
{
    new reason[144];
    if (!PracticeStartReady(zone, playerid, reason))
        return SendClientMessage(playerid, COLOR_FADE3, reason);

    // TODO(SP-RECORDING): select training/evaluation before starting; freeze the
    // zone version and history policy. Evaluation requires stable identity and
    // confirmed durable start registration before countdown. Never downgrade
    // to training on a storage failure; revalidate player/zone on async reply.
    PracticeCleanup(zone);
    PracticeHideAllPreviews(zone);
    PracticeHidePreview(playerid);
    PracticeSessionStartSlot[zone] = PracticeNearestStart(zone, playerid, PRACTICE_START_RADIUS);
    PracticeSessionOrigin[zone] = PracticePoints[zone][PracticeSessionStartSlot[zone]];
    PracticeSession[zone][PracticePlayer] = playerid;
    PracticeSession[zone][PracticePurpose] = SP_PURPOSE_TRAINING;
    PracticePlayerZone[playerid] = zone;
    // TODO(SP-DB-IDENTITY): for recorded evaluations, capture the selected zone
    // version, stable host subject, unique attempt ID, UTC start and sequence.
    // playerid and GetTickCount are runtime values, not persistent identity keys.
    PracticeSession[zone][PracticeTotal] = PracticeBuildSequence(zone, PracticeSequenceSlots[zone], PracticeSequenceNoShoot[zone], PracticeSequenceModels[zone]);
    PracticeSession[zone][PracticeNoShootTotal] = PracticeCountNoShootTargets(zone);
    PracticeSession[zone][PracticeShootTotal] = PracticeCountShootTargets(zone);
    PracticeSession[zone][PracticeRequiredPoints] = PracticeRules[zone][PracticePassPoints];
    PracticeSession[zone][PracticeAllowedErrors] = PracticeRules[zone][PracticeMaxErrors];
    PracticeSession[zone][PracticeAllowedWeapons] = PracticeRules[zone][PracticeWeaponMask];
    PracticeSession[zone][PracticeLastWeapon] = GetPlayerWeapon(playerid);
    if (!PracticeIntegrationBeforeStart(zone, PracticeSession[zone], PracticeSessionOrigin[zone],
        PracticeSessionStartSlot[zone], PracticeSequenceSlots[zone],
        PracticeSequenceModels[zone], PracticeSequenceNoShoot[zone]))
    {
        PracticeCleanup(zone);
        return SendClientMessage(playerid, COLOR_FADE3, "No se pudo preparar la práctica. Inténtalo de nuevo.");
    }
    PracticeSession[zone][PracticePhase] = PRACTICE_COUNTDOWN;
    PracticeSession[zone][PracticePhaseStarted] = GetTickCount();
    PracticeSession[zone][PracticePhaseDuration] = PracticeRules[zone][PracticePreparation] * 1000;
    PracticeSession[zone][PracticeLastCountdown] = PracticeRules[zone][PracticePreparation];
    PracticeSession[zone][PracticeCountdownCueAt] = PracticeSession[zone][PracticePhaseStarted];
    PracticePresentStart(zone, playerid);
    return 1;
}

CMD:practica(playerid, const params[])
{
    return PracticePlayerCommand(playerid, params);
}
