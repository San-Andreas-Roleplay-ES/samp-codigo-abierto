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

#include <YSI_Coding\y_hooks>

#define MAX_LUMBERJACK_TREES        50
#define MAX_TREE_OBJECTS            100

#define LUMBERJACK_PICKUP_ID        341
#define LUMBERJACK_PICKUP_TYPE      1
#define LUMBERJACK_TREE_MODEL       655
#define LUMBERJACK_CUTTING_TIME     10

#define TREE_DETECTION_RANGE        100.0

new const TREE_MODELS[] = 
{
    620, 621, 622, 623, 624, 629, 652, 654, 655,
    656, 657, 658, 659, 660, 661, 700, 703, 704,
    705, 706, 707, 708, 709, 710, 782, 767, 776,
    778, 772, 774, 763, 729, 727, 731, 732, 714,
    771, 616, 886, 735, 733, 723, 721, 777, 720,
    890, 893, 685, 687, 3517, 3512, 3511, 615, 3510,
    671, 713, 691, 673, 672, 669, 3508, 664, 661,
    659, 658, 656, 654, 618, 617, 736, 767, 776,
    3507, 3506, 3505, 892, 782, 645
};

stock bool:IsTree(modelid)
{
    for (new i = 0; i < sizeof(TREE_MODELS); i++)
    {
        if (modelid == TREE_MODELS[i])
        {
            return true;
        }
    }
    return false;
}

// Función que verifica si el jugador está en una zona conocida con árboles
stock bool:IsInTree2DArea(Float:x, Float:y)
{
    // Angel Pine (bosque norte) - área con muchos árboles
    if(x >= -2172.0 && x <= -1938.0 && y >= -2324.0 && y <= -2100.0)
        return true;
    
    // Palomino Creek - área rural con vegetación
    if(x >= 2200.0 && x <= 2400.0 && y >= -1900.0 && y <= -1700.0)
        return true;
    
    // Back o Beyond - zona rural norte
    if(x >= -700.0 && x <= -400.0 && y >= -500.0 && y <= -200.0)
        return true;
    
    // Blueberry Acres - zona agrícola
    if(x >= 0.0 && x <= 300.0 && y >= -300.0 && y <= 0.0)
        return true;
    
    // Mount Chiliad - zona montañosa con árboles
    if(x >= -2400.0 && x <= -2100.0 && y >= -1700.0 && y <= -1400.0)
        return true;
    
    return false;
}

enum LumberjackPlayerData {
    bool:isActive,
    bool:isCutting,
    detectedTrees[MAX_LUMBERJACK_TREES],
    playerMapIcons[MAX_LUMBERJACK_TREES],
    treesFound,
    treesCut,
    cuttingTimer,
    currentTree,
    pickupNotification,
    PlayerText:cuttingTextDraw
}

new LumberjackPlayer[MAX_PLAYERS][LumberjackPlayerData];
new LumberjackPickup;

hook OnScriptInit()
{
    CreateDynamicObject(657, -2017.16565, -2428.55908, 29.80477,   0.00000, 0.00000, 0.00000);
    CreateDynamicObject(657, -2014.78162, -2439.72339, 29.80477,   0.00000, 0.00000, 0.00000);
    CreateDynamicObject(657, -2002.42322, -2439.06812, 29.80477,   0.00000, 0.00000, 0.00000);

    LumberjackPickup = CreateDynamicPickup(LUMBERJACK_PICKUP_ID, LUMBERJACK_PICKUP_TYPE, -2008.92, -2412.19, 30.62);
    
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        ResetLumberjackPlayer(i);
    }
    
    SetTimer("CheckLumberjackPickupProximity", 1000, true);
    return 1;
}

stock ResetLumberjackPlayer(playerid)
{
    LumberjackPlayer[playerid][isActive] = false;
    LumberjackPlayer[playerid][isCutting] = false;
    LumberjackPlayer[playerid][treesFound] = 0;
    LumberjackPlayer[playerid][treesCut] = 0;
    LumberjackPlayer[playerid][cuttingTimer] = 0;
    LumberjackPlayer[playerid][currentTree] = -1;
    LumberjackPlayer[playerid][pickupNotification] = -1;
    LumberjackPlayer[playerid][cuttingTextDraw] = PlayerText:INVALID_TEXT_DRAW;
    
    for(new i = 0; i < MAX_LUMBERJACK_TREES; i++)
    {
        LumberjackPlayer[playerid][detectedTrees][i] = INVALID_STREAMER_ID;
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
    
    new Float:pos[3];
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    
    for(new i = 0; i < LumberjackPlayer[playerid][treesFound]; i++)
    {
        if(LumberjackPlayer[playerid][detectedTrees][i] != INVALID_STREAMER_ID)
        {
            new Float:objPos[3];
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, LumberjackPlayer[playerid][detectedTrees][i], E_STREAMER_X, objPos[0]);
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, LumberjackPlayer[playerid][detectedTrees][i], E_STREAMER_Y, objPos[1]);
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, LumberjackPlayer[playerid][detectedTrees][i], E_STREAMER_Z, objPos[2]);
            
            if(IsPlayerInRangeOfPoint(playerid, 3.0, objPos[0], objPos[1], objPos[2]))
            {
                return i;
            }
        }
    }
    return -1;
}

// Nueva función para detectar árboles cercanos al jugador
stock DetectNearbyTrees(playerid)
{
    new Float:pos[3];
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    
    new object_array[MAX_OBJECTS];
    new objectsFound = Streamer_GetNearbyItems(pos[0], pos[1], pos[2], STREAMER_TYPE_OBJECT, object_array, MAX_OBJECTS, 100.0);
    
    new modelid = 0;
    new tree_count = 0;
    
    for(new i = 0; i < objectsFound; i++)
    {
        if(!IsValidDynamicObject(object_array[i]))
        {
            printf("Objeto inválido detectado en la lista de árboles.");
            continue;
        }

        modelid = Streamer_GetIntData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_MODEL_ID);

        if(IsTree(modelid) && tree_count < MAX_LUMBERJACK_TREES)
        {
            printf("Árbol detectado: Modelo ID ", modelid);

            LumberjackPlayer[playerid][detectedTrees][tree_count] = object_array[i];
            
            new Float:objPos[3];
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_X, objPos[0]);
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_Y, objPos[1]);
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_Z, objPos[2]);
            
            LumberjackPlayer[playerid][playerMapIcons][tree_count] = CreateDynamicMapIcon(
                objPos[0], objPos[1], objPos[2], 
                0, 0x8B4513FF, -1, -1, playerid);
            
            tree_count++;
        }
    }

    LumberjackPlayer[playerid][treesFound] = tree_count;
    return tree_count;
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
    
    // Detectar árboles cercanos al jugador
    new tree_count = DetectNearbyTrees(playerid);
    
    if(tree_count == 0)
    {
        SendClientMessage(playerid, COLOR_FADE3, "No se encontraron árboles cerca de tu ubicación. Muévete a una zona con más vegetación.");
        return 0;
    }
    
    LumberjackPlayer[playerid][isActive] = true;
    LumberjackPlayer[playerid][treesCut] = 0;
    
    if(IsPlayerNearLumberjackPickup(playerid))
    {
        ShowLumberjackPickupNotif(playerid);
    }
    
    SendClientMessage(playerid, COLOR_GREEN, "Has comenzado tu jornada como talador.");
    SendClientMessage(playerid, COLOR_GREEN, sprintf("Se encontraron %d árboles cerca de tu ubicación. ¡Ve y tálalos todos!", tree_count));
    
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
    
    for(new i = 0; i < LumberjackPlayer[playerid][treesFound]; i++)
    {
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
    
    for(new i = 0; i < LumberjackPlayer[playerid][treesFound]; i++)
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
        
        // Destruir el objeto del árbol talado
        if(LumberjackPlayer[playerid][detectedTrees][treeIndex] != INVALID_STREAMER_ID)
        {
            DestroyDynamicObject(LumberjackPlayer[playerid][detectedTrees][treeIndex]);
            LumberjackPlayer[playerid][detectedTrees][treeIndex] = INVALID_STREAMER_ID;
        }
        
        // Remover el icono del mapa
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
        
        SendClientMessage(playerid, COLOR_GRAD1, sprintf("Progreso: (%d/%d)", LumberjackPlayer[playerid][treesCut], LumberjackPlayer[playerid][treesFound]));
        
        if(LumberjackPlayer[playerid][treesCut] >= LumberjackPlayer[playerid][treesFound])
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
        SendClientMessage(playerid, COLOR_DARKRED, "Me parece que no eres tan listo.");
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
    
    // Ejecutar automáticamente /ame tala el arbol.
    PC_EmulateCommand(playerid, "/ame tala el arbol.");
    
    LumberjackPlayer[playerid][cuttingTimer] = SetTimerEx("LumberjackCuttingTimer", 1000, false, "ii", playerid, LUMBERJACK_CUTTING_TIME);
    
    //SendClientMessage(playerid, COLOR_GRAD1, "* Enciendes la motosierra y comienzas a cortar el árbol. ¡Mantente quieto!");
    return 1;
}

// Comando de debug para verificar árboles cercanos
CMD:debugarboles(playerid, params[])
{
    new Float:pos[3];
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    
    SendClientMessage(playerid, COLOR_LIGHTGREEN, sprintf("=== DEBUG ÁRBOLES ==="));
    SendClientMessage(playerid, COLOR_WHITE, sprintf("Tu posición: %0.2f, %0.2f, %0.2f", pos[0], pos[1], pos[2]));
    SendClientMessage(playerid, COLOR_WHITE, sprintf("Virtual World: %d", GetPlayerVirtualWorld(playerid)));
    
    // Intentar detección con límites seguros
    new object_array[50]; // Reducir cantidad para evitar problemas
    new objectsFound = Streamer_GetNearbyItems(pos[0], pos[1], pos[2], STREAMER_TYPE_OBJECT, object_array, 50, 10.0, GetPlayerVirtualWorld(playerid));
    
    SendClientMessage(playerid, COLOR_WHITE, sprintf("Objetos detectados en 10m: %d", objectsFound));
    
    if(objectsFound > 100 || objectsFound < 0)
    {
        SendClientMessage(playerid, COLOR_RED, "ERROR: Cantidad de objetos anormal. Usando detección por áreas.");
        
        // Usar detección por áreas conocidas
        if(IsInTree2DArea(pos[0], pos[1]))
        {
            SendClientMessage(playerid, COLOR_GREEN, "? Estás en una zona con árboles conocida");
        }
        else
        {
            SendClientMessage(playerid, COLOR_ORANGE, "Muévete a una zona rural con árboles");
            SendClientMessage(playerid, COLOR_WHITE, "Zonas recomendadas:");
            SendClientMessage(playerid, COLOR_WHITE, "• Angel Pine (bosque norte)");
            SendClientMessage(playerid, COLOR_WHITE, "• Palomino Creek (áreas verdes)");
            SendClientMessage(playerid, COLOR_WHITE, "• Back o Beyond (zona rural)");
        }
        return 1;
    }
    
    new validTreesFound = 0;
    for(new i = 0; i < objectsFound && i < 10; i++)
    {
        if(object_array[i] <= 0) continue; // Saltar IDs inválidos
        
        new modelid = Streamer_GetIntData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_MODEL_ID);
        if(modelid <= 0) continue; // Saltar modelos inválidos
        
        new Float:objPos[3];
        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_X, objPos[0]);
        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_Y, objPos[1]);
        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_Z, objPos[2]);
        
        new Float:distance = GetPlayerDistanceFromPoint(playerid, objPos[0], objPos[1], objPos[2]);
        
        if(IsTree(modelid))
        {
            SendClientMessage(playerid, COLOR_GREEN, sprintf("? ÁRBOL - ID:%d Modelo:%d Dist:%0.1fm", object_array[i], modelid, distance));
            validTreesFound++;
        }
        else
        {
            SendClientMessage(playerid, COLOR_GRAD1, sprintf("• Objeto - ID:%d Modelo:%d Dist:%0.1fm", object_array[i], modelid, distance));
        }
    }
    
    SendClientMessage(playerid, COLOR_LIGHTGREEN, sprintf("Árboles válidos encontrados: %d", validTreesFound));
    return 1;
}

// Comando adicional para verificar modelos específicos
CMD:testmodelo(playerid, params[])
{
    new modelid;
    if(sscanf(params, "i", modelid))
    {
        SendClientMessage(playerid, COLOR_WHITE, "Uso: /testmodelo [modelid]");
        SendClientMessage(playerid, COLOR_WHITE, "Ejemplo: /testmodelo 791");
        return 1;
    }
    
    new Float:pos[3], object_array[MAX_TREE_OBJECTS];
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    
    new objectsFound = Streamer_GetNearbyItems(pos[0], pos[1], pos[2], STREAMER_TYPE_OBJECT, object_array, MAX_TREE_OBJECTS, TREE_DETECTION_RANGE, GetPlayerVirtualWorld(playerid));
    
    new found = 0;
    for(new i = 0; i < objectsFound; i++)
    {
        new objModelid = Streamer_GetIntData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_MODEL_ID);
        if(objModelid == modelid)
        {
            new Float:objPos[3];
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_X, objPos[0]);
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_Y, objPos[1]);
            Streamer_GetFloatData(STREAMER_TYPE_OBJECT, object_array[i], E_STREAMER_Z, objPos[2]);
            
            new Float:distance = GetPlayerDistanceFromPoint(playerid, objPos[0], objPos[1], objPos[2]);
            SendClientMessage(playerid, COLOR_GREEN, sprintf("? Encontrado modelo %d - ID:%d Dist:%0.1fm", modelid, object_array[i], distance));
            found++;
        }
    }
    
    if(found == 0)
    {
        SendClientMessage(playerid, COLOR_ORANGE, sprintf("No se encontró ningún objeto con modelo %d en %0.1fm", modelid, TREE_DETECTION_RANGE));
    }
    else
    {
        SendClientMessage(playerid, COLOR_LIGHTGREEN, sprintf("Total encontrados del modelo %d: %d", modelid, found));
    }
    
    // Verificar si está en nuestra lista
    if(IsTree(modelid))
    {
        SendClientMessage(playerid, COLOR_GREEN, "? Este modelo está en la lista de árboles válidos");
    }
    else
    {
        SendClientMessage(playerid, COLOR_RED, "? Este modelo NO está en la lista de árboles válidos");
    }
    
    return 1;
}

hook OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{    
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

hook OnPlayerDisconnect(playerid, reason)
{    
    HideLumberjackPickupNotif(playerid);
    
    if(LumberjackPlayer[playerid][isActive])
    {
        CancelLumberjackMinigame(playerid);
    }
    return 1;
}