#if defined SP_EDITOR_INCLUDED
    #endinput
#endif
#define SP_EDITOR_INCLUDED

// Each zone has one writer. A player's draft and private preview use its stable
// selected zone binding; unrelated zones retain their locks and object handles.
new PracticeEditorOwner[PRACTICE_MAX_ZONES] = {INVALID_PLAYER_ID, ...};
new PracticeDraftSlot[MAX_PLAYERS] = {-1, ...};
new PracticeDraftRevision[MAX_PLAYERS];
new PracticeDraft[MAX_PLAYERS][E_PRACTICE_POINT];
new bool:PracticeDraftNew[MAX_PLAYERS], bool:PracticeDraftWizard[MAX_PLAYERS], bool:PracticeDraftFixTypes[MAX_PLAYERS];
new STREAMER_TAG_OBJECT:PracticeEditObject[MAX_PLAYERS];
new bool:PracticePreviewActive[MAX_PLAYERS], bool:PracticeSetupView[MAX_PLAYERS];
new PracticeViewPoints[MAX_PLAYERS][PRACTICE_POINT_COUNT][E_PRACTICE_POINT];
new STREAMER_TAG_OBJECT:PracticePreviewObjects[MAX_PLAYERS][PRACTICE_MAX_TARGETS];
new STREAMER_TAG_3D_TEXT_LABEL:PracticePreviewLabels[MAX_PLAYERS][PRACTICE_POINT_COUNT];
new STREAMER_TAG_CP:PracticePreviewCheckpoint[MAX_PLAYERS][PRACTICE_POINT_COUNT];
new bool:PracticePreviewRepair[MAX_PLAYERS][PRACTICE_MAX_TARGETS], bool:PracticePreviewRepairPending;

stock PracticeConfigurationEditing(zone)
{
    return zone >= 0 && zone < PRACTICE_MAX_ZONES && PracticeEditorOwner[zone] != INVALID_PLAYER_ID;
}

stock PracticeHasDraft(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    return PracticeEditorOwner[zone] == playerid && PracticeDraftSlot[playerid] >= 0;
}

// Administration currently requires RCON. Host administrative roles belong in
// one shared policy used here, the setup command/menu and preview maintenance.
// Group/faction grants for practicing must not bypass this policy.
stock bool:PracticeAdminAvailable(playerid)
{
    new Float:health;
    return PracticeIsAdministrator(playerid) && GetPlayerState(playerid) == PLAYER_STATE_ONFOOT && GetPlayerHealth(playerid, health) && health > 0.0;
}

stock bool:PracticeMayChange(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return false;
    if (PracticeStorageBlocked)
    {
        SendClientMessage(playerid, COLOR_FADE3, "El archivo de zonas no pudo recuperarse. Corrígelo antes de guardar cambios.");
        return false;
    }
    if (!PracticeAdminAvailable(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Necesitas ser administrador RCON, estar vivo y a pie.");
        return false;
    }
    if (PracticeIsBusy(zone))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Espera a que termine la práctica para modificar el campo de tiro.");
        return false;
    }
    if (PracticeConfigurationEditing(zone) && PracticeEditorOwner[zone] != playerid)
    {
        SendClientMessage(playerid, COLOR_FADE3, "Otro administrador está editando este campo de tiro.");
        return false;
    }
    return true;
}

stock PracticeCapturePoint(playerid, point[E_PRACTICE_POINT])
{
    GetPlayerPos(playerid, point[PracticePointX], point[PracticePointY], point[PracticePointZ]);
    GetPlayerFacingAngle(playerid, point[PracticePointAngle]);
    point[PracticePointInterior] = GetPlayerInterior(playerid);
    point[PracticePointWorld] = GetPlayerVirtualWorld(playerid);
    point[PracticePointSet] = true;
    return 1;
}

stock PracticeRemoveViewSlot(playerid, slot)
{
    if (!PracticeIsStartSlot(slot))
    {
        PracticePreviewRepair[playerid][slot - 1] = false;
        if (IsValidDynamicObject(PracticePreviewObjects[playerid][slot - 1]))
            DestroyDynamicObject(PracticePreviewObjects[playerid][slot - 1]);
        PracticePreviewObjects[playerid][slot - 1] = INVALID_STREAMER_ID;
    }
    else
    {
        if (IsValidDynamicCP(PracticePreviewCheckpoint[playerid][slot])) DestroyDynamicCP(PracticePreviewCheckpoint[playerid][slot]);
        PracticePreviewCheckpoint[playerid][slot] = STREAMER_TAG_CP:INVALID_STREAMER_ID;
    }
    if (IsValidDynamic3DTextLabel(PracticePreviewLabels[playerid][slot]))
        DestroyDynamic3DTextLabel(PracticePreviewLabels[playerid][slot]);
    PracticePreviewLabels[playerid][slot] = STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID;
    return 1;
}

stock PracticeHidePreview(playerid)
{
    PracticePreviewActive[playerid] = PracticeSetupView[playerid] = false;
    for (new slot = 0; slot < PRACTICE_POINT_COUNT; slot++) PracticeRemoveViewSlot(playerid, slot);
    return 1;
}

stock PracticeHideAllPreviews(zone)
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (PracticeSelectedZone[playerid] == zone && (PracticePreviewActive[playerid] || PracticeSetupView[playerid])) PracticeHidePreview(playerid);
    return 1;
}

stock STREAMER_TAG_OBJECT:PracticeCreatePrivateObject(playerid, const point[E_PRACTICE_POINT])
{
    new STREAMER_TAG_OBJECT:objectid = CreateDynamicObject(point[PracticePointModel],
        point[PracticePointX], point[PracticePointY], point[PracticePointZ],
        point[PracticePointRotX], point[PracticePointRotY], point[PracticePointAngle],
        point[PracticePointWorld], point[PracticePointInterior], playerid);
    if (!IsValidDynamicObject(objectid)) return STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
    if (!PracticeApplyTargetAppearance(objectid, point[PracticePointNoShoot], point[PracticePointModel]))
    {
        DestroyDynamicObject(objectid);
        return STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
    }
    return objectid;
}

stock bool:PracticeViewSlotCurrent(playerid, slot)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return false;
    if (!IsValidDynamic3DTextLabel(PracticePreviewLabels[playerid][slot])) return false;
    if (!PracticeIsStartSlot(slot))
    {
        if (!IsValidDynamicObject(PracticePreviewObjects[playerid][slot - 1])) return false;
    }
    else if (!IsValidDynamicCP(PracticePreviewCheckpoint[playerid][slot])) return false;
    for (new field = 0; field < _:E_PRACTICE_POINT; field++)
        if (PracticeViewPoints[playerid][slot][E_PRACTICE_POINT:field] != PracticePoints[zone][slot][E_PRACTICE_POINT:field]) return false;
    return true;
}

stock bool:PracticeSynchronizeView(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) { PracticeHidePreview(playerid); return false; }
    if (!PracticeAdminAvailable(playerid) || PracticeIsBusy(zone) || !PracticeInStartDimension(zone, playerid))
    { PracticeHidePreview(playerid); return false; }

    for (new slot = 0; slot < PRACTICE_POINT_COUNT; slot++)
    {
        new bool:editingSlot = !PracticeIsStartSlot(slot) && PracticeHasDraft(playerid)
            && PracticeEditObject[playerid] != INVALID_STREAMER_ID && PracticeDraftSlot[playerid] == slot;
        if (!PracticePoints[zone][slot][PracticePointSet] || editingSlot)
        { PracticeRemoveViewSlot(playerid, slot); continue; }
        // Keep unchanged neighbours alive, including while the selected object moves.
        if (PracticeViewSlotCurrent(playerid, slot)) continue;
        PracticeRemoveViewSlot(playerid, slot);
        new label[100], name[PRACTICE_NAME_SIZE], bool:success;
        PracticePointDisplayName(PracticePoints[zone][slot], slot, name);
        if (!PracticeIsStartSlot(slot))
        {
            PracticePreviewObjects[playerid][slot - 1] = PracticeCreatePrivateObject(playerid, PracticePoints[zone][slot]);
            success = IsValidDynamicObject(PracticePreviewObjects[playerid][slot - 1]) != 0;
            format(label, sizeof(label), "%s\n%s", name,
                PracticePoints[zone][slot][PracticePointNoShoot] ? ("NO DISPARAR") : ("DISPARAR"));
        }
        else
        {
            PracticePreviewCheckpoint[playerid][slot] = CreateDynamicCP(PracticePoints[zone][slot][PracticePointX],
                PracticePoints[zone][slot][PracticePointY], PracticePoints[zone][slot][PracticePointZ], PRACTICE_START_RADIUS,
                PracticePoints[zone][slot][PracticePointWorld], PracticePoints[zone][slot][PracticePointInterior], playerid, 200.0);
            success = IsValidDynamicCP(PracticePreviewCheckpoint[playerid][slot]) != 0;
            format(label, sizeof(label), "%s\nINICIO DE PRÁCTICA", name);
        }
        if (success)
        {
            PracticePreviewLabels[playerid][slot] = CreateDynamic3DTextLabel(label,
                PracticeIsStartSlot(slot) ? 0xFFFF66FF : (PracticePoints[zone][slot][PracticePointNoShoot] ? 0x66BFFFFF : 0xFF7777FF),
                PracticePoints[zone][slot][PracticePointX], PracticePoints[zone][slot][PracticePointY], PracticePoints[zone][slot][PracticePointZ] + 1.4,
                60.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0,
                PracticePoints[zone][slot][PracticePointWorld], PracticePoints[zone][slot][PracticePointInterior], playerid);
            success = IsValidDynamic3DTextLabel(PracticePreviewLabels[playerid][slot]) != 0;
        }
        if (!success)
        {
            PracticeHidePreview(playerid);
            SendClientMessage(playerid, COLOR_FADE3, "No se pudieron mostrar los blancos. Vuelve a intentarlo.");
            return false;
        }
        PracticeViewPoints[playerid][slot] = PracticePoints[zone][slot];
    }
    Streamer_Update(playerid);
    return true;
}

stock bool:PracticeProtectPreviewShot(playerid, STREAMER_TAG_OBJECT:objectid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || objectid == INVALID_STREAMER_ID
        || !(PracticePreviewActive[playerid] || PracticeSetupView[playerid])) return false;
    for (new index = 0; index < PRACTICE_MAX_TARGETS; index++)
        if (PracticePreviewObjects[playerid][index] == objectid)
        {
            // The GTA client can locally break a model before the server veto.
            // Defer replacement until after the streamer has finished dispatching
            // this shot. Repeated hits coalesce and only this private copy changes.
            PracticePreviewRepair[playerid][index] = true;
            PracticePreviewRepairPending = true;
            return true;
        }
    return false;
}

stock PracticeRepairPreviews()
{
    if (!PracticePreviewRepairPending) return 1;
    PracticePreviewRepairPending = false;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        new bool:changed;
        for (new index = 0; index < PRACTICE_MAX_TARGETS; index++)
        {
            if (!PracticePreviewRepair[playerid][index]) continue;
            PracticePreviewRepair[playerid][index] = false;
            if (!(PracticePreviewActive[playerid] || PracticeSetupView[playerid])) continue;
            PracticeRemoveViewSlot(playerid, index + 1);
            changed = true;
        }
        // Revalidates permissions, dimension, session and the current saved record.
        // Hidden views, cancelled drafts and reused player IDs cannot resurrect.
        if (changed) PracticeSynchronizeView(playerid);
    }
    return 1;
}

stock bool:PracticeInSetupRange(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return false;
    return PracticeNearestStart(zone, playerid, PRACTICE_MAX_DISTANCE) >= 0;
}

stock PracticeEnsureSetupView(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    if (!PracticeCountStarts(zone) || !PracticeAdminAvailable(playerid) || PracticeIsBusy(zone) || !PracticeInSetupRange(playerid))
        return 0;
    PracticeSetupView[playerid] = true;
    return PracticeSynchronizeView(playerid);
}

stock PracticeLeaveSetupView(playerid)
{
    if (!PracticeSetupView[playerid]) return 1;
    PracticeSetupView[playerid] = false;
    if (PracticePreviewActive[playerid]) PracticeSynchronizeView(playerid);
    else PracticeHidePreview(playerid);
    return 1;
}

stock PracticeShowPreview(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    new first = PracticeFirstStart(zone);
    if (!PracticeMayChange(playerid) || PracticeHasDraft(playerid)) return 0;
    if (!PracticeCountStarts(zone))
        return SendClientMessage(playerid, COLOR_FADE3, "Marca primero el punto de inicio desde el menú.");
    if (GetPlayerInterior(playerid) != PracticePoints[zone][first][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[zone][first][PracticePointWorld])
        return SendClientMessage(playerid, COLOR_FADE3, "Ve al mismo interior y mundo del puesto para ver sus posiciones.");
    PracticePreviewActive[playerid] = true;
    if (!PracticeSynchronizeView(playerid)) return 0;
    SendClientMessage(playerid, COLOR_GRAD1, "Vista previa privada activa. Puedes caminar. Abre /practicaconfig para ocultarla.");
    return 1;
}

stock PracticeRemoveEditObject(playerid, bool:cancelNative = true)
{
    new STREAMER_TAG_OBJECT:objectid = PracticeEditObject[playerid];
    PracticeEditObject[playerid] = INVALID_STREAMER_ID;
    DeletePVar(playerid, "SP_ObjectEditor");
    if (objectid != INVALID_STREAMER_ID)
    {
        if (cancelNative && IsPlayerConnected(playerid)) CancelEdit(playerid);
        if (IsValidDynamicObject(objectid)) DestroyDynamicObject(objectid);
    }
    return 1;
}

stock PracticeDiscardDraft(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    // Clear state before native cancellation can trigger another callback.
    PracticeDraftSlot[playerid] = -1;
    PracticeDraftWizard[playerid] = PracticeDraftNew[playerid] = PracticeDraftFixTypes[playerid] = false;
    if (zone >= 0 && zone < PRACTICE_MAX_ZONES && PracticeEditorOwner[zone] == playerid) PracticeEditorOwner[zone] = INVALID_PLAYER_ID;
    PracticeRemoveEditObject(playerid);
    PracticeHideCatalogue(playerid);
    new empty[E_PRACTICE_POINT];
    PracticeDraft[playerid] = empty;
    return 1;
}

// Confirmed start/target/rule/delete/reset changes converge here. Persistence
// stays behind PracticeSaveAndNotify; native UPDATE, previews and cancelled drafts
// never write storage. This boundary persists only the selected zone mutation.
// The file repository serializes the registry; a future SQL repository owns its transaction.
stock PracticeConfigurationChanged(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    new reason[144];
    if (PracticeZoneEnabled[zone] && !PracticeConfigurationReady(zone, reason))
    {
        PracticeZoneEnabled[zone] = false;
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "La zona quedó inactiva porque su configuración está incompleta. Corrígela y vuelve a activarla.");
    }
    PracticeRevision[zone]++;
    PracticeRegistryRevision++;
    PracticeSavePending[zone] = true;
    PracticeSynchronizeIndicators(zone);
    for (new viewer = 0; viewer < MAX_PLAYERS; viewer++)
        if (PracticeSelectedZone[viewer] == zone && (PracticePreviewActive[viewer] || PracticeSetupView[viewer])) PracticeSynchronizeView(viewer);
    return PracticeSaveAndNotify(playerid, zone);
}

stock PracticeFreeTargetSlot(zone)
{
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        if (!PracticePoints[zone][slot][PracticePointSet]) return slot;
    return -1;
}

stock PracticeBeginDraft(playerid, slot, bool:create = false)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    new first = PracticeFirstStart(zone);
    if (!PracticeMayChange(playerid)) return 0;
    if (playerEditingObjectID[playerid] != -1)
    { SendClientMessage(playerid, COLOR_FADE3, "Termina primero la edición del objeto que tienes abierta."); return 0; }
    if (slot < 1 || slot > PRACTICE_MAX_TARGETS || (!create && !PracticePoints[zone][slot][PracticePointSet])) return 0;
    if (first < 0)
    { SendClientMessage(playerid, COLOR_FADE3, "Marca primero el punto de inicio."); return 0; }
    if (GetPlayerInterior(playerid) != PracticePoints[zone][first][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[zone][first][PracticePointWorld])
    { SendClientMessage(playerid, COLOR_FADE3, "La edición debe realizarse en el interior y mundo del puesto."); return 0; }
    if (!PracticeInSetupRange(playerid))
    { SendClientMessage(playerid, COLOR_FADE3, "Acércate al campo de tiro. Debes estar a menos de 100 metros de un punto de inicio para colocar blancos."); return 0; }
    PracticeDiscardDraft(playerid);
    PracticeEditorOwner[zone] = playerid;
    PracticeDraftSlot[playerid] = slot;
    PracticeDraftRevision[playerid] = PracticeRevision[zone];
    PracticeDraftNew[playerid] = create;
    PracticeDraftWizard[playerid] = create;
    if (create)
    {
        PracticeCapturePoint(playerid, PracticeDraft[playerid]);
        PracticeDraft[playerid][PracticePointModel] = PRACTICE_TARGET_MODEL;
        // Place the temporary object just ahead, then let the visual editor place it precisely.
        new Float:angle = PracticeDraft[playerid][PracticePointAngle];
        PracticeDraft[playerid][PracticePointX] += 3.5 * floatsin(-angle, degrees);
        PracticeDraft[playerid][PracticePointY] += 3.5 * floatcos(-angle, degrees);
        // Keep the initial editor object within the same final placement bounds.
        if (!PracticeTargetFitsStarts(zone, PracticeDraft[playerid]))
        {
            GetPlayerPos(playerid, PracticeDraft[playerid][PracticePointX], PracticeDraft[playerid][PracticePointY], PracticeDraft[playerid][PracticePointZ]);
            if (!PracticeTargetFitsStarts(zone, PracticeDraft[playerid]))
            {
                PracticeDraft[playerid][PracticePointX] = PracticePoints[zone][first][PracticePointX] + 3.5 * floatsin(-angle, degrees);
                PracticeDraft[playerid][PracticePointY] = PracticePoints[zone][first][PracticePointY] + 3.5 * floatcos(-angle, degrees);
                PracticeDraft[playerid][PracticePointZ] = PracticePoints[zone][first][PracticePointZ];
            }
        }
    }
    else PracticeDraft[playerid] = PracticePoints[zone][slot];
    PracticeEnsureSetupView(playerid);
    return 1;
}

stock bool:PracticeDraftCurrent(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return false;
    return PracticeHasDraft(playerid) && PracticeAdminAvailable(playerid) && !PracticeIsBusy(zone)
        && PracticeDraftRevision[playerid] == PracticeRevision[zone]
        && GetPlayerInterior(playerid) == PracticeDraft[playerid][PracticePointInterior]
        && GetPlayerVirtualWorld(playerid) == PracticeDraft[playerid][PracticePointWorld]
        // Start capture may create or relocate the station anywhere. The radius
        // belongs only to target drafts relative to an already marked start.
        && (PracticeDraftSlot[playerid] == 0 || PracticeInSetupRange(playerid));
}

stock PracticeCommitDraft(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    if (!PracticeDraftCurrent(playerid))
    {
        PracticeDiscardDraft(playerid);
        return 0;
    }
    new slot = PracticeDraftSlot[playerid];
    if (slot < 1 || slot > PRACTICE_MAX_TARGETS || !PracticePointValid(PracticeDraft[playerid])
        || PracticeModelIndex(PracticeDraft[playerid][PracticePointModel]) < 0
        || !PracticeTargetFitsStarts(zone, PracticeDraft[playerid]))
        return PracticeShowEditError(playerid);
    PracticePoints[zone][slot] = PracticeDraft[playerid];
    if (PracticeDraftFixTypes[playerid]) PracticeRules[zone][PracticeRandomTypes] = 0;
    PracticeDiscardDraft(playerid);
    PracticeConfigurationChanged(playerid);
    return PracticeShowTargets(playerid);
}

stock PracticeStartVisualEdit(playerid)
{
    if (!PracticeDraftCurrent(playerid) || PracticeDraftSlot[playerid] < 1) return 0;
    if (playerEditingObjectID[playerid] != -1)
        return SendClientMessage(playerid, COLOR_FADE3, "Termina primero el otro editor de objetos.");
    PracticeRemoveEditObject(playerid);
    PracticeHideCatalogue(playerid);
    PracticeEditObject[playerid] = PracticeCreatePrivateObject(playerid, PracticeDraft[playerid]);
    Streamer_Update(playerid, STREAMER_TYPE_OBJECT);
    if (!IsValidDynamicObject(PracticeEditObject[playerid])
        || !Streamer_IsItemVisible(playerid, STREAMER_TYPE_OBJECT, PracticeEditObject[playerid]))
    {
        PracticeRemoveEditObject(playerid);
        SendClientMessage(playerid, COLOR_FADE3, "No se pudo abrir el editor. Acércate al blanco y comprueba su posición.");
        return PracticeShowDraftMenu(playerid);
    }
    PracticeEnsureSetupView(playerid);
    SetPVarInt(playerid, "SP_ObjectEditor", 1);
    if (!EditDynamicObject(playerid, PracticeEditObject[playerid]))
    {
        PracticeRemoveEditObject(playerid);
        PracticeEnsureSetupView(playerid);
        return PracticeShowDraftMenu(playerid);
    }
    SendClientMessage(playerid, COLOR_GRAD1, "Mueve y gira el blanco. Confirmar guarda todos sus cambios. Cancelar conserva la configuración anterior.");
    return 1;
}

stock Float:PracticeNormalizeRotation(Float:angle)
{
    return angle - float(floatround(angle / 360.0, floatround_floor)) * 360.0;
}

stock PracticeHandleObjectEdit(playerid, STREAMER_TAG_OBJECT:objectid, response, Float:x, Float:y, Float:z,
    Float:rx, Float:ry, Float:rz)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    if (objectid == INVALID_STREAMER_ID || objectid != PracticeEditObject[playerid]) return 0;
    if (!PracticeDraftCurrent(playerid))
    {
        PracticeDiscardDraft(playerid);
        return 1;
    }
    if (response == EDIT_RESPONSE_CANCEL)
    {
        PracticeRemoveEditObject(playerid, false);
        PracticeDiscardDraft(playerid);
        return PracticeShowTargets(playerid);
    }
    // Updates are client previews only. FINAL coordinates are authoritative;
    // the last UPDATE can lag behind the actual released mouse position.
    if (response != EDIT_RESPONSE_FINAL) return 1;
    PracticeRemoveEditObject(playerid, false);
    if (!(x >= -20000.0 && x <= 20000.0 && y >= -20000.0 && y <= 20000.0 && z >= -20000.0 && z <= 20000.0
        && rx >= -36000.0 && rx <= 36000.0 && ry >= -36000.0 && ry <= 36000.0 && rz >= -36000.0 && rz <= 36000.0))
        return PracticeShowEditError(playerid);
    new candidate[E_PRACTICE_POINT];
    candidate = PracticeDraft[playerid];
    candidate[PracticePointX] = x; candidate[PracticePointY] = y; candidate[PracticePointZ] = z;
    candidate[PracticePointRotX] = PracticeNormalizeRotation(rx);
    candidate[PracticePointRotY] = PracticeNormalizeRotation(ry);
    candidate[PracticePointAngle] = PracticeNormalizeRotation(rz);
    if (!PracticePointValid(candidate) || !PracticeTargetFitsStarts(zone, candidate))
        return PracticeShowEditError(playerid);
    PracticeDraft[playerid] = candidate;
    return PracticeCommitDraft(playerid);
}

stock PracticeEditorTick()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        PracticeMaintainPausedMenu(playerid);
        new zone = PracticeSelectedZone[playerid];
        if (PracticeDraftSlot[playerid] >= 0 && (!PracticeDraftCurrent(playerid)
            || (PracticeEditObject[playerid] != INVALID_STREAMER_ID && !IsValidDynamicObject(PracticeEditObject[playerid]))))
            PracticeResetUI(playerid);
        if ((PracticePreviewActive[playerid] || PracticeSetupView[playerid]) && (!PracticeSelectedZoneValid(playerid) || !PracticeAdminAvailable(playerid) || PracticeIsBusy(zone)
            || !PracticeCountStarts(zone)
            || !PracticeInStartDimension(zone, playerid)
            || (PracticeSetupView[playerid] && !PracticeInSetupRange(playerid)))) PracticeHidePreview(playerid);
    }
    return 1;
}
