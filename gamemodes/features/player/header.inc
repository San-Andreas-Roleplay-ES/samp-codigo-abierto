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

#include "features/player/spawn"
#include "features/player/chat"
#include "features/player/minigame-lumberjack/header"
#include "features/player/color"
#include "features/player/rapel/header"

CMD:ayuda(playerid, params[])
{
    if(isnull(params))
    {
        SendClientMessage(playerid, COLOR_GRAD1, "Uso: /ayuda [opci�n]");
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "Opci�n: (teleport, animacion, objetos)");
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "Opci�n: (coordenadas, admin, chat)");
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "Opci�n: (vehiculos, talador)");
        return 1;
    }

    if(!strcmp(params, "vehiculos", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA VEHICULOS _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[VEHICULOS]{b2ffcd}: (/crearveh)iculo (/borrarveh)iculo (/irveh)iculo");
        SendClientMessage(playerid, COLOR_RADIO2, "[VEHICULOS]{b2ffcd}: (/traerveh)iculo (/repararveh)iculo (/dardlveh)iculo");
        SendClientMessage(playerid, COLOR_RADIO2, "[VEHICULOS]{b2ffcd}: /irvehin) /traervehin");
        return 1;
    }
    
    if(!strcmp(params, "teleport", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA TELEPORT _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[TELEPORT]{b2ffcd}: /cj /idlewood /ls /lv /area51 /ap");
        return 1;
    }
    
    if(!strcmp(params, "animacion", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA ANIMACI�N _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[ANIMACI�N]{b2ffcd}: /anims /pararanim");
        return 1;
    }
    
    if(!strcmp(params, "objetos", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA OBJETOS _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[OBJETOS]{b2ffcd}: (/crearobj)eto (/editarobj)eto (/eliminarobj)eto");
        SendClientMessage(playerid, COLOR_RADIO2, "[OBJETOS]{b2ffcd}: /listarobjetos (/irobj)eto /objetostats");
        return 1;
    }
    
    if(!strcmp(params, "coordenadas", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA COORDENADAS _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[COORDENADAS]{b2ffcd}: (/marcarc)oord (/listarc)oords");
        return 1;
    }
    
    if(!strcmp(params, "admin", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA ADMIN _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[ADMIN]{b2ffcd}: /irpos /adelante /atras /arriba /abajo");
        SendClientMessage(playerid, COLOR_RADIO2, "[ADMIN]{b2ffcd}: /ir /traer /darskin /jetpack");
        SendClientMessage(playerid, COLOR_RADIO2, "[ADMIN]{b2ffcd}: /ropa /hora /clima /creararma");
        SendClientMessage(playerid, COLOR_RADIO2, "[ADMIN]{b2ffcd}: /slap /godmode");
        return 1;
    }
    
    if(!strcmp(params, "chat", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA CHAT _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[CHAT]{b2ffcd}: /me /ame /do (/c)ercano /b");
        SendClientMessage(playerid, COLOR_RADIO2, "[CHAT]{b2ffcd}: (/gr)itar /limpiarchat");
        return 1;
    }
    
    if(!strcmp(params, "skin", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA SKIN _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[SKIN]{b2ffcd}: /skin");
        return 1;
    }
    
    if(!strcmp(params, "talador", true))
    {
        SendClientMessage(playerid, COLOR_HELP_1, "[ _______________ AYUDA TALADOR _______________ ]");
        SendClientMessage(playerid, COLOR_RADIO2, "[TALADOR]{b2ffcd}: /iniciarlbj /cancelarlbj /talar");
        return 1;
    }
    
    SendClientMessage(playerid, COLOR_FADE3, "Opci�n no v�lida. Usa /ayuda para ver las opciones disponibles.");
    return 1;
}
