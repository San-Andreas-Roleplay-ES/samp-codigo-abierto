#if defined SP_ZONE_SETUP_INCLUDED
    #endinput
#endif
#define SP_ZONE_SETUP_INCLUDED

// List rows capture stable IDs, never their ordinal position in the registry.
// Only this administrative surface creates/renames/activates/archives zones.
stock PracticeAddZoneRow(playerid, id, body[], size, const label[], const detail[] = "")
{
    new line[160];
    format(line, sizeof(line), "\n%s\t%s", label, detail);
    strcat(body, line, size);
    PracticeZoneListRows[playerid][PracticeZoneListCount[playerid]++] = id;
    return 1;
}

stock PracticeShowZones(playerid, page = 0)
{
    if (!PracticeIsAdministrator(playerid)) return 0;
    PracticeResetUI(playerid);
    new total;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
        if (PracticeZoneExists(zone) && !PracticeZoneArchived[zone]) total++;
    new pages = total ? (total - 1) / PRACTICE_ZONE_PAGE_SIZE + 1 : 1;
    if (page < 0) page = 0;
    if (page >= pages) page = pages - 1;
    PracticeZoneListPage[playerid] = page;
    PracticeZoneListCount[playerid] = 0;
    static body[2000];
    format(body, sizeof(body), "Zona\tEstado");
    new label[80], detail[64], title[64], index;
    PracticeAddZoneRow(playerid, SP_ZONE_CREATE, body, sizeof(body), "Crear zona", "Nueva configuración");
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        if (!PracticeZoneExists(zone) || PracticeZoneArchived[zone]) continue;
        if (index++ < page * PRACTICE_ZONE_PAGE_SIZE) continue;
        if (index > (page + 1) * PRACTICE_ZONE_PAGE_SIZE) break;
        format(label, sizeof(label), "#%d: %s", PracticeZoneID[zone], PracticeZoneName[zone]);
        format(detail, sizeof(detail), "%s | %d/10 blancos", PracticeZoneEnabled[zone] ? ("Activa") : ("Inactiva"), PracticeCountTargets(zone));
        PracticeAddZoneRow(playerid, PracticeZoneID[zone], body, sizeof(body), label, detail);
    }
    if (page > 0) PracticeAddZoneRow(playerid, SP_ZONE_PREVIOUS, body, sizeof(body), "Página anterior");
    if (page + 1 < pages) PracticeAddZoneRow(playerid, SP_ZONE_NEXT, body, sizeof(body), "Página siguiente");
    if (PracticeRegistrySavePending) PracticeAddZoneRow(playerid, SP_ZONE_RETRY_SAVE, body, sizeof(body), "Reintentar guardado", "Cambios sin guardar");
    format(title, sizeof(title), "Zonas de tiro (%d/%d)", page + 1, pages);
    return PracticeDialog(playerid, SP_MENU_ZONES, DIALOG_STYLE_TABLIST_HEADERS, title, body, "Seleccionar", "Cerrar");
}

stock PracticeHandleZoneList(playerid, page, response, listitem, const input[])
{
    if (!PracticeIsAdministrator(playerid))
    { PracticeResetUI(playerid); return 1; }
    if (!response) return page == SP_MENU_CREATE_ZONE ? PracticeShowZones(playerid, PracticeZoneListPage[playerid]) : 1;
    if (PracticeZoneListRevision[playerid] != PracticeRegistryRevision)
    {
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "El listado cambió. Revisa las zonas actualizadas.");
        return PracticeShowZones(playerid, PracticeZoneListPage[playerid]);
    }
    if (page == SP_MENU_CREATE_ZONE)
    {
        if (!PracticeAdminAvailable(playerid) || PracticeStorageBlocked)
        {
            SendClientMessage(playerid, COLOR_FADE3, "Debes estar vivo, a pie y tener un archivo de zonas válido para crear una zona.");
            return PracticeShowZones(playerid);
        }
        if (!PracticeZoneNameValid(input))
        {
            SendClientMessage(playerid, COLOR_FADE3, "Usa de 1 a 48 caracteres, sin códigos de color ni saltos de línea.");
            return PracticeDialog(playerid, SP_MENU_CREATE_ZONE, DIALOG_STYLE_INPUT, "Crear zona de tiro",
                "Nombre de la nueva zona (hasta 48 caracteres).\nSe creará inactiva; configura su inicio y blancos antes de activarla.", "Crear", "Volver");
        }
        new zone = PracticeCreateZone(input);
        if (zone < 0)
        {
            SendClientMessage(playerid, COLOR_FADE3, "No hay capacidad para otra zona. El límite incluye las zonas archivadas.");
            return PracticeShowZones(playerid);
        }
        PracticeBindZone(playerid, zone);
        PracticeConfigurationChanged(playerid);
        return PracticeShowMain(playerid);
    }
    if (listitem < 0 || listitem >= PracticeZoneListCount[playerid]) return PracticeShowZones(playerid, PracticeZoneListPage[playerid]);
    new action = PracticeZoneListRows[playerid][listitem];
    switch (action)
    {
        case SP_ZONE_CREATE: return PracticeDialog(playerid, SP_MENU_CREATE_ZONE, DIALOG_STYLE_INPUT, "Crear zona de tiro",
            "Nombre de la nueva zona (hasta 48 caracteres).\nSe creará inactiva; configura su inicio y blancos antes de activarla.", "Crear", "Volver");
        case SP_ZONE_PREVIOUS: return PracticeShowZones(playerid, PracticeZoneListPage[playerid] - 1);
        case SP_ZONE_NEXT: return PracticeShowZones(playerid, PracticeZoneListPage[playerid] + 1);
        case SP_ZONE_RETRY_SAVE:
        {
            if (PracticeAdminAvailable(playerid) && !PracticeStorageBlocked && PracticeRegistrySavePending)
                PracticeSaveAndNotify(playerid);
            return PracticeShowZones(playerid, PracticeZoneListPage[playerid]);
        }
    }
    new zone = PracticeFindZone(action);
    if (!PracticeBindZone(playerid, zone)) return PracticeShowZones(playerid);
    return PracticeShowMain(playerid);
}

stock PracticeToggleZone(playerid)
{
    if (!PracticeMayChange(playerid)) return PracticeShowMain(playerid);
    new zone = PracticeSelectedZone[playerid], reason[144];
    if (!PracticeZoneEnabled[zone] && !PracticeConfigurationReady(zone, reason))
    {
        SendClientMessage(playerid, COLOR_FADE3, reason);
        return PracticeShowMain(playerid);
    }
    PracticeZoneEnabled[zone] = !PracticeZoneEnabled[zone];
    PracticeConfigurationChanged(playerid);
    return PracticeShowMain(playerid);
}

stock PracticeArchiveZone(playerid)
{
    if (!PracticeMayChange(playerid)) return PracticeShowMain(playerid);
    new zone = PracticeSelectedZone[playerid];
    // Consume UI and native resources against the old, still valid binding first.
    for (new viewer = 0; viewer < MAX_PLAYERS; viewer++)
        if (PracticeSelectedZone[viewer] == zone)
        {
            PracticeResetUI(viewer);
            PracticeSelectedZone[viewer] = -1;
            PracticeSelectedZoneID[viewer] = 0;
        }
    PracticeZoneEnabled[zone] = false;
    PracticeZoneArchived[zone] = true;
    PracticeSynchronizeIndicators(zone);
    PracticeRevision[zone]++;
    PracticeRegistryRevision++;
    PracticeSavePending[zone] = true;
    PracticeSaveAndNotify(playerid, zone);
    return PracticeShowZones(playerid);
}

stock PracticeAdminCommand(playerid, const params[])
{
    if (!PracticeIsAdministrator(playerid))
    {
        PracticeResetUI(playerid);
        return SendClientMessage(playerid, COLOR_FADE3, "Necesitas ser administrador RCON para configurar las zonas.");
    }
    if (PracticeEditObject[playerid] != INVALID_STREAMER_ID)
        return SendClientMessage(playerid, COLOR_GRAD1, "Confirma o cancela el editor visual antes de volver al menú.");
    if (isnull(params))
    {
        if (PracticePausedMenu[playerid] != SP_MENU_NONE) return PracticeResumeMenu(playerid);
        return PracticeShowZones(playerid);
    }
    new id;
    if (!PracticeParseZoneID(params, id))
        return SendClientMessage(playerid, COLOR_GRAD1, "Uso: /practicaconfig o /practicaconfig <ID de zona>.");
    new zone = PracticeFindZone(id);
    if (!PracticeZoneExists(zone) || PracticeZoneArchived[zone])
        return SendClientMessage(playerid, COLOR_FADE3, "No existe una zona disponible con ese ID.");
    if (PracticeSelectedZoneValid(playerid) && PracticeSelectedZone[playerid] == zone
        && PracticePausedMenu[playerid] != SP_MENU_NONE) return PracticeResumeMenu(playerid);
    if (PracticeSelectedZoneValid(playerid) && PracticeSelectedZone[playerid] == zone && PracticeHasDraft(playerid))
    {
        PracticeHideCatalogue(playerid);
        if (PracticeDraftSlot[playerid] == 0) { PracticeDiscardDraft(playerid); return PracticeShowMain(playerid); }
        return PracticeShowDraftMenu(playerid);
    }
    if (!PracticeSelectedZoneValid(playerid) || PracticeSelectedZone[playerid] != zone) PracticeBindZone(playerid, zone);
    PracticeHideCatalogue(playerid);
    return PracticeShowMain(playerid);
}

stock bool:PracticeNearZone(zone, playerid)
{
    return IsPlayerConnected(playerid) && PracticeCanUseStation(zone, playerid) && PracticeZoneEnabled[zone]
        && PracticeNearestStart(zone, playerid, PRACTICE_START_RADIUS) >= 0;
}

stock PracticeShowPlayerZones(playerid, bool:help = false, page = 0)
{
    new count, first = -1;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
        if (PracticeNearZone(zone, playerid)) { count++; if (first < 0) first = zone; }
    if (!count) return SendClientMessage(playerid, COLOR_FADE3, "Acércate al punto de inicio de una zona activa para practicar.");
    if (count == 1)
    {
        if (help) return PracticeShowPlayerHelp(first, playerid);
        return PracticeStart(first, playerid);
    }
    PracticeResetUI(playerid);
    new pages = (count - 1) / PRACTICE_ZONE_PAGE_SIZE + 1;
    if (page < 0) page = 0;
    if (page >= pages) page = pages - 1;
    PracticeZoneListPage[playerid] = page;
    PracticeZoneListHelp[playerid] = help;
    PracticeZoneListCount[playerid] = 0;
    static body[1800];
    format(body, sizeof(body), "Zona\tBlancos");
    new label[80], detail[32], title[64], index;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        if (!PracticeNearZone(zone, playerid)) continue;
        if (index++ < page * PRACTICE_ZONE_PAGE_SIZE) continue;
        if (index > (page + 1) * PRACTICE_ZONE_PAGE_SIZE) break;
        format(label, sizeof(label), "#%d: %s", PracticeZoneID[zone], PracticeZoneName[zone]);
        format(detail, sizeof(detail), "%d blancos%s", PracticeCountTargets(zone), PracticeIsBusy(zone) ? (" | Ocupada") : (""));
        PracticeAddZoneRow(playerid, PracticeZoneID[zone], body, sizeof(body), label, detail);
    }
    if (page > 0) PracticeAddZoneRow(playerid, SP_ZONE_PREVIOUS, body, sizeof(body), "Página anterior");
    if (page + 1 < pages) PracticeAddZoneRow(playerid, SP_ZONE_NEXT, body, sizeof(body), "Página siguiente");
    format(title, sizeof(title), "Elegir zona cercana (%d/%d)", page + 1, pages);
    return PracticeDialog(playerid, SP_MENU_PLAYER_ZONES, DIALOG_STYLE_TABLIST_HEADERS, title, body, "Seleccionar", "Cerrar");
}

stock PracticeHandlePlayerZones(playerid, response, listitem)
{
    if (!response || !IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 1;
    if (listitem < 0 || listitem >= PracticeZoneListCount[playerid]) return 1;
    new id = PracticeZoneListRows[playerid][listitem];
    if (id == SP_ZONE_PREVIOUS || id == SP_ZONE_NEXT)
        return PracticeShowPlayerZones(playerid, PracticeZoneListHelp[playerid], PracticeZoneListPage[playerid] + (id == SP_ZONE_NEXT ? 1 : -1));
    new zone = PracticeFindZone(id);
    if (!PracticeNearZone(zone, playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "La zona ya no está disponible aquí. Usa /practica de nuevo.");
    if (PracticeZoneListHelp[playerid]) return PracticeShowPlayerHelp(zone, playerid);
    // Start rechecks occupancy, permissions, weapon, player state and position.
    return PracticeStart(zone, playerid);
}

stock PracticePlayerCommand(playerid, const params[])
{
    new zone = PracticeGetPlayerZone(playerid);
    if (!isnull(params) && !strcmp(params, "cancelar", true))
    {
        if (zone >= 0) return PracticeEnd(zone, false, "cancelada por el participante.");
        return SendClientMessage(playerid, COLOR_FADE3, "No tienes una práctica en curso.");
    }
    if (PracticeEditObject[playerid] != INVALID_STREAMER_ID || PracticeHasDraft(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "Confirma o descarta tu edición antes de abrir una práctica.");
    if (!isnull(params) && !strcmp(params, "ayuda", true))
    {
        if (zone >= 0) return PracticeShowPlayerHelp(zone, playerid);
        return PracticeShowPlayerZones(playerid, true);
    }
    if (zone >= 0) return SendClientMessage(playerid, COLOR_FADE3, "Ya tienes una práctica en curso. Usa /practica cancelar para terminarla.");
    if (isnull(params)) return PracticeShowPlayerZones(playerid);
    new id;
    if (!PracticeParseZoneID(params, id))
        return SendClientMessage(playerid, COLOR_GRAD1, "Uso: /practica [ID de zona | ayuda | cancelar].");
    zone = PracticeFindZone(id);
    if (!PracticeZoneExists(zone) || PracticeZoneArchived[zone])
        return SendClientMessage(playerid, COLOR_FADE3, "No existe una zona disponible con ese ID.");
    return PracticeStart(zone, playerid);
}
