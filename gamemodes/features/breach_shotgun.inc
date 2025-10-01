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

hook OnScriptInit()
{
    CreateDynamicObject(1498, 1522.50525, -1678.60767, 12.54600,   0.00000, 0.00000, 90.00000);
    return 1;
}

IsDoor(objectid)
{
    // For testing purposes, replace with your door models or furnitures dynamic ids
    new modelid = Streamer_GetIntData(STREAMER_TYPE_OBJECT, objectid, E_STREAMER_MODEL_ID);
    return (modelid == 1498);
}

hook OnPlayerShootDynObject(playerid, weaponid, objectid, Float:x, Float:y, Float:z)
{
    if(weaponid != WEAPON_SHOTGUN && weaponid != WEAPON_SAWEDOFF && weaponid != WEAPON_SHOTGSPA)
        return 1;

    if(!IsDoor(objectid))
        return 1;

    new Float:ox, Float:oy, Float:oz, Float:rx, Float:ry, Float:rz;
    GetDynamicObjectPos(objectid, ox, oy, oz);
    GetDynamicObjectRot(objectid, rx, ry, rz);

    // Particle effects
    new particle_1 = CreateDynamicObject(18675, ox, oy, oz, 0.0, 0.0, 0.0);
    new particle_2 = CreateDynamicObject(18679, ox, oy, oz, 0.0, 0.0, 0.0);

    // Sound effects
    PlayerPlaySound(playerid, 1140, ox, oy, oz);

    // Show the particles immediately
    Streamer_Update(playerid, STREAMER_TYPE_OBJECT);

    // Replace with your open door function
    SetDynamicObjectRot(objectid, rx, ry, rz + 90.0);

    // Remove the particles after a few seconds
    SetTimerEx("DeleteParticle", 500, false, "d", particle_1);
    SetTimerEx("DeleteParticle", 1000, false, "d", particle_2);
    return 1;
}

function DeleteParticle(objectid)
{
    if(!IsValidDynamicObject(objectid))
        return 0;

    return DestroyDynamicObject(objectid);
}