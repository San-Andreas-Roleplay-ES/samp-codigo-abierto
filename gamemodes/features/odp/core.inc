#pragma dynamic 32768

// SA-MP 0.3DL. ASCII intentionally used for the legacy client/compiler.
// SARP module core. Lifecycle and commands are wired in header.inc.
// Replace fixture functions with the private gamemode's data providers.
#define ODP_MAX_TD 180
// Smaller and lower: reserve the upper third for the chat.
#define ODP_UI_SCALE (0.64)
#define PANEL_DOCUMENT 0
#define PANEL_INPUT 1
#define PANEL_CHOICES 2
#define D_SEARCH 24700
#define D_RESULT 24701
#define D_CALLSIGN 24702
#define D_STATUS 24703
// Worst case (an emission with every long field near 1024 chars) is ~6 KB;
// 6400 leaves margin without reserving the original 8192 for every player.
#define ODP_EMISSION_BODY_SIZE 6400
#define C_BG 0xDDDDDDFF
#define C_SIDE 0xCECECEFF
#define C_CARD 0xFFFFFFFF
#define C_LINE 0x999999FF
#define C_TEXT 0x202020FF
#define C_MUTED 0x555555FF
#define C_ACCENT 0x19245DFF
#define C_RED 0xA52018FF
#define C_AMBER 0x895B00FF

new PlayerText:ODPTD[MAX_PLAYERS][ODP_MAX_TD];
new ODPAction[MAX_PLAYERS][ODP_MAX_TD];
new ODPCount[MAX_PLAYERS];
// Slot of the PANEL_INPUT text line in ODPTD[playerid][], or -1 when the
// current screen has none. Lets ODPPanelAction update just that textdraw
// while typing instead of rebuilding the whole panel on every keystroke.
new ODPInputLineTD[MAX_PLAYERS] = {-1, ...};
new bool:ODPOpen[MAX_PLAYERS], bool:ODPPanel[MAX_PLAYERS];
new ODPPage[MAX_PLAYERS], ODPSearch[MAX_PLAYERS], ODPOrder[MAX_PLAYERS];
new ODPCallsign[MAX_PLAYERS][17];
new ODPStatus[MAX_PLAYERS];
new ODPReturnPage[MAX_PLAYERS], ODPPanelId[MAX_PLAYERS], ODPPanelStyle[MAX_PLAYERS];
new ODPDocumentPage[MAX_PLAYERS];
new ODPPanelTitle[MAX_PLAYERS][80], ODPDocument[MAX_PLAYERS][ODP_EMISSION_BODY_SIZE];
new ODPInput[MAX_PLAYERS][33], ODPSubmitLabel[MAX_PLAYERS][24];
new const ODPKeys[] = "1234567890QWERTYUIOPASDFGHJKLZXCVBNM-_";
new const ODPStatuses[][] = { "Disponible", "En intervencion", "Fuera de servicio" };
new const ODPNavigation[][] = {
    "Inicio", "Servicio y unidades", "Personas", "Vehiculos",
    "Propiedades", "Telefonia", "Armas", "Ordenes judiciales", "Actividad operativa", "Codigo penal", "Emisiones de crimenes"
};
#include "features/odp/records.inc"
#include "features/odp/emissions.inc"
new ODPPenalCategory[MAX_PLAYERS];
new const ODPPenalCategories[][] = {
    "Delitos contra las personas", "Delitos contra el patrimonio", "Orden publico",
    "Armas y materiales", "Seguridad vial", "Administracion de justicia"
};
new const ODPPenalArticles[][] = {
    "CP-DEMO-01 | Agresion", "CP-DEMO-02 | Robo de vehiculo", "CP-DEMO-03 | Desordenes publicos",
    "CP-DEMO-04 | Tenencia irregular", "CP-DEMO-05 | Conduccion temeraria", "CP-DEMO-06 | Evasion"
};
new const ODPPenalDescriptions[][] = {
    "Ejemplo de una conducta contra la integridad de una persona.",
    "Ejemplo de sustraccion de un vehiculo sin consentimiento.",
    "Ejemplo de alteracion del orden en un espacio publico.",
    "Ejemplo de posesion de un arma sin autorizacion aplicable.",
    "Ejemplo de conduccion que genera un peligro para terceros.",
    "Ejemplo de huida durante una intervencion policial."
};

stock ODPOpenPenalArticle(playerid, article)
{
    if (article < 0 || article >= sizeof ODPPenalArticles) return 0;
    new document[1024];
    format(document, sizeof document,
        "EJEMPLO FICTICIO / NO ES EL CODIGO PENAL DE SARP.ES\n\nCATEGORIA\n%s\n\nDESCRIPCION DE EJEMPLO\n%s\n\nSANCIONES\nMulta: pendiente del texto oficial.\nPrision: pendiente del texto oficial.\n\nOBSERVACIONES\nNumeracion, definicion y sanciones deben sustituirse\npor los articulos vigentes facilitados por el servidor.\nEste contenido sirve solo para evaluar la interfaz.",
        ODPPenalCategories[article], ODPPenalDescriptions[article]);
    return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, ODPPenalArticles[article], document);
}

stock ODPClear(playerid)
{
    for (new i; i < ODPCount[playerid]; i++) PlayerTextDrawDestroy(playerid, ODPTD[playerid][i]);
    ODPCount[playerid] = 0;
    ODPInputLineTD[playerid] = -1;
}

stock Float:ODPX(Float:x)
{
    return float(floatround(320.0 + (x - 320.0) * ODP_UI_SCALE));
}

stock Float:ODPY(Float:y)
{
    return float(floatround(290.0 + (y - 249.0) * ODP_UI_SCALE));
}

stock ODPBox(playerid, Float:x, Float:y, Float:w, Float:h, color, action = 0)
{
    if (ODPCount[playerid] >= ODP_MAX_TD) return -1;
    new i = ODPCount[playerid]++;
    ODPTD[playerid][i] = CreatePlayerTextDraw(playerid, ODPX(x), ODPY(y), "LD_SPAC:white");
    PlayerTextDrawFont(playerid, ODPTD[playerid][i], 4);
    PlayerTextDrawTextSize(playerid, ODPTD[playerid][i], ODPX(x + w) - ODPX(x), ODPY(y + h) - ODPY(y));
    PlayerTextDrawColor(playerid, ODPTD[playerid][i], color);
    PlayerTextDrawSetSelectable(playerid, ODPTD[playerid][i], action != 0);
    ODPAction[playerid][i] = action;
    return i;
}

stock ODPText(playerid, Float:x, Float:y, const text[], color = C_TEXT, Float:sx = 0.22, Float:sy = 1.10)
{
    if (ODPCount[playerid] >= ODP_MAX_TD) return -1;
    new i = ODPCount[playerid]++;
    ODPTD[playerid][i] = CreatePlayerTextDraw(playerid, ODPX(x), ODPY(y), text);
    PlayerTextDrawFont(playerid, ODPTD[playerid][i], 1);
    PlayerTextDrawLetterSize(playerid, ODPTD[playerid][i], sx * ODP_UI_SCALE, sy * ODP_UI_SCALE);
    PlayerTextDrawColor(playerid, ODPTD[playerid][i], color);
    PlayerTextDrawSetOutline(playerid, ODPTD[playerid][i], 0);
    PlayerTextDrawSetShadow(playerid, ODPTD[playerid][i], 0);
    PlayerTextDrawSetProportional(playerid, ODPTD[playerid][i], 1);
    ODPAction[playerid][i] = 0;
    return i;
}

stock ODPButton(playerid, Float:x, Float:y, Float:w, const text[], action, bool:accent = false)
{
    ODPBox(playerid, x, y, w, 24.0, C_LINE);
    ODPBox(playerid, x + 1.0, y + 1.0, w - 2.0, 22.0, accent ? C_ACCENT : 0xB8B8B8FF, action);
    ODPText(playerid, x + 8.0, y + 6.0, text, accent ? 0xFFFFFFFF : C_TEXT);
}

stock ODPPreview(playerid, Float:x, Float:y, Float:w, Float:h, model)
{
    if (ODPCount[playerid] >= ODP_MAX_TD) return -1;
    new i = ODPCount[playerid]++;
    ODPTD[playerid][i] = CreatePlayerTextDraw(playerid, ODPX(x), ODPY(y), "_");
    PlayerTextDrawFont(playerid, ODPTD[playerid][i], 5);
    PlayerTextDrawTextSize(playerid, ODPTD[playerid][i], w * ODP_UI_SCALE, h * ODP_UI_SCALE);
    PlayerTextDrawBackgroundColor(playerid, ODPTD[playerid][i], C_BG);
    PlayerTextDrawSetPreviewModel(playerid, ODPTD[playerid][i], model);
    PlayerTextDrawSetPreviewRot(playerid, ODPTD[playerid][i], 0.0, 0.0, -20.0, 1.0);
    ODPAction[playerid][i] = 0;
    return i;
}

new bool:ODPSearchDialog[MAX_PLAYERS];

// Searches use the physical keyboard; other panels remain TextDraws.
stock ODPShowPanel(playerid, panelid, style, title[], const body[], button[] = "Volver")
{
    if (!ODPPanel[playerid]) ODPReturnPage[playerid] = ODPPage[playerid];
    ODPPanelId[playerid] = panelid;
    ODPPanelStyle[playerid] = style;
    ODPDocumentPage[playerid] = 0;
    format(ODPPanelTitle[playerid], 80, "%s", title);
    ODPDocument[playerid][0] = EOS;
    strcat(ODPDocument[playerid], body, ODP_EMISSION_BODY_SIZE);
    format(ODPSubmitLabel[playerid], 24, "%s", button);
    ODPInput[playerid][0] = '\0';
    if (panelid == D_SEARCH)
    {
        ODPPanel[playerid] = true;
        ODPSearchDialog[playerid] = true;
        CancelSelectTextDraw(playerid);
        Dialog_Show(playerid, ODP_SEARCH_INPUT, DIALOG_STYLE_INPUT, title, body, "Buscar", "Atras");
        return 1;
    }
    return ODPRender(playerid, 20);
}

stock ODPSearchResponse(playerid, response, inputtext[])
{
    if (!ODPOpen[playerid] || !ODPSearchDialog[playerid]) return 0;
    ODPSearchDialog[playerid] = false;
    // easyDialog represents an empty response as the string "\1".
    if (inputtext[0] == '\1') inputtext[0] = '\0';
    if (response && strlen(inputtext) > 32)
        return ODPShowPanel(playerid, D_SEARCH, PANEL_INPUT, "Busqueda demasiado larga", "Introduce un maximo de 32 caracteres.", "Buscar");
    return ODPPanelResponse(playerid, D_SEARCH, response, 0, inputtext);
}

stock ODPDocumentLine(playerid, wanted, output[], capacity)
{
    new offset, row, length = strlen(ODPDocument[playerid]);
    output[0] = '\0';
    while (offset < length)
    {
        new end = offset, split = -1;
        while (end < length && end - offset < 62 && ODPDocument[playerid][end] != '\n')
        {
            if (ODPDocument[playerid][end] == ' ') split = end;
            end++;
        }
        if (end < length && ODPDocument[playerid][end] != '\n' && end - offset == 62 && split > offset) end = split;
        if (row == wanted)
        {
            strmid(output, ODPDocument[playerid], offset, end, capacity);
            for (new c; output[c]; c++) if (output[c] == '\t') output[c] = ' ';
            if (!strlen(output)) format(output, capacity, " ");
            return 1;
        }
        offset = end;
        if (ODPDocument[playerid][offset] == '\n' || ODPDocument[playerid][offset] == ' ') offset++;
        row++;
    }
    return 0;
}

stock ODPDrawPanel(playerid)
{
    ODPText(playerid, 193.0, 121.0, ODPPanelTitle[playerid], C_ACCENT, 0.25, 1.25);
    new line[80];
    if (ODPPanelStyle[playerid] == PANEL_INPUT)
    {
        // Two lines of hints; all input comes from the clickable keyboard.
        for (new row; row < 2; row++)
        {
            if (ODPDocumentLine(playerid, row, line, sizeof line)) ODPText(playerid, 193.0, 145.0 + row * 15.0, line, C_MUTED, 0.19, 0.95);
        }
        ODPBox(playerid, 193.0, 182.0, 399.0, 29.0, C_LINE);
        ODPBox(playerid, 194.0, 183.0, 397.0, 27.0, C_CARD);
        format(line, sizeof line, "%s_", ODPInput[playerid]);
        ODPInputLineTD[playerid] = ODPText(playerid, 201.0, 190.0, line, C_TEXT, 0.25, 1.25);
        for (new key; key < sizeof(ODPKeys) - 1; key++)
        {
            new glyph[2]; glyph[0] = ODPKeys[key]; glyph[1] = '\0';
            // Only two TDs per key to stay below the 180 budget.
            new Float:x = 193.0 + (key % 10) * 40.0;
            new Float:y = 223.0 + (key / 10) * 28.0;
            ODPBox(playerid, x, y, 35.0, 23.0, 0xB8B8B8FF, 500 + key);
            ODPText(playerid, x + 11.0, y + 5.0, glyph, C_TEXT, 0.24, 1.2);
        }
        ODPButton(playerid, 193.0, 338.0, 125.0, "Espacio", 401);
        ODPButton(playerid, 328.0, 338.0, 125.0, "Borrar", 402);
        ODPButton(playerid, 463.0, 338.0, 129.0, "Limpiar", 403);
        ODPButton(playerid, 193.0, 373.0, 194.0, ODPSubmitLabel[playerid], 404, true);
        ODPButton(playerid, 398.0, 373.0, 194.0, "Cancelar", 400);
    }
    else if (ODPPanelStyle[playerid] == PANEL_CHOICES)
    {
        for (new choice; choice < sizeof ODPStatuses; choice++) ODPButton(playerid, 193.0, 177.0 + choice * 36.0, 399.0, ODPStatuses[choice], 410 + choice);
        ODPButton(playerid, 193.0, 373.0, 194.0, "Volver", 400);
    }
    else
    {
        ODPBox(playerid, 193.0, 156.0, 399.0, 199.0, C_CARD);
        for (new row; row < 10; row++)
        {
            if (ODPDocumentLine(playerid, ODPDocumentPage[playerid] * 10 + row, line, sizeof line))
                ODPText(playerid, 203.0, 166.0 + row * 18.0, line, C_TEXT, 0.20, 1.0);
        }
        format(line, sizeof line, "Pagina %d", ODPDocumentPage[playerid] + 1);
        ODPText(playerid, 193.0, 358.0, line, C_MUTED, 0.18, 0.9);
        ODPButton(playerid, 193.0, 373.0, 125.0, "Volver", 400, true);
        if (ODPDocumentPage[playerid] > 0) ODPButton(playerid, 328.0, 373.0, 125.0, "Anterior", 405);
        if (ODPDocumentLine(playerid, (ODPDocumentPage[playerid] + 1) * 10, line, sizeof line)) ODPButton(playerid, 463.0, 373.0, 129.0, "Siguiente", 406);
    }
    return 1;
}

stock ODPPanelAction(playerid, action)
{
    if (!ODPPanel[playerid]) return 0;
    if (action == 400) return ODPPanelResponse(playerid, ODPPanelId[playerid], 0, 0, ODPInput[playerid]);
    if (action == 404) return ODPPanelResponse(playerid, ODPPanelId[playerid], 1, 0, ODPInput[playerid]);
    if (action >= 410 && action <= 412) return ODPPanelResponse(playerid, ODPPanelId[playerid], 1, action - 410, ODPInput[playerid]);
    if (action == 405 && ODPDocumentPage[playerid] > 0) { ODPDocumentPage[playerid]--; return ODPRender(playerid, 20); }
    if (action == 406) { ODPDocumentPage[playerid]++; return ODPRender(playerid, 20); }
    new bool:isKeyboardEdit = action == 402 || action == 403 || action == 401 || (action >= 500 && action < 500 + (sizeof(ODPKeys) - 1));
    if (!isKeyboardEdit) return ODPRender(playerid, 20);
    new length = strlen(ODPInput[playerid]);
    new limit = ODPPanelId[playerid] == D_CALLSIGN ? 16 : 32;
    if (action == 402 && length) ODPInput[playerid][length - 1] = '\0';
    if (action == 403) ODPInput[playerid][0] = '\0';
    if ((action == 401 || action >= 500) && length < limit)
    {
        ODPInput[playerid][length] = action == 401 ? ' ' : ODPKeys[action - 500];
        ODPInput[playerid][length + 1] = '\0';
    }
    // Only the input line changed: update that one textdraw in place instead
    // of tearing down and recreating the whole ~140-textdraw panel per key.
    if (ODPInputLineTD[playerid] != -1)
    {
        new line[40];
        format(line, sizeof line, "%s_", ODPInput[playerid]);
        PlayerTextDrawSetString(playerid, ODPTD[playerid][ODPInputLineTD[playerid]], line);
        PlayerTextDrawShow(playerid, ODPTD[playerid][ODPInputLineTD[playerid]]);
        return 1;
    }
    return ODPRender(playerid, 20);
}

stock ODPHeading(playerid, const title[], const subtitle[])
{
    ODPText(playerid, 193.0, 119.0, title, C_TEXT, 0.34, 1.65);
    ODPText(playerid, 193.0, 143.0, subtitle, C_MUTED, 0.20, 1.0);
}

stock ODPClose(playerid)
{
    ODPOpen[playerid] = false;
    if (ODPSearchDialog[playerid] || ODPEmissionInput[playerid]) ShowPlayerDialog(playerid, -1, DIALOG_STYLE_MSGBOX, "", "", "", "");
    ODPEmissionInput[playerid] = false;
    ODPSearchDialog[playerid] = false;
    ODPPanel[playerid] = false;
    CancelSelectTextDraw(playerid);
    ODPClear(playerid);
    return 1;
}


stock ODPFixtureResult(playerid, action)
{
    switch (action)
    {
        case 201: return ODPOpenList(playerid, LIST_UNITS, 1);
        case 213: return ODPOpenList(playerid, LIST_DEBTORS, 2);
        case 221: return ODPOpenList(playerid, LIST_STOLEN, 3);
        case 271: return ODPOpenList(playerid, LIST_SEARCH_WARRANTS, 7);
        case 280: return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Ultimos incidentes", "No se han reportado incidentes en las ultimas horas.\n\nEstado vacio de demostracion, basado en la captura 238.");
        case 281: return ODPOpenList(playerid, LIST_CALLS, 8);
    }
    return 0;
}

stock ODPRender(playerid, page)
{
    ODPClear(playerid);
    ODPPage[playerid] = page;
    ODPOpen[playerid] = true;
    ODPPanel[playerid] = page == 20;
    ODPBox(playerid, 30.0, 66.0, 580.0, 366.0, 0x070D14EE);
    ODPBox(playerid, 34.0, 70.0, 572.0, 358.0, C_BG);
    ODPBox(playerid, 34.0, 70.0, 572.0, 2.0, C_ACCENT);
    ODPBox(playerid, 34.0, 72.0, 572.0, 36.0, C_ACCENT);
    ODPText(playerid, 47.0, 80.0, "ODP", 0xFFFFFFFF, 0.36, 1.7);
    ODPText(playerid, 99.0, 79.0, "MOBILE DATA COMPUTER", 0xFFFFFFFF, 0.23, 1.10);
    ODPText(playerid, 99.0, 93.0, "TERMINAL COMPARTIDO ENTRE AGENCIAS", 0xD6DBEEFF, 0.16, 0.8);
    ODPText(playerid, 412.0, 80.0, "ENTORNO DE PRUEBAS", 0xFFFFFFFF, 0.19, 0.95);
    ODPText(playerid, 412.0, 94.0, "ODP / V0.4", 0xD6DBEEFF, 0.16, 0.8);
    ODPBox(playerid, 560.0, 78.0, 34.0, 24.0, C_RED, 1);
    ODPText(playerid, 573.0, 83.0, "X", 0xFFFFFFFF, 0.25, 1.25);
    ODPBox(playerid, 34.0, 109.0, 145.0, 299.0, C_SIDE);
    ODPBox(playerid, 178.0, 109.0, 1.0, 299.0, C_LINE);
    new name[MAX_PLAYER_NAME], line[72];
    GetPlayerName(playerid, name, sizeof name);
    ODPText(playerid, 46.0, 121.0, "SESION DE AGENTE", C_MUTED, 0.17, 0.85);
    ODPText(playerid, 46.0, 136.0, name, C_TEXT, 0.20, 1.0);
    format(line, sizeof line, "Unidad: %s", ODPCallsign[playerid]);
    ODPText(playerid, 46.0, 151.0, line, C_ACCENT, 0.18, 0.9);
    new selected = page;
    if (page == 20) selected = ODPReturnPage[playerid];
    if (selected == 21) selected = ODPListSection(playerid);
    if (selected == 9) selected = 2;
    if (selected == 12) selected = 3;
    if (selected == 13 || selected == 14) selected = 9;
    if (selected == 10 || selected == 11) selected = 7;
    if (selected == 22 || selected == 23) selected = 10;
    for (new navIndex; navIndex < sizeof ODPNavigation; navIndex++)
    {
        new Float:y = 174.0 + navIndex * 21.0;
        ODPBox(playerid, 42.0, y, 129.0, 19.0, selected == navIndex ? C_ACCENT : 0xB8B8B8FF, 100 + navIndex);
        ODPText(playerid, 50.0, y + 5.0, ODPNavigation[navIndex], selected == navIndex ? 0xFFFFFFFF : C_TEXT, 0.21, 1.05);
    }
    ODPBox(playerid, 34.0, 408.0, 572.0, 20.0, C_SIDE);
    ODPText(playerid, 46.0, 414.0, "ODP / DEMOSTRACION LOCAL", C_MUTED, 0.17, 0.85);
    ODPText(playerid, 372.0, 414.0, "ESC CERRAR   |   /odphelp AYUDA", C_MUTED, 0.17, 0.85);
    switch (page)
    {
        case 22: ODPEmissionListDraw(playerid);
        case 23: ODPEmissionFormDraw(playerid);
        case 21: ODPDrawList(playerid);
        case 13:
        {
            ODPHeading(playerid, "Codigo penal", "Biblioteca de consulta / Selecciona una categoria.");
            for (new category; category < sizeof ODPPenalCategories; category++)
                ODPButton(playerid, 193.0, 173.0 + category * 31.0, 399.0, ODPPenalCategories[category], 310 + category);
            ODPBox(playerid, 193.0, 367.0, 399.0, 30.0, 0xEAC96CFF);
            ODPText(playerid, 202.0, 377.0, "Ejemplos ficticios. Pendiente del codigo oficial de sarp.es.", C_TEXT, 0.19, 0.95);
        }
        case 14:
        {
            new category = ODPPenalCategory[playerid];
            ODPHeading(playerid, "Codigo penal / Articulos", ODPPenalCategories[category]);
            ODPButton(playerid, 193.0, 178.0, 399.0, ODPPenalArticles[category], 320 + category, true);
            ODPText(playerid, 193.0, 229.0, "1 articulo de demostracion en esta categoria.", C_MUTED, 0.21, 1.05);
            ODPText(playerid, 193.0, 255.0, "Abre la ficha para consultar descripcion y sanciones.", C_MUTED, 0.20, 1.0);
            ODPButton(playerid, 193.0, 365.0, 194.0, "Volver a categorias", 109);
        }
        case 20: ODPDrawPanel(playerid);
        case 0:
        {
            ODPHeading(playerid, "Centro de operaciones", "Consultas y comunicaciones compartidas entre agencias.");
            ODPPreview(playerid, 193.0, 159.0, 90.0, 90.0, GetPlayerSkin(playerid));
            ODPBox(playerid, 295.0, 170.0, 297.0, 23.0, 0xB8B8B8FF);
            ODPText(playerid, 303.0, 176.0, name, C_TEXT, 0.24, 1.2);
            ODPText(playerid, 303.0, 203.0, "ORDENES ACTIVAS", C_TEXT, 0.21, 1.05);
            ODPText(playerid, 561.0, 203.0, "07", C_RED, 0.23, 1.15);
            ODPText(playerid, 303.0, 220.0, "UNIDADES EN TURNO", C_TEXT, 0.21, 1.05);
            ODPText(playerid, 561.0, 220.0, "02", C_ACCENT, 0.23, 1.15);
            ODPText(playerid, 303.0, 237.0, "LLAMADOS ABIERTOS", C_TEXT, 0.21, 1.05);
            ODPText(playerid, 561.0, 237.0, "02", C_AMBER, 0.23, 1.15);
            ODPText(playerid, 193.0, 253.0, "ACCESOS FRECUENTES", C_MUTED, 0.19, 0.95);
            ODPButton(playerid, 193.0, 275.0, 194.0, "Buscar persona", 210, true);
            ODPButton(playerid, 398.0, 275.0, 194.0, "Consultar matricula", 220);
            ODPButton(playerid, 193.0, 307.0, 194.0, "Ordenes de arresto", 270);
            ODPButton(playerid, 398.0, 307.0, 194.0, "Llamados de servicio", 281);
            ODPBox(playerid, 193.0, 349.0, 399.0, 43.0, C_CARD);
            ODPText(playerid, 204.0, 358.0, "LABORATORIO VISUAL / SIN DATOS REALES", C_ACCENT, 0.20, 1.0);
            ODPText(playerid, 204.0, 375.0, "Prueba: Alex Morgan, Daniel Vega o matricula DEMO-01.", C_MUTED, 0.19, 0.95);
        }
        case 1:
        {
            ODPHeading(playerid, "Servicio y unidades", "Identificacion operativa y personal en turno.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Cambiar estado / callsign", 200, true);
            ODPButton(playerid, 193.0, 212.0, 399.0, "Ver callsigns en turno [2 demo]", 201);
            ODPButton(playerid, 193.0, 247.0, 399.0, "Cambiar estado de servicio", 202);
            format(line, sizeof line, "Estado actual: %s", ODPStatuses[ODPStatus[playerid]]);
            ODPText(playerid, 193.0, 291.0, line, C_ACCENT);
            ODPText(playerid, 193.0, 320.0, "Los cambios duran solo durante esta conexion.", C_MUTED);
        }
        case 2:
        {
            ODPHeading(playerid, "Personas", "Identificacion, historial y obligaciones pendientes.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Buscar informacion por nombre", 210, true);
            ODPButton(playerid, 193.0, 212.0, 399.0, "Buscar arrestos por nombre", 211);
            ODPButton(playerid, 193.0, 247.0, 399.0, "Buscar multas por nombre", 212);
            ODPButton(playerid, 193.0, 282.0, 399.0, "Ver personas con multas impagadas [5 demo]", 213);
            ODPText(playerid, 193.0, 335.0, "Registros demo: Alex Morgan / Daniel Vega", C_MUTED);
        }
        case 3:
        {
            ODPHeading(playerid, "Vehiculos", "Titularidad y denuncias de sustraccion.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Buscar informacion por matricula", 220, true);
            ODPButton(playerid, 193.0, 212.0, 399.0, "Ver denuncias de vehiculos robados [6 demo]", 221);
        }
        case 4:
        {
            ODPHeading(playerid, "Propiedades", "Consulta de inmuebles y titularidad.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Buscar informacion por propiedad", 230, true);
        }
        case 5:
        {
            ODPHeading(playerid, "Telefonia", "Consulta de abonados por numero de telefono.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Buscar informacion por telefono", 240, true);
        }
        case 6:
        {
            ODPHeading(playerid, "Armas", "Registro de armas y propietarios.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Buscar informacion por serial de arma", 250, true);
            ODPButton(playerid, 193.0, 212.0, 399.0, "Buscar armas por propietario", 251);
        }
        case 7:
        {
            ODPHeading(playerid, "Ordenes judiciales", "Mandatos pendientes y nivel de riesgo.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Ordenes de arresto pendientes [7 demo]", 270, true);
            ODPButton(playerid, 193.0, 212.0, 399.0, "Ordenes de allanamiento pendientes [3 demo]", 271);
        }
        case 8:
        {
            ODPHeading(playerid, "Actividad operativa", "Seguimiento de incidentes y avisos de servicio.");
            ODPButton(playerid, 193.0, 177.0, 399.0, "Ver ultimos incidentes [0 demo]", 280, true);
            ODPButton(playerid, 193.0, 212.0, 399.0, "Ver ultimos llamados de servicio [2 demo]", 281);
        }
        case 9:
        {
            new bool:vega = ODPOrder[playerid] == 0;
            ODPHeading(playerid, "Ficha de persona", "PERSONAS / RESULTADO DE CONSULTA");
            ODPPreview(playerid, 193.0, 167.0, 65.0, 68.0, vega ? 7 : 280);
            ODPBox(playerid, 267.0, 173.0, 325.0, 53.0, C_CARD);
            if (vega) ODPText(playerid, 276.0, 181.0, "Daniel Vega", C_TEXT, 0.30, 1.5); else ODPText(playerid, 276.0, 181.0, "Alex Morgan", C_TEXT, 0.30, 1.5);
            if (vega) ODPText(playerid, 276.0, 207.0, "DEMO-P002  /  34 anos", C_MUTED); else ODPText(playerid, 276.0, 207.0, "DEMO-P001  /  29 anos", C_MUTED);
            if (vega) ODPText(playerid, 431.0, 191.0, "ORDEN ACTIVA", vega ? C_RED : C_ACCENT); else ODPText(playerid, 431.0, 191.0, "SIN ALERTAS", vega ? C_RED : C_ACCENT);
            ODPText(playerid, 193.0, 244.0, "LICENCIAS", C_MUTED, 0.18, 0.9);
            ODPText(playerid, 193.0, 260.0, "Conducir: vigente", C_TEXT);
            ODPText(playerid, 403.0, 244.0, "TELEFONO", C_MUTED, 0.18, 0.9);
            if (vega) ODPText(playerid, 403.0, 260.0, "5550102", C_TEXT); else ODPText(playerid, 403.0, 260.0, "5550101", C_TEXT);
            ODPText(playerid, 193.0, 289.0, "PROPIEDADES", C_MUTED, 0.18, 0.9);
            if (vega) ODPText(playerid, 193.0, 305.0, "DEMO-P01 / Idlewood", C_TEXT, 0.20, 1.0); else ODPText(playerid, 193.0, 305.0, "Sin propiedades registradas", C_TEXT, 0.20, 1.0);
            ODPText(playerid, 403.0, 289.0, "VEHICULOS", C_MUTED, 0.18, 0.9);
            if (vega) ODPText(playerid, 403.0, 305.0, "DEMO-01 / Premier", C_TEXT, 0.20, 1.0); else ODPText(playerid, 403.0, 305.0, "Sin vehiculos registrados", C_TEXT, 0.20, 1.0);
            ODPButton(playerid, 193.0, 353.0, 125.0, "Arrestos", 290);
            ODPButton(playerid, 326.0, 353.0, 125.0, "Multas", 291);
            ODPButton(playerid, 459.0, 353.0, 133.0, "Otra consulta", 210, true);
        }
        case 10:
        {
            ODPHeading(playerid, "Ordenes de arresto", "2 pendientes / Selecciona una fila para abrir su expediente.");
            ODPBox(playerid, 193.0, 174.0, 399.0, 23.0, C_CARD);
            ODPText(playerid, 202.0, 180.0, "EXPEDIENTE / PERSONA", C_MUTED, 0.18, 0.9);
            ODPText(playerid, 422.0, 180.0, "RIESGO", C_MUTED, 0.18, 0.9);
            ODPText(playerid, 500.0, 180.0, "FECHA", C_MUTED, 0.18, 0.9);
            for (new row; row < 2; row++)
            {
                new Float:y = 202.0 + row * 55.0;
                ODPBox(playerid, 193.0, y, 399.0, 48.0, C_CARD, 300 + row);
                if (row)
                {
                    ODPText(playerid, 202.0, y + 7.0, "Taylor Brooks", C_TEXT, 0.25, 1.25);
                    ODPText(playerid, 202.0, y + 29.0, "DEMO-002 / Agente J. Reyes", C_MUTED, 0.18, 0.9);
                    ODPText(playerid, 422.0, y + 17.0, "BAJO", C_AMBER);
                }
                else
                {
                    ODPText(playerid, 202.0, y + 7.0, "Daniel Vega", C_TEXT, 0.25, 1.25);
                    ODPText(playerid, 202.0, y + 29.0, "DEMO-001 / Agente A. Morgan", C_MUTED, 0.18, 0.9);
                    ODPText(playerid, 422.0, y + 17.0, "ALTO", C_RED);
                }
                ODPText(playerid, 500.0, y + 17.0, "17/09/2026", C_MUTED, 0.19, 0.95);
            }
            ODPText(playerid, 193.0, 328.0, "Registros ficticios para probar lectura y seleccion.", C_MUTED, 0.20, 1.0);
            ODPButton(playerid, 193.0, 362.0, 165.0, "Volver a ordenes", 107);
        }
        case 11:
        {
            new bool:low = ODPOrder[playerid] == 1;
            if (low) ODPHeading(playerid, "DEMO-002 / Taylor Brooks", "ORDENES JUDICIALES / DETALLE DEL EXPEDIENTE"); else ODPHeading(playerid, "DEMO-001 / Daniel Vega", "ORDENES JUDICIALES / DETALLE DEL EXPEDIENTE");
            ODPBox(playerid, 193.0, 173.0, 399.0, 37.0, C_CARD);
            ODPText(playerid, 203.0, 184.0, "ESTADO: ABIERTA", C_ACCENT);
            if (low) ODPText(playerid, 410.0, 184.0, "RIESGO: BAJO", low ? C_AMBER : C_RED); else ODPText(playerid, 410.0, 184.0, "RIESGO: ALTO", low ? C_AMBER : C_RED);
            ODPText(playerid, 193.0, 224.0, "HECHOS / RESUMEN", C_MUTED, 0.18, 0.9);
            if (low) ODPText(playerid, 193.0, 242.0, "Incomparecencia vinculada al expediente DEMO-I013.~n~Localizar e identificar a la persona requerida.", C_TEXT, 0.21, 1.10); else ODPText(playerid, 193.0, 242.0, "Persona requerida por el incidente ficticio DEMO-I014.~n~Huida en vehiculo durante una intervencion policial.~n~Consultar las instrucciones del expediente completo.", C_TEXT, 0.21, 1.10);
            ODPText(playerid, 193.0, 291.0, "CARGOS", C_MUTED, 0.18, 0.9);
            if (low) ODPText(playerid, 193.0, 309.0, "Incomparecencia (ejemplo ficticio)", C_AMBER, 0.21, 1.05); else ODPText(playerid, 193.0, 309.0, "Evasion / Robo de vehiculo (ejemplos ficticios)", C_AMBER, 0.21, 1.05);
            if (low) ODPText(playerid, 193.0, 333.0, "Agente: Jordan Reyes | Fecha: 17/09/2026", C_MUTED, 0.20, 1.0); else ODPText(playerid, 193.0, 333.0, "Agente: Alex Morgan | Fecha: 17/09/2026", C_MUTED, 0.20, 1.0);
            ODPButton(playerid, 193.0, 365.0, 194.0, "Leer expediente completo", 302, true);
            ODPButton(playerid, 398.0, 365.0, 194.0, "Volver al listado", 270);
        }
        case 12:
        {
            ODPHeading(playerid, "Ficha de vehiculo", "VEHICULOS / RESULTADO POR MATRICULA");
            ODPPreview(playerid, 193.0, 174.0, 150.0, 100.0, 426);
            ODPBox(playerid, 355.0, 174.0, 237.0, 25.0, C_ACCENT);
            ODPText(playerid, 364.0, 180.0, "DEMO-01 / PREMIER", 0xFFFFFFFF, 0.24, 1.2);
            ODPText(playerid, 355.0, 215.0, "Titular: Daniel Vega", C_TEXT, 0.23, 1.15);
            ODPText(playerid, 355.0, 239.0, "Color registrado: azul", C_TEXT, 0.22, 1.1);
            ODPText(playerid, 355.0, 263.0, "Robo activo: No", C_TEXT, 0.22, 1.1);
            ODPBox(playerid, 193.0, 302.0, 399.0, 31.0, 0xEAC96CFF);
            ODPText(playerid, 202.0, 312.0, "Vehiculo y titular ficticios para esta demostracion.", C_TEXT, 0.20, 1.0);
            ODPButton(playerid, 193.0, 365.0, 194.0, "Otra matricula", 220, true);
            ODPButton(playerid, 398.0, 365.0, 194.0, "Volver a vehiculos", 103);
        }
    }
    for (new i; i < ODPCount[playerid]; i++) PlayerTextDrawShow(playerid, ODPTD[playerid][i]);
    SelectTextDraw(playerid, 0x426F79FF);
    return 1;
}

stock ODPSearchPanel(playerid, action)
{
    ODPSearch[playerid] = action;
    new prompt[256], title[64];
    switch (action)
    {
        case 210: title = "Busqueda de personas";
        case 211: title = "Busqueda de arrestos";
        case 212: title = "Busqueda de multas";
        case 220: title = "Busqueda de vehiculos";
        case 230: title = "Busqueda de propiedades";
        case 240: title = "Busqueda de telefono";
        case 250, 251: title = "Busqueda de armas";
    }
    switch (action)
    {
        case 210, 211, 212: prompt = "Introduce un nombre o parte del nombre.\nRegistros ficticios: Alex Morgan / Daniel Vega";
        case 220: prompt = "Introduce la matricula.\nRegistro ficticio: DEMO-01";
        case 230: prompt = "Introduce el numero de propiedad.\nRegistros ficticios: DEMO-P01 / DEMO-P02";
        case 240: prompt = "Introduce el telefono.\nRegistro ficticio: 5550102";
        case 250: prompt = "Introduce el serial de arma.\nRegistro ficticio: DEMO-W01";
        case 251: prompt = "Introduce el propietario.\nRegistros ficticios: Daniel Vega / Alex Morgan";
    }
    ODPShowPanel(playerid, D_SEARCH, PANEL_INPUT, title, prompt, "Buscar");
    return 1;
}

stock ODPHistory(playerid, bool:fines, bool:vega)
{
    if (!vega)
    {
        if (fines) return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Busqueda de multas", "Alex Morgan no tiene multas registradas.\n\nDatos ficticios.");
        return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Busqueda de arrestos", "Alex Morgan no tiene ningun reporte de arresto.\n\nDatos ficticios.");
    }
    format(ODPListPerson[playerid], 32, "Daniel Vega");
    return ODPOpenList(playerid, fines ? LIST_FINES : LIST_ARRESTS, ODPPage[playerid]);
}

stock ODPInit()
{
    ODPEmissionCount = 0;
    for (new p; p < MAX_PLAYERS; p++) format(ODPCallsign[p], 17, "SIN ASIGNAR");
    print("[ODP] Multi-agency v0.4 loaded. /odp | Demo queries; shared session emissions.");
    return 1;
}

stock ODPExit()
{
    for (new p; p < MAX_PLAYERS; p++) if (ODPOpen[p]) ODPClose(p);
    return 1;
}

stock ODPConnect(playerid)
{
    ODPEmissionResetDraft(playerid);
    ODPEmissionListPage[playerid] = 0;
    ODPSearchDialog[playerid] = false;
    ODPCount[playerid] = 0;
    ODPOpen[playerid] = false;
    ODPPanel[playerid] = false;
    ODPOrder[playerid] = 0;
    ODPStatus[playerid] = 0;
    format(ODPCallsign[playerid], 17, "SIN ASIGNAR");
    return 1;
}

stock ODPDisconnect(playerid, reason)
{
    ODPEmissionResetDraft(playerid);
    ODPSearchDialog[playerid] = false;
    #pragma unused reason
    // Per-player textdraws are destroyed by SA-MP on disconnect.
    ODPCount[playerid] = 0;
    ODPOpen[playerid] = false;
    ODPPanel[playerid] = false;
    return 1;
}

stock ODPCommand(playerid, const cmdtext[])
{
    if (!strcmp(cmdtext, "/odp", true))
    {
        if (ODPOpen[playerid]) ODPClose(playerid);
        else ODPRender(playerid, 0);
        return 1;
    }
    if (!strcmp(cmdtext, "/odphelp", true))
    {
        SendClientMessage(playerid, 0xB9CDFFFF, "ODP: /odp abre/cierra. Raton para navegar, ESC para salir. F7 cambia la visibilidad del chat.");
        SendClientMessage(playerid, 0xFFFFFFFF, "Demo: Alex Morgan / Daniel Vega / DEMO-01 / DEMO-P01 / 5550102 / DEMO-W01.");
        SendClientMessage(playerid, 0xFFFFFFFF, "Busqueda: escribe con tu teclado y pulsa ENTER. ESC vuelve al ODP.");
        return 1;
    }
    return 0;
}

stock ODPClickGlobal(playerid, Text:clickedid)
{
    if (ODPSearchDialog[playerid] || ODPEmissionInput[playerid]) return 0;
    if (clickedid == Text:INVALID_TEXT_DRAW && ODPOpen[playerid] ) return ODPClose(playerid);
    return 0;
}

stock ODPClickPlayer(playerid, PlayerText:playertextid)
{
    if (!ODPOpen[playerid]) return 0;
    new action;
    for (new i; i < ODPCount[playerid]; i++) if (ODPTD[playerid][i] == playertextid) { action = ODPAction[playerid][i]; break; }
    return ODPHandleAction(playerid, action);
}

stock ODPHandleAction(playerid, action)
{
    if (!ODPOpen[playerid] || !action) return 0;
    if (action >= 700) return ODPEmissionAction(playerid, action);
    if (action >= 600) return ODPListAction(playerid, action);
    if (action >= 400) return ODPPanelAction(playerid, action);
    if (action == 1) return ODPClose(playerid);
    if (action == 109) return ODPRender(playerid, 13);
    if (action == 110) { ODPEmissionListPage[playerid] = 0; return ODPRender(playerid, 22); }
    if (action >= 310 && action <= 315)
    {
        ODPPenalCategory[playerid] = action - 310;
        return ODPRender(playerid, 14);
    }
    if (action >= 320 && action <= 325) return ODPOpenPenalArticle(playerid, action - 320);
    if (action >= 100 && action <= 108) return ODPRender(playerid, action - 100);
    switch (action)
    {
        case 200: ODPShowPanel(playerid, D_CALLSIGN, PANEL_INPUT, "DEMO | Estado del callsign", "Escribe tu callsign (maximo 16 caracteres).\nEjemplo: 1-ADAM-12\nEscribe SIN ASIGNAR para quedar sin callsign.", "Guardar");
        case 202: ODPShowPanel(playerid, D_STATUS, PANEL_CHOICES, "DEMO | Estado de servicio", "Disponible\nEn intervencion\nFuera de servicio", "Guardar");
        case 210, 211, 212, 220, 230, 240, 250, 251: ODPSearchPanel(playerid, action);
        case 270: ODPOpenList(playerid, LIST_ARREST_WARRANTS, 7);
        case 290: ODPHistory(playerid, false, ODPOrder[playerid] == 0);
        case 291: ODPHistory(playerid, true, ODPOrder[playerid] == 0);
        case 300, 301: { ODPOrder[playerid] = action - 300; ODPRender(playerid, 11); }
        case 302:
        {
            if (ODPOrder[playerid] == 0) {
                new document[1024];
                format(document, sizeof document,
                    "Sospechoso: Daniel Vega\nEstado: Abierta | Riesgo: Alto\nFecha: 17/09/2026 | Agente: Alex Morgan\n\nHECHOS\nEn una intervencion ficticia en Idlewood, la persona identificada\nabandono el lugar en un vehiculo denunciado como sustraido.\nLa unidad documento los hechos en el expediente DEMO-I014.\n\nINSTRUCCIONES\nLocalizar e identificar al requerido. Coordinar la intervencion\ncon las unidades disponibles segun los protocolos del servidor.\n\nCARGOS DE EJEMPLO\nEvasion y robo de vehiculo.\n\nFirma: Supervisor de demostracion\nTodos los nombres y hechos de este prototipo son ficticios.");
                ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "DEMO-001 | Orden de arresto", document);
            }
            else {
                new document[1024];
                format(document, sizeof document,
                    "Sospechoso: Taylor Brooks\nEstado: Abierta | Riesgo: Bajo\nFecha: 17/09/2026 | Agente: Jordan Reyes\n\nHECHOS\nIncomparecencia en una citacion del expediente ficticio DEMO-I013.\n\nINSTRUCCIONES\nLocalizar e identificar a la persona requerida.\n\nCargo de ejemplo: Incomparecencia.\nFirma: Supervisor de demostracion\n\nTodos los nombres y hechos de este prototipo son ficticios.");
                ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "DEMO-002 | Orden de arresto", document);
            }
        }
        default: ODPFixtureResult(playerid, action);
    }
    return 1;
}

stock ODPPanelResponse(playerid, panelid, response, listitem, inputtext[])
{
    if (!ODPOpen[playerid] || !ODPPanel[playerid] || panelid < D_SEARCH || panelid > D_STATUS) return 0;
    ODPPage[playerid] = ODPReturnPage[playerid];
    ODPPanel[playerid] = false;
    if (!response || panelid == D_RESULT) return ODPRender(playerid, ODPPage[playerid]);
    if (panelid == D_STATUS)
    {
        if (listitem >= 0 && listitem < sizeof ODPStatuses) ODPStatus[playerid] = listitem;
        return ODPRender(playerid, 1);
    }
    if (panelid == D_CALLSIGN)
    {
        new bool:valid = strlen(inputtext) > 0 && strlen(inputtext) <= 16;
        for (new i; inputtext[i] != '\0'; i++)
            if (!(inputtext[i] >= 'A' && inputtext[i] <= 'Z') && !(inputtext[i] >= 'a' && inputtext[i] <= 'z') && !(inputtext[i] >= '0' && inputtext[i] <= '9') && inputtext[i] != '-' && inputtext[i] != ' ') valid = false;
        if (!valid) return ODPShowPanel(playerid, D_CALLSIGN, PANEL_INPUT, "ODP | Callsign no valido", "Utiliza de 1 a 16 letras, numeros, espacios o guiones.", "Guardar");
        format(ODPCallsign[playerid], 17, "%s", inputtext);
        return ODPRender(playerid, 1);
    }
    new action = ODPSearch[playerid];
    if (!strlen(inputtext)) return ODPSearchPanel(playerid, action);
    // The original ODP accepts character names with spaces or underscores.
    for (new c; inputtext[c]; c++) if (inputtext[c] == '_') inputtext[c] = ' ';
    new bool:vega = strfind("Daniel Vega", inputtext, true) != -1;
    new bool:alex = strfind("Alex Morgan", inputtext, true) != -1;
    if (action == 210 || action == 211 || action == 212)
    {
        if (vega && alex) return ODPShowPanel(playerid, D_SEARCH, PANEL_INPUT, "ODP | Concreta la busqueda", "La consulta coincide con Alex Morgan y Daniel Vega.\nIntroduce un nombre mas completo.", "Buscar");
        if (vega || alex)
        {
            if (action == 210) { ODPOrder[playerid] = vega ? 0 : 1; return ODPRender(playerid, 9); }
            return ODPHistory(playerid, action == 212, vega);
        }
    }
    if (action == 220 && !strcmp(inputtext, "DEMO-01", true)) return ODPRender(playerid, 12);
    if (action == 230 && !strcmp(inputtext, "DEMO-P01", true)) return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Busqueda de propiedades", "RESULTADOS POR PROPIEDAD\n\nNumero: DEMO-P01\nTipo: Casa\nDireccion: Calle de ejemplo, Idlewood, Los Santos\nDueno: Daniel Vega\n\nDatos ficticios.");
    if (action == 230 && !strcmp(inputtext, "DEMO-P02", true)) return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Busqueda de propiedades", "RESULTADOS POR PROPIEDAD\n\nNumero: DEMO-P02\nTipo: Especial\nDireccion: Avenida de ejemplo, Rodeo, Los Santos\nDueno: Sin informacion disponible\n\nDatos ficticios.");
    if (action == 240 && !strcmp(inputtext, "5550102", true)) return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Busqueda de telefono", "RESULTADOS POR NUMERO DE TELEFONO\n\nNumero: 5550102\nTipo de linea: Personal\nTitular: Daniel Vega\nFecha de contrato: 2025-06-30 09:44:46\n\nDatos ficticios.");
    if (action == 250 && !strcmp(inputtext, "DEMO-W01", true)) return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Busqueda de armas", "RESULTADOS POR SERIAL DE ARMA\n\nModelo: Pistola\nSerial: DEMO-W01\nPropietario: Daniel Vega\nRegistrada en: Armeria de demostracion\n\nDatos ficticios.");
    if (action == 251 && vega)
    {
        format(ODPListPerson[playerid], 32, "Daniel Vega");
        return ODPOpenList(playerid, LIST_WEAPONS, ODPPage[playerid]);
    }
    if (action == 251 && alex) return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "Armas registradas de Alex Morgan", "No tiene armas con seriales registradas a su nombre.\n\nDatos ficticios.");
    return ODPShowPanel(playerid, D_RESULT, PANEL_DOCUMENT, "ODP | Sin resultados", "No hay coincidencias en los datos de demostracion.\nVuelve a la consulta y utiliza el ejemplo indicado.");
}
