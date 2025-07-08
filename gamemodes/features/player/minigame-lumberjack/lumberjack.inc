#define MAX_LUMBERJACK_TREES 11
#define LUMBERJACK_PICKUP_ID 341
#define LUMBERJACK_PICKUP_TYPE 1
#define LUMBERJACK_TREE_MODEL 655
#define LUMBERJACK_CUTTING_TIME 10

new Float:LumberjackTreePositions[MAX_LUMBERJACK_TREES][3] = {
    {-2012.9, -2428.9, 29.7},
    {-2013.0, -2453.6, 29.8},
    {-2045.8, -2437.1, 29.7},
    {-2047.9, -2403.2, 29.6},
    {-2078.7, -2393.4, 29.7},
    {-2071.4, -2409.9, 29.7},
    {-2089.8, -2403.5, 29.7},
    {-2071.2, -2356.1, 29.7},
    {-2064.6, -2376.4, 29.7},
    {-2061.4, -2394.5, 29.7},
    {-2058.5, -2424.7, 29.7}
};

enum LumberjackPlayerData {
    bool:isActive,
    bool:isCutting,
    playerTrees[MAX_LUMBERJACK_TREES],
    playerMapIcons[MAX_LUMBERJACK_TREES],
    treesCut,
    cuttingTimer,
    currentTree,
    pickupNotification,
    PlayerText:cuttingTextDraw
}

new LumberjackPlayer[MAX_PLAYERS][LumberjackPlayerData];
new LumberjackPickup;

stock InitLumberjackSystem()
{
    LumberjackPickup = CreateDynamicPickup(LUMBERJACK_PICKUP_ID, LUMBERJACK_PICKUP_TYPE, -2008.92, -2412.19, 30.62);
    
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        ResetLumberjackPlayer(i);
    }
    
    SetTimer("CheckLumberjackPickupProximity", 1000, true);
    
    print("[LumberjackSystem] Sistema de talador de árboles inicializado.");
}

stock ResetLumberjackPlayer(playerid)
{
    LumberjackPlayer[playerid][isActive] = false;
    LumberjackPlayer[playerid][isCutting] = false;
    LumberjackPlayer[playerid][treesCut] = 0;
    LumberjackPlayer[playerid][cuttingTimer] = 0;
    LumberjackPlayer[playerid][currentTree] = -1;
    LumberjackPlayer[playerid][pickupNotification] = -1;
    LumberjackPlayer[playerid][cuttingTextDraw] = PlayerText:INVALID_TEXT_DRAW;
    
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        LumberjackPlayer[playerid][playerTrees][i] = INVALID_STREAMER_ID;
        LumberjackPlayer[playerid][playerMapIcons][i] = -1;
    }
}

stock IsPlayerNearLumberjackPickup(playerid)
{
    return IsPlayerInRangeOfPoint(playerid, 2.0, -2008.92, -2412.19, 30.62);
}

stock ShowLumberjackPickupNotif(playerid)
{
    if(LumberjackPlayer[playerid][pickupNotification] != -1)
    {
        CallLocalFunction("hideTDN", "ii", playerid, LumberjackPlayer[playerid][pickupNotification]);
        LumberjackPlayer[playerid][pickupNotification] = -1;
    }
    
    new message[128];
    if(LumberjackPlayer[playerid][isActive])
    {
        format(message, sizeof(message), "Presiona N para abandonar el trabajo de talador.");
    }
    else
    {
        format(message, sizeof(message), "Presiona Y para iniciar el trabajo de talador.");
    }
    
    LumberjackPlayer[playerid][pickupNotification] = CallLocalFunction("ShowTDN_Manual", "is", playerid, message);
}

stock HideLumberjackPickupNotif(playerid)
{
    if(LumberjackPlayer[playerid][pickupNotification] != -1)
    {
        CallLocalFunction("hideTDN", "ii", playerid, LumberjackPlayer[playerid][pickupNotification]);
        LumberjackPlayer[playerid][pickupNotification] = -1;
    }
}

forward UpdateLumberjackNotification(playerid);
public UpdateLumberjackNotification(playerid)
{
    if(IsPlayerConnected(playerid) && IsPlayerNearLumberjackPickup(playerid))
    {
        ShowLumberjackPickupNotif(playerid);
    }
}

forward CheckLumberjackPickupProximity();
public CheckLumberjackPickupProximity()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(!IsPlayerConnected(i)) continue;
        
        if(IsPlayerNearLumberjackPickup(i))
        {
            if(LumberjackPlayer[i][pickupNotification] == -1)
            {
                ShowLumberjackPickupNotif(i);
            }
        }
        else
        {
            if(LumberjackPlayer[i][pickupNotification] != -1)
            {
                HideLumberjackPickupNotif(i);
            }
        }
    }
}

stock GetNearestTree(playerid)
{
    if(!LumberjackPlayer[playerid][isActive]) return -1;
    
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        if(LumberjackPlayer[playerid][playerTrees][i] != INVALID_STREAMER_ID)
        {
            if(IsPlayerInRangeOfPoint(playerid, 3.0, LumberjackTreePositions[i][0], LumberjackTreePositions[i][1], LumberjackTreePositions[i][2]))
            {
                return i;
            }
        }
    }
    return -1;
}

stock StartLumberjackMinigame(playerid)
{
    if(IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "No puedes iniciar el trabajo de talador mientras estás en un vehículo.");
        return 0;
    }
    
    if(LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "Ya tienes un trabajo activo como talador. Termina tu turno actual primero.");
        return 0;
    }
    
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        LumberjackPlayer[playerid][playerTrees][i] = CreateDynamicObject(LUMBERJACK_TREE_MODEL, 
            LumberjackTreePositions[i][0], LumberjackTreePositions[i][1], LumberjackTreePositions[i][2], 
            0.0, 0.0, 0.0, -1, -1, playerid);
        
        LumberjackPlayer[playerid][playerMapIcons][i] = CreateDynamicMapIcon(
            LumberjackTreePositions[i][0], LumberjackTreePositions[i][1], LumberjackTreePositions[i][2], 
            0, 0x8B4513FF, -1, -1, playerid);
    }
    
    LumberjackPlayer[playerid][isActive] = true;
    LumberjackPlayer[playerid][treesCut] = 0;
    
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        ShowLumberjackPickupNotif(playerid);
    }
    
    SendClientMessage(playerid, COLOR_GREEN, "Has comenzado tu jornada como talador.");
    SendClientMessage(playerid, COLOR_GREEN, "Tienes que talar todos los arboles que aparecen en el minimapa.");
    
    return 1;
}

stock CancelLumberjackMinigame(playerid)
{
    if(IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "No puedes cancelar el trabajo de talador mientras estás en un vehículo.");
        return 0;
    }
    
    if(!LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "No tienes ningún trabajo activo como talador en este momento.");
        return 0;
    }
    
    HideLumberjackPickupNotif(playerid);
    
    if(LumberjackPlayer[playerid][cuttingTimer] != 0)
    {
        KillTimer(LumberjackPlayer[playerid][cuttingTimer]);
        LumberjackPlayer[playerid][cuttingTimer] = 0;
    }
    
    if(LumberjackPlayer[playerid][cuttingTextDraw] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, LumberjackPlayer[playerid][cuttingTextDraw]);
        LumberjackPlayer[playerid][cuttingTextDraw] = PlayerText:INVALID_TEXT_DRAW;
    }
    
    TogglePlayerControllable(playerid, 1);
    ClearAnimations(playerid);
    
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        if(LumberjackPlayer[playerid][playerTrees][i] != INVALID_STREAMER_ID)
        {
            DestroyDynamicObject(LumberjackPlayer[playerid][playerTrees][i]);
            LumberjackPlayer[playerid][playerTrees][i] = INVALID_STREAMER_ID;
        }
        
        if(LumberjackPlayer[playerid][playerMapIcons][i] != -1)
        {
            DestroyDynamicMapIcon(LumberjackPlayer[playerid][playerMapIcons][i]);
            LumberjackPlayer[playerid][playerMapIcons][i] = -1;
        }
    }
    
    ResetLumberjackPlayer(playerid);
    
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        SetTimerEx("UpdateLumberjackNotification", 100, false, "i", playerid);
    }
    
    SendClientMessage(playerid, COLOR_FADE3, "Abandonaste tu turno de talador.");
    return 1;
}

stock FinishLumberjackMinigame(playerid)
{
    if(!LumberjackPlayer[playerid][isActive]) return 0;
    
    HideLumberjackPickupNotif(playerid);
    
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        if(LumberjackPlayer[playerid][playerMapIcons][i] != -1)
        {
            DestroyDynamicMapIcon(LumberjackPlayer[playerid][playerMapIcons][i]);
            LumberjackPlayer[playerid][playerMapIcons][i] = -1;
        }
    }
    
    new reward = 1200 + random(601); // Entre $1200 y $1800
    GivePlayerMoney(playerid, reward);
    
    ResetLumberjackPlayer(playerid);
    
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        SetTimerEx("UpdateLumberjackNotification", 100, false, "i", playerid);
    }
    
    SendClientMessage(playerid, COLOR_LIGHTGREEN, "¡Recolectaste $%d, por todos los arboles!", reward);
    
    return 1;
}

forward LumberjackCuttingTimer(playerid, seconds);
public LumberjackCuttingTimer(playerid, seconds)
{
    if(!IsPlayerConnected(playerid) || !LumberjackPlayer[playerid][isActive] || !LumberjackPlayer[playerid][isCutting])
    {
        return 0;
    }
    
    if(seconds > 0)
    {
        new text[64];
        format(text, sizeof(text), "Espera %d segundos hasta talar el arbol.", seconds);
        
        if(LumberjackPlayer[playerid][cuttingTextDraw] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, LumberjackPlayer[playerid][cuttingTextDraw]);
        }
        
        LumberjackPlayer[playerid][cuttingTextDraw] = CreatePlayerTextDraw(playerid, 325.000, 369.000, text);
        PlayerTextDrawLetterSize(playerid, LumberjackPlayer[playerid][cuttingTextDraw], 0.300, 1.500);
        PlayerTextDrawAlignment(playerid, LumberjackPlayer[playerid][cuttingTextDraw], 2);
        PlayerTextDrawColor(playerid, LumberjackPlayer[playerid][cuttingTextDraw], -1);
        PlayerTextDrawSetShadow(playerid, LumberjackPlayer[playerid][cuttingTextDraw], 1);
        PlayerTextDrawSetOutline(playerid, LumberjackPlayer[playerid][cuttingTextDraw], 1);
        PlayerTextDrawBackgroundColor(playerid, LumberjackPlayer[playerid][cuttingTextDraw], 150);
        PlayerTextDrawFont(playerid, LumberjackPlayer[playerid][cuttingTextDraw], 1);
        PlayerTextDrawSetProportional(playerid, LumberjackPlayer[playerid][cuttingTextDraw], true);
        PlayerTextDrawShow(playerid, LumberjackPlayer[playerid][cuttingTextDraw]);
        
        LumberjackPlayer[playerid][cuttingTimer] = SetTimerEx("LumberjackCuttingTimer", 1000, false, "ii", playerid, seconds - 1);
    }
    else
    {
        new treeIndex = LumberjackPlayer[playerid][currentTree];
        
        if(LumberjackPlayer[playerid][playerTrees][treeIndex] != INVALID_STREAMER_ID)
        {
            DestroyDynamicObject(LumberjackPlayer[playerid][playerTrees][treeIndex]);
            LumberjackPlayer[playerid][playerTrees][treeIndex] = INVALID_STREAMER_ID;
        }
        
        if(LumberjackPlayer[playerid][playerMapIcons][treeIndex] != -1)
        {
            DestroyDynamicMapIcon(LumberjackPlayer[playerid][playerMapIcons][treeIndex]);
            LumberjackPlayer[playerid][playerMapIcons][treeIndex] = -1;
        }
        
        LumberjackPlayer[playerid][treesCut]++;
        LumberjackPlayer[playerid][isCutting] = false;
        LumberjackPlayer[playerid][currentTree] = -1;
        LumberjackPlayer[playerid][cuttingTimer] = 0;
        
        if(LumberjackPlayer[playerid][cuttingTextDraw] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, LumberjackPlayer[playerid][cuttingTextDraw]);
            LumberjackPlayer[playerid][cuttingTextDraw] = PlayerText:INVALID_TEXT_DRAW;
        }
        
        TogglePlayerControllable(playerid, 1);
        ClearAnimations(playerid);
        
        // SendClientMessage(playerid, COLOR_GRAD1, "Progreso: (%d/%d)", LumberjackPlayer[playerid][treesCut], MAX_LUMBERJACK_TREES);
        
        if(LumberjackPlayer[playerid][treesCut] >= MAX_LUMBERJACK_TREES)
        {
            FinishLumberjackMinigame(playerid);
        }
    }
    
    return 1;
}

CMD:iniciarlbj(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "No puedes iniciar el trabajo de talador mientras estás en un vehículo.");
        return 1;
    }
    
    if(!IsPlayerNearLumberjackPickup(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Debes estar cerca del punto para poder comenzar a talar arboles.");
        return 1;
    }
    
    StartLumberjackMinigame(playerid);
    return 1;
}

CMD:cancelarlbj(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "No puedes cancelar el trabajo de talador mientras estás en un vehículo.");
        return 1;
    }
    
    if(!LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "No tienes ningún turno activo de talador para cancelar.");
        return 1;
    }
    
    if(!IsPlayerNearLumberjackPickup(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Debes regresar al punto para poder abandonar tu turno.");
        return 1;
    }
    
    CancelLumberjackMinigame(playerid);
    return 1;
}

CMD:talar(playerid, params[])
{
    if(IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "No puedes talar árboles mientras estás en un vehículo.");
        return 1;
    }
    
    if(!LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "No tienes el trabajo de talador activo en este momento.");
        return 1;
    }
    
    if(LumberjackPlayer[playerid][isCutting])
    {
        SendClientMessage(playerid, COLOR_FADE3, "Ya estás talando un árbol desesperado culiao.");
        return 1;
    }
    
    new weapon = GetPlayerWeapon(playerid);
    if(weapon != 9) // ID 9 es la motosierra (chainsaw)
    {
        SendClientMessage(playerid, COLOR_DARKRED, "No puedes talar el arbol con la mano. Necesitas una motosierra en tu mano derecha.");
        return 1;
    }
    
    new treeIndex = GetNearestTree(playerid);
    if(treeIndex == -1)
    {
        SendClientMessage(playerid, COLOR_FADE3, "No hay árboles cerca de tu posición.");
        return 1;
    }
    
    LumberjackPlayer[playerid][isCutting] = true;
    LumberjackPlayer[playerid][currentTree] = treeIndex;
    
    TogglePlayerControllable(playerid, 0);
    ApplyAnimation(playerid, "CHAINSAW", "WEAPON_csaw", 4.1, 1, 0, 0, 1, 0, 1);
    
    LumberjackPlayer[playerid][cuttingTimer] = SetTimerEx("LumberjackCuttingTimer", 1000, false, "ii", playerid, LUMBERJACK_CUTTING_TIME);
    
    //SendClientMessage(playerid, COLOR_GRAD1, "* Enciendes la motosierra y comienzas a cortar el árbol. ¡Mantente quieto!");
    return 1;
}

stock Lumberjack_OnPlayerPickUpPickup(playerid, pickupid)
{
    #pragma unused playerid, pickupid
    return 0;
}

stock Lumberjack_OnPlayerKeyStateChg(playerid, newkeys, oldkeys)
{
    #pragma unused oldkeys
    
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        if(newkeys & KEY_YES) // Tecla Y
        {
            if(IsPlayerInAnyVehicle(playerid))
            {
                SendClientMessage(playerid, COLOR_FADE3, "No puedes iniciar el trabajo de talador mientras estás en un vehículo.");
                return 1;
            }
            
            if(!LumberjackPlayer[playerid][isActive])
            {
                StartLumberjackMinigame(playerid);
            }
            return 1;
        }
        
        if(newkeys & KEY_NO) // Tecla N
        {
            if(IsPlayerInAnyVehicle(playerid))
            {
                SendClientMessage(playerid, COLOR_FADE3, "No puedes cancelar el trabajo de talador mientras estás en un vehículo.");
                return 1;
            }
            
            if(LumberjackPlayer[playerid][isActive])
            {
                CancelLumberjackMinigame(playerid);
            }
            return 1;
        }
    }
    
    return 0;
}

stock Lumberjack_OnPlayerDisconnect(playerid, reason)
{
    #pragma unused reason
    
    HideLumberjackPickupNotif(playerid);
    
    if(LumberjackPlayer[playerid][isActive])
    {
        CancelLumberjackMinigame(playerid);
    }
    return 1;
}

stock Lumberjack_OnGameModeInit()
{
    InitLumberjackSystem();
    return 1;
}