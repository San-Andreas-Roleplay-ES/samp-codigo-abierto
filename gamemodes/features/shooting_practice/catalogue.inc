#if defined SP_CATALOGUE_INCLUDED
    #endinput
#endif
#define SP_CATALOGUE_INCLUDED

#define PRACTICE_CATALOGUE_CARDS (6)
#define PRACTICE_CATALOGUE_DRAW_LIMIT (40)
#define PRACTICE_CATALOGUE_OPEN_DELAY (150)
#define SP_CATALOGUE_BACKGROUND (0x101922E6)
#define SP_CATALOGUE_CARD (0x233240FF)
#define SP_CATALOGUE_SELECTED (0x354C60FF)
#define SP_CATALOGUE_ACCENT (0xA8C4D9FF)
#define SP_CATALOGUE_MUTED (0xB4BEC8FF)
#define SP_CATALOGUE_DISABLED (0x75808AFF)

enum {
    SP_CATALOGUE_NONE = -1, SP_CATALOGUE_BACK = -2,
    SP_CATALOGUE_PREVIOUS = -3, SP_CATALOGUE_NEXT = -4,
    SP_CATALOGUE_CATEGORY = -20
}
new const PracticeCategoryNames[][12] = { "Todos", "Siluetas", "Botellas", "Latas", "Otros" };
new bool:PracticeCatalogueOpen[MAX_PLAYERS], bool:PracticeCatalogueSelecting[MAX_PLAYERS];
new bool:PracticeCatalogueFailed[MAX_PLAYERS], bool:PracticeCataloguePending[MAX_PLAYERS];
new PracticeCatalogueRequestedAt[MAX_PLAYERS], PracticeCatalogueCancelEchoes[MAX_PLAYERS];
new E_PRACTICE_CATEGORY:PracticeCatalogueCategory[MAX_PLAYERS];
new PracticeCataloguePage[MAX_PLAYERS], PracticeCatalogueDrawCount[MAX_PLAYERS];
new PlayerText:PracticeCatalogueDraws[MAX_PLAYERS][PRACTICE_CATALOGUE_DRAW_LIMIT];
new PracticeCatalogueActions[MAX_PLAYERS][PRACTICE_CATALOGUE_DRAW_LIMIT];

stock PracticeHideCatalogue(playerid, bool:closeSelection = true)
{
    // CancelSelectTextDraw emits an INVALID_TEXT_DRAW callback. Clear ownership
    // before calling it so cancellation cannot re-enter the back action.
    PracticeCatalogueOpen[playerid] = false;
    PracticeCataloguePending[playerid] = false;
    if (closeSelection && PracticeCatalogueSelecting[playerid])
    {
        PracticeCatalogueSelecting[playerid] = false;
        if (IsPlayerConnected(playerid))
        {
            // SA-MP can report the cancellation after the next panel has opened.
            PracticeCatalogueCancelEchoes[playerid]++;
            CancelSelectTextDraw(playerid);
        }
    }
    for (new i = 0; i < PracticeCatalogueDrawCount[playerid]; i++)
    {
        PlayerTextDrawDestroy(playerid, PracticeCatalogueDraws[playerid][i]);
        PracticeCatalogueDraws[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
        PracticeCatalogueActions[playerid][i] = SP_CATALOGUE_NONE;
    }
    PracticeCatalogueDrawCount[playerid] = 0;
    return 1;
}

stock PlayerText:PracticeCatalogueAdd(playerid, Float:x, Float:y, const text[], action = SP_CATALOGUE_NONE)
{
    if (PracticeCatalogueFailed[playerid]) return PlayerText:INVALID_TEXT_DRAW;
    new count = PracticeCatalogueDrawCount[playerid];
    if (count >= PRACTICE_CATALOGUE_DRAW_LIMIT)
    {
        PracticeCatalogueFailed[playerid] = true;
        return PlayerText:INVALID_TEXT_DRAW;
    }
    new PlayerText:draw = CreatePlayerTextDraw(playerid, x, y, text);
    if (draw == PlayerText:INVALID_TEXT_DRAW)
    {
        PracticeCatalogueFailed[playerid] = true;
        return draw;
    }
    PracticeCatalogueDraws[playerid][count] = draw;
    PracticeCatalogueActions[playerid][count] = action;
    PracticeCatalogueDrawCount[playerid]++;
    PlayerTextDrawFont(playerid, draw, 1);
    PlayerTextDrawSetShadow(playerid, draw, 0);
    PlayerTextDrawSetOutline(playerid, draw, 0);
    PlayerTextDrawBackgroundColor(playerid, draw, 0);
    PlayerTextDrawSetProportional(playerid, draw, 1);
    PlayerTextDrawSetSelectable(playerid, draw, action != SP_CATALOGUE_NONE);
    return draw;
}

stock PracticeCatalogueBox(playerid, Float:x, Float:y, Float:width, Float:height, colour, action = SP_CATALOGUE_NONE)
{
    // A sprite rectangle has explicit dimensions, independent of text baselines.
    new PlayerText:draw = PracticeCatalogueAdd(playerid, x, y, "LD_SPAC:white", action);
    if (draw == PlayerText:INVALID_TEXT_DRAW) return 0;
    PlayerTextDrawFont(playerid, draw, 4);
    PlayerTextDrawAlignment(playerid, draw, 1);
    PlayerTextDrawTextSize(playerid, draw, width, height);
    PlayerTextDrawColor(playerid, draw, colour);
    return 1;
}

stock PracticeCatalogueText(playerid, Float:x, Float:y, const text[],
    Float:width = 0.18, Float:height = 0.85, colour = -1, bool:center = false)
{
    new PlayerText:draw = PracticeCatalogueAdd(playerid, x, y, text);
    if (draw == PlayerText:INVALID_TEXT_DRAW) return 0;
    PlayerTextDrawLetterSize(playerid, draw, width, height);
    PlayerTextDrawColor(playerid, draw, colour);
    if (center) PlayerTextDrawAlignment(playerid, draw, 2);
    return 1;
}

stock PracticeCatalogueButton(playerid, Float:x, Float:y, Float:width, const text[],
    action, bool:active = false, bool:enabled = true)
{
    // The full rectangle is clickable. Text is visual only and stays inside it.
    PracticeCatalogueBox(playerid, x, y, width, 26.0, active ? SP_CATALOGUE_SELECTED : SP_CATALOGUE_CARD,
        enabled ? action : SP_CATALOGUE_NONE);
    new PlayerText:draw = PracticeCatalogueAdd(playerid, x + width / 2.0, y + 7.0, text);
    if (draw == PlayerText:INVALID_TEXT_DRAW) return 0;
    PlayerTextDrawAlignment(playerid, draw, 2);
    PlayerTextDrawLetterSize(playerid, draw, 0.22, 1.05);
    PlayerTextDrawColor(playerid, draw, enabled ? (active ? SP_CATALOGUE_ACCENT : -1) : SP_CATALOGUE_DISABLED);
    PlayerTextDrawTextSize(playerid, draw, 0.0, width - 12.0);
    return 1;
}

stock PracticeCatalogueModels(playerid, models[], size = sizeof(models))
{
    new count;
    for (new i = 0; i < sizeof(PracticeModels) && count < size; i++)
        if (PracticeCatalogueCategory[playerid] == SP_CATEGORY_ALL
            || PracticeModels[i][PracticeModelCategory] == PracticeCatalogueCategory[playerid])
            models[count++] = i;
    return count;
}

stock PracticeCatalogueCard(playerid, card, model)
{
    new Float:x = 72.0 + float(card % 3) * 166.0;
    new Float:y = 140.0 + float(card / 3) * 104.0;
    new bool:current = PracticeDraft[playerid][PracticePointModel] == PracticeModels[model][PracticeModelID];
    PracticeCatalogueBox(playerid, x, y, 152.0, 100.0, current ? SP_CATALOGUE_SELECTED : SP_CATALOGUE_CARD, model);
    new PlayerText:draw = PracticeCatalogueAdd(playerid, x + 6.0, y + 5.0, "");
    if (draw == PlayerText:INVALID_TEXT_DRAW) return 0;
    PlayerTextDrawFont(playerid, draw, 5);
    PlayerTextDrawTextSize(playerid, draw, 140.0, 65.0);
    PlayerTextDrawBackgroundColor(playerid, draw, current ? SP_CATALOGUE_SELECTED : SP_CATALOGUE_CARD);
    PlayerTextDrawColor(playerid, draw, -1);
    PlayerTextDrawSetPreviewModel(playerid, draw, PracticeModels[model][PracticeModelID]);
    PlayerTextDrawSetPreviewRot(playerid, draw, -15.0, 0.0, 25.0, 1.0);
    draw = PracticeCatalogueAdd(playerid, x + 76.0, y + 79.0, PracticeModels[model][PracticeModelName]);
    if (draw == PlayerText:INVALID_TEXT_DRAW) return 0;
    PlayerTextDrawAlignment(playerid, draw, 2);
    PlayerTextDrawLetterSize(playerid, draw, 0.19, 0.95);
    PlayerTextDrawColor(playerid, draw, current ? SP_CATALOGUE_ACCENT : -1);
    PlayerTextDrawTextSize(playerid, draw, 0.0, 136.0);
    return 1;
}

stock PracticeRenderCatalogue(playerid)
{
    if (!PracticeDraftCurrent(playerid)) { PracticeResetUI(playerid); return 0; }
    // Redrawing a page retains the selection cursor and the owned draft.
    PracticeHideCatalogue(playerid, false);
    PracticeCatalogueFailed[playerid] = false;
    new models[sizeof(PracticeModels)], count = PracticeCatalogueModels(playerid, models);
    new pages = (count + PRACTICE_CATALOGUE_CARDS - 1) / PRACTICE_CATALOGUE_CARDS;
    if (pages < 1) pages = 1;
    if (PracticeCataloguePage[playerid] < 0) PracticeCataloguePage[playerid] = 0;
    if (PracticeCataloguePage[playerid] >= pages) PracticeCataloguePage[playerid] = pages - 1;
    PracticeCatalogueBox(playerid, 50.0, 38.0, 540.0, 372.0, SP_CATALOGUE_BACKGROUND);
    PracticeCatalogueText(playerid, 72.0, 55.0, "Seleccionar objeto", 0.30, 1.45, SP_CATALOGUE_ACCENT);
    PracticeCatalogueText(playerid, 72.0, 80.0, "Elige el objeto que quieres usar como blanco.", 0.19, 0.95, SP_CATALOGUE_MUTED);
    for (new i = 0; i < sizeof(PracticeCategoryNames); i++)
        PracticeCatalogueButton(playerid, 72.0 + float(i) * 98.0, 105.0, 94.0, PracticeCategoryNames[i],
            SP_CATALOGUE_CATEGORY - i, _:PracticeCatalogueCategory[playerid] == i);
    new first = PracticeCataloguePage[playerid] * PRACTICE_CATALOGUE_CARDS;
    for (new card = 0; card < PRACTICE_CATALOGUE_CARDS && first + card < count; card++)
        PracticeCatalogueCard(playerid, card, models[first + card]);
    PracticeCatalogueButton(playerid, 72.0, 367.0, 88.0, "Anterior", SP_CATALOGUE_PREVIOUS, false, PracticeCataloguePage[playerid] > 0);
    PracticeCatalogueButton(playerid, 168.0, 367.0, 88.0, "Siguiente", SP_CATALOGUE_NEXT, false, PracticeCataloguePage[playerid] + 1 < pages);
    // GTA textdraws use GXT 0x98 (152) for á. Dialog strings remain Windows-1252.
    new pageText[32];
    format(pageText, sizeof(pageText), "P\152gina %d de %d", PracticeCataloguePage[playerid] + 1, pages);
    PracticeCatalogueText(playerid, 342.0, 374.0, pageText, 0.19, 0.95, SP_CATALOGUE_MUTED, true);
    PracticeCatalogueButton(playerid, 476.0, 367.0, 90.0, "Atr\152s", SP_CATALOGUE_BACK);
    if (PracticeCatalogueFailed[playerid])
    {
        PracticeHideCatalogue(playerid);
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "No se pudo abrir el selector de objetos. Inténtalo de nuevo.");
        if (PracticeDraftWizard[playerid]) { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
        return PracticeShowDraftMenu(playerid);
    }
    PracticeCatalogueOpen[playerid] = true;
    if (!PracticeCatalogueSelecting[playerid])
    {
        PracticeCatalogueSelecting[playerid] = true;
        SelectTextDraw(playerid, SP_CATALOGUE_SELECTED);
    }
    for (new i = 0; i < PracticeCatalogueDrawCount[playerid]; i++)
        PlayerTextDrawShow(playerid, PracticeCatalogueDraws[playerid][i]);
    return 1;
}

stock PracticeShowCatalogue(playerid)
{
    if (!PracticeDraftCurrent(playerid)) { PracticeResetUI(playerid); return 0; }
    PracticeEnsureSetupView(playerid);
    PracticeHideCatalogue(playerid);
    PracticeCatalogueCategory[playerid] = SP_CATEGORY_ALL;
    PracticeCataloguePage[playerid] = 0;
    PracticeMenuPage[playerid] = SP_MENU_NONE;
    // Finish the native dialog response before enabling a new mouse selection.
    // The gamemode pulse opens it once, without a timer tied to a reusable player ID.
    PracticeCatalogueRequestedAt[playerid] = GetTickCount();
    PracticeCataloguePending[playerid] = true;
    return 1;
}

stock PracticeCatalogueTick(now)
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (!PracticeCataloguePending[playerid]) continue;
        if (!PracticeDraftCurrent(playerid)) { PracticeResetUI(playerid); continue; }
        if (now - PracticeCatalogueRequestedAt[playerid] < PRACTICE_CATALOGUE_OPEN_DELAY) continue;
        PracticeCataloguePending[playerid] = false;
        PracticeRenderCatalogue(playerid);
    }
    return 1;
}

stock PracticeCatalogueBack(playerid)
{
    PracticeHideCatalogue(playerid);
    if (!PracticeDraftCurrent(playerid)) { PracticeResetUI(playerid); return 1; }
    if (PracticeDraftWizard[playerid]) { PracticeDiscardDraft(playerid); return PracticeShowTargets(playerid); }
    return PracticeShowDraftMenu(playerid);
}

stock PracticeHandleCatalogueEscape(playerid)
{
    if (PracticeCatalogueCancelEchoes[playerid] > 0)
    {
        PracticeCatalogueCancelEchoes[playerid]--;
        return 1;
    }
    if (!PracticeCatalogueOpen[playerid]) return 0;
    // ESC already releases the native cursor. Do not call CancelSelectTextDraw.
    PracticeCatalogueSelecting[playerid] = false;
    PracticeCatalogueBack(playerid);
    return 1;
}

stock PracticeHandleCatalogueClick(playerid, PlayerText:clickedid)
{
    if (!PracticeCatalogueOpen[playerid] || clickedid == PlayerText:INVALID_TEXT_DRAW) return 0;
    new action = SP_CATALOGUE_NONE, bool:owned;
    for (new i = 0; i < PracticeCatalogueDrawCount[playerid]; i++)
        if (PracticeCatalogueDraws[playerid][i] == clickedid)
        {
            action = PracticeCatalogueActions[playerid][i];
            owned = true;
            break;
        }
    if (!owned) return 0;
    if (!PracticeDraftCurrent(playerid)) { PracticeResetUI(playerid); return 1; }
    if (action == SP_CATALOGUE_NONE) return 1;
    if (action == SP_CATALOGUE_BACK) { PracticeCatalogueBack(playerid); return 1; }
    if (action <= SP_CATALOGUE_CATEGORY)
    {
        new category = SP_CATALOGUE_CATEGORY - action;
        if (category >= sizeof(PracticeCategoryNames)) return 1;
        PracticeCatalogueCategory[playerid] = E_PRACTICE_CATEGORY:category;
        PracticeCataloguePage[playerid] = 0;
        PracticeRenderCatalogue(playerid);
        return 1;
    }
    if (action == SP_CATALOGUE_PREVIOUS || action == SP_CATALOGUE_NEXT)
    {
        PracticeCataloguePage[playerid] += action == SP_CATALOGUE_NEXT ? 1 : -1;
        PracticeRenderCatalogue(playerid);
        return 1;
    }
    // Only an index bound to a visible card can choose a model.
    if (action < 0 || action >= sizeof(PracticeModels)) return 1;
    PracticeDraft[playerid][PracticePointModel] = PracticeModels[action][PracticeModelID];
    PracticeHideCatalogue(playerid);
    if (PracticeDraftWizard[playerid]) PracticeShowBehaviour(playerid);
    else PracticeShowDraftMenu(playerid);
    return 1;
}
