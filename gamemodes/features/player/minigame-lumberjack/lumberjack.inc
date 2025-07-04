// SAN ANDREAS ROLEPLAY
// Copyright (c) 2021 - 2025 pigeon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// ========================================
// SISTEMA DE MINIJUEGO - TALADOR DE ÁRBOLES
// ========================================

#define MAX_LUMBERJACK_TREES 11
#define LUMBERJACK_PICKUP_ID 1219
#define LUMBERJACK_PICKUP_TYPE 2
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
    currentTree
}

new LumberjackPlayer[MAX_PLAYERS][LumberjackPlayerData];
new LumberjackPickup = INVALID_STREAMER_ID;

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
        SendClientMessage(playerid, COLOR_FADE3, "Ya tienes el trabajo de talador activo.");
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
    
    SendClientMessage(playerid, COLOR_GRAD1, "¡Trabajo de talador iniciado! Acércate a los árboles y usa /talar para cortarlos.");
    SendClientMessage(playerid, COLOR_GRAD2, "Presiona 2 nuevamente o usa /cancelarlbj para cancelar el trabajo.");
    
    return 1;
}

// Cancelar el minijuego
stock CancelLumberjackMinigame(playerid)
{
    if(!LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "No tienes el trabajo de talador activo.");
        return 0;
    }
    
    // Detener timer si está cortando
    if(LumberjackPlayer[playerid][cuttingTimer] != 0)
    {
        KillTimer(LumberjackPlayer[playerid][cuttingTimer]);
        LumberjackPlayer[playerid][cuttingTimer] = 0;
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
    
    SendClientMessage(playerid, COLOR_FADE3, "Trabajo de talador cancelado.");
    return 1;
}

// Finalizar el minijuego con recompensa
stock FinishLumberjackMinigame(playerid)
{
    if(!LumberjackPlayer[playerid][isActive]) return 0;
    
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
    
    SendClientMessage(playerid, COLOR_GRAD1, "¡Haz finalizado el minijuego!");
    SendClientMessage(playerid, COLOR_GRAD2, "Recompensa recibida: $%d", reward);
    
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
        GameTextForPlayer(playerid, va_return("Espera %d segundos para talar el arbol...", seconds), 1000, 17);
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
        
        // Descongelar al jugador
        TogglePlayerControllable(playerid, 1);
        ClearAnimations(playerid);
        
        SendClientMessage(playerid, COLOR_GRAD1, "¡Árbol cortado! (%d/%d)", LumberjackPlayer[playerid][treesCut], MAX_LUMBERJACK_TREES);
        
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
        SendClientMessage(playerid, COLOR_FADE3, "Debes estar cerca del pickup para iniciar el trabajo de talador.");
        return 1;
    }
    
    StartLumberjackMinigame(playerid);
    return 1;
}

// Comando para cancelar el trabajo
CMD:cancelarlbj(playerid, params[])
{
    CancelLumberjackMinigame(playerid);
    return 1;
}

// Comando para talar árboles
CMD:talar(playerid, params[])
{
    if(!LumberjackPlayer[playerid][isActive])
    {
        SendClientMessage(playerid, COLOR_FADE3, "No tienes el trabajo de talador activo.");
        return 1;
    }
    
    if(LumberjackPlayer[playerid][isCutting])
    {
        SendClientMessage(playerid, COLOR_FADE3, "Ya estás cortando un árbol.");
        return 1;
    }
    
    new treeIndex = GetNearestTree(playerid);
    if(treeIndex == -1)
    {
        SendClientMessage(playerid, COLOR_FADE3, "No hay árboles cerca para talar.");
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
    
    SendClientMessage(playerid, COLOR_GRAD1, "Cortando árbol... ¡No te muevas!");
    return 1;
}

// Callback para el sistema de pickups
stock Lumberjack_OnPlayerPickUpPickup(playerid, pickupid)
{
    if(pickupid == LumberjackPickup)
    {
        if(LumberjackPlayer[playerid][isActive])
        {
            SendClientMessage(playerid, COLOR_GRAD2, "Presiona 2 para cancelar el trabajo de talador.");
        }
        else
        {
            SendClientMessage(playerid, COLOR_GRAD2, "Presiona 2 para iniciar el trabajo de talador.");
        }
        return 1;
    }
    return 0;
}

// Callback para el sistema de teclas
stock Lumberjack_OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
    #pragma unused oldkeys
    if(newkeys & KEY_SUBMISSION) // Tecla 2
    {
        if(IsPlayerNearLumberjackPickup(playerid))
        {
            if(LumberjackPlayer[playerid][isActive])
            {
                CancelLumberjackMinigame(playerid);
            }
            else
            {
                StartLumberjackMinigame(playerid);
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
