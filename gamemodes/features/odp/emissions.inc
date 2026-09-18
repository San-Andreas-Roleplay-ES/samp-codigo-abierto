// Shared, append-only records for this gamemode run. No disk/SQL persistence.
#define ODP_EMISSION_MAX 200
#define ODP_EMISSION_FIELD_SIZE 1025
// Short fields are capped at 96 chars by ODPEmissionResponse; only the 5
// "long" fields (see ODPEmissionLongField) need the full 1024-char storage.
#define ODP_EMISSION_FIELD_SHORT_SIZE 97
new ODPEmissionCount;
new ODPEmissionBody[ODP_EMISSION_MAX][ODP_EMISSION_BODY_SIZE];
new ODPEmissionAuthor[ODP_EMISSION_MAX][MAX_PLAYER_NAME];
new ODPEmissionUnit[ODP_EMISSION_MAX][17];
new ODPEmissionDate[ODP_EMISSION_MAX][24];
new ODPEmissionKind[ODP_EMISSION_MAX];
// Field id (0-13, matches ODPEmissionLabels) -> compact slot in the
// short/long backing array picked by ODPEmissionLongField(field).
new const ODPEmissionSlot[14] = {0, 1, 2, 3, 4, 5, 0, 1, 6, 7, 8, 2, 3, 4};
new ODPEmissionDraftShort[MAX_PLAYERS][9][ODP_EMISSION_FIELD_SHORT_SIZE];
new ODPEmissionDraftLong[MAX_PLAYERS][5][ODP_EMISSION_FIELD_SIZE];
new ODPEmissionDraftKind[MAX_PLAYERS], ODPEmissionFormPage[MAX_PLAYERS];
new ODPEmissionListPage[MAX_PLAYERS], ODPEmissionRows[MAX_PLAYERS][4];
new bool:ODPEmissionInput[MAX_PLAYERS], ODPEmissionField[MAX_PLAYERS];
new ODPEmissionEditBuffer[MAX_PLAYERS][ODP_EMISSION_FIELD_SIZE];

stock bool:ODPEmissionLongField(field)
{
    return field == 6 || field == 7 || field >= 11;
}
new const ODPEmissionLabels[][] = {
    "Sexo", "Etnia", "Edad estimada", "Altura", "Peso", "Complexion",
    "Vestimenta", "Accesorios", "Modelo", "Color", "Matricula",
    "Ultima ubicacion", "Direccion de huida", "Hechos / motivo"
};
new const ODPEmissionPersonFields[] = {0,1,2,3,4,5,6,7,11,12,13};
new const ODPEmissionVehicleFields[] = {8,9,10,11,12,13};

// Field storage is split short/long (see the arrays above); these three
// helpers keep the field-id (0-13) as the only thing call sites need to know.
stock ODPEmissionFieldGet(playerid, field, output[], capacity)
{
    if (ODPEmissionLongField(field)) format(output, capacity, "%s", ODPEmissionDraftLong[playerid][ODPEmissionSlot[field]]);
    else format(output, capacity, "%s", ODPEmissionDraftShort[playerid][ODPEmissionSlot[field]]);
    return 1;
}

stock bool:ODPEmissionFieldEmpty(playerid, field)
{
    if (ODPEmissionLongField(field)) return !ODPEmissionDraftLong[playerid][ODPEmissionSlot[field]][0];
    return !ODPEmissionDraftShort[playerid][ODPEmissionSlot[field]][0];
}

stock ODPEmissionFieldSet(playerid, field, const value[])
{
    if (ODPEmissionLongField(field)) format(ODPEmissionDraftLong[playerid][ODPEmissionSlot[field]], ODP_EMISSION_FIELD_SIZE, "%s", value);
    else format(ODPEmissionDraftShort[playerid][ODPEmissionSlot[field]], ODP_EMISSION_FIELD_SHORT_SIZE, "%s", value);
    return 1;
}

stock ODPEmissionResetDraft(playerid)
{
    for (new f; f < 9; f++) ODPEmissionDraftShort[playerid][f][0] = EOS;
    for (new f; f < 5; f++) ODPEmissionDraftLong[playerid][f][0] = EOS;
    ODPEmissionDraftKind[playerid] = 0;
    ODPEmissionFormPage[playerid] = 0;
    ODPEmissionInput[playerid] = false;
    return 1;
}

stock ODPEmissionVisibleField(playerid, position)
{
    if (ODPEmissionDraftKind[playerid])
        return position < sizeof ODPEmissionVehicleFields ? ODPEmissionVehicleFields[position] : -1;
    return position < sizeof ODPEmissionPersonFields ? ODPEmissionPersonFields[position] : -1;
}

stock ODPEmissionCompose(playerid, output[], capacity)
{
    format(output, capacity, "%s, %s", ODPCallsign[playerid], ODPEmissionDraftKind[playerid] ? "vehiculo" : "persona");
    // Preserve the attribute order from the reference; skip unknown values.
    new value[ODP_EMISSION_FIELD_SIZE];
    for (new pos; pos < 11; pos++)
    {
        new field = ODPEmissionVisibleField(playerid, pos);
        if (field == -1) break;
        if (ODPEmissionFieldEmpty(playerid, field)) continue;
        ODPEmissionFieldGet(playerid, field, value, sizeof value);
        strcat(output, ", ", capacity);
        strcat(output, ODPEmissionLabels[field], capacity);
        strcat(output, ": ", capacity);
        strcat(output, value, capacity);
    }
    strcat(output, ".", capacity);
    return 1;
}

stock ODPEmissionListDraw(playerid)
{
    new text[100];
    ODPHeading(playerid, "Emisiones de crimenes", "Comunicaciones compartidas / Personas y vehiculos.");
    ODPButton(playerid, 193.0, 171.0, 194.0, "Redactar emision", 700, true);
    ODPButton(playerid, 398.0, 171.0, 194.0, "Actualizar listado", 701);
    new lastpage = ODPEmissionCount ? (ODPEmissionCount - 1) / 4 : 0;
    if (ODPEmissionListPage[playerid] > lastpage) ODPEmissionListPage[playerid] = lastpage;
    for (new row; row < 4; row++)
    {
        // Store the displayed IDs so new records cannot shift a pending click.
        new index = ODPEmissionCount - 1 - ODPEmissionListPage[playerid] * 4 - row;
        ODPEmissionRows[playerid][row] = index;
        if (index < 0) continue;
        new Float:y = 204.0 + row * 36.0;
        ODPBox(playerid, 193.0, y, 399.0, 33.0, C_CARD, 710 + row);
        format(text, sizeof text, "#%03d / %s / %.16s", index + 1, ODPEmissionKind[index] ? "Vehiculo" : "Persona", ODPEmissionUnit[index]);
        ODPText(playerid, 201.0, y + 3.0, text, C_ACCENT, 0.22, 1.1);
        format(text, sizeof text, "%s | %s", ODPEmissionDate[index], ODPEmissionAuthor[index]);
        ODPText(playerid, 201.0, y + 18.0, text, C_MUTED, 0.19, 0.95);
    }
    if (!ODPEmissionCount) ODPText(playerid, 201.0, 230.0, "No se han emitido comunicaciones en esta sesion.", C_MUTED, 0.21, 1.05);
    format(text, sizeof text, "Pagina %d/%d | %d de %d emisiones", ODPEmissionListPage[playerid]+1, lastpage+1, ODPEmissionCount, ODP_EMISSION_MAX);
    ODPText(playerid, 193.0, 352.0, text, C_MUTED, 0.20, 1.0);
    ODPButton(playerid, 193.0, 369.0, 194.0, "Anterior", 702);
    ODPButton(playerid, 398.0, 369.0, 194.0, "Siguiente", 703);
    ODPText(playerid, 193.0, 398.0, "Se conservan hasta el reinicio del servidor.", C_MUTED, 0.18, 0.9);
    return 1;
}

stock ODPEmissionFormDraw(playerid)
{
    ODPHeading(playerid, "Redactar emision", "Completa lo conocido; los datos vacios se omiten.");
    ODPButton(playerid, 193.0, 171.0, 194.0, "Persona", 704, !ODPEmissionDraftKind[playerid]);
    ODPButton(playerid, 398.0, 171.0, 194.0, "Vehiculo", 705, ODPEmissionDraftKind[playerid] == 1);
    for (new row; row < 4; row++)
    {
        new field = ODPEmissionVisibleField(playerid, ODPEmissionFormPage[playerid] * 4 + row);
        if (field == -1) break;
        new text[90], Float:y = 202.0 + row * 32.0, value[ODP_EMISSION_FIELD_SIZE];
        ODPEmissionFieldGet(playerid, field, value, sizeof value);
        format(text, sizeof text, "%s: %.38s", ODPEmissionLabels[field], value[0] ? value : "Sin dato");
        for (new c; text[c]; c++) if (text[c] == '\n') text[c] = ' ';
        ODPButton(playerid, 193.0, y, 399.0, text, 720 + field);
    }
    ODPButton(playerid, 193.0, 333.0, 126.0, "Anterior", 706);
    ODPButton(playerid, 329.0, 333.0, 126.0, "Siguiente", 707);
    ODPButton(playerid, 465.0, 333.0, 127.0, "Vista previa", 708);
    ODPButton(playerid, 193.0, 366.0, 194.0, "Emitir y guardar", 709, true);
    ODPButton(playerid, 398.0, 366.0, 194.0, "Volver al listado", 701);
    ODPText(playerid, 193.0, 398.0, "Puedes volver y continuar el borrador durante tu sesion.", C_MUTED, 0.18, 0.9);
    return 1;
}

stock ODPEmissionEdit(playerid, field)
{
    if (field < 0 || field >= 14) return 0;
    ODPEmissionFieldGet(playerid, field, ODPEmissionEditBuffer[playerid], ODP_EMISSION_FIELD_SIZE);
    ODPEmissionField[playerid] = field;
    return ODPEmissionPrompt(playerid);
}

stock ODPEmissionPrompt(playerid, const error[] = "")
{
    new field = ODPEmissionField[playerid];
    ODPEmissionInput[playerid] = true;
    CancelSelectTextDraw(playerid);
    new prompt[1600];
    if (ODPEmissionLongField(field))
    {
        format(prompt, sizeof prompt, "%s\n%s\nTexto acumulado (%d/1024):\n%s\n\nEnviar: anadir una linea y continuar.\nAtras: guardar y volver. /borrar: vaciar el texto.", ODPEmissionLabels[field], error, strlen(ODPEmissionEditBuffer[playerid]), ODPEmissionEditBuffer[playerid]);
        Dialog_Show(playerid, ODP_EMISSION_INPUT, DIALOG_STYLE_INPUT, "Emision / Texto por lineas", prompt, "Enviar", "Atras");
        return 1;
    }
    new value[ODP_EMISSION_FIELD_SHORT_SIZE];
    ODPEmissionFieldGet(playerid, field, value, sizeof value);
    format(prompt, sizeof prompt, "%s\nActual: %s\nEscribe hasta 96 caracteres. Vacio = sin dato.\nESC conserva el valor actual.", ODPEmissionLabels[field], value);
    Dialog_Show(playerid, ODP_EMISSION_INPUT, DIALOG_STYLE_INPUT, "Emision / Editar dato", prompt, "Guardar", "Cancelar");
    return 1;
}

stock ODPEmissionResponse(playerid, response, inputtext[])
{
    if (!ODPOpen[playerid] || !ODPEmissionInput[playerid] || ODPPage[playerid] != 23) return 0;
    ODPEmissionInput[playerid] = false;
    new field = ODPEmissionField[playerid];
    if (ODPEmissionLongField(field))
    {
        if (inputtext[0] == '\1') inputtext[0] = EOS;
        for (new c; inputtext[c]; c++) if (inputtext[c] < 32 || inputtext[c] == '~') inputtext[c] = ' ';
        while (inputtext[0] == ' ') strdel(inputtext, 0, 1);
        new length = strlen(inputtext);
        while (length && inputtext[length - 1] == ' ') inputtext[--length] = EOS;
        if (length && !strcmp(inputtext, "/borrar", true)) ODPEmissionEditBuffer[playerid][0] = EOS;
        else if (length)
        {
            new current = strlen(ODPEmissionEditBuffer[playerid]);
            if (current + length + (current ? 1 : 0) >= ODP_EMISSION_FIELD_SIZE)
                return ODPEmissionPrompt(playerid, "No cabe esta linea. Texto anterior conservado; maximo 1024 caracteres por campo.");
            if (current) strcat(ODPEmissionEditBuffer[playerid], "\n", ODP_EMISSION_FIELD_SIZE);
            strcat(ODPEmissionEditBuffer[playerid], inputtext, ODP_EMISSION_FIELD_SIZE);
        }
        if (response) return ODPEmissionPrompt(playerid);
        ODPEmissionFieldSet(playerid, field, ODPEmissionEditBuffer[playerid]);
        return ODPRender(playerid, 23);
    }
    if (response)
    {
        if (strlen(inputtext) > 96) return ODPEmissionEdit(playerid, field);
        if (inputtext[0] == '\1') inputtext[0] = EOS;
        for (new c; inputtext[c]; c++) if (inputtext[c] < 32 || inputtext[c] == '~') inputtext[c] = ' ';
        // Trim so a string of spaces is not treated as a known attribute.
        while (inputtext[0] == ' ') strdel(inputtext, 0, 1);
        new length = strlen(inputtext);
        while (length && inputtext[length - 1] == ' ') inputtext[--length] = EOS;
        ODPEmissionFieldSet(playerid, field, inputtext);
    }
    return ODPRender(playerid, 23);
}

stock ODPEmissionPublish(playerid)
{
    if (!ODPOpen[playerid] || ODPPage[playerid] != 23 || ODPEmissionInput[playerid]) return 0;
    new known;
    for (new pos; pos < 11; pos++)
    {
        new field = ODPEmissionVisibleField(playerid, pos);
        if (field == -1) break;
        if (!ODPEmissionFieldEmpty(playerid, field)) known++;
    }
    if (!known) return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Emision vacia", "Completa al menos un dato antes de emitir.");
    if (ODPEmissionCount >= ODP_EMISSION_MAX)
        return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Listado completo", "Se ha alcanzado el limite de 200 emisiones.\nNo se han borrado registros. Tu borrador sigue disponible.\nEl listado se vaciara en el siguiente reinicio.");
    new index = ODPEmissionCount++, year, month, day, hour, minute, second;
    ODPEmissionCompose(playerid, ODPEmissionBody[index], ODP_EMISSION_BODY_SIZE);
    GetPlayerName(playerid, ODPEmissionAuthor[index], MAX_PLAYER_NAME);
    format(ODPEmissionUnit[index], 17, "%s", ODPCallsign[playerid]);
    getdate(year, month, day); gettime(hour, minute, second);
    format(ODPEmissionDate[index], 24, "%02d/%02d/%04d %02d:%02d:%02d", day, month, year, hour, minute, second);
    ODPEmissionKind[index] = ODPEmissionDraftKind[playerid];
    ODPEmissionResetDraft(playerid);
    ODPEmissionListPage[playerid] = 0;
    return ODPRender(playerid, 22);
}

stock ODPEmissionAction(playerid, action)
{
    if (ODPPage[playerid] != 22 && ODPPage[playerid] != 23) return 0;
    if (ODPEmissionInput[playerid]) return 0;
    if (action == 700) return ODPRender(playerid, 23);
    if (action == 701) { ODPEmissionListPage[playerid] = 0; return ODPRender(playerid, 22); }
    if (ODPPage[playerid] == 22)
    {
        if (action == 702 && ODPEmissionListPage[playerid] > 0) ODPEmissionListPage[playerid]--;
        if (action == 703 && (ODPEmissionListPage[playerid]+1)*4 < ODPEmissionCount) ODPEmissionListPage[playerid]++;
        if (action >= 710 && action <= 713)
        {
            new index = ODPEmissionRows[playerid][action-710];
            if (index < 0 || index >= ODPEmissionCount) return 0;
            new document[ODP_EMISSION_BODY_SIZE], title[64];
            format(title, sizeof title, "Emision #%03d / %s", index+1, ODPEmissionKind[index] ? "Vehiculo" : "Persona");
            format(document, sizeof document, "Fecha: %s\nAutor: %s\nUnidad: %s\n\n", ODPEmissionDate[index], ODPEmissionAuthor[index], ODPEmissionUnit[index]);
            strcat(document, ODPEmissionBody[index], sizeof document);
            return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, title, document);
        }
        return ODPRender(playerid, 22);
    }
    if (action == 704 || action == 705) { ODPEmissionDraftKind[playerid] = action-704; ODPEmissionFormPage[playerid] = 0; }
    if (action == 706 && ODPEmissionFormPage[playerid] > 0) ODPEmissionFormPage[playerid]--;
    if (action == 707 && ODPEmissionVisibleField(playerid, (ODPEmissionFormPage[playerid]+1)*4) != -1) ODPEmissionFormPage[playerid]++;
    if (action >= 720 && action <= 733) return ODPEmissionEdit(playerid, action-720);
    if (action == 708)
    {
        new document[ODP_EMISSION_BODY_SIZE]; ODPEmissionCompose(playerid, document, sizeof document);
        return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Vista previa / Emision sin publicar", document);
    }
    if (action == 709) return ODPEmissionPublish(playerid);
    return ODPRender(playerid, 23);
}
