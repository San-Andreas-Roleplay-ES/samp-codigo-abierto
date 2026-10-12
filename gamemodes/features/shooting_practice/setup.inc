#if defined SP_SETUP_INCLUDED
    #endinput
#endif
#define SP_SETUP_INCLUDED

enum {
    SP_MENU_NONE, SP_MENU_MAIN, SP_MENU_TARGETS, SP_MENU_DRAFT,
    SP_MENU_NAME, SP_MENU_BEHAVIOUR, SP_MENU_RULES, SP_MENU_NUMBER, SP_MENU_WEAPON,
    SP_MENU_DELETE, SP_MENU_RESET, SP_MENU_START_CONFIRM, SP_MENU_EDIT_ERROR, SP_MENU_RULE_INFO,
    SP_MENU_ZONES, SP_MENU_CREATE_ZONE, SP_MENU_RENAME_ZONE, SP_MENU_ARCHIVE_ZONE, SP_MENU_PLAYER_ZONES,
    SP_MENU_STARTS, SP_MENU_START, SP_MENU_START_NAME, SP_MENU_START_DELETE
}
enum {
    SP_MAIN_START_POINT, SP_MAIN_TARGETS, SP_MAIN_RULES, SP_MAIN_PREVIEW,
    SP_MAIN_START_PRACTICE, SP_MAIN_RESET, SP_MAIN_RETRY_SAVE, SP_MAIN_CANCEL_PRACTICE,
    SP_MAIN_RENAME_ZONE, SP_MAIN_TOGGLE_ZONE, SP_MAIN_ARCHIVE_ZONE, SP_MAIN_INDICATORS
}
new PracticeMainRows[MAX_PLAYERS][12], PracticeMainRowCount[MAX_PLAYERS];
new PracticeMenuPage[MAX_PLAYERS], PracticeMenuRevision[MAX_PLAYERS], PracticeMenuZoneID[MAX_PLAYERS];
new PracticePausedMenu[MAX_PLAYERS], PracticePausedZoneID[MAX_PLAYERS], PracticePausedRevision[MAX_PLAYERS];
#define SP_CONFIG_CLOSE (-2)
new PracticeZoneListRows[MAX_PLAYERS][PRACTICE_ZONE_PAGE_SIZE + 4], PracticeZoneListCount[MAX_PLAYERS];
new PracticeZoneListPage[MAX_PLAYERS], PracticeZoneListRevision[MAX_PLAYERS];
new bool:PracticeZoneListHelp[MAX_PLAYERS];
enum { SP_ZONE_CREATE = -1, SP_ZONE_PREVIOUS = -2, SP_ZONE_NEXT = -3, SP_ZONE_RETRY_SAVE = -4 }
new PracticeSelectedStart[MAX_PLAYERS] = {-1, ...};
new PracticeStartRows[MAX_PLAYERS][PRACTICE_MAX_STARTS + 2], PracticeStartRowCount[MAX_PLAYERS];
new PracticeMenuRows[MAX_PLAYERS][PRACTICE_MAX_TARGETS + 2], PracticeMenuRowCount[MAX_PLAYERS];
new E_PRACTICE_RULE:PracticeRuleField[MAX_PLAYERS];

stock PracticeDialog(playerid, page, style, const title[], const body[], const accept[] = "Seleccionar", const cancel[] = "Volver")
{
    PracticePausedMenu[playerid] = SP_MENU_NONE;
    PracticePausedZoneID[playerid] = 0;
    PracticeMenuPage[playerid] = page;
    PracticeMenuZoneID[playerid] = PracticeSelectedZoneID[playerid];
    PracticeMenuRevision[playerid] = PracticeSelectedZoneValid(playerid) ? PracticeRevision[PracticeSelectedZone[playerid]] : -1;
    PracticeZoneListRevision[playerid] = PracticeRegistryRevision;
    return Dialog_Show(playerid, SP_Menu, style, title, body, accept, cancel);
}

stock PracticeResetUI(playerid)
{
    if (PracticeMenuPage[playerid] != SP_MENU_NONE && IsPlayerConnected(playerid)) HidePlayerDialog(playerid);
    PracticeMenuPage[playerid] = PracticePausedMenu[playerid] = SP_MENU_NONE;
    PracticePausedZoneID[playerid] = 0;
    PracticePausedRevision[playerid] = 0;
    PracticeMenuRowCount[playerid] = PracticeMainRowCount[playerid] = PracticeZoneListCount[playerid] = PracticeStartRowCount[playerid] = 0;
    PracticeSelectedStart[playerid] = -1;
    PracticeDiscardDraft(playerid);
    PracticeHidePreview(playerid);
    return 1;
}


stock PracticePauseMenu(playerid, page)
{
    if (!PracticeSelectedZoneValid(playerid) || !PracticeAdminAvailable(playerid))
    { PracticeResetUI(playerid); return 1; }
    if (page == SP_MENU_DRAFT && !PracticeDraftCurrent(playerid))
    { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
    PracticePausedMenu[playerid] = page;
    PracticePausedZoneID[playerid] = PracticeSelectedZoneID[playerid];
    PracticePausedRevision[playerid] = PracticeRevision[PracticeSelectedZone[playerid]];
    PracticeMenuPage[playerid] = SP_MENU_NONE;
    // Hide only the modal. The owned draft and view remain available while walking.
    HidePlayerDialog(playerid);
    SendClientMessage(playerid, COLOR_GRAD1, "Menú cerrado. Usa /practicaconfig para continuar la configuración.");
    return 1;
}

stock PracticeMaintainPausedMenu(playerid)
{
    if (PracticePausedMenu[playerid] != SP_MENU_NONE
        && (!PracticeSelectedZoneValid(playerid) || !PracticeAdminAvailable(playerid)))
        PracticeResetUI(playerid);
    return 1;
}

stock PracticeResumeMenu(playerid)
{
    new page = PracticePausedMenu[playerid], zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid) || !PracticeAdminAvailable(playerid)
        || PracticePausedZoneID[playerid] != PracticeSelectedZoneID[playerid])
    { PracticeResetUI(playerid); return PracticeShowZones(playerid); }
    new bool:changed = PracticePausedRevision[playerid] != PracticeRevision[zone];
    PracticePausedMenu[playerid] = SP_MENU_NONE;
    PracticePausedZoneID[playerid] = 0;
    if (changed)
    {
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "La configuración cambió. Se muestra la sección actualizada.");
        if (page == SP_MENU_START)
        {
            PracticeSelectedStart[playerid] = -1;
            return PracticeShowStarts(playerid);
        }
    }
    switch (page)
    {
        case SP_MENU_TARGETS: return PracticeShowTargets(playerid);
        case SP_MENU_STARTS: return PracticeShowStarts(playerid);
        case SP_MENU_START: return PracticeShowStart(playerid);
        case SP_MENU_DRAFT:
        {
            if (!PracticeDraftCurrent(playerid))
            { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
            return PracticeShowDraftMenu(playerid);
        }
    }
    return PracticeShowMain(playerid);
}

stock PracticeAddMainRow(playerid, action, body[], size, const label[], const detail[] = "")
{
    new line[256];
    format(line, sizeof(line), "\n%s\t%s", label, detail);
    strcat(body, line, size);
    PracticeMainRows[playerid][PracticeMainRowCount[playerid]++] = action;
    return 1;
}

stock PracticeShowMain(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return PracticeShowZones(playerid);
    PracticeLeaveSetupView(playerid);
    static body[1300];
    format(body, sizeof(body), "Opción\tDetalle");
    new detail[96], weapon[32], title[72];
    PracticeMainRowCount[playerid] = 0;
    PracticeWeaponsSummary(PracticeRules[zone][PracticeWeaponMask], weapon);
    format(detail, sizeof(detail), "%d puntos configurados", PracticeCountStarts(zone));
    PracticeAddMainRow(playerid, SP_MAIN_START_POINT, body, sizeof(body), "Puntos de inicio", detail);
    format(detail, sizeof(detail), "%d blancos de tiro, %d protegidos", PracticeCountShootTargets(zone), PracticeCountNoShootTargets(zone));
    PracticeAddMainRow(playerid, SP_MAIN_TARGETS, body, sizeof(body), "Blancos", detail);
    PracticeAddMainRow(playerid, SP_MAIN_RULES, body, sizeof(body), "Reglas del ejercicio", weapon);
    PracticeAddMainRow(playerid, SP_MAIN_PREVIEW, body, sizeof(body),
        PracticePreviewActive[playerid] ? ("Ocultar vista previa") : ("Vista previa"), "Ver puestos y blancos");
    if (PracticeIsBusy(zone) && PracticeSession[zone][PracticePlayer] == playerid)
        PracticeAddMainRow(playerid, SP_MAIN_CANCEL_PRACTICE, body, sizeof(body), "Cancelar práctica");
    else if (PracticeCanStart(zone, playerid))
        PracticeAddMainRow(playerid, SP_MAIN_START_PRACTICE, body, sizeof(body), "Iniciar práctica");
    PracticeAddMainRow(playerid, SP_MAIN_RESET, body, sizeof(body), "Reiniciar configuración", "Borrar el campo de tiro");
    PracticeAddMainRow(playerid, SP_MAIN_INDICATORS, body, sizeof(body), "Indicadores de inicio",
        PracticeZoneIndicators[zone] ? ("Pistola en cada punto") : ("Desactivados"));
    if (PracticeSavePending[zone])
        PracticeAddMainRow(playerid, SP_MAIN_RETRY_SAVE, body, sizeof(body), "Reintentar guardado", "Cambios sin guardar");
    PracticeAddMainRow(playerid, SP_MAIN_RENAME_ZONE, body, sizeof(body), "Nombre de la zona", PracticeZoneName[zone]);
    PracticeAddMainRow(playerid, SP_MAIN_TOGGLE_ZONE, body, sizeof(body), PracticeZoneEnabled[zone] ? ("Desactivar zona") : ("Activar zona"),
        PracticeZoneEnabled[zone] ? ("Disponible para practicar") : ("Cerrada para practicar"));
    PracticeAddMainRow(playerid, SP_MAIN_ARCHIVE_ZONE, body, sizeof(body), "Archivar zona", "Retirar del listado y del uso");
    format(title, sizeof(title), "Zona #%d: %s", PracticeZoneID[zone], PracticeZoneName[zone]);
    return PracticeDialog(playerid, SP_MENU_MAIN, DIALOG_STYLE_TABLIST_HEADERS, title, body, "Seleccionar", "Zonas");
}

stock PracticeShowTargets(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    PracticeEnsureSetupView(playerid);
    new body[1400] = "Blanco\tObjeto\tComportamiento\nAñadir blanco\tElegir y colocar\t", line[130], name[PRACTICE_NAME_SIZE], modelName[24];
    PracticeMenuRowCount[playerid] = 1;
    PracticeMenuRows[playerid][0] = 0;
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        if (!PracticePoints[zone][slot][PracticePointSet]) continue;
        PracticePointDisplayName(PracticePoints[zone][slot], slot, name);
        new model = PracticeModelIndex(PracticePoints[zone][slot][PracticePointModel]);
        if (model >= 0) format(modelName, sizeof(modelName), "%s", PracticeModels[model][PracticeModelName]);
        else format(modelName, sizeof(modelName), "Modelo inválido");
        format(line, sizeof(line), "\n%s\t%s\t%s", name,
            modelName,
            PracticePoints[zone][slot][PracticePointNoShoot] ? ("No disparar") : ("Disparar"));
        strcat(body, line, sizeof(body));
        PracticeMenuRows[playerid][PracticeMenuRowCount[playerid]++] = slot;
    }
    strcat(body, "\nCerrar menú\tContinuar con /practicaconfig\t", sizeof(body));
    PracticeMenuRows[playerid][PracticeMenuRowCount[playerid]++] = SP_CONFIG_CLOSE;
    return PracticeDialog(playerid, SP_MENU_TARGETS, DIALOG_STYLE_TABLIST_HEADERS, "Blancos", body);
}

stock PracticeShowDraftMenu(playerid)
{
    if (!PracticeHasDraft(playerid) || PracticeDraftSlot[playerid] < 1) return PracticeShowTargets(playerid);
    PracticeEnsureSetupView(playerid);
    new name[PRACTICE_NAME_SIZE], title[64], body[700], modelName[24];
    PracticePointDisplayName(PracticeDraft[playerid], PracticeDraftSlot[playerid], name);
    new model = PracticeModelIndex(PracticeDraft[playerid][PracticePointModel]);
    if (model >= 0) format(modelName, sizeof(modelName), "%s", PracticeModels[model][PracticeModelName]);
    else format(modelName, sizeof(modelName), "Elegir objeto");
    format(title, sizeof(title), "%s", PracticeDraftNew[playerid] ? ("Nuevo blanco") : ("Editar blanco"));
    format(body, sizeof(body), "Ajuste\tValor\nNombre\t%s\nObjeto\t%s\nComportamiento\t%s\nMover y orientar\tEditor visual",
        name, modelName,
        PracticeDraft[playerid][PracticePointNoShoot] ? ("Blanco protegido (no disparar)") : ("Blanco de tiro (disparar)"));
    if (!PracticeDraftNew[playerid]) strcat(body, "\nDuplicar blanco\tColocar una copia\nEliminar blanco\tRequiere confirmación", sizeof(body));
    strcat(body, "\nCerrar menú\tContinuar con /practicaconfig", sizeof(body));
    strcat(body, "\nGuardar cambios\tConfirmar esta configuración", sizeof(body));
    return PracticeDialog(playerid, SP_MENU_DRAFT, DIALOG_STYLE_TABLIST_HEADERS, title, body, "Seleccionar", "Descartar");
}

stock PracticeShowBehaviour(playerid)
{
    return PracticeDialog(playerid, SP_MENU_BEHAVIOUR, DIALOG_STYLE_LIST, "Tipo de blanco",
        "Blanco de tiro (disparar)\nBlanco protegido (no disparar)");
}

stock PracticeShowEditError(playerid)
{
    PracticeEnsureSetupView(playerid);
    return PracticeDialog(playerid, SP_MENU_EDIT_ERROR, DIALOG_STYLE_MSGBOX, "Revisar posición",
        "El blanco debe quedar entre 3 y 100 metros de cada punto de inicio, en el mismo interior y mundo.\nLa posición y la orientación deben ser válidas.\n\nLa configuración guardada se ha conservado.", "Editar otra vez", "Descartar");
}

stock PracticeShowRules(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    new body[640], weapon[32];
    PracticeWeaponsSummary(PracticeRules[zone][PracticeWeaponMask], weapon);
    format(body, sizeof(body), "Regla\tValor\nArmas permitidas\t%s\nPuntos mínimos\t%d\nErrores máximos\t%d\nPreparación\t%d segundos\nTiempo por blanco\t%.1f segundos\nIntervalo\t%.1f segundos\nOrden de aparición\t%s\nDistribución de blancos\t%s",
        weapon, PracticeRules[zone][PracticePassPoints], PracticeRules[zone][PracticeMaxErrors],
        PracticeRules[zone][PracticePreparation], float(PracticeRules[zone][PracticeExposure]) / 1000.0, float(PracticeRules[zone][PracticeInterval]) / 1000.0,
        PracticeRules[zone][PracticeRandomOrder] ? ("Aleatorio") : ("Fijo"),
        PracticeRules[zone][PracticeRandomTypes] ? ("Aleatoria entre posiciones") : ("Fija por posición"));
    return PracticeDialog(playerid, SP_MENU_RULES, DIALOG_STYLE_TABLIST_HEADERS, "Reglas del ejercicio", body);
}

stock PracticeShowWeapons(playerid)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    new body[1024] = "Arma\tSelecciona para permitir o quitar", line[96];
    for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
    {
        format(line, sizeof(line), "\n%s\t%s", PracticeWeaponNames[weaponid - PRACTICE_WEAPON_FIRST],
            PracticeWeaponAllowed(weaponid, PracticeRules[zone][PracticeWeaponMask]) ? ("Permitida") : ("No permitida"));
        strcat(body, line, sizeof(body));
    }
    return PracticeDialog(playerid, SP_MENU_WEAPON, DIALOG_STYLE_TABLIST_HEADERS, "Armas permitidas", body, "Cambiar", "Volver");
}

stock PracticeShowRuleInput(playerid, E_PRACTICE_RULE:rule)
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return 0;
    PracticeRuleField[playerid] = rule;
    new title[64], body[800], help[560], minimum, maximum;
    switch (rule)
    {
        case PracticePassPoints:
        {
            format(title, sizeof(title), "Puntos mínimos"); minimum = 1; maximum = PracticeCountShootTargets(zone);
            if (!maximum)
                return PracticeDialog(playerid, SP_MENU_RULE_INFO, DIALOG_STYLE_MSGBOX, title,
                    "Primero añade al menos un blanco de tipo Blanco de tiro (disparar).\nCada blanco de tiro permite conseguir un punto.\nDespués podrás elegir cuántos puntos netos se necesitan para aprobar.", "Volver", "");
            format(help, sizeof(help), "Puntuación neta necesaria para aprobar: aciertos menos errores de identificación.\nCada blanco de tiro impactado suma 1. Disparar ante un blanco protegido resta 1.\nUn blanco de tiro que desaparece sin impacto no suma puntos.\nEjemplo: 4 aciertos y 1 error dejan 3 puntos netos.\nTambién se debe respetar el máximo de errores.\n\nCon los blancos de tiro actuales se pueden obtener hasta %d puntos.", maximum);
        }
        case PracticeMaxErrors:
        {
            format(title, sizeof(title), "Errores máximos"); maximum = PracticeCountNoShootTargets(zone);
            format(help, sizeof(help), "Máximo de errores de identificación permitidos para aprobar.\nDisparar mientras aparece un blanco protegido cuenta como un error, incluso al aire,\ny resta 1 punto. Se cuenta como máximo un error por blanco protegido.\nSuperar este límite impide aprobar, aunque se alcancen los puntos mínimos.\nEl ejercicio continúa hasta el final.\n\n0 exige no cometer errores. Hay %d blancos protegidos configurados.", maximum);
        }
        case PracticePreparation:
        {
            format(title, sizeof(title), "Segundos de preparación"); minimum = 1; maximum = 30;
            format(help, sizeof(help), "Tiempo desde que se inicia la práctica hasta que aparece el primer blanco.\nSe muestra una cuenta regresiva y suena un pitido por segundo.\nLos disparos durante la preparación no cuentan para el resultado.");
        }
        case PracticeExposure:
        {
            format(title, sizeof(title), "Segundos por blanco"); minimum = 100; maximum = 60000;
            format(help, sizeof(help), "Tiempo máximo que permanece visible cada blanco.\nUn blanco de tiro desaparece al recibir un impacto válido o al agotarse el tiempo.\nAnte un blanco protegido debes esperar sin disparar hasta que termine ese tiempo.\nSi disparas, desaparece y se registra un error de identificación.");
        }
        case PracticeInterval:
        {
            format(title, sizeof(title), "Segundos entre blancos"); maximum = 30000;
            format(help, sizeof(help), "Pausa desde que desaparece un blanco hasta que aparece el siguiente.\nDurante la pausa no hay un blanco activo. Los disparos reducen la precisión.\n0 elimina la pausa programada. No cambia la preparación del primer blanco.");
        }
        default: return PracticeShowRules(playerid);
    }
    if (rule == PracticeExposure || rule == PracticeInterval)
        format(body, sizeof(body), "%s\n\nValor actual: %.1f segundos\nIntroduce un valor entre %.1f y %.1f segundos.\nPuedes usar un decimal con punto o coma, por ejemplo 0.5 o 0,5.",
            help, float(PracticeRules[zone][rule]) / 1000.0, float(minimum) / 1000.0, float(maximum) / 1000.0);
    else
        format(body, sizeof(body), "%s\n\nValor actual: %d\nIntroduce un número entero entre %d y %d.", help, PracticeRules[zone][rule], minimum, maximum);
    return PracticeDialog(playerid, SP_MENU_NUMBER, DIALOG_STYLE_INPUT, title, body, "Guardar", "Volver");
}

stock bool:PracticeApplyRuleInput(playerid, const input[])
{
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid)) return false;
    new length = strlen(input), value, E_PRACTICE_RULE:rule = PracticeRuleField[playerid], rules[E_PRACTICE_RULE];
    if (rule == PracticeExposure || rule == PracticeInterval)
    {
        // Fixed-point parsing avoids rounding, exponent notation and overflow.
        // Only whole seconds or one decimal digit are accepted.
        if (length < 1 || length > 4) return false;
        new digits, bool:fraction;
        for (new i = 0; i < length; i++)
        {
            if (input[i] == '.' || input[i] == ',')
            {
                if (!digits || fraction || i != length - 2) return false;
                fraction = true;
            }
            else
            {
                if (input[i] < '0' || input[i] > '9' || (!fraction && ++digits > 2)) return false;
                value = value * 10 + input[i] - '0';
            }
        }
        value *= fraction ? 100 : 1000;
    }
    else
    {
        if (length < 1 || length > 3) return false;
        for (new i = 0; i < length; i++)
        {
            if (input[i] < '0' || input[i] > '9') return false;
            value = value * 10 + input[i] - '0';
        }
    }
    rules = PracticeRules[zone];
    rules[rule] = value;
    if (!PracticeRulesValid(rules)
        || (rule == PracticePassPoints && value > PracticeCountShootTargets(zone))
        || (rule == PracticeMaxErrors && value > PracticeCountNoShootTargets(zone))) return false;
    PracticeRules[zone] = rules;
    PracticeConfigurationChanged(playerid);
    return true;
}

stock PracticeHandleMenu(playerid, response, listitem, const inputtext[])
{
    new page = PracticeMenuPage[playerid];
    if (page == SP_MENU_NONE) return 0;
    PracticeMenuPage[playerid] = SP_MENU_NONE; // consume this response exactly once
    // Public selection is handled before administrative authorization; it cannot
    // dispatch any editor action or change the administrator's selected zone.
    if (page == SP_MENU_PLAYER_ZONES) return PracticeHandlePlayerZones(playerid, response, listitem);
    if (page == SP_MENU_ZONES || page == SP_MENU_CREATE_ZONE)
        return PracticeHandleZoneList(playerid, page, response, listitem, inputtext);
    new zone = PracticeSelectedZone[playerid];
    if (!PracticeSelectedZoneValid(playerid))
    {
        PracticeResetUI(playerid);
        if (PracticeIsAdministrator(playerid)) return PracticeShowZones(playerid);
        return 1;
    }
    if (!PracticeIsAdministrator(playerid))
    { PracticeResetUI(playerid); return 1; }
    if (PracticeMenuZoneID[playerid] != PracticeZoneID[zone] || PracticeMenuRevision[playerid] != PracticeRevision[zone])
    {
        PracticeDiscardDraft(playerid);
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "La configuración cambió. Revisa el menú actualizado.");
        return PracticeShowMain(playerid);
    }
    if (response && (
        (page == SP_MENU_TARGETS && listitem >= 0 && listitem < PracticeMenuRowCount[playerid]
            && PracticeMenuRows[playerid][listitem] == SP_CONFIG_CLOSE)
        || (page == SP_MENU_STARTS && listitem >= 0 && listitem < PracticeStartRowCount[playerid]
            && PracticeStartRows[playerid][listitem] == SP_CONFIG_CLOSE)
        || (page == SP_MENU_START && listitem == 3)
        || (page == SP_MENU_DRAFT && listitem == (PracticeDraftNew[playerid] ? 4 : 6))))
        return PracticePauseMenu(playerid, page);
    if (!response)
    {
        switch (page)
        {
            case SP_MENU_MAIN: return PracticeShowZones(playerid);
            case SP_MENU_DRAFT: { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
            case SP_MENU_NAME, SP_MENU_DELETE: return PracticeShowDraftMenu(playerid);
            case SP_MENU_RENAME_ZONE, SP_MENU_ARCHIVE_ZONE: return PracticeShowMain(playerid);
            case SP_MENU_BEHAVIOUR:
            {
                if (PracticeDraftWizard[playerid]) return PracticeShowCatalogue(playerid);
                return PracticeShowDraftMenu(playerid);
            }
            case SP_MENU_NUMBER, SP_MENU_WEAPON, SP_MENU_RULE_INFO: return PracticeShowRules(playerid);
            case SP_MENU_EDIT_ERROR: { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
            case SP_MENU_START_CONFIRM: { PracticeDiscardDraft(playerid); return PracticeShowStart(playerid); }
            case SP_MENU_STARTS: return PracticeShowMain(playerid);
            case SP_MENU_START: return PracticeShowStarts(playerid);
            case SP_MENU_START_NAME, SP_MENU_START_DELETE: return PracticeShowStart(playerid);
            default: return PracticeShowMain(playerid);
        }
    }
    // Use the actions captured for this exact rendered menu, as optional rows shift.
    if (page == SP_MENU_MAIN)
    {
        if (listitem < 0 || listitem >= PracticeMainRowCount[playerid]) return PracticeShowMain(playerid);
        listitem = PracticeMainRows[playerid][listitem];
        switch (listitem)
        {
            case SP_MAIN_START_POINT: return PracticeShowStarts(playerid);
            case SP_MAIN_TARGETS: return PracticeShowTargets(playerid);
            case SP_MAIN_RULES: return PracticeShowRules(playerid);
            case SP_MAIN_START_PRACTICE:
            {
                if (!PracticeCanStart(zone, playerid)) return PracticeShowMain(playerid);
                PracticeStart(zone, playerid);
                if (!PracticeIsBusy(zone)) return PracticeShowMain(playerid);
                return 1;
            }
            case SP_MAIN_CANCEL_PRACTICE:
            {
                if (PracticeIsBusy(zone) && PracticeSession[zone][PracticePlayer] == playerid)
                    PracticeEnd(zone, false, "cancelada por el participante.");
                return PracticeShowMain(playerid);
            }
        }
    }
    if (page == SP_MENU_RULE_INFO) return PracticeShowRules(playerid);
    if (!PracticeMayChange(playerid)) return PracticeShowMain(playerid);
    if (page == SP_MENU_STARTS || page == SP_MENU_START || page == SP_MENU_START_NAME || page == SP_MENU_START_DELETE)
        return PracticeHandleStartMenu(playerid, page, listitem, inputtext);
    switch (page)
    {
        case SP_MENU_MAIN:
        {
            switch (listitem)
            {
                case SP_MAIN_START_POINT: return PracticeShowStarts(playerid);
                case SP_MAIN_PREVIEW:
                {
                    if (PracticePreviewActive[playerid]) { PracticeHidePreview(playerid); return PracticeShowMain(playerid); }
                    return PracticeShowPreview(playerid);
                }
                case SP_MAIN_RESET: return PracticeDialog(playerid, SP_MENU_RESET, DIALOG_STYLE_MSGBOX, "Reiniciar configuración",
                    "Se borrarán todos los puntos de inicio, los blancos y sus ajustes.\nLas reglas volverán a sus valores iniciales.\n\n¿Quieres continuar?", "Reiniciar", "Cancelar");
                case SP_MAIN_RETRY_SAVE: { if (PracticeSavePending[zone] || PracticeRegistrySavePending) PracticeSaveAndNotify(playerid); return PracticeShowMain(playerid); }
                case SP_MAIN_RENAME_ZONE: return PracticeDialog(playerid, SP_MENU_RENAME_ZONE, DIALOG_STYLE_INPUT, "Nombre de la zona",
                    "Introduce un nombre de hasta 48 caracteres, sin saltos de línea ni códigos de color.", "Guardar", "Volver");
                case SP_MAIN_INDICATORS:
                {
                    PracticeZoneIndicators[zone] = !PracticeZoneIndicators[zone];
                    PracticeConfigurationChanged(playerid);
                    return PracticeShowMain(playerid);
                }
                case SP_MAIN_TOGGLE_ZONE: return PracticeToggleZone(playerid);
                case SP_MAIN_ARCHIVE_ZONE: return PracticeDialog(playerid, SP_MENU_ARCHIVE_ZONE, DIALOG_STYLE_MSGBOX, "Archivar zona",
                    "La zona dejará de aparecer y no se podrá usar.\nSe conservarán su ID y configuración en el archivo.\n\n¿Quieres archivarla?", "Archivar", "Cancelar");
            }
        }
        case SP_MENU_RENAME_ZONE:
        {
            if (!PracticeZoneNameValid(inputtext))
            { SendClientMessage(playerid, COLOR_FADE3, "Nombre inválido. Usa de 1 a 48 caracteres, sin códigos de color ni saltos de línea."); return PracticeShowMain(playerid); }
            format(PracticeZoneName[zone], PRACTICE_ZONE_NAME_SIZE, "%s", inputtext);
            PracticeConfigurationChanged(playerid);
            return PracticeShowMain(playerid);
        }
        case SP_MENU_ARCHIVE_ZONE: return PracticeArchiveZone(playerid);
        case SP_MENU_TARGETS:
        {
            if (listitem < 0 || listitem >= PracticeMenuRowCount[playerid]) return PracticeShowTargets(playerid);
            new slot = PracticeMenuRows[playerid][listitem];
            if (!slot)
            {
                slot = PracticeFreeTargetSlot(zone);
                if (slot < 0) { SendClientMessage(playerid, COLOR_FADE3, "Ya hay diez blancos. Edita o elimina uno."); return PracticeShowTargets(playerid); }
                if (PracticeBeginDraft(playerid, slot, true) && PracticeHasDraft(playerid)) return PracticeShowCatalogue(playerid);
            }
            else if (PracticeBeginDraft(playerid, slot) && PracticeHasDraft(playerid)) return PracticeShowDraftMenu(playerid);
            return PracticeShowTargets(playerid);
        }
        case SP_MENU_DRAFT:
        {
            if (!PracticeDraftCurrent(playerid)) { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
            // Close is handled above. Save remains the last row in both draft forms.
            if (listitem < 0 || listitem > (PracticeDraftNew[playerid] ? 5 : 7)) return PracticeShowDraftMenu(playerid);
            new action = listitem;
            if (action == (PracticeDraftNew[playerid] ? 5 : 7)) action = 6;
            switch (action)
            {
                case 0: return PracticeDialog(playerid, SP_MENU_NAME, DIALOG_STYLE_INPUT, "Nombre del blanco",
                    "Introduce un nombre de hasta 32 caracteres, sin saltos de línea ni códigos de color.\nEscribe - para volver al nombre automático.", "Aceptar", "Volver");
                case 1: return PracticeShowCatalogue(playerid);
                case 2: return PracticeShowBehaviour(playerid);
                case 3: return PracticeStartVisualEdit(playerid);
                case 6: return PracticeCommitDraft(playerid);
                case 4:
                {
                    if (PracticeDraftNew[playerid]) return PracticeShowDraftMenu(playerid);
                    new slot = PracticeFreeTargetSlot(zone);
                    if (slot < 0) { SendClientMessage(playerid, COLOR_FADE3, "Ya hay diez blancos."); return PracticeShowDraftMenu(playerid); }
                    PracticeDraftSlot[playerid] = slot;
                    PracticeDraftNew[playerid] = true;
                    return PracticeStartVisualEdit(playerid);
                }
                case 5:
                {
                    if (PracticeDraftNew[playerid]) return PracticeShowDraftMenu(playerid);
                    return PracticeDialog(playerid, SP_MENU_DELETE, DIALOG_STYLE_MSGBOX, "Eliminar blanco",
                        "¿Quieres eliminar este blanco?\n\nSe borrará su configuración guardada.", "Eliminar", "Cancelar");
                }
            }
        }
        case SP_MENU_NAME:
        {
            if (!PracticeDraftCurrent(playerid)) { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
            if (!PracticeNameValid(inputtext) || isnull(inputtext))
            { SendClientMessage(playerid, COLOR_FADE3, "Nombre inválido. Usa hasta 32 caracteres. Escribe - para restaurar el nombre automático."); return PracticeShowDraftMenu(playerid); }
            if (!strcmp(inputtext, "-")) PracticeDraft[playerid][PracticePointName][0] = EOS;
            else format(PracticeDraft[playerid][PracticePointName], PRACTICE_NAME_SIZE, "%s", inputtext);
            return PracticeShowDraftMenu(playerid);
        }
        case SP_MENU_BEHAVIOUR:
        {
            if (!PracticeDraftCurrent(playerid)) { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
            if (listitem != 0 && listitem != 1) return PracticeShowBehaviour(playerid);
            PracticeDraft[playerid][PracticePointNoShoot] = bool:listitem;
            PracticeDraftFixTypes[playerid] = true;
            if (PracticeDraftWizard[playerid]) { PracticeDraftWizard[playerid] = false; return PracticeStartVisualEdit(playerid); }
            return PracticeShowDraftMenu(playerid);
        }
        case SP_MENU_EDIT_ERROR: return PracticeStartVisualEdit(playerid);
        case SP_MENU_DELETE:
        {
            if (!PracticeDraftCurrent(playerid) || PracticeDraftNew[playerid]) { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
            new empty[E_PRACTICE_POINT];
            PracticePoints[zone][PracticeDraftSlot[playerid]] = empty;
            PracticeDiscardDraft(playerid);
            PracticeConfigurationChanged(playerid);
            return PracticeShowTargets(playerid);
        }
        case SP_MENU_RULES:
        {
            switch (listitem)
            {
                case 0: return PracticeShowWeapons(playerid);
                case 1: return PracticeShowRuleInput(playerid, PracticePassPoints);
                case 2: return PracticeShowRuleInput(playerid, PracticeMaxErrors);
                case 3: return PracticeShowRuleInput(playerid, PracticePreparation);
                case 4: return PracticeShowRuleInput(playerid, PracticeExposure);
                case 5: return PracticeShowRuleInput(playerid, PracticeInterval);
                case 6: PracticeRules[zone][PracticeRandomOrder] = !PracticeRules[zone][PracticeRandomOrder];
                case 7: PracticeRules[zone][PracticeRandomTypes] = !PracticeRules[zone][PracticeRandomTypes];
                default: return PracticeShowRules(playerid);
            }
            PracticeConfigurationChanged(playerid);
            return PracticeShowRules(playerid);
        }
        case SP_MENU_NUMBER:
        {
            if (!PracticeApplyRuleInput(playerid, inputtext))
            { SendClientMessage(playerid, COLOR_FADE3, "Valor fuera de rango o formato inválido."); return PracticeShowRuleInput(playerid, PracticeRuleField[playerid]); }
            return PracticeShowRules(playerid);
        }
        case SP_MENU_WEAPON:
        {
            if (listitem >= 0 && listitem < sizeof(PracticeWeaponNames))
            {
                new mask = PracticeRules[zone][PracticeWeaponMask] ^ (1 << listitem);
                if (!PracticeWeaponsValid(mask))
                    SendClientMessage(playerid, COLOR_FADE3, "Debes mantener al menos un arma permitida.");
                else
                {
                    PracticeRules[zone][PracticeWeaponMask] = mask;
                    PracticeConfigurationChanged(playerid);
                }
            }
            return PracticeShowWeapons(playerid);
        }
        case SP_MENU_START_CONFIRM: return PracticeCommitStart(playerid);
        case SP_MENU_RESET:
        {
            new empty[E_PRACTICE_POINT], defaults[E_PRACTICE_RULE] = {PRACTICE_WEAPON_DEFAULT, 1, 3, 5000, 1000, 0, 0, 0};
            for (new slot = 0; slot < PRACTICE_POINT_COUNT; slot++) PracticePoints[zone][slot] = empty;
            PracticeRules[zone] = defaults;
            PracticeZoneIndicators[zone] = false;
            PracticeConfigurationChanged(playerid);
            return PracticeShowMain(playerid);
        }
    }
    return PracticeShowMain(playerid);
}

Dialog:SP_Menu(playerid, response, listitem, inputtext[])
{
    return PracticeHandleMenu(playerid, response, listitem, inputtext);
}

CMD:practicaconfig(playerid, const params[])
{
    return PracticeAdminCommand(playerid, params);
}
