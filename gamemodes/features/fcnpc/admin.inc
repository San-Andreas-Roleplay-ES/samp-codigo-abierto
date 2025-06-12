// SAN ANDREAS ROLEPLAY
// Copyright (c) 2021 - 2025
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

CMD:crearfcnpc(playerid, params[])
{
    new fcnpc_type;
    
    if(sscanf(params, "d", fcnpc_type))
    {
        SendClientMessage(playerid, COLOR_GRAD2, "USO: /crearfcnpc [tipo de fcnpc]");
        SendClientMessage(playerid, COLOR_ORANGE, "Tipos de actores: 0 None, 1 Ciervo");
        return 1;
    }

    if(fcnpc_type < E_FCNPC_DATA_MIN_TYPE || fcnpc_type > E_FCNPC_DATA_MAX_TYPE)
        return SendClientMessage(playerid, COLOR_FADE3, "Tipo de actor inválido.");
    
    new Float:pos_x, Float:pos_y, Float:pos_z;
    GetPlayerPos(playerid, pos_x, pos_y, pos_z);
    
    new fcnpc_id = Iter_Free(FCNPC_Iterator);

    if(fcnpc_id == ITER_NONE)
        return SendClientMessage(playerid, COLOR_FADE3, va_return("No se pueden crear más actores, máximo alcanzado (%d).", MAX_FCNPC));

    new fcnpc = FCNPC_Create(va_return("Ciervo_%d", fcnpc_id));

    if(FCNPC_IsValid(fcnpc) == false)
        return SendClientMessage(playerid, COLOR_FADE3, "No se pudo crear el actor.");

    Iter_Add(FCNPC_Iterator, fcnpc_id);

    FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC] = fcnpc;
    FCNPC[fcnpc_id][E_FCNPC_DATA_TYPE] = E_FCNPC_TYPE:fcnpc_type;
    FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_IDLE;
    FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_X] = pos_x + 1.5;
    FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_Y] = pos_y + 1.5;
    FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_Z] = pos_z;

    FCNPC_Spawn(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], 167, FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_X], FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_Y], FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_Z]);
    FCNPC_SetHealth(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], 100.0);
    FCNPC_SetArmour(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], 0.0);
    FCNPC_SetInvulnerable(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], false);
    FCNPC_ShowInTabListForPlayer(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], playerid);

    FCNPC_SetBehaviour(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], FCNPC_BEHAVIOUR_NEUTRAL);

    FCNPC_UseAggroLineOfSight(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], true);
    FCNPC_SetAggroSightRange(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], 10.0);

    FCNPC_SetLeashRange(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], 10.0);
    return 1;
}

CMD:controlar(playerid, params[])
{
    new fcnpc_id, mode;
    if(sscanf(params, "dd", fcnpc_id, mode))
    {
        SendClientMessage(playerid, COLOR_GRAD2, "USO: /controlar [ID del actor] [modo]");
        SendClientMessage(playerid, COLOR_ORANGE, "Modos: 0 Respawn, 1 Matar, 2 Perseguir jugador, 3 Detener, 4 Deambular, 5 Atacar jugador, 6 Detener ataque");
        return 1;
    }

    if(fcnpc_id < 0 || fcnpc_id >= MAX_FCNPC)
        return SendClientMessage(playerid, COLOR_FADE3, "ID de actor inválido.");

    if(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC] == 0)
        return SendClientMessage(playerid, COLOR_FADE3, "No existe un actor con ese ID.");

    if(mode < 0 || mode > 10)
        return SendClientMessage(playerid, COLOR_FADE3, "Modo inválido.");

    switch(mode)
    {
        case 0:
        {
            FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_IDLE;
            FCNPC_Respawn(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC]);
            FCNPC_SetPosition(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_X], FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_Y], FCNPC[fcnpc_id][E_FCNPC_DATA_SPAWN_Z]);
            FCNPC_SetHealth(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], 100.0);
        }
        case 1:
        {
            FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_DEAD;
            FCNPC_Kill(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC]);
        }
        case 2:
        {
            FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_CHASE;
            FCNPC_GoToPlayer(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], playerid, FCNPC_MOVE_TYPE_AUTO, FCNPC_MOVE_SPEED_AUTO, FCNPC_MOVE_MODE_COLANDREAS, FCNPC_MOVE_PATHFINDING_RAYCAST);
        }
        case 3:
        {
            FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_IDLE;
            FCNPC_Stop(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC]);
        }
        case 4:
        {
            FCNPC_SetWander(fcnpc_id);
        }
        case 5:
        {
            FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_ATTACK;
            FCNPC_SetTarget(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], playerid);
        }
        case 6:
        {
            FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_IDLE;
            FCNPC_StopAttack(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC]);
        }
    }
    return 1;
}
