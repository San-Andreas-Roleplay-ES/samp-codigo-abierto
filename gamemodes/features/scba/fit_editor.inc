stock bool:ScbaValidFitPose(const pose[E_SCBA_POSE])
{
    for (new E_SCBA_POSE:axis; axis < E_SCBA_POSE; axis++)
        if ((_:pose[axis] & 0x7F800000) == 0x7F800000) return false; // NaN or infinity.
    return floatabs(pose[ScbaOffsetX]) <= SCBA_FIT_OFFSET_LIMIT && floatabs(pose[ScbaOffsetY]) <= SCBA_FIT_OFFSET_LIMIT
        && floatabs(pose[ScbaOffsetZ]) <= SCBA_FIT_OFFSET_LIMIT;
}

stock bool:ScbaSetCharacterFit(playerid, E_SCBA_PART:part, const pose[E_SCBA_POSE], bool:saveToStats = false)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return false;
    if (_:part < 0 || part >= SCBA_PART_COUNT) return false;
    new slot = ScbaAttachmentSlots[playerid][part];
    new bool:accepted = ScbaValidFitPose(pose);
    if (slot != SCBA_EMPTY_SLOT)
    {
        new fit = ScbaFindAttachmentFit(playerid, part);
        if (fit == SCBA_EMPTY_SLOT || !IsPlayerAttachedObjectSlotUsed(playerid, slot)) return false;
        if (accepted) accepted = ScbaPlacePart(playerid, part, slot, fit, pose);
    }
    if (accepted && saveToStats) accepted = ScbaHostSaveFit(playerid, part, pose);
    if (!accepted)
    {
        if (saveToStats && slot != SCBA_EMPTY_SLOT) ScbaAttachPart(playerid, part);
        return false;
    }
    ScbaCharacterPose[playerid][part] = pose;
    ScbaHasCharacterPose[playerid][part] = true;
    return true;
}

stock ScbaStopFitEdit(playerid)
{
    if (ScbaPlayer[playerid][ScbaEditingPart] == SCBA_PART_COUNT) return;
    ScbaPlayer[playerid][ScbaEditingPart] = SCBA_PART_COUNT;
    if (IsPlayerConnected(playerid) && !ScbaHostIsMapping(playerid)) CancelEdit(playerid);
}

stock ScbaBeginFitEdit(playerid, E_SCBA_PART:part)
{
    if (ScbaHostIsMapping(playerid))
        return ScbaReject(playerid, "Termina la edición del objeto antes de ajustar el SCBA.");
    if (ScbaPlayer[playerid][ScbaEditingPart] != SCBA_PART_COUNT)
        return ScbaReject(playerid, "Ya estás ajustando una pieza. Guarda el ajuste o pulsa Esc.");
    if (!ScbaHasEquipment(playerid))
        return ScbaReject(playerid, "Primero ponte el equipo con /scba equipar.");
    if (part == SCBA_MASK && !ScbaPlayer[playerid][ScbaMaskOn])
        return ScbaReject(playerid, "Primero ponte la máscara con /scba mascara.");
    new slot = ScbaAttachmentSlots[playerid][part];
    if (slot == SCBA_EMPTY_SLOT || !IsPlayerAttachedObjectSlotUsed(playerid, slot) || !EditAttachedObject(playerid, slot))
        return ScbaReject(playerid, "No se pudo abrir el editor de la pieza.");
    ScbaPlayer[playerid][ScbaEditingPart] = part;
    return SendClientMessage(playerid, SCBA_COLOUR_MUTED, "Mueve o gira la pieza. Pulsa el icono de guardar para confirmar o Esc para cancelar.");
}

stock ScbaFinishFitEdit(playerid, response, index, modelid, boneid, const pose[E_SCBA_POSE])
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || index < 0 || index >= MAX_PLAYER_ATTACHED_OBJECTS) return 0;
    new E_SCBA_PART:part = ScbaPlayer[playerid][ScbaEditingPart];
    if (part == SCBA_PART_COUNT || index != ScbaAttachmentSlots[playerid][part]
        || modelid != ScbaModels[part][ScbaCustomModel] || boneid != ScbaModels[part][ScbaModelBone]) return 0;
    if (response != EDIT_RESPONSE_FINAL && response != EDIT_RESPONSE_CANCEL) return 0;
    ScbaPlayer[playerid][ScbaEditingPart] = SCBA_PART_COUNT;
    if (!ScbaHostCanUse(playerid) || !ScbaPlayerAlive(playerid) || !ScbaHasEquipment(playerid)
        || !IsPlayerAttachedObjectSlotUsed(playerid, index)) return 1;
    if (response == EDIT_RESPONSE_CANCEL || GetPlayerSkin(playerid) != ScbaPlayer[playerid][ScbaAttachmentSkin]
        || ScbaHostIsMapping(playerid))
    {
        ScbaAttachPart(playerid, part);
        return 1;
    }
    if (ScbaSetCharacterFit(playerid, part, pose, true))
        SendClientMessage(playerid, SCBA_COLOUR_TEXT, "Ajuste de la pieza guardado.");
    else
        SendClientMessage(playerid, SCBA_COLOUR_MUTED, "No se pudo guardar el ajuste. Se conserva el anterior.");
    return 1;
}
