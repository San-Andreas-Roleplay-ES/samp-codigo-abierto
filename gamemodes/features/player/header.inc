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

CMD:ayuda(playerid, params[])
{
    SendClientMessage(playerid, COLOR_YELLOW, "----------------------Comandos disponibles------------------------");
    SendClientMessage(playerid, COLOR_LIGHTYELLOW, "Comandos de teleport: /jetpack, /irpos, /cj, /idlewood, /ls, /lv, /area51");
    SendClientMessage(playerid, COLOR_DARKYELLOW, "Comandos de animación: /anims, /pararanim");
    SendClientMessage(playerid, COLOR_LIGHTYELLOW, "Comandos de objetos: /crearobjeto, /editarobjeto, /eliminarobjeto");
    SendClientMessage(playerid, COLOR_DARKYELLOW, "Comandos de coordenadas: /marcarc(oord), /listarc(oords)");
    SendClientMessage(playerid, COLOR_LIGHTYELLOW, "Comandos de utilidades: /ropa, /hora, /clima, /crearveh, /creararma");
    SendClientMessage(playerid, COLOR_DARKYELLOW, "Comandos de chat: /limpiarchat");
    return 1;
}