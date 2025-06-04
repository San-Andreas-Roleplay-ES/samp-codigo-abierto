/* ========================================================================== *\
    Vehicle Neon System

| Description:
    Sistema para agregar y remover neones a vehículos

| License:
    Copyright (c) 2024 San Andreas Roleplay

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgement in the product documentation would be
        appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

| Metadata:
    [Functions]:
        Stock:
            - AddNeonToVehicle
            - RemoveNeonFromVehicle
        Static:
            -
        Public:
            - OnVehicleDeath
        Hooks:
            -
        [Callbacks]:
            - NEON@OnVehicleDeath
        [Definitions]:
            -
        [Enumerations]:
            - E_NEON_COLOR
            - E_NEON_OBJECTS
        [Macros]:
            -
        [Tags]:
            -
        [Variables]:
            Global scope:
                -
            Static scope:
                - g_sNeon
                - g_sIsNeonAttached
        [Natives]:
            -
        [Operators]:
            -
| Dependencies:
    -
\* ========================================================================== */

#include <a_samp>
#include <streamer>

#if defined _inc_neon
    #undef _inc_neon
#endif

#if defined _NEON_SYS
    #endinput
#endif

#define _NEON_SYS

// -----------------------------------------------------------------------------
// Definitions:
#define NEON_COLOR_RED 18647
#define NEON_COLOR_BLUE 18648
#define NEON_COLOR_GREEN 18649
#define NEON_COLOR_YELLOW 18650
#define NEON_COLOR_PINK 18651
#define NEON_COLOR_WHITE 18652

enum E_NEON_OBJECTS
{
    EI_NEON_LEFT_ID,
    EI_NEON_RIGHT_ID
};

// -----------------------------------------------------------------------------
// Static scope:

static stock
    g_sNeon[MAX_VEHICLES][E_NEON_OBJECTS],
    bool:g_sIsNeonAttached[MAX_VEHICLES char];


// -----------------------------------------------------------------------------
// Functions:

stock AddNeonToVehicle(vehicleid, color)
{
    new
        vehicle_model;

    if ((vehicle_model = GetVehicleModel(vehicleid)) == 0)
        return 0;

    new
        Float:size_x,
        Float:size_y,
        Float:size_z;

    GetVehicleModelInfo(vehicle_model, VEHICLE_MODEL_INFO_SIZE, size_x, size_y,
        size_z);

    new
        objectid;
    const Float:OFFSET_Z = -0.6;

    objectid = g_sNeon[vehicleid][EI_NEON_LEFT_ID] = CreateDynamicObject(color,
        0.0, 0.0, 0.0, // Positions
        0.0, 0.0, 0.0 // Rotations
    );
    AttachDynamicObjectToVehicle(objectid, vehicleid,
        -size_x / 2.8, 0.0, OFFSET_Z, // Positions
        0.0, 0.0, 0.0 // Rotations
    );
    objectid = g_sNeon[vehicleid][EI_NEON_RIGHT_ID] = CreateDynamicObject(color,
        0.0, 0.0, 0.0, // Positions
        0.0, 0.0, 0.0 // Rotations
    );
    AttachDynamicObjectToVehicle(objectid, vehicleid,
        size_x / 2.8, 0.0, OFFSET_Z, // Positions
        0.0, 0.0, 0.0 // Rotations
    );

    g_sIsNeonAttached{vehicleid} = true;

    return 1;
}

stock RemoveNeonFromVehicle(vehicleid)
{
    if(IsValidDynamicObject(g_sNeon[vehicleid][EI_NEON_LEFT_ID]))
        DestroyDynamicObject(g_sNeon[vehicleid][EI_NEON_LEFT_ID]);

    if(IsValidDynamicObject(g_sNeon[vehicleid][EI_NEON_RIGHT_ID]))
        DestroyDynamicObject(g_sNeon[vehicleid][EI_NEON_RIGHT_ID]);

    g_sIsNeonAttached{vehicleid} = false;
    g_sNeon[vehicleid][EI_NEON_LEFT_ID] = INVALID_STREAMER_ID;
    g_sNeon[vehicleid][EI_NEON_RIGHT_ID] = INVALID_STREAMER_ID;
    return 1;
}

stock bool:IsNeonAttached(vehicleid)
{
    return g_sIsNeonAttached{vehicleid};
} 