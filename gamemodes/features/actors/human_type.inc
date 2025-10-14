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

OnPlayerTargetHumanActor(playerid, interactive_actor_id)
{
    if(IsActorWounded(interactive_actor_id))
    {
        #if defined DEBUG_INTERACTIVE_ACTORS
            va_SendClientMessage(playerid, -1, "This actor is dead.");
        #endif
        return 0;
    }

    new current_status = InteractiveActors[interactive_actor_id][E_ACTOR_Status];

    if(current_status == E_ACTOR_STATUS_NONE)
    {
        SetHumanActorStatus(interactive_actor_id, E_ACTOR_STATUS_SURRENDERED);
    }
    return 1;
}

OnPlayerShootNearbyHumanActor(playerid, weaponid, interactive_actor_id)
{
    if(IsActorWounded(interactive_actor_id))
    {
        #if defined DEBUG_INTERACTIVE_ACTORS
            va_SendClientMessage(playerid, -1, "This actor is dead.");
        #endif
        return 0;
    }

    new current_status = InteractiveActors[interactive_actor_id][E_ACTOR_Status];

    if(current_status != E_ACTOR_STATUS_SCARED)
    {
        SetHumanActorStatus(interactive_actor_id, E_ACTOR_STATUS_SCARED);
    }
    return 1;
}

SetHumanActorStatus(interactive_actor_id, status)
{
    #if defined DEBUG_INTERACTIVE_ACTORS
        va_SendClientMessageToAll(-1, "Set Human Actor %d Status: %d", interactive_actor_id, status);
    #endif

    InteractiveActors[interactive_actor_id][E_ACTOR_Status] = status;

    switch(status)
    {
        case E_ACTOR_STATUS_SURRENDERED:
        {
            ApplyDynamicActorAnimation(InteractiveActors[interactive_actor_id][E_ACTOR_ID], "ped", "handsup", 4.1, false, true, true, true, 0);
        }
        case E_ACTOR_STATUS_SCARED:
        {
            ApplyDynamicActorAnimation(InteractiveActors[interactive_actor_id][E_ACTOR_ID], "ped", "cower", 4.1, false, false, false, true, 0);
        }
        case E_ACTOR_STATUS_WOUNDED:
        {
            ApplyDynamicActorAnimation(InteractiveActors[interactive_actor_id][E_ACTOR_ID], "SWEET", "Sweet_injuredloop", 4.1, true, false, false, false, 0);
        }
        case E_ACTOR_STATUS_DIED:
        {
            ApplyDynamicActorAnimation(InteractiveActors[interactive_actor_id][E_ACTOR_ID], "PED", "FLOOR_hit_f", 6.0, false, true, true, true, 0);
        }
    }
    return 1;
}