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

#pragma warning disable 239 // ignore warning 239: literal array/string passed to a non-const parameter
#pragma warning disable 204
#pragma warning disable 200
#pragma warning disable 213

#include <a_samp>
#include <a_mysql>
#include <a_http>
#include <crashdetect>
#include <colandreas>
#include <streamer>
#include <FCNPC>
#include <YSI_Coding\y_va>
#include <YSI_Data\y_iterate>
#include <YSI_Data\y_bit>
#include <YSI_Game\y_vehicledata>
#include <physics>
#include <Pawn.CMD>
#include <Pawn.Regex>
#include <sscanf2>
#include <samp_bcrypt>
#include <strlib>
#include <math>
#include <a_graphfunc>
#include <easyDialog>
#include <mSelection>
#include <components>
#include <float>
#include <VehiclePartPosition>
#include <weapon-data>

#define TDN_MODE_DEFAULT
#include <td-notification>

//defines
#define SERVER_NAME "San Andreas Roleplay"
#define SERVER_NICK "SA-RP" 
#define SERVER_DISCORD "discord.gg/sa-rp"
#define SERVER_URL "sarp.es"
#define SERVER_LANGUAGE "Español - Spanish"
#define SERVER_MAP "San Andreas"
#define SERVER_BRAND_COLOR "{ED801A}"
#define COPYRIGHT "Copyright (c) 2021 - 2025 San Andreas Roleplay"

//utils
#include "utils/defines"
#include "utils/open.mp"

//config
#include "config/timers"

//features
#include "features/admin/header"
#include "features/player/header"
#include "features/actors/core"

#include "features/breach_shotgun.inc"

main() 
{
	print(""SERVER_NICK"");
	print(""COPYRIGHT"");
	print("  ");
}

public OnGameModeInit()
{
	InitCameras();
	SendRconCommand("hostname "SERVER_NAME" | "SERVER_URL" [Código abierto]");
	SetGameModeText("Servidor de Prueba");

	AddPlayerClassEx(1, 102, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(2, 106, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(3, 108, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(4, 116, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(5, 274, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(6, 277, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(7, 280, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(8, 285, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	AddPlayerClassEx(9, 287, 1544.3810, -1675.4711, 13.5583, 90.0000, -1, -1, -1, -1, -1, -1);
	return 1;
}

public OnPlayerEditDynamicObject(playerid, STREAMER_TAG_OBJECT:objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
    if(response == EDIT_RESPONSE_FINAL)
    {
        if(GetPVarType(playerid, "EditingCameraID") != PLAYER_VARTYPE_NONE)
        {
            new cameraid = GetPVarInt(playerid, "EditingCameraID");
            
            if(CameraData[cameraid][cam_Object] == objectid)
            {
                // Marcar como existente y guardar posición
                CameraData[cameraid][cam_Exists] = true;
                CameraData[cameraid][cam_PosX] = x;
                CameraData[cameraid][cam_PosY] = y;
                CameraData[cameraid][cam_PosZ] = z;
                CameraData[cameraid][cam_RotX] = rx;
                CameraData[cameraid][cam_RotY] = ry;
                CameraData[cameraid][cam_RotZ] = rz;
                
                SetDynamicObjectPos(objectid, x, y, z);
                SetDynamicObjectRot(objectid, rx, ry, rz);
                
                new msg[128];
                format(msg, sizeof(msg), "INFO: Camara ID %d instalada correctamente.", cameraid);
                SendClientMessage(playerid, COLOR_LIGHTGREEN, msg);
            }
            
            DeletePVar(playerid, "EditingCameraID");
        }
    }
    else if(response == EDIT_RESPONSE_CANCEL)
    {
        if(GetPVarType(playerid, "EditingCameraID") != PLAYER_VARTYPE_NONE)
        {
            new cameraid = GetPVarInt(playerid, "EditingCameraID");
            
            // Si se cancela, eliminar el objeto
            if(IsValidDynamicObject(CameraData[cameraid][cam_Object]))
            {
                DestroyDynamicObject(CameraData[cameraid][cam_Object]);
                CameraData[cameraid][cam_Object] = INVALID_OBJECT_ID;
            }
            
            CameraData[cameraid][cam_Exists] = false;
            
            DeletePVar(playerid, "EditingCameraID");
            SendClientMessage(playerid, COLOR_LIGHTRED, "ERROR: Instalacion de camara cancelada.");
        }
    }
    
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    // Destruir TextDraws si existen
    DestroyProgressTextDraws(playerid);
    
    // Resetear variables
    PlayerCameraData[playerid][pc_Watching] = false;
    PlayerCameraData[playerid][pc_DamagingCamera] = false;
    PlayerCameraData[playerid][pc_RepairingCamera] = false;
    PlayerCameraData[playerid][pc_DamageProgress] = 0;
    PlayerCameraData[playerid][pc_RepairProgress] = 0;
    
    return 1;
}