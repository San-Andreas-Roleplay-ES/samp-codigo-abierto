#include <YSI_Coding\y_hooks>
#define _lottery_included

// ==================== CONFIGURACIÓN ====================
#define MAX_LOTTERY_NUMBERS     100
#define LOTTERY_TICKET_PRICE    100000
#define LOTTERY_MAIN_PRIZE      9000000
#define LOTTERY_MAX_SELECTIONS  6

// Colores
#define LOTTERY_COLOR_BG            0x87CEEBFF  // Azul cielo
#define LOTTERY_COLOR_UNSELECTED    0x5BC0DEFF  // Azul claro
#define LOTTERY_COLOR_SELECTED      0xFF6A00FF  // Naranja
#define LOTTERY_COLOR_HEADER        0x4A90E2FF  // Azul texto header

// ==================== ENUM ====================
enum E_LOTTERY_DATA
{
    bool:InLottery,
    SelectedCount,
    SelectedNumbers[LOTTERY_MAX_SELECTIONS],
    PlayerText:BackgroundTD,
    PlayerText:LogoTD,
    PlayerText:HeaderTD[2],
    PlayerText:NumberBoxTD[100],
    PlayerText:NumberTextTD[100],
    PlayerText:InstructionTD
}

// ==================== VARIABLES ====================
new PlayerLotteryData[MAX_PLAYERS][E_LOTTERY_DATA];

// ==================== FUNCIONES ====================

stock Lottery_CreateTextdraws(playerid)
{
    new Float:baseX = 130.0;
    new Float:baseY = 100.0;
    new Float:boxSize = 25.0;
    new Float:spacing = 2.0;
    new numbersPerRow = 10;
    new str[16];

    // Fondo principal
    PlayerLotteryData[playerid][BackgroundTD] = CreatePlayerTextDraw(playerid, baseX - 15.0, baseY - 20.0, "_");
    PlayerTextDrawFont(playerid, PlayerLotteryData[playerid][BackgroundTD], 1);
    PlayerTextDrawLetterSize(playerid, PlayerLotteryData[playerid][BackgroundTD], 0.0, 35.0);
    PlayerTextDrawTextSize(playerid, PlayerLotteryData[playerid][BackgroundTD], 520.0, 0.0);
    PlayerTextDrawColor(playerid, PlayerLotteryData[playerid][BackgroundTD], -1);
    PlayerTextDrawUseBox(playerid, PlayerLotteryData[playerid][BackgroundTD], 1);
    PlayerTextDrawBoxColor(playerid, PlayerLotteryData[playerid][BackgroundTD], LOTTERY_COLOR_BG);
    PlayerTextDrawSetOutline(playerid, PlayerLotteryData[playerid][BackgroundTD], 0);
    PlayerTextDrawSetProportional(playerid, PlayerLotteryData[playerid][BackgroundTD], 1);
    PlayerTextDrawSetShadow(playerid, PlayerLotteryData[playerid][BackgroundTD], 0);

    // Logo
    PlayerLotteryData[playerid][LogoTD] = CreatePlayerTextDraw(playerid, 150.0, 110.0, "LOTTO SARP");
    PlayerTextDrawFont(playerid, PlayerLotteryData[playerid][LogoTD], 1);
    PlayerTextDrawLetterSize(playerid, PlayerLotteryData[playerid][LogoTD], 0.5, 2.0);
    PlayerTextDrawColor(playerid, PlayerLotteryData[playerid][LogoTD], LOTTERY_COLOR_SELECTED);
    PlayerTextDrawSetOutline(playerid, PlayerLotteryData[playerid][LogoTD], 1);
    PlayerTextDrawBackgroundColor(playerid, PlayerLotteryData[playerid][LogoTD], 255);

    // Precio del ticket
    PlayerLotteryData[playerid][HeaderTD][0] = CreatePlayerTextDraw(playerid, 330.0, 115.0, "Precio del ticket     $100,000");
    PlayerTextDrawFont(playerid, PlayerLotteryData[playerid][HeaderTD][0], 2);
    PlayerTextDrawLetterSize(playerid, PlayerLotteryData[playerid][HeaderTD][0], 0.2, 0.9);
    PlayerTextDrawColor(playerid, PlayerLotteryData[playerid][HeaderTD][0], LOTTERY_COLOR_HEADER);
    PlayerTextDrawSetOutline(playerid, PlayerLotteryData[playerid][HeaderTD][0], 1);

    // Premio principal
    PlayerLotteryData[playerid][HeaderTD][1] = CreatePlayerTextDraw(playerid, 330.0, 130.0, "Premio principal       $9,000,000");
    PlayerTextDrawFont(playerid, PlayerLotteryData[playerid][HeaderTD][1], 2);
    PlayerTextDrawLetterSize(playerid, PlayerLotteryData[playerid][HeaderTD][1], 0.2, 0.9);
    PlayerTextDrawColor(playerid, PlayerLotteryData[playerid][HeaderTD][1], LOTTERY_COLOR_HEADER);
    PlayerTextDrawSetOutline(playerid, PlayerLotteryData[playerid][HeaderTD][1], 1);

    // Instrucciones
    PlayerLotteryData[playerid][InstructionTD] = CreatePlayerTextDraw(playerid, 230.0, 390.0, "Selecciona 6 numeros - Presiona ESC para comprar o salir");
    PlayerTextDrawFont(playerid, PlayerLotteryData[playerid][InstructionTD], 2);
    PlayerTextDrawLetterSize(playerid, PlayerLotteryData[playerid][InstructionTD], 0.18, 0.8);
    PlayerTextDrawColor(playerid, PlayerLotteryData[playerid][InstructionTD], 0xFFDD00FF);
    PlayerTextDrawSetOutline(playerid, PlayerLotteryData[playerid][InstructionTD], 1);
    PlayerTextDrawSetShadow(playerid, PlayerLotteryData[playerid][InstructionTD], 1);

    // Números
    for (new i = 0; i < MAX_LOTTERY_NUMBERS; i++)
    {
        new row = i / numbersPerRow;
        new col = i % numbersPerRow;

        new Float:posX = baseX + (col * (boxSize + spacing));
        new Float:posY = baseY + 60.0 + (row * (boxSize + spacing));

        // Caja
        PlayerLotteryData[playerid][NumberBoxTD][i] = CreatePlayerTextDraw(playerid, posX, posY, "_");
        PlayerTextDrawFont(playerid, PlayerLotteryData[playerid][NumberBoxTD][i], 1);
        PlayerTextDrawLetterSize(playerid, PlayerLotteryData[playerid][NumberBoxTD][i], 0.0, 2.5);
        PlayerTextDrawTextSize(playerid, PlayerLotteryData[playerid][NumberBoxTD][i], posX + boxSize, 0.0);
        PlayerTextDrawColor(playerid, PlayerLotteryData[playerid][NumberBoxTD][i], -1);
        PlayerTextDrawUseBox(playerid, PlayerLotteryData[playerid][NumberBoxTD][i], 1);
        PlayerTextDrawBoxColor(playerid, PlayerLotteryData[playerid][NumberBoxTD][i], LOTTERY_COLOR_UNSELECTED);
        PlayerTextDrawSetSelectable(playerid, PlayerLotteryData[playerid][NumberBoxTD][i], 1);

        // Número
        format(str, sizeof(str), "%d", i + 1);
        PlayerLotteryData[playerid][NumberTextTD][i] = CreatePlayerTextDraw(playerid, posX + 8.0, posY + 3.0, str);
        PlayerTextDrawFont(playerid, PlayerLotteryData[playerid][NumberTextTD][i], 2);
        PlayerTextDrawLetterSize(playerid, PlayerLotteryData[playerid][NumberTextTD][i], 0.20, 1.0);
        PlayerTextDrawColor(playerid, PlayerLotteryData[playerid][NumberTextTD][i], 0xFFFFFFFF);
        PlayerTextDrawSetOutline(playerid, PlayerLotteryData[playerid][NumberTextTD][i], 1);
    }

    return 1;
}

stock Lottery_Show(playerid)
{
    if (PlayerLotteryData[playerid][InLottery]) return 0;

    Lottery_CreateTextdraws(playerid);

    PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][BackgroundTD]);
    PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][LogoTD]);

    for (new i = 0; i < 2; i++)
        PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][HeaderTD][i]);

    PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][InstructionTD]);

    for (new i = 0; i < MAX_LOTTERY_NUMBERS; i++)
    {
        PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][NumberBoxTD][i]);
        PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][NumberTextTD][i]);
    }

    PlayerLotteryData[playerid][SelectedCount] = 0;
    PlayerLotteryData[playerid][InLottery] = true;
    SelectTextDraw(playerid, 0xFF6A00AA);
    return 1;
}

stock Lottery_Hide(playerid)
{
    if (!PlayerLotteryData[playerid][InLottery]) return 0;

    PlayerTextDrawDestroy(playerid, PlayerLotteryData[playerid][BackgroundTD]);
    PlayerTextDrawDestroy(playerid, PlayerLotteryData[playerid][LogoTD]);
    PlayerTextDrawDestroy(playerid, PlayerLotteryData[playerid][InstructionTD]);

    for (new i = 0; i < 2; i++)
        PlayerTextDrawDestroy(playerid, PlayerLotteryData[playerid][HeaderTD][i]);

    for (new i = 0; i < MAX_LOTTERY_NUMBERS; i++)
    {
        PlayerTextDrawDestroy(playerid, PlayerLotteryData[playerid][NumberBoxTD][i]);
        PlayerTextDrawDestroy(playerid, PlayerLotteryData[playerid][NumberTextTD][i]);
    }

    PlayerLotteryData[playerid][InLottery] = false;
    PlayerLotteryData[playerid][SelectedCount] = 0;
    CancelSelectTextDraw(playerid);
    return 1;
}

stock Lottery_IsNumberSelected(playerid, number)
{
    for (new i = 0; i < PlayerLotteryData[playerid][SelectedCount]; i++)
        if (PlayerLotteryData[playerid][SelectedNumbers][i] == number)
            return i;
    return -1;
}

stock Lottery_ToggleNumber(playerid, number)
{
    new selectedIndex = Lottery_IsNumberSelected(playerid, number);
    new arrayIndex = number - 1;

    if (selectedIndex != -1)
    {
        for (new j = selectedIndex; j < PlayerLotteryData[playerid][SelectedCount] - 1; j++)
            PlayerLotteryData[playerid][SelectedNumbers][j] = PlayerLotteryData[playerid][SelectedNumbers][j + 1];

        PlayerLotteryData[playerid][SelectedCount]--;
        PlayerTextDrawBoxColor(playerid, PlayerLotteryData[playerid][NumberBoxTD][arrayIndex], LOTTERY_COLOR_UNSELECTED);
        PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][NumberBoxTD][arrayIndex]);

        new msg[48];
        format(msg, sizeof(msg), "~r~Numero %d deseleccionado (%d/6)", number, PlayerLotteryData[playerid][SelectedCount]);
        GameTextForPlayer(playerid, msg, 1000, 3);
    }
    else
    {
        if (PlayerLotteryData[playerid][SelectedCount] >= LOTTERY_MAX_SELECTIONS)
        {
            GameTextForPlayer(playerid, "~r~Maximo 6 numeros!", 2000, 3);
            return 0;
        }

        PlayerLotteryData[playerid][SelectedNumbers][PlayerLotteryData[playerid][SelectedCount]] = number;
        PlayerLotteryData[playerid][SelectedCount]++;
        PlayerTextDrawBoxColor(playerid, PlayerLotteryData[playerid][NumberBoxTD][arrayIndex], LOTTERY_COLOR_SELECTED);
        PlayerTextDrawShow(playerid, PlayerLotteryData[playerid][NumberBoxTD][arrayIndex]);

        new msg[64];
        format(msg, sizeof(msg), "~g~Numero %d seleccionado (%d/6)", number, PlayerLotteryData[playerid][SelectedCount]);
        GameTextForPlayer(playerid, msg, 1000, 3);
    }
    return 1;
}

stock Lottery_ProcessPurchase(playerid)
{
    if (PlayerLotteryData[playerid][SelectedCount] == 0)
    {
        SendClientMessage(playerid, -1, "Has cancelado la compra del ticket.");
        Lottery_Hide(playerid);
        return 0;
    }

    if (PlayerLotteryData[playerid][SelectedCount] != LOTTERY_MAX_SELECTIONS)
    {
        new msg[72];
        format(msg, sizeof(msg), "Debes seleccionar %d numeros! (%d/%d seleccionados)",
            LOTTERY_MAX_SELECTIONS,
            PlayerLotteryData[playerid][SelectedCount],
            LOTTERY_MAX_SELECTIONS);
        SendClientMessage(playerid, -1, msg);
        return 0;
    }

    if (GetPlayerMoney(playerid) < LOTTERY_TICKET_PRICE)
    {
        SendClientMessage(playerid, -1, "No tienes suficiente dinero para comprar el ticket ($100,000)!");
        return 0;
    }

    if (Lottery_PlayerHasTicket(playerid))
    {
        SendClientMessage(playerid, -1, "Ya tienes un ticket comprado para este sorteo.");
        return 0;
    }

    GivePlayerMoney(playerid, -LOTTERY_TICKET_PRICE);

    new numbersString[144];
    format(numbersString, sizeof(numbersString),
        "{00FF00}Ticket comprado! {FFFFFF}Numeros: {FF6A00}%d, %d, %d, %d, %d, %d",
        PlayerLotteryData[playerid][SelectedNumbers][0],
        PlayerLotteryData[playerid][SelectedNumbers][1],
        PlayerLotteryData[playerid][SelectedNumbers][2],
        PlayerLotteryData[playerid][SelectedNumbers][3],
        PlayerLotteryData[playerid][SelectedNumbers][4],
        PlayerLotteryData[playerid][SelectedNumbers][5]
    );

    SendClientMessage(playerid, -1, numbersString);
    SendClientMessage(playerid, 0xFFDD00FF, "Tu ticket ha sido registrado para el próximo sorteo. ¡Buena suerte!");

    Lottery_SavePlayerTicket(playerid);
    Lottery_Hide(playerid);
    return 1;
}

stock Lottery_PlayerHasTicket(playerid) return false;

stock Lottery_SavePlayerTicket(playerid)
{
    printf("Ticket guardado para playerid %d con numeros: %d,%d,%d,%d,%d,%d",
        playerid,
        PlayerLotteryData[playerid][SelectedNumbers][0],
        PlayerLotteryData[playerid][SelectedNumbers][1],
        PlayerLotteryData[playerid][SelectedNumbers][2],
        PlayerLotteryData[playerid][SelectedNumbers][3],
        PlayerLotteryData[playerid][SelectedNumbers][4],
        PlayerLotteryData[playerid][SelectedNumbers][5]);
    return 1;
}

// ==================== HOOKS ====================

hook OnPlayerConnect(playerid)
{
    PlayerLotteryData[playerid][InLottery] = false;
    PlayerLotteryData[playerid][SelectedCount] = 0;
    for (new i = 0; i < LOTTERY_MAX_SELECTIONS; i++)
        PlayerLotteryData[playerid][SelectedNumbers][i] = 0;
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    if (PlayerLotteryData[playerid][InLottery])
        Lottery_Hide(playerid);
    return 1;
}

hook OnPlayerClickPlayerTDraw(playerid, PlayerText:playertextid)
{
    if (!PlayerLotteryData[playerid][InLottery]) return 1;

    for (new i = 0; i < MAX_LOTTERY_NUMBERS; i++)
        if (playertextid == PlayerLotteryData[playerid][NumberBoxTD][i])
            return Lottery_ToggleNumber(playerid, i + 1);
    return 1;
}

hook OnPlayerClickTextDraw(playerid, Text:clickedid)
{
    if (PlayerLotteryData[playerid][InLottery] && clickedid == Text:INVALID_TEXT_DRAW)
        Lottery_ProcessPurchase(playerid);
    return 1;
}

// ==================== COMANDO ====================

CMD:loteria(playerid, params[])
{
    if (Lottery_PlayerHasTicket(playerid))
    {
        SendClientMessage(playerid, -1, "Ya tienes un ticket comprado para este sorteo.");
        return 1;
    }
    Lottery_Show(playerid);
    return 1;
}
