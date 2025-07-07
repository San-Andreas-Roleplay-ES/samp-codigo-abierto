// ===========================================
// MINIJUEGO - TALADOR DE ARBOLES - LUMBERJACK
// ===========================================

#define MAX_LUMBERJACK_TREES 11
#define LUMBERJACK_PICKUP_ID 341
#define LUMBERJACK_PICKUP_TYPE 1
#define LUMBERJACK_TREE_MODEL 655
#define LUMBERJACK_CUTTING_TIME 10

// Posiciones de los árboles
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

// Estructura de datos del jugador
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

// Inicialización del sistema
stock InitLumberjackSystem()
{
    // Crear el pickup principal
    LumberjackPickup = CreateDynamicPickup(LUMBERJACK_PICKUP_ID, LUMBERJACK_PICKUP_TYPE, -2008.92, -2412.19, 30.62);
    
    // Inicializar datos de jugadores
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        ResetLumberjackPlayer(i);
    }
    
    // Iniciar timer para verificar proximidad al pickup
    SetTimer("CheckLumberjackPickupProximity", 1000, true);
    
    print("[LumberjackSystem] Sistema de talador de árboles inicializado.");
}

// Resetear datos del jugador
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

// Verificar si el jugador está cerca del pickup
stock IsPlayerNearLumberjackPickup(playerid)
{
    return IsPlayerInRangeOfPoint(playerid, 2.0, -2008.92, -2412.19, 30.62);
}

// Mostrar notificación del pickup
stock ShowLumberjackPickupNotif(playerid)
{
    // Ocultar notificación anterior si existe
    if(LumberjackPlayer[playerid][pickupNotification] != -1)
    {
        CallLocalFunction("hideTDN", "ii", playerid, LumberjackPlayer[playerid][pickupNotification]);
        LumberjackPlayer[playerid][pickupNotification] = -1;
    }
    
    new message[128];
    if(LumberjackPlayer[playerid][isActive])
    {
        format(message, sizeof(message), "Presiona N para abandonar tu turno de trabajo.");
    }
    else
    {
        format(message, sizeof(message), "Presiona Y para registrarte como talador.");
    }
    
    LumberjackPlayer[playerid][pickupNotification] = CallLocalFunction("ShowTDN_Manual", "is", playerid, message);
}

// Ocultar notificación del pickup
stock HideLumberjackPickupNotif(playerid)
{
    if(LumberjackPlayer[playerid][pickupNotification] != -1)
    {
        CallLocalFunction("hideTDN", "ii", playerid, LumberjackPlayer[playerid][pickupNotification]);
        LumberjackPlayer[playerid][pickupNotification] = -1;
    }
}

// Actualizar notificación con delay
forward UpdateLumberjackNotification(playerid);
public UpdateLumberjackNotification(playerid)
{
    if(IsPlayerConnected(playerid) && IsPlayerNearLumberjackPickup(playerid))
    {
        ShowLumberjackPickupNotif(playerid);
    }
}

// Timer para verificar proximidad al pickup
forward CheckLumberjackPickupProximity();
public CheckLumberjackPickupProximity()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(!IsPlayerConnected(i)) continue;
        
        if(IsPlayerNearLumberjackPickup(i))
        {
            // Solo mostrar si no tiene notificación activa
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

// Verificar si el jugador está cerca de un árbol
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

// Iniciar el minijuego
stock StartLumberjackMinigame(playerid)
{
    if(LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "Ya tienes un trabajo activo como talador. Termina tu turno actual primero.");
        return 0;
    }
    
    // Crear árboles para el jugador
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        LumberjackPlayer[playerid][playerTrees][i] = CreateDynamicObject(LUMBERJACK_TREE_MODEL, 
            LumberjackTreePositions[i][0], LumberjackTreePositions[i][1], LumberjackTreePositions[i][2], 
            0.0, 0.0, 0.0, -1, -1, playerid);
        
        // Crear mapicon para cada árbol
        LumberjackPlayer[playerid][playerMapIcons][i] = CreateDynamicMapIcon(
            LumberjackTreePositions[i][0], LumberjackTreePositions[i][1], LumberjackTreePositions[i][2], 
            0, 0x8B4513FF, -1, -1, playerid);
    }
    
    LumberjackPlayer[playerid][isActive] = true;
    LumberjackPlayer[playerid][treesCut] = 0;
    
    // Actualizar notificación si está cerca del pickup
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        ShowLumberjackPickupNotif(playerid);
    }
    
    SendClientMessage(playerid, COLOR_GREEN, "Has comenzado tu jornada como talador.");
    SendClientMessage(playerid, COLOR_GREEN, "Tienes que talar todos los arboles que aparecen en el minimapa.");
    
    return 1;
}

// Cancelar el minijuego
stock CancelLumberjackMinigame(playerid)
{
    if(!LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "No tienes ningún trabajo activo como talador en este momento.");
        return 0;
    }
    
    // Ocultar notificación actual antes de resetear
    HideLumberjackPickupNotif(playerid);
    
    // Detener timer si está cortando
    if(LumberjackPlayer[playerid][cuttingTimer] != 0)
    {
        KillTimer(LumberjackPlayer[playerid][cuttingTimer]);
        LumberjackPlayer[playerid][cuttingTimer] = 0;
    }
    
    // Ocultar TextDraw de corte si existe
    if(LumberjackPlayer[playerid][cuttingTextDraw] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, LumberjackPlayer[playerid][cuttingTextDraw]);
        LumberjackPlayer[playerid][cuttingTextDraw] = PlayerText:INVALID_TEXT_DRAW;
    }
    
    // Descongelar al jugador
    TogglePlayerControllable(playerid, 1);
    ClearAnimations(playerid);
    
    // Destruir objetos y mapicons
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
    
    // Actualizar notificación con delay si está cerca del pickup
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        SetTimerEx("UpdateLumberjackNotification", 100, false, "i", playerid);
    }
    
    SendClientMessage(playerid, COLOR_FADE3, "Abandonaste tu turno de talador.");
    return 1;
}

// Finalizar el minijuego con recompensa
stock FinishLumberjackMinigame(playerid)
{
    if(!LumberjackPlayer[playerid][isActive]) return 0;
    
    // Ocultar notificación actual antes de resetear
    HideLumberjackPickupNotif(playerid);
    
    // Limpiar recursos
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        if(LumberjackPlayer[playerid][playerMapIcons][i] != -1)
        {
            DestroyDynamicMapIcon(LumberjackPlayer[playerid][playerMapIcons][i]);
            LumberjackPlayer[playerid][playerMapIcons][i] = -1;
        }
    }
    
    // Dar recompensa
    new reward = 1200 + random(601); // Entre $1200 y $1800
    GivePlayerMoney(playerid, reward);
    
    ResetLumberjackPlayer(playerid);
    
    // Actualizar notificación con delay si está cerca del pickup
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        SetTimerEx("UpdateLumberjackNotification", 100, false, "i", playerid);
    }
    
    SendClientMessage(playerid, COLOR_LIGHTGREEN, "¡Recolectaste $%d, por todos los arboles!", reward);
    
    return 1;
}

// Timer para el proceso de corte
forward LumberjackCuttingTimer(playerid, seconds);
public LumberjackCuttingTimer(playerid, seconds)
{
    if(!IsPlayerConnected(playerid) || !LumberjackPlayer[playerid][isActive] || !LumberjackPlayer[playerid][isCutting])
    {
        return 0;
    }
    
    if(seconds > 0)
    {
        // Crear TextDraw personalizado para el contador
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
        // Terminar el corte
        new treeIndex = LumberjackPlayer[playerid][currentTree];
        
        // Destruir el árbol y su mapicon
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
        
        // Ocultar TextDraw de corte
        if(LumberjackPlayer[playerid][cuttingTextDraw] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, LumberjackPlayer[playerid][cuttingTextDraw]);
            LumberjackPlayer[playerid][cuttingTextDraw] = PlayerText:INVALID_TEXT_DRAW;
        }
        
        // Descongelar al jugador
        TogglePlayerControllable(playerid, 1);
        ClearAnimations(playerid);
        
        // SendClientMessage(playerid, COLOR_GRAD1, "Progreso: (%d/%d)", LumberjackPlayer[playerid][treesCut], MAX_LUMBERJACK_TREES);
        
        // Verificar si terminó todos los árboles
        if(LumberjackPlayer[playerid][treesCut] >= MAX_LUMBERJACK_TREES)
        {
            FinishLumberjackMinigame(playerid);
        }
    }
    
    return 1;
}

// Comando para iniciar el trabajo
CMD:iniciarlbj(playerid, params[])
{
    if(!IsPlayerNearLumberjackPickup(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Debes estar cerca del punto para poder comenzar a talar arboles.");
        return 1;
    }
    
    StartLumberjackMinigame(playerid);
    return 1;
}

// Comando para cancelar el trabajo
CMD:cancelarlbj(playerid, params[])
{
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

// Comando para talar árboles
CMD:talar(playerid, params[])
{
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
    
    // Verificar si el jugador tiene una motosierra
    new weapon = GetPlayerWeapon(playerid);
    if(weapon != 9) // ID 9 es la motosierra (chainsaw)
    {
        SendClientMessage(playerid, COLOR_DARKRED, "QUIERES TALAR UN ARBOL CON LA MANO CABEZA DE VERGA. Necesitas una motosierra en tu mano derecha.");
        return 1;
    }
    
    new treeIndex = GetNearestTree(playerid);
    if(treeIndex == -1)
    {
        SendClientMessage(playerid, COLOR_FADE3, "* No hay árboles cerca de tu posición.");
        return 1;
    }
    
    // Iniciar proceso de corte
    LumberjackPlayer[playerid][isCutting] = true;
    LumberjackPlayer[playerid][currentTree] = treeIndex;
    
    // Congelar al jugador y aplicar animación
    TogglePlayerControllable(playerid, 0);
    ApplyAnimation(playerid, "CHAINSAW", "WEAPON_csaw", 4.1, 1, 0, 0, 1, 0, 1);
    
    // Iniciar el timer
    LumberjackPlayer[playerid][cuttingTimer] = SetTimerEx("LumberjackCuttingTimer", 1000, false, "ii", playerid, LUMBERJACK_CUTTING_TIME);
    
    //SendClientMessage(playerid, COLOR_GRAD1, "* Enciendes la motosierra y comienzas a cortar el árbol. ¡Mantente quieto!");
    return 1;
}

// Callback para el sistema de pickups
stock Lumberjack_OnPlayerPickUpPickup(playerid, pickupid)
{
    #pragma unused playerid, pickupid
    // El sistema de notificaciones se maneja automáticamente con el timer
    return 0;
}

// Callback para el sistema de teclas
stock Lumberjack_OnPlayerKeyStateChg(playerid, newkeys, oldkeys)
{
    #pragma unused oldkeys
    
    // Verificar si está cerca del pickup
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        if(newkeys & KEY_YES) // Tecla Y
        {
            if(!LumberjackPlayer[playerid][isActive])
            {
                StartLumberjackMinigame(playerid);
            }
            return 1;
        }
        
        if(newkeys & KEY_NO) // Tecla N
        {
            if(LumberjackPlayer[playerid][isActive])
            {
                CancelLumberjackMinigame(playerid);
            }
            return 1;
        }
    }
    
    return 0;
}

// Callback para desconexión del jugador
stock Lumberjack_OnPlayerDisconnect(playerid, reason)
{
    #pragma unused reason
    
    // Ocultar notificación del pickup
    HideLumberjackPickupNotif(playerid);
    
    if(LumberjackPlayer[playerid][isActive])
    {
        CancelLumberjackMinigame(playerid);
    }
    return 1;
}

// Callback para inicialización del gamemode
stock Lumberjack_OnGameModeInit()
{
    InitLumberjackSystem();
    return 1;
}
