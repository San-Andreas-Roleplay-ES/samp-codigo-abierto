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

stock bool:ScbaAttachPart(playerid, E_SCBA_PART:part)
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
    if (!SetPlayerAttachedObject(playerid, slot, ScbaModels[part][ScbaCustomModel], ScbaModels[part][ScbaModelBone],
        ScbaAttachmentFits[fit][ScbaFitX], ScbaAttachmentFits[fit][ScbaFitY], ScbaAttachmentFits[fit][ScbaFitZ],
        ScbaAttachmentFits[fit][ScbaFitRotX], ScbaAttachmentFits[fit][ScbaFitRotY], ScbaAttachmentFits[fit][ScbaFitRotZ],
        ScbaAttachmentFits[fit][ScbaFitScaleX], ScbaAttachmentFits[fit][ScbaFitScaleY], ScbaAttachmentFits[fit][ScbaFitScaleZ]))
        return false;
    ScbaAttachmentSlots[playerid][part] = slot;
    return true;
}

stock ScbaRemovePart(playerid, E_SCBA_PART:part)
{
    new slot = ScbaAttachmentSlots[playerid][part];
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
    if (ScbaPlayer[playerid][ScbaAttachmentSkin] != GetPlayerSkin(playerid))
    {
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
