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

#define MAX_INTERACTIVE_ACTORS 128
//#define DEBUG_INTERACTIVE_ACTORS

// globals
enum E_ACTOR_TYPE
{
    E_ACTOR_TYPE_NONE = 0,
    E_ACTOR_TYPE_HUMAN,
    E_ACTOR_TYPE_ANIMAL
}

enum E_ACTOR_STATUS
{
    E_ACTOR_STATUS_NONE = 0,
    E_ACTOR_STATUS_SURRENDERED,
    E_ACTOR_STATUS_SCARED,
    E_ACTOR_STATUS_WOUNDED,
    E_ACTOR_STATUS_DIED
}

enum E_INTERACTIVE_ACTORS
{
    E_ACTOR_ID,
    E_ACTOR_Type,
    E_ACTOR_Status,
    Float:E_ACTOR_Health
}

new InteractiveActors[MAX_INTERACTIVE_ACTORS][E_INTERACTIVE_ACTORS];
new Iterator:IActors<MAX_INTERACTIVE_ACTORS>;

// functions
hook OnScriptInit()
{
    for(new i = 0; i < MAX_INTERACTIVE_ACTORS; i++)
    {
        InteractiveActors[i][E_ACTOR_ID] = INVALID_STREAMER_ID;
        InteractiveActors[i][E_ACTOR_Type] = E_ACTOR_TYPE_NONE;
    }

    CreateInteractiveActor(78, 1537.0714, -1686.4196, 13.5469, 0.0000, E_ACTOR_TYPE_HUMAN);
    CreateInteractiveActor(10, 1534.8600, -1683.9851, 13.5469, 270.0000, E_ACTOR_TYPE_HUMAN);
    CreateInteractiveActor(71, 1540.1755, -1683.9851, 13.5501, 90.0000, E_ACTOR_TYPE_HUMAN);
    return 1;
}

CreateInteractiveActor(modelid, Float:x, Float:y, Float:z, Float:r, type, Float:health = 100.0, worldid = -1, interiorid = -1, playerid = -1, Float:streamdistance = STREAMER_ACTOR_SD, areaid = -1, priority = 0)
{
    new interactive_actor = Iter_Free(IActors);

    if(interactive_actor == -1)
        return INVALID_STREAMER_ID;

    Iter_Add(IActors, interactive_actor);

    InteractiveActors[interactive_actor][E_ACTOR_ID] = CreateDynamicActor(modelid, x, y, z, r, 0, health, worldid, interiorid, playerid, streamdistance, areaid, priority);
    InteractiveActors[interactive_actor][E_ACTOR_Type] = type;
    InteractiveActors[interactive_actor][E_ACTOR_Status] = E_ACTOR_STATUS_NONE;
    InteractiveActors[interactive_actor][E_ACTOR_Health] = health;

    #if defined DEBUG_INTERACTIVE_ACTORS
        printf("[debug] Created Interactive Actor %d (Dynamic Actor %d) (Type: %d)", interactive_actor, InteractiveActors[interactive_actor][E_ACTOR_ID], InteractiveActors[interactive_actor][E_ACTOR_Type]);
    #endif
    return interactive_actor;
}

stock IsValidInteractiveActor(actorid)
{
    if(actorid < 0 || actorid >= MAX_INTERACTIVE_ACTORS)
        return 0;

    if(!IsValidDynamicActor(InteractiveActors[actorid][E_ACTOR_ID]))
        return 0;

    return 1;
}

GetInteractiveActorDynamicID(actorid)
{
    if(!IsValidDynamicActor(actorid))
        return INVALID_STREAMER_ID;

    foreach(new interactive_actor : IActors)
    {
        if(InteractiveActors[interactive_actor][E_ACTOR_ID] == actorid)
        {
            #if defined DEBUG_INTERACTIVE_ACTORS
                printf("[debug] Get Interactive Actor %d from Dynamic Actor %d", interactive_actor, actorid);
            #endif

            return interactive_actor;
        }
    }
    return -1;
}

IsActorWounded(interactive_actor)
{
    return (InteractiveActors[interactive_actor][E_ACTOR_Status] == E_ACTOR_STATUS_WOUNDED || InteractiveActors[interactive_actor][E_ACTOR_Status] == E_ACTOR_STATUS_DIED);
}

hook OnPlayerUpdate(playerid)
{
    new target_actor_id = GetPlayerTargetDynamicActor(playerid);

    if(!IsValidDynamicActor(target_actor_id))
        return 1;

    new weaponid = GetPlayerWeapon(playerid);

    if(!weaponid)
        return 1;

    new interactive_actor_id = GetInteractiveActorDynamicID(target_actor_id);

    if(interactive_actor_id == -1)
        return 1;

    #if defined DEBUG_INTERACTIVE_ACTORS
        va_SendClientMessage(playerid, -1, "Target Actor ID: %d (Dynamic Actor %d) (Type: %d)", target_actor_id, interactive_actor_id, InteractiveActors[interactive_actor_id][E_ACTOR_Type]);
    #endif

    new type = InteractiveActors[interactive_actor_id][E_ACTOR_Type];

    switch(type)
    {
        case E_ACTOR_TYPE_HUMAN: OnPlayerTargetHumanActor(playerid, interactive_actor_id);
        default:
        {
            #if defined DEBUG_INTERACTIVE_ACTORS
                va_SendClientMessage(playerid, -1, "This actor type (%d) is not implemented yet.", InteractiveActors[interactive_actor_id][E_ACTOR_Type]);
            #endif
            return 1;
        }
    }
    return 1;
}

hook OnPlayerShootDynObject(playerid, weaponid, STREAMER_TAG_OBJECT:objectid, Float:x, Float:y, Float:z)
{
    return OnPlayerShootNearbyActor(playerid, weaponid);
}

hook OnPlayerWeaponShot(playerid, weaponid, hittype,  hitid, Float:x, Float:y, Float:z)
{
    return OnPlayerShootNearbyActor(playerid, weaponid);
}

OnPlayerShootNearbyActor(playerid, weaponid)
{
    new Float:pos[3];
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

    new actor_array[MAX_ACTORS];
    new actors = Streamer_GetNearbyItems(pos[0], pos[1], pos[2], STREAMER_TYPE_ACTOR, actor_array, MAX_ACTORS, 50.0);

    if(!actors)
        return 1;

    for(new i = 0; i < actors; i++)
    {
        new interactive_actor_id = GetInteractiveActorDynamicID(actor_array[i]);

        if(interactive_actor_id == -1)
            continue;

        #if defined DEBUG_INTERACTIVE_ACTORS
            va_SendClientMessage(playerid, -1, "Shot Nearby Actor ID: %d (Dynamic Actor %d) (Type: %d)", actor_array[i], interactive_actor_id, InteractiveActors[interactive_actor_id][E_ACTOR_Type]);
        #endif

        new type = InteractiveActors[interactive_actor_id][E_ACTOR_Type];

        switch(type)
        {
            case E_ACTOR_TYPE_HUMAN: OnPlayerShootNearbyHumanActor(playerid, weaponid, interactive_actor_id);
            default:
            {
                #if defined DEBUG_INTERACTIVE_ACTORS
                    va_SendClientMessage(playerid, -1, "This actor type (%d) is not implemented yet.", InteractiveActors[interactive_actor_id][E_ACTOR_Type]);
                #endif
                continue;
            }
        }
    }
    return 1;
}

public OnPlayerGiveDamageDynamicActor(playerid, STREAMER_TAG_ACTOR:actorid, Float:amount, weaponid, bodypart)
{
    new interactive_actor_id = GetInteractiveActorDynamicID(actorid);

    if(interactive_actor_id == -1)
        return 1;

    if(InteractiveActors[interactive_actor_id][E_ACTOR_Status] == E_ACTOR_STATUS_DIED)
        return 1;

    InteractiveActors[interactive_actor_id][E_ACTOR_Health] -= amount;
    if(InteractiveActors[interactive_actor_id][E_ACTOR_Health] < 0.0) InteractiveActors[interactive_actor_id][E_ACTOR_Health] = 0.0;

    #if defined DEBUG_INTERACTIVE_ACTORS
        va_SendClientMessageToAll(-1, "Interactive Actor %d took %.2f damage from Player %d (Health: %.2f)", interactive_actor_id, amount, playerid, InteractiveActors[interactive_actor_id][E_ACTOR_Health]);
    #endif

    new type = InteractiveActors[interactive_actor_id][E_ACTOR_Type];
    new current_status = InteractiveActors[interactive_actor_id][E_ACTOR_Status];

    if(InteractiveActors[interactive_actor_id][E_ACTOR_Health] <= 0.0)
    {
        switch(type)
        {
            case E_ACTOR_TYPE_HUMAN:
            {
                if(current_status == E_ACTOR_STATUS_WOUNDED) SetHumanActorStatus(interactive_actor_id, E_ACTOR_STATUS_DIED);
                else SetHumanActorStatus(interactive_actor_id, E_ACTOR_STATUS_WOUNDED);
            }
            default:
            {
                #if defined DEBUG_INTERACTIVE_ACTORS
                    va_SendClientMessage(playerid, -1, "This actor type (%d) is not implemented yet.", InteractiveActors[interactive_actor_id][E_ACTOR_Type]);
                #endif
            }
        }
    }
    return 1;
}

#include "features/actors/human_type"