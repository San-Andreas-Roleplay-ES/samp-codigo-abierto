// Screen fields and list/detail flows transcribed from sa-mp-209..239.
// Fixture values are invented; this module does not query sarp.es.
#define LIST_ARRESTS 0
#define LIST_FINES 1
#define LIST_DEBTORS 2
#define LIST_SEARCH_WARRANTS 3
#define LIST_STOLEN 4
#define LIST_CALLS 5
#define LIST_UNITS 6
#define LIST_ARREST_WARRANTS 7
#define LIST_WEAPONS 8
new ODPListKind[MAX_PLAYERS], ODPListPage[MAX_PLAYERS], ODPListParent[MAX_PLAYERS];
new ODPListPerson[MAX_PLAYERS][32];
new const ODPListTitles[][] = {
    "Busqueda de arrestos", "Busqueda de multas", "Personas con multas pendientes",
    "Ordenes de allanamiento", "Denuncias de vehiculos robados", "Llamadas 9-1-1",
    "Callsigns en el turno", "Ordenes de arresto pendientes", "Armas por propietario"
};
new const ODPListHeaders[][][] = {
    {"Fecha", "Delito", "Codigo", ""},
    {"Fecha", "", "", ""},
    {"Persona", "Cantidad pendiente de pago", "", ""},
    {"Sospechoso", "Agente", "Riesgo", "Fecha"},
    {"Modelo", "Matricula", "Colores", "Denunciante"},
    {"Telefono", "Rastreo", "Estado", "Tiempo"},
    {"Callsign", "Miembros", "Estado", "Ubicacion"},
    {"Sospechoso", "Agente", "Riesgo", "Fecha"},
    {"Modelo", "Serial", "Registro", ""}
};
new const ODPDemoPeople[][] = { "Daniel Vega", "Taylor Brooks", "Jamie Parker", "Robin Hayes", "Casey Reed", "Avery Collins", "Morgan Lane" };
new const ODPDemoOffences[][] = { "Agresion", "Robo de vehiculo", "Evasion" };

stock ODPListTotal(playerid)
{
    switch (ODPListKind[playerid])
    {
        case LIST_ARRESTS: return 7;
        case LIST_FINES: return 6;
        case LIST_DEBTORS: return 5;
        case LIST_SEARCH_WARRANTS: return 3;
        case LIST_STOLEN: return 6;
        case LIST_CALLS: return 2;
        case LIST_UNITS: return 2;
        case LIST_ARREST_WARRANTS: return 7;
        case LIST_WEAPONS: return 1;
    }
    return 0;
}

stock ODPListSection(playerid)
{
    switch (ODPListKind[playerid])
    {
        case LIST_ARRESTS, LIST_FINES, LIST_DEBTORS: return 2;
        case LIST_SEARCH_WARRANTS, LIST_ARREST_WARRANTS: return 7;
        case LIST_STOLEN: return 3;
        case LIST_CALLS: return 8;
        case LIST_UNITS: return 1;
        case LIST_WEAPONS: return 6;
    }
    return 0;
}

stock ODPOpenList(playerid, kind, parent)
{
    ODPListKind[playerid] = kind;
    ODPListPage[playerid] = 0;
    ODPListParent[playerid] = parent;
    return ODPRender(playerid, 21);
}

stock ODPRowFields(playerid, row, cells[4][80])
{
    for (new c; c < 4; c++) cells[c][0] = '\0';
    switch (ODPListKind[playerid])
    {
        case LIST_ARRESTS:
        {
            format(cells[0], 80, "2026-09-%02d 14:30:00", 17 - row);
            format(cells[1], 80, "%s", ODPDemoOffences[row % 3]);
            format(cells[2], 80, "CP-DEMO-%02d", row % 3 + 1);
        }
        case LIST_FINES: format(cells[0], 80, "2026-09-%02d 10:15:00", 17 - row);
        case LIST_DEBTORS:
        {
            format(cells[0], 80, "%s", ODPDemoPeople[row]);
            format(cells[1], 80, "$450");
        }
        case LIST_SEARCH_WARRANTS, LIST_ARREST_WARRANTS:
        {
            format(cells[0], 80, "%s", ODPDemoPeople[row]);
            format(cells[1], 80, "Officer A. Morgan");
            if (row == 0) format(cells[2], 80, "Alto"); else format(cells[2], 80, "Bajo");
            format(cells[3], 80, "2026-09-%02d 08:00:00", 17 - row);
        }
        case LIST_STOLEN:
        {
            format(cells[0], 80, "Sultan");
            format(cells[1], 80, "DEMO-%02d", row + 2);
            format(cells[2], 80, "Gris");
            format(cells[3], 80, "%s", ODPDemoPeople[row + 1]);
        }
        case LIST_CALLS:
        {
            format(cells[0], 80, "0");
            if (row == 0) format(cells[1], 80, "Vermont Avenue, East Beach, Los Santos 616");
            else format(cells[1], 80, "San Pedro Avenue, East Beach, Los Santos 616");
            format(cells[2], 80, "Registrada");
            format(cells[3], 80, "Hace %d minutos", 9 + row * 20);
        }
        case LIST_UNITS:
        {
            if (row == 0)
            {
                format(cells[0], 80, "%s", ODPCallsign[playerid]);
                GetPlayerName(playerid, cells[1], 80);
                format(cells[2], 80, "%s", ODPStatuses[ODPStatus[playerid]]);
                format(cells[3], 80, "Pershing Square");
            }
            else
            {
                format(cells[0], 80, "1-LINCOLN-04");
                format(cells[1], 80, "Jordan Reyes");
                format(cells[2], 80, "Disponible");
                format(cells[3], 80, "Market");
            }
        }
        case LIST_WEAPONS:
        {
            format(cells[0], 80, "Pistola");
            format(cells[1], 80, "DEMO-W01");
            format(cells[2], 80, "Armeria demo");
        }
    }
    return 1;
}

stock ODPDrawList(playerid)
{
    new kind = ODPListKind[playerid], total = ODPListTotal(playerid), line[100];
    if (kind == LIST_ARRESTS || kind == LIST_FINES || kind == LIST_WEAPONS)
        format(line, sizeof line, "%s / Registros ficticios", ODPListPerson[playerid]);
    else format(line, sizeof line, "%d registros de demostracion / Pulsa una fila para abrir.", total);
    ODPHeading(playerid, ODPListTitles[kind], line);
    ODPBox(playerid, 193.0, 172.0, 399.0, 22.0, C_ACCENT);
    new Float:columns[4] = {202.0, 306.0, 436.0, 502.0};
    for (new c; c < 4; c++) ODPText(playerid, columns[c], 178.0, ODPListHeaders[kind][c], 0xFFFFFFFF, 0.17, 0.85);
    for (new visible; visible < 4; visible++)
    {
        new row = ODPListPage[playerid] * 4 + visible;
        if (row >= total) break;
        new cells[4][80], Float:y = 199.0 + visible * 39.0;
        ODPRowFields(playerid, row, cells);
        ODPBox(playerid, 193.0, y, 399.0, 36.0, visible % 2 ? 0xCFCFCFFF : C_CARD, 600 + visible);
        for (new c; c < 4; c++)
        {
            new shorttext[40], limit = c == 1 ? 22 : 17;
            if (kind == LIST_FINES || kind == LIST_DEBTORS) limit = 32;
            strmid(shorttext, cells[c], 0, limit, sizeof shorttext);
            if (strlen(cells[c]) > limit) strcat(shorttext, "..");
            new color = C_TEXT;
            if ((kind == LIST_SEARCH_WARRANTS || kind == LIST_ARREST_WARRANTS) && c == 2) color = row == 0 ? C_RED : C_AMBER;
            ODPText(playerid, columns[c], y + 6.0, shorttext, color, 0.17, 0.85);
        }
        if (kind == LIST_CALLS) format(line, sizeof line, "Rastreo: %s", cells[1]);
        else if (kind == LIST_UNITS) format(line, sizeof line, "Estado: %s / Ubicacion: %s", cells[2], cells[3]);
        else if (kind == LIST_ARRESTS) format(line, sizeof line, "Reporte DEMO-AR%03d / %s", row + 1, ODPListPerson[playerid]);
        else if (kind == LIST_SEARCH_WARRANTS || kind == LIST_ARREST_WARRANTS) format(line, sizeof line, "Fecha: %s / Ver orden completa", cells[3]);
        else if (kind == LIST_STOLEN) format(line, sizeof line, "Colores: %s / Denunciante: %s", cells[2], cells[3]);
        else format(line, sizeof line, "Ver registro completo");
        ODPText(playerid, 202.0, y + 22.0, line, C_MUTED, 0.17, 0.85);
    }
    format(line, sizeof line, "Pagina %d de %d", ODPListPage[playerid] + 1, (total + 3) / 4);
    ODPText(playerid, 193.0, 358.0, line, C_MUTED, 0.18, 0.9);
    ODPButton(playerid, 193.0, 376.0, 125.0, "Volver", 652, true);
    if (ODPListPage[playerid] > 0) ODPButton(playerid, 328.0, 376.0, 125.0, "Anterior", 650);
    if ((ODPListPage[playerid] + 1) * 4 < total) ODPButton(playerid, 463.0, 376.0, 129.0, "Siguiente", 651);
    return 1;
}

stock ODPOpenListRecord(playerid, row)
{
    if (row < 0 || row >= ODPListTotal(playerid)) return 0;
    new kind = ODPListKind[playerid], cells[4][80], document[1800], title[80], part[256];
    ODPRowFields(playerid, row, cells);
    if (kind == LIST_DEBTORS)
    {
        format(ODPListPerson[playerid], 32, "%s", cells[0]);
        return ODPOpenList(playerid, LIST_FINES, -1);
    }
    switch (kind)
    {
        case LIST_ARRESTS:
        {
            format(title, sizeof title, "[DEMO-AR%03d] Reporte de arresto", row + 1);
            format(document, sizeof document, "Fecha: %s\nAgente: Alex Morgan\n\nSospechoso: %s\n", cells[0], ODPListPerson[playerid]);
            strcat(document, "Descripcion: Intervencion ficticia documentada por la unidad.\n");
            strcat(document, "La persona fue identificada y trasladada para tramitar\nlos hechos del reporte de demostracion.\n");
            format(part, sizeof part, "\nDelito: %s\nCodigo: %s\n", cells[1], cells[2]); strcat(document, part);
        }
        case LIST_FINES:
        {
            format(title, sizeof title, "[DEMO-M%03d] Multa", row + 1);
            format(document, sizeof document, "Fecha: %s\nAgente: Jordan Reyes\nInfractor: %s\n\n", cells[0], ODPListPerson[playerid]);
            if (row == 0) strcat(document, "Motivo: Estacionamiento indebido (ejemplo)\nMulta: $150\nEstado: Pendiente de pago\n");
            else if (row == 1) strcat(document, "Motivo: Conduccion imprudente (ejemplo)\nMulta: $300\nEstado: Pendiente de pago\n");
            else strcat(document, "Motivo: Infraccion de demostracion\nMulta: $100\nEstado: Pagada\nFecha de pago: 2026-09-17 16:00:00\n");
        }
        case LIST_SEARCH_WARRANTS, LIST_ARREST_WARRANTS:
        {
            if (kind == LIST_SEARCH_WARRANTS) format(title, sizeof title, "[DEMO-A%03d] Orden de allanamiento", row + 1);
            else format(title, sizeof title, "[DEMO-O%03d] Orden de arresto", row + 1);
            format(document, sizeof document, "Fecha: %s\nAgente: %s\n\nEstado: Abierta\nRiesgo: %s\n\nSospechoso: %s\n", cells[3], cells[1], cells[2], cells[0]);
            strcat(document, "Descripcion: Solicitud ficticia asociada a un expediente\nde demostracion, presentada para su revision.\n\n");
            if (kind == LIST_SEARCH_WARRANTS) strcat(document, "Objeto: Consulta de evidencias en propiedad DEMO-P01.\n\n");
            strcat(document, "Advertencia de seguridad: Consultar las instrucciones\ndel expediente antes de intervenir.\n\n");
            strcat(document, "Cargos:\nRobo de vehiculo (CP-DEMO-02)\nEvasion (CP-DEMO-06)\n\n");
            strcat(document, "Firmado por: Supervisor de demostracion\n");
        }
        case LIST_STOLEN:
        {
            format(title, sizeof title, "[DEMO-V%03d] Denuncia de robo", row + 1);
            format(document, sizeof document, "Fecha: 2026-09-17 18:00:33\nDenunciante: %s\n\nEstado: Abierta\nModelo: %s\nMatricula: %s\nColores: %s\n", cells[3], cells[0], cells[1], cells[2]);
            strcat(document, "\nDescripcion: El denunciante indica que dejo estacionado\nel vehiculo y no lo encontro al regresar. Relato ficticio.\n");
        }
        case LIST_CALLS:
        {
            format(title, sizeof title, "Llamada de emergencia / %s", cells[3]);
            format(document, sizeof document, "Nombre: Anonimo\nTelefono: %s\nRastreo: %s\nEstado: %s\n\nServicio requerido: Policia\n", cells[0], cells[1], cells[2]);
            format(part, sizeof part, "Localizacion: %s\n\n", cells[1]); strcat(document, part);
            strcat(document, "Situacion: Aviso ficticio de un vehiculo retirado por\nuna grua. Se solicita comprobar la situacion.\n");
        }
        case LIST_UNITS:
        {
            format(title, sizeof title, "Unidad / %s", cells[0]);
            format(document, sizeof document, "Callsign: %s\nMiembros: %s\nEstado: %s\nUbicacion: %s\n", cells[0], cells[1], cells[2], cells[3]);
        }
        case LIST_WEAPONS:
        {
            format(title, sizeof title, "Busqueda de armas / Serial");
            format(document, sizeof document, "Modelo: %s\nSerial: %s\nPropietario: %s\nRegistrada en: Armeria de demostracion\n", cells[0], cells[1], ODPListPerson[playerid]);
        }
    }
    strcat(document, "\nDATOS FICTICIOS / PROTOTIPO SIN CONEXION CON SARP.ES");
    return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, title, document);
}

stock ODPListAction(playerid, action)
{
    if (ODPPage[playerid] != 21) return 0;
    if (action >= 600 && action <= 603) return ODPOpenListRecord(playerid, ODPListPage[playerid] * 4 + action - 600);
    if (action == 650 && ODPListPage[playerid] > 0) ODPListPage[playerid]--;
    if (action == 651 && (ODPListPage[playerid] + 1) * 4 < ODPListTotal(playerid)) ODPListPage[playerid]++;
    if (action == 652)
    {
        if (ODPListParent[playerid] == -1) return ODPOpenList(playerid, LIST_DEBTORS, 2);
        return ODPRender(playerid, ODPListParent[playerid]);
    }
    return ODPRender(playerid, 21);
}
