stock bool:ScbaReject(playerid, const message[])
{
    SendClientMessage(playerid, SCBA_COLOUR_MUTED, message);
    return false;
}

stock ScbaEndSupply(playerid)
{
    ScbaStopServiceAnimation(playerid);
    ScbaPlayer[playerid][ScbaSupplySource] = SCBA_SOURCE_NONE;
    ScbaPlayer[playerid][ScbaSupplyIndex] = SCBA_EMPTY_SLOT;
    return 1;
}

stock ScbaCancelSupply(playerid, bool:notify)
{
    if (ScbaPlayer[playerid][ScbaSupplySource] == SCBA_SOURCE_NONE) return 1;
    ScbaEndSupply(playerid);
    if (notify)
    {
        ScbaUpdateGauge(playerid, GetTickCount());
        SendClientMessage(playerid, SCBA_COLOUR_MUTED, "Abastecimiento interrumpido. La reserva no ha cambiado.");
    }
    return 1;
}

stock ScbaRemoveMask(playerid, bool:announce)
{
    ScbaPlayer[playerid][ScbaMaskOn] = false;
    ScbaPlayer[playerid][ScbaRegulatorConnected] = false;
    ScbaPlayer[playerid][ScbaBreathingIndoors] = false;
    ScbaRemovePart(playerid, SCBA_MASK);
    ScbaClearSymptoms(playerid);
    ScbaUpdateGauge(playerid, GetTickCount());
    if (announce) ScbaSendMe(playerid, "se retira la máscara.");
    return 1;
}

stock ScbaCleanup(playerid)
{
    ScbaHideGauge(playerid);
    ScbaCancelSupply(playerid, false);
    ScbaRemoveMask(playerid, false);
    ScbaRemovePart(playerid, SCBA_BACKPACK);
    ScbaPlayer[playerid][ScbaEquipped] = false;
    return 1;
}

stock bool:ScbaEquip(playerid)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
        return ScbaReject(playerid, "Debes estar a pie para equiparte.");
    if (ScbaPlayer[playerid][ScbaEquipped])
        return ScbaReject(playerid, "Ya llevas el equipo de respiración.");
    new E_SCBA_SOURCE:source, index;
    if (!ScbaFindSource(playerid, SCBA_EQUIP, source, index))
        return ScbaReject(playerid, "Acércate a un punto de equipamiento o vehículo autorizado.");
    if (!ScbaModelsRegistered || !ScbaPlayer[playerid][ScbaAssetsReady])
        return ScbaReject(playerid, "Espera a que termine la descarga del equipamiento.");
    if (!ScbaAttachPart(playerid, SCBA_BACKPACK))
        return ScbaReject(playerid, "No se pudo colocar el equipo. Comprueba los accesorios disponibles.");
    if (!ScbaShowGauge(playerid, GetTickCount()))
    {
        ScbaRemovePart(playerid, SCBA_BACKPACK);
        return ScbaReject(playerid, "No se pudo mostrar el manómetro. Inténtalo de nuevo.");
    }
    ScbaPlayer[playerid][ScbaEquipped] = true;
    ScbaPreloadServiceAnimations(playerid);
    ScbaPlayer[playerid][ScbaAttachmentSkin] = GetPlayerSkin(playerid);
    ScbaSendMe(playerid, "se coloca el equipo de respiración y ajusta el arnés.");
    return true;
}

stock bool:ScbaToggleMask(playerid, now)
{
    if (!ScbaHasEquipment(playerid))
        return ScbaReject(playerid, "Primero debes equiparte con /scba equipar.");
    if (ScbaPlayer[playerid][ScbaMaskOn])
    {
        ScbaRemoveMask(playerid, true);
        return true;
    }
    if (!ScbaAttachPart(playerid, SCBA_MASK))
        return ScbaReject(playerid, "No se pudo colocar la máscara. Necesitas un espacio libre para accesorios.");
    ScbaPlayer[playerid][ScbaMaskOn] = true;
    ScbaUpdateBreathing(playerid, now, GetPlayerInterior(playerid));
    ScbaUpdateGauge(playerid, now);
    ScbaSendMe(playerid, "se coloca la máscara y ajusta las correas.");
    SendClientMessage(playerid, SCBA_COLOUR_MUTED, "Usa /scba aire para conectar o desconectar el regulador.");
    return true;
}

stock bool:ScbaToggleAir(playerid, now)
{
    if (!ScbaPlayer[playerid][ScbaMaskOn])
        return ScbaReject(playerid, "Ponte la máscara con /scba mascara para conectar el regulador.");
    if (ScbaPlayer[playerid][ScbaRegulatorConnected])
    {
        ScbaPlayer[playerid][ScbaRegulatorConnected] = false;
        ScbaSendMe(playerid, "desacopla el regulador de la máscara.");
    }
    else
    {
        if (ScbaPlayer[playerid][ScbaAirRemainingMs] <= 0)
            return ScbaReject(playerid, "El cilindro está vacío. Retírate la máscara y usa /scba recargar en un punto de abastecimiento.");
        ScbaPlayer[playerid][ScbaRegulatorConnected] = true;
        ScbaPlayer[playerid][ScbaLastAirUpdate] = now;
        ScbaSendMe(playerid, "acopla el regulador a la máscara y empieza a respirar del cilindro.");
        ScbaNotifyReserve(playerid, now);
    }
    ScbaUpdateGauge(playerid, now);
    return true;
}

stock bool:ScbaStore(playerid)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
        return ScbaReject(playerid, "Debes estar a pie para guardar el equipo.");
    if (!ScbaPlayer[playerid][ScbaEquipped])
        return ScbaReject(playerid, "No llevas el equipo de respiración.");
    new E_SCBA_SOURCE:source, index;
    if (!ScbaFindSource(playerid, SCBA_EQUIP, source, index))
        return ScbaReject(playerid, "Acércate a un punto de equipamiento para guardarlo.");
    ScbaCleanup(playerid);
    ScbaSendMe(playerid, "se retira el equipo de respiración y lo guarda.");
    return true;
}

stock ScbaRefillDuration(remainingMs)
{
    new missingMs = SCBA_FULL_AIR_MS - clamp(remainingMs, 0, SCBA_FULL_AIR_MS);
    new Float:fraction = float(missingMs) / float(SCBA_FULL_AIR_MS);
    return SCBA_SUPPLY_MIN_MS + floatround(fraction * (SCBA_SUPPLY_MAX_MS - SCBA_SUPPLY_MIN_MS));
}

stock ScbaStartSupplyWork(playerid, now)
{
    ScbaPlayer[playerid][ScbaServiceAnimSeen] = true;
    ScbaPlayer[playerid][ScbaSupplyStartedAt] = now;
    if (ScbaPlayer[playerid][ScbaSupplySource] == SCBA_SOURCE_VEHICLE)
        ScbaBroadcast(playerid, "", SCBA_FALLBACK_TRUCK_SOUND, SCBA_TRUCK_CLIP);
    else
        ScbaBroadcast(playerid, "", SCBA_FALLBACK_STATION_SOUND, SCBA_STATION_CLIP);
    return 1;
}

stock bool:ScbaBeginSupply(playerid, now)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
        return ScbaReject(playerid, "Debes estar a pie para abastecer el equipo.");
    if (ScbaPlayer[playerid][ScbaEditingPart] != SCBA_PART_COUNT)
        return ScbaReject(playerid, "Termina de ajustar la pieza antes de recargar.");
    if (!ScbaHasEquipment(playerid))
        return ScbaReject(playerid, "Primero debes equiparte con /scba equipar.");
    if (ScbaPlayer[playerid][ScbaMaskOn])
        return ScbaReject(playerid, "Retírate la máscara antes de abastecer el equipo.");
    if (ScbaPlayer[playerid][ScbaAirRemainingMs] >= SCBA_FULL_AIR_MS)
        return ScbaReject(playerid, "El cilindro ya está cargado.");
    new E_SCBA_SOURCE:source, index;
    if (!ScbaFindSource(playerid, SCBA_REFILL, source, index))
        return ScbaReject(playerid, "Acércate a un punto de recarga o vehículo autorizado detenido.");
    ScbaPlayer[playerid][ScbaSupplySource] = source;
    ScbaPlayer[playerid][ScbaSupplyIndex] = index;
    ScbaPlayer[playerid][ScbaSupplyDurationMs] = ScbaRefillDuration(ScbaPlayer[playerid][ScbaAirRemainingMs]);
    ScbaPlayer[playerid][ScbaSupplyStartedAt] = now;
    GetPlayerPos(playerid, ScbaPlayer[playerid][ScbaSupplyStartX],
        ScbaPlayer[playerid][ScbaSupplyStartY], ScbaPlayer[playerid][ScbaSupplyStartZ]);
    if (source == SCBA_SOURCE_VEHICLE)
    {
        new vehicleid = ScbaSupplyVehicles[index][ScbaVehicleId];
        ScbaPlayer[playerid][ScbaSupplyVehicleId] = vehicleid;
        ScbaPlayer[playerid][ScbaSupplyVehicleModel] = GetVehicleModel(vehicleid);
        ScbaSendMe(playerid, "comienza a sustituir el cilindro del equipo de respiración.");
    }
    else ScbaSendMe(playerid, "coloca el cilindro en el puesto de carga.");
    ScbaPrepareServiceAnimation(playerid);
    ScbaPlayServiceAnimation(playerid, now);
    if (ScbaServiceAnimationPlaying(playerid)) ScbaStartSupplyWork(playerid, now);
    ScbaUpdateGauge(playerid, now);
    return true;
}

stock ScbaCompleteSupply(playerid)
{
    ScbaEndSupply(playerid);
    ScbaPlayer[playerid][ScbaAirRemainingMs] = SCBA_FULL_AIR_MS;
    ScbaPlayer[playerid][ScbaReportedBand] = SCBA_AIR_NORMAL;
    ScbaPlayer[playerid][ScbaHasAlarmTime] = false;
    ScbaPlayer[playerid][ScbaHasDistressTime] = false;
    ScbaUpdateGauge(playerid, GetTickCount());
    ScbaBroadcast(playerid, "El cilindro del equipo de respiración queda cargado.");
    return 1;
}

stock ScbaTickSupply(playerid, now)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT || !ScbaServiceSourceValid(playerid))
        return ScbaCancelSupply(playerid, true);
    if (!ScbaServiceAnimationPlaying(playerid))
    {
        if (ScbaPlayer[playerid][ScbaServiceAnimSeen]
            || ScbaElapsed(now, ScbaPlayer[playerid][ScbaSupplyStartedAt]) >= SCBA_ANIM_START_TIMEOUT_MS)
            return ScbaCancelSupply(playerid, true);
        if (ScbaElapsed(now, ScbaPlayer[playerid][ScbaLastAnimAttemptAt]) >= SCBA_ANIM_RETRY_MS)
            ScbaPlayServiceAnimation(playerid, now);
        return 1;
    }
    if (!ScbaPlayer[playerid][ScbaServiceAnimSeen]) ScbaStartSupplyWork(playerid, now);
    if (ScbaElapsed(now, ScbaPlayer[playerid][ScbaSupplyStartedAt]) >= ScbaPlayer[playerid][ScbaSupplyDurationMs])
        return ScbaCompleteSupply(playerid);
    return 1;
}

stock ScbaAdvanceAir(playerid, now)
{
    new elapsed = ScbaElapsed(now, ScbaPlayer[playerid][ScbaLastAirUpdate]);
    ScbaPlayer[playerid][ScbaLastAirUpdate] = now;
    if (elapsed <= 0) return 1;
    new remaining = ScbaPlayer[playerid][ScbaAirRemainingMs];
    if (remaining > elapsed) ScbaPlayer[playerid][ScbaAirRemainingMs] = remaining - elapsed;
    else if (remaining > 0)
    {
        ScbaPlayer[playerid][ScbaAirRemainingMs] = 0;
        new emptyAt = now - (elapsed - remaining);
        ScbaPlayer[playerid][ScbaNextDamageAt] = emptyAt + SCBA_DAMAGE_INTERVAL_MS;
    }
    return 1;
}

stock ScbaUpdateBreathing(playerid, now, interiorid)
{
    if (ScbaPlayer[playerid][ScbaRegulatorConnected]) ScbaAdvanceAir(playerid, now);
    new bool:indoors = ScbaHostIsIndoors(playerid, interiorid);
    if (indoors == ScbaPlayer[playerid][ScbaBreathingIndoors]) return 1;
    ScbaPlayer[playerid][ScbaBreathingIndoors] = indoors;
    if (indoors) ScbaPlayer[playerid][ScbaNextDamageAt] = now + SCBA_DAMAGE_INTERVAL_MS;
    else ScbaClearSymptoms(playerid);
    return 1;
}

stock ScbaApplyAirDamage(playerid, now)
{
    if (!ScbaPlayer[playerid][ScbaBreathingIndoors] || ScbaPlayer[playerid][ScbaAirRemainingMs] > 0) return 1;
    new lateBy = ScbaElapsed(now, ScbaPlayer[playerid][ScbaNextDamageAt]);
    if (lateBy < 0) return 1;
    new Float:health;
    GetPlayerHealth(playerid, health);
    health -= SCBA_DAMAGE_AMOUNT;
    if (health < 0.0) health = 0.0;
    SetPlayerHealth(playerid, health);
    // Absorb normal pulse jitter. A missed damage interval starts a fresh countdown.
    ScbaPlayer[playerid][ScbaNextDamageAt] = now + SCBA_DAMAGE_INTERVAL_MS;
    if (lateBy < SCBA_DAMAGE_INTERVAL_MS)
    {
        new maxCorrection = min(SCBA_PULSE_INTERVAL_MS, SCBA_DAMAGE_INTERVAL_MS / 2);
        ScbaPlayer[playerid][ScbaNextDamageAt] -= min(lateBy, maxCorrection);
    }
    if (health > 0.0) ScbaReportSymptoms(playerid, health, now);
    else ScbaCleanup(playerid);
    return 1;
}

stock ScbaTickPlayer(playerid, now)
{
    if (!IsPlayerConnected(playerid) || !ScbaPlayer[playerid][ScbaEquipped]) return 1;
    if (!ScbaHostCanUse(playerid) || !ScbaPlayerAlive(playerid) || !ScbaRefreshEquipment(playerid))
        return ScbaCleanup(playerid);
    if (ScbaPlayer[playerid][ScbaSupplySource] != SCBA_SOURCE_NONE)
        return ScbaTickSupply(playerid, now);
    if (!ScbaPlayer[playerid][ScbaMaskOn]) return 1;
    if (ScbaPlayer[playerid][ScbaDizzinessLevel] > 0 && GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
        ScbaSetDizziness(playerid, 0);
    ScbaUpdateBreathing(playerid, now, GetPlayerInterior(playerid));
    if (ScbaPlayer[playerid][ScbaRegulatorConnected]) ScbaNotifyReserve(playerid, now);
    ScbaApplyAirDamage(playerid, now);
    if (ScbaPlayer[playerid][ScbaRegulatorConnected]
        && ScbaElapsed(now, ScbaPlayer[playerid][ScbaLastHudUpdate]) >= SCBA_HUD_INTERVAL_MS)
        ScbaUpdateGauge(playerid, now);
    return 1;
}
