#if defined SP_START_SETUP_INCLUDED
    #endinput
#endif
#define SP_START_SETUP_INCLUDED

stock PracticeShowStarts(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    PracticeEnsureSetupView(playerid);
    new body[1400] = "Punto\tUbicación\nAñadir punto\tMarcar mi posición actual", line[128], name[PRACTICE_NAME_SIZE];
    PracticeStartRowCount[playerid] = 1;
    PracticeStartRows[playerid][0] = -1;
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
    {
        new slot = PracticeStartSlot(index);
        if (!PracticePoints[zone][slot][PracticePointSet]) continue;
        PracticePointDisplayName(PracticePoints[zone][slot], slot, name);
        format(line, sizeof(line), "\n%s\t%.1f, %.1f, %.1f", name, PracticePoints[zone][slot][PracticePointX],
            PracticePoints[zone][slot][PracticePointY], PracticePoints[zone][slot][PracticePointZ]);
        strcat(body, line, sizeof(body));
        PracticeStartRows[playerid][PracticeStartRowCount[playerid]++] = slot;
    }
    strcat(body, "\nCerrar menú\tContinuar con /practicaconfig", sizeof(body));
    PracticeStartRows[playerid][PracticeStartRowCount[playerid]++] = SP_CONFIG_CLOSE;
    return PracticeDialog(playerid, SP_MENU_STARTS, DIALOG_STYLE_TABLIST_HEADERS, "Puntos de inicio", body);
}

stock PracticeShowStart(playerid)
{
    new zone = PracticeSelectedZone[playerid], slot = PracticeSelectedStart[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    if (!PracticeIsStartSlot(slot) || !PracticePoints[zone][slot][PracticePointSet]) return PracticeShowStarts(playerid);
    PracticeEnsureSetupView(playerid);
    new body[400], name[PRACTICE_NAME_SIZE];
    PracticePointDisplayName(PracticePoints[zone][slot], slot, name);
    format(body, sizeof(body), "Ajuste\tDetalle\nNombre\t%s\nMarcar mi posición\tMover este punto a mi posición actual\nEliminar punto\tRequiere confirmación\nCerrar menú\tContinuar con /practicaconfig", name);
    return PracticeDialog(playerid, SP_MENU_START, DIALOG_STYLE_TABLIST_HEADERS, "Editar punto de inicio", body);
}

// With shared targets all starts must belong to the same physical range.
// Relocating the only start retains the legacy confirmation for dimension changes.
stock PracticeMarkStart(playerid, slot, bool:create = false)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid) || !PracticeMayChange(playerid)) return 0;
    if (!PracticeIsStartSlot(slot) || (!create && !PracticePoints[zone][slot][PracticePointSet])
        || (create && PracticePoints[zone][slot][PracticePointSet])) return PracticeShowStarts(playerid);
    PracticeDiscardDraft(playerid);
    PracticeSelectedStart[playerid] = slot;
    PracticeEditorOwner[zone] = playerid;
    PracticeDraftSlot[playerid] = 0; // Captured start draft, independent of its storage slot.
    PracticeDraftRevision[playerid] = PracticeRevision[zone];
    if (!create) PracticeDraft[playerid] = PracticePoints[zone][slot];
    PracticeCapturePoint(playerid, PracticeDraft[playerid]);
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
    {
        new other = PracticeStartSlot(index);
        if (other == slot || !PracticePoints[zone][other][PracticePointSet]) continue;
        if (!PracticeSameDimension(PracticePoints[zone][other], PracticeDraft[playerid]))
        {
            PracticeDiscardDraft(playerid);
            SendClientMessage(playerid, COLOR_FADE3, "Todos los puntos del campo deben estar en el mismo interior y mundo.");
            return PracticeShowStart(playerid);
        }
        new Float:dx = PracticePoints[zone][other][PracticePointX] - PracticeDraft[playerid][PracticePointX];
        new Float:dy = PracticePoints[zone][other][PracticePointY] - PracticeDraft[playerid][PracticePointY];
        new Float:dz = PracticePoints[zone][other][PracticePointZ] - PracticeDraft[playerid][PracticePointZ];
        if (dx*dx + dy*dy + dz*dz > PRACTICE_MAX_DISTANCE * PRACTICE_MAX_DISTANCE)
        {
            PracticeDiscardDraft(playerid);
            SendClientMessage(playerid, COLOR_FADE3, "Coloca el punto a menos de 100 metros de los demás puestos del campo.");
            return PracticeShowStart(playerid);
        }
    }
    if (create || PracticeCountStarts(zone) > 1)
        for (new target = 1; target <= PRACTICE_MAX_TARGETS; target++)
        {
            if (!PracticePoints[zone][target][PracticePointSet]) continue;
            if (!PracticeSameDimension(PracticeDraft[playerid], PracticePoints[zone][target])
                || !PracticeTargetDistanceValid(PracticeDraft[playerid], PracticePoints[zone][target]))
            {
                PracticeDiscardDraft(playerid);
                SendClientMessage(playerid, COLOR_FADE3, "Cada blanco debe quedar entre 3 y 100 metros del nuevo punto, en el mismo interior y mundo.");
                return PracticeShowStart(playerid);
            }
        }
    if (!create && PracticeCountTargets(zone)
        && !PracticeSameDimension(PracticePoints[zone][slot], PracticeDraft[playerid]))
        return PracticeDialog(playerid, SP_MENU_START_CONFIRM, DIALOG_STYLE_MSGBOX, "Cambiar interior o mundo",
            "El nuevo punto está en otro interior o mundo.\nConfirmar eliminará todos los blancos anteriores.\n\nLas reglas del ejercicio se conservan.", "Confirmar", "Cancelar");
    return PracticeCommitStart(playerid);
}

stock PracticeCommitStart(playerid)
{
    new zone = PracticeSelectedZone[playerid], slot = PracticeSelectedStart[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    if (!PracticeDraftCurrent(playerid) || PracticeDraftSlot[playerid] != 0 || !PracticeIsStartSlot(slot))
    { PracticeDiscardDraft(playerid); return PracticeShowStarts(playerid); }
    if (!PracticePointValid(PracticeDraft[playerid]))
    { PracticeDiscardDraft(playerid); return PracticeShowStarts(playerid); }
    if (PracticePoints[zone][slot][PracticePointSet]
        && !PracticeSameDimension(PracticePoints[zone][slot], PracticeDraft[playerid]))
    {
        new empty[E_PRACTICE_POINT];
        for (new target = 1; target <= PRACTICE_MAX_TARGETS; target++) PracticePoints[zone][target] = empty;
    }
    PracticePoints[zone][slot] = PracticeDraft[playerid];
    PracticeDiscardDraft(playerid);
    PracticeConfigurationChanged(playerid);
    return PracticeShowStart(playerid);
}

stock PracticeHandleStartMenu(playerid, page, row, const inputtext[])
{
    new zone = PracticeSelectedZone[playerid], slot = PracticeSelectedStart[playerid];
    if (page == SP_MENU_STARTS)
    {
        if (row < 0 || row >= PracticeStartRowCount[playerid]) return PracticeShowStarts(playerid);
        slot = PracticeStartRows[playerid][row];
        if (slot >= 0)
        {
            PracticeSelectedStart[playerid] = slot;
            return PracticeShowStart(playerid);
        }
        for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
        {
            slot = PracticeStartSlot(index);
            if (!PracticePoints[zone][slot][PracticePointSet]) return PracticeMarkStart(playerid, slot, true);
        }
        SendClientMessage(playerid, COLOR_FADE3, "Este campo admite hasta 10 puntos de inicio.");
        return PracticeShowStarts(playerid);
    }
    if (!PracticeIsStartSlot(slot) || !PracticePoints[zone][slot][PracticePointSet]) return PracticeShowStarts(playerid);
    switch (page)
    {
        case SP_MENU_START:
        {
            switch (row)
            {
                case 0: return PracticeDialog(playerid, SP_MENU_START_NAME, DIALOG_STYLE_INPUT, "Nombre del punto",
                    "Escribe un nombre de hasta 32 caracteres.\nUsa - para recuperar el nombre automático.", "Guardar", "Volver");
                case 1: return PracticeMarkStart(playerid, slot);
                case 2: return PracticeDialog(playerid, SP_MENU_START_DELETE, DIALOG_STYLE_MSGBOX, "Eliminar punto",
                    "Se eliminará este punto de inicio. Los blancos y las reglas se conservarán.\nSi es el último punto, el campo quedará inactivo.\n\n¿Quieres continuar?", "Eliminar", "Cancelar");
            }
        }
        case SP_MENU_START_NAME:
        {
            if (!inputtext[0] || !PracticeNameValid(inputtext))
            {
                SendClientMessage(playerid, COLOR_FADE3, "Usa de 1 a 32 caracteres, sin saltos de línea ni códigos de color.");
                return PracticeShowStart(playerid);
            }
            if (!strcmp(inputtext, "-")) PracticePoints[zone][slot][PracticePointName][0] = EOS;
            else format(PracticePoints[zone][slot][PracticePointName], PRACTICE_NAME_SIZE, "%s", inputtext);
            PracticeConfigurationChanged(playerid);
            return PracticeShowStart(playerid);
        }
        case SP_MENU_START_DELETE:
        {
            new empty[E_PRACTICE_POINT];
            PracticePoints[zone][slot] = empty;
            PracticeConfigurationChanged(playerid);
            return PracticeShowStarts(playerid);
        }
    }
    return PracticeShowStart(playerid);
}
