stock ScbaFindAttachmentFit(playerid, E_SCBA_PART:part)
{
    new skin = GetPlayerSkin(playerid), fallback = SCBA_EMPTY_SLOT;
    for (new i = 0; i < sizeof(ScbaAttachmentFits); i++)
    {
        if (ScbaAttachmentFits[i][ScbaFitPart] != part) continue;
        if (ScbaAttachmentFits[i][ScbaFitSkin] == skin) return i;
        if (ScbaAttachmentFits[i][ScbaFitSkin] == -1) fallback = i;
    }
    return fallback;
}

stock bool:ScbaPlacePart(playerid, E_SCBA_PART:part, slot, fit, const pose[E_SCBA_POSE])
{
    return SetPlayerAttachedObject(playerid, slot, ScbaModels[part][ScbaCustomModel], ScbaModels[part][ScbaModelBone],
        pose[ScbaOffsetX], pose[ScbaOffsetY], pose[ScbaOffsetZ], pose[ScbaRotationX], pose[ScbaRotationY], pose[ScbaRotationZ],
        ScbaAttachmentFits[fit][ScbaFitScaleX], ScbaAttachmentFits[fit][ScbaFitScaleY], ScbaAttachmentFits[fit][ScbaFitScaleZ]) != 0;
}

stock bool:ScbaAttachPart(playerid, E_SCBA_PART:part, bool:useDefault = false)
{
    new slot = ScbaAttachmentSlots[playerid][part];
    if (slot == SCBA_EMPTY_SLOT)
    {
        for (new i = MAX_PLAYER_ATTACHED_OBJECTS - 1; i >= 0; i--)
            if (!IsPlayerAttachedObjectSlotUsed(playerid, i))
            {
                slot = i;
                break;
            }
    }
    new fit = ScbaFindAttachmentFit(playerid, part);
    if (slot == SCBA_EMPTY_SLOT || fit == SCBA_EMPTY_SLOT) return false;
    new pose[E_SCBA_POSE];
    if (!useDefault && ScbaHasCharacterPose[playerid][part]) pose = ScbaCharacterPose[playerid][part];
    else
    {
        pose[ScbaOffsetX] = ScbaAttachmentFits[fit][ScbaFitX];
        pose[ScbaOffsetY] = ScbaAttachmentFits[fit][ScbaFitY];
        pose[ScbaOffsetZ] = ScbaAttachmentFits[fit][ScbaFitZ];
        pose[ScbaRotationX] = ScbaAttachmentFits[fit][ScbaFitRotX];
        pose[ScbaRotationY] = ScbaAttachmentFits[fit][ScbaFitRotY];
        pose[ScbaRotationZ] = ScbaAttachmentFits[fit][ScbaFitRotZ];
    }
    if (!ScbaPlacePart(playerid, part, slot, fit, pose)) return false;
    ScbaAttachmentSlots[playerid][part] = slot;
    return true;
}

stock ScbaRemovePart(playerid, E_SCBA_PART:part)
{
    new slot = ScbaAttachmentSlots[playerid][part];
    if (ScbaPlayer[playerid][ScbaEditingPart] == part) ScbaStopFitEdit(playerid);
    if (slot != SCBA_EMPTY_SLOT) RemovePlayerAttachedObject(playerid, slot);
    ScbaAttachmentSlots[playerid][part] = SCBA_EMPTY_SLOT;
    return 1;
}

stock bool:ScbaHasEquipment(playerid)
{
    new slot = ScbaAttachmentSlots[playerid][SCBA_BACKPACK];
    return ScbaPlayer[playerid][ScbaEquipped] && slot != SCBA_EMPTY_SLOT
        && IsPlayerAttachedObjectSlotUsed(playerid, slot) != 0;
}

stock bool:ScbaRefreshEquipment(playerid)
{
    if (!ScbaHasEquipment(playerid)) return false;
    new E_SCBA_PART:editing = ScbaPlayer[playerid][ScbaEditingPart];
    if (editing != SCBA_PART_COUNT && ScbaHostIsMapping(playerid))
    {
        ScbaStopFitEdit(playerid);
        new slot = ScbaAttachmentSlots[playerid][editing];
        if (slot != SCBA_EMPTY_SLOT && IsPlayerAttachedObjectSlotUsed(playerid, slot)) ScbaAttachPart(playerid, editing);
    }
    if (ScbaPlayer[playerid][ScbaAttachmentSkin] != GetPlayerSkin(playerid))
    {
        ScbaStopFitEdit(playerid);
        if (!ScbaAttachPart(playerid, SCBA_BACKPACK)) return false;
        if (ScbaPlayer[playerid][ScbaMaskOn] && !ScbaAttachPart(playerid, SCBA_MASK)) ScbaRemoveMask(playerid, false);
        ScbaPlayer[playerid][ScbaAttachmentSkin] = GetPlayerSkin(playerid);
    }
    if (ScbaPlayer[playerid][ScbaMaskOn])
    {
        new slot = ScbaAttachmentSlots[playerid][SCBA_MASK];
        if (slot == SCBA_EMPTY_SLOT || !IsPlayerAttachedObjectSlotUsed(playerid, slot)) ScbaRemoveMask(playerid, false);
    }
    return true;
}
