#if defined SP_CONFIG_INCLUDED
    #endinput
#endif
#define SP_CONFIG_INCLUDED

#if !defined PRACTICE_MAX_ZONES
    #define PRACTICE_MAX_ZONES (64)
#endif
#define PRACTICE_MAX_TARGETS (10)
#define PRACTICE_MAX_STARTS (10)
#define PRACTICE_POINT_COUNT (PRACTICE_MAX_TARGETS + PRACTICE_MAX_STARTS)
#define PRACTICE_START_RADIUS (1.5)
#define PRACTICE_ACTIVE_RADIUS (3.0)
#define PRACTICE_START_PICKUP_MODEL (348)
#define PRACTICE_START_PICKUP_TYPE (1)
// Stored starts use the character origin. Native weapon pickups also float/bob.
#define PRACTICE_START_PICKUP_Z (-0.65)
#define PRACTICE_SECTION_COLOUR (0xA8C4D9FF)
#define PRACTICE_CHAT_SEPARATOR " {A8C4D9}|{FFFFFF} "
#define PRACTICE_RESULT_BORDER "----------------------------------------------------------------------------------------------"
#define PRACTICE_MAX_DISTANCE (100.0)
// Legacy layouts keep the marked box; each target can now select its own model.
#define PRACTICE_TARGET_MODEL (1271)
#define PRACTICE_TARGET_Z_OFFSET (0.0)
#define PRACTICE_NAME_SIZE (33)
#define PRACTICE_WEAPON_FIRST (22)
#define PRACTICE_WEAPON_LAST (34)
#define PRACTICE_WEAPONS_ALL ((1 << (PRACTICE_WEAPON_LAST - PRACTICE_WEAPON_FIRST + 1)) - 1)
#define PRACTICE_WEAPON_DEFAULT (1 << (24 - PRACTICE_WEAPON_FIRST))
#define PRACTICE_CONFIG_FILE "shooting_practice.cfg"
// SQL keys are decimal strings when resolved by the host, never truncated Pawn cells.
#define PRACTICE_SUBJECT_KEY_SIZE (21)
enum E_PRACTICE_PURPOSE { SP_PURPOSE_TRAINING, SP_PURPOSE_EVALUATION }
enum E_PRACTICE_HISTORY_STATUS { SP_HISTORY_NOT_RECORDED, SP_HISTORY_UNAVAILABLE }
enum E_PRACTICE_POINT
{
    bool:PracticePointSet,
    Float:PracticePointX,
    Float:PracticePointY,
    Float:PracticePointZ,
    Float:PracticePointAngle,
    PracticePointInterior,
    PracticePointWorld,
    bool:PracticePointNoShoot,
    PracticePointModel,
    Float:PracticePointRotX,
    Float:PracticePointRotY,
    PracticePointName[PRACTICE_NAME_SIZE]
}

enum E_PRACTICE_RULE
{
    PracticeWeaponMask,
    PracticePassPoints,
    PracticePreparation,
    PracticeExposure,
    PracticeInterval,
    PracticeMaxErrors,
    PracticeRandomOrder,
    PracticeRandomTypes
}

// Keep the legacy slot 0 for start #1 and slots 1..10 for targets.
// Additional starts occupy slots 11..19. Use the helpers to address starts.
// TODO(SP-ACCESS/HISTORY): versioned group/faction whitelists and history policy
// require host identity adapters. Current zones are public and training is transient.
// The ten-target limit is per zone. Registry indices are not persistent IDs.
new PracticePoints[PRACTICE_MAX_ZONES][PRACTICE_POINT_COUNT][E_PRACTICE_POINT];
// Preparation is whole seconds. Exposure and interval are milliseconds in 100 ms steps.
new PracticeRules[PRACTICE_MAX_ZONES][E_PRACTICE_RULE];
new const PracticeDefaultRules[E_PRACTICE_RULE] = {PRACTICE_WEAPON_DEFAULT, 1, 3, 5000, 1000, 0, 0, 0};
// Runtime menu/draft revision only; it is not a durable database version.
new PracticeRevision[PRACTICE_MAX_ZONES];
new bool:PracticeSavePending[PRACTICE_MAX_ZONES];

enum E_PRACTICE_CATEGORY { SP_CATEGORY_ALL, SP_CATEGORY_SILHOUETTES, SP_CATEGORY_BOTTLES, SP_CATEGORY_CANS, SP_CATEGORY_OTHER }
enum E_PRACTICE_MODEL { PracticeModelID, PracticeModelName[24], E_PRACTICE_CATEGORY:PracticeModelCategory }
new const PracticeModels[][E_PRACTICE_MODEL] = {
    {1271, "Caja marcada", SP_CATEGORY_OTHER},
    {1583, "Silueta armada A", SP_CATEGORY_SILHOUETTES},
    {1584, "Silueta armada B", SP_CATEGORY_SILHOUETTES},
    {1585, "Silueta civil A", SP_CATEGORY_SILHOUETTES},
    {1586, "Silueta civil B", SP_CATEGORY_SILHOUETTES},
    {1543, "Botella de vidrio A", SP_CATEGORY_BOTTLES},
    {1544, "Botella de vidrio B", SP_CATEGORY_BOTTLES},
    {2601, "Lata de bebida", SP_CATEGORY_CANS},
    {1238, "Cono", SP_CATEGORY_OTHER}
};

new const PracticeWeaponNames[][32] = {
    "Pistola", "Pistola silenciada", "Desert Eagle", "Escopeta", "Escopeta recortada",
    "Escopeta de combate", "Micro Uzi", "MP5", "AK-47", "M4", "Tec-9", "Rifle", "Rifle de francotirador"
};

stock PracticeWeaponName(weaponid, name[], size = sizeof(name))
{
    if (weaponid >= 22 && weaponid <= 34) format(name, size, "%s", PracticeWeaponNames[weaponid - 22]);
    else format(name, size, "Sin seleccionar");
    return 1;
}

stock PracticeModelIndex(modelid)
{
    for (new index = 0; index < sizeof(PracticeModels); index++)
        if (PracticeModels[index][PracticeModelID] == modelid) return index;
    return -1;
}

stock bool:PracticeNameValid(const name[])
{
    if (strlen(name) >= PRACTICE_NAME_SIZE) return false;
    for (new i = 0; name[i]; i++)
        if (name[i] < ' ' || name[i] == '{' || name[i] == '}') return false;
    return true;
}

stock bool:PracticeIsSupportedWeapon(weaponid)
{
    return weaponid >= PRACTICE_WEAPON_FIRST && weaponid <= PRACTICE_WEAPON_LAST;
}

stock bool:PracticeWeaponAllowed(weaponid, mask)
{
    return PracticeIsSupportedWeapon(weaponid) && (mask & (1 << (weaponid - PRACTICE_WEAPON_FIRST))) != 0;
}

stock bool:PracticeWeaponsValid(mask)
{
    return mask > 0 && (mask & ~PRACTICE_WEAPONS_ALL) == 0;
}

stock PracticeWeaponCount(mask)
{
    new count;
    for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
        if (PracticeWeaponAllowed(weaponid, mask)) count++;
    return count;
}

stock PracticeWeaponsSummary(mask, text[], size = sizeof(text))
{
    new count = PracticeWeaponCount(mask);
    if (count == 1)
    {
        for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
            if (PracticeWeaponAllowed(weaponid, mask)) return PracticeWeaponName(weaponid, text, size);
    }
    format(text, size, "%d armas seleccionadas", count);
    return 1;
}

stock PracticeFormatWeapons(mask, text[], size = sizeof(text))
{
    text[0] = EOS;
    for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
    {
        if (!PracticeWeaponAllowed(weaponid, mask)) continue;
        if (text[0]) strcat(text, ", ", size);
        strcat(text, PracticeWeaponNames[weaponid - PRACTICE_WEAPON_FIRST], size);
    }
    return 1;
}

stock PracticeCountTargets(zone)
{
    new count;
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        if (PracticePoints[zone][slot][PracticePointSet]) count++;
    return count;
}

stock bool:PracticeRulesValid(const rules[E_PRACTICE_RULE])
{
    return PracticeWeaponsValid(rules[PracticeWeaponMask])
        && rules[PracticePassPoints] >= 1 && rules[PracticePassPoints] <= PRACTICE_MAX_TARGETS
        && rules[PracticePreparation] >= 1 && rules[PracticePreparation] <= 30
        && rules[PracticeExposure] >= 100 && rules[PracticeExposure] <= 60000 && rules[PracticeExposure] % 100 == 0
        && rules[PracticeInterval] >= 0 && rules[PracticeInterval] <= 30000 && rules[PracticeInterval] % 100 == 0
        && rules[PracticeMaxErrors] >= 0 && rules[PracticeMaxErrors] <= PRACTICE_MAX_TARGETS
        && (rules[PracticeRandomOrder] == 0 || rules[PracticeRandomOrder] == 1)
        && (rules[PracticeRandomTypes] == 0 || rules[PracticeRandomTypes] == 1);
}

stock PracticeCountNoShootTargets(zone)
{
    new count;
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        if (PracticePoints[zone][slot][PracticePointSet] && PracticePoints[zone][slot][PracticePointNoShoot]) count++;
    return count;
}

stock PracticeCountShootTargets(zone)
{
    return PracticeCountTargets(zone) - PracticeCountNoShootTargets(zone);
}

stock bool:PracticePointValid(const point[E_PRACTICE_POINT])
{
    return (point[PracticePointSet] == false || point[PracticePointSet] == true)
        && (point[PracticePointNoShoot] == false || point[PracticePointNoShoot] == true)
        && (point[PracticePointSet] || !point[PracticePointNoShoot])
        && point[PracticePointX] >= -20000.0 && point[PracticePointX] <= 20000.0
        && point[PracticePointY] >= -20000.0 && point[PracticePointY] <= 20000.0
        && point[PracticePointZ] >= -20000.0 && point[PracticePointZ] <= 20000.0
        && point[PracticePointAngle] >= 0.0 && point[PracticePointAngle] <= 360.0
        && point[PracticePointRotX] >= 0.0 && point[PracticePointRotX] <= 360.0
        && point[PracticePointRotY] >= 0.0 && point[PracticePointRotY] <= 360.0
        && (point[PracticePointModel] == 0 || PracticeModelIndex(point[PracticePointModel]) >= 0)
        && PracticeNameValid(point[PracticePointName])
        && point[PracticePointInterior] >= 0 && point[PracticePointInterior] <= 255
        && point[PracticePointWorld] >= 0;
}

stock bool:PracticeSameDimension(const first[E_PRACTICE_POINT], const second[E_PRACTICE_POINT])
{
    return first[PracticePointInterior] == second[PracticePointInterior]
        && first[PracticePointWorld] == second[PracticePointWorld];
}

stock bool:PracticeTargetDistanceValid(const start[E_PRACTICE_POINT], const target[E_PRACTICE_POINT])
{
    new Float:x = start[PracticePointX] - target[PracticePointX];
    new Float:y = start[PracticePointY] - target[PracticePointY];
    new Float:z = start[PracticePointZ] - target[PracticePointZ];
    new Float:distanceSquared = x*x + y*y + z*z;
    return distanceSquared >= PRACTICE_ACTIVE_RADIUS * PRACTICE_ACTIVE_RADIUS
        && distanceSquared <= PRACTICE_MAX_DISTANCE * PRACTICE_MAX_DISTANCE;
}

stock PracticeStartSlot(index)
{
    return index == 0 ? 0 : PRACTICE_MAX_TARGETS + index;
}

stock bool:PracticeIsStartSlot(slot)
{
    return slot == 0 || (slot > PRACTICE_MAX_TARGETS && slot < PRACTICE_POINT_COUNT);
}

stock PracticeStartNumber(slot)
{
    return slot == 0 ? 1 : slot - PRACTICE_MAX_TARGETS + 1;
}

stock PracticeCountStarts(zone)
{
    new count;
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
        if (PracticePoints[zone][PracticeStartSlot(index)][PracticePointSet]) count++;
    return count;
}

stock PracticeFirstStart(zone)
{
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
    {
        new slot = PracticeStartSlot(index);
        if (PracticePoints[zone][slot][PracticePointSet]) return slot;
    }
    return -1;
}

stock bool:PracticeInStartDimension(zone, playerid)
{
    new first = PracticeFirstStart(zone);
    return first >= 0 && GetPlayerInterior(playerid) == PracticePoints[zone][first][PracticePointInterior]
        && GetPlayerVirtualWorld(playerid) == PracticePoints[zone][first][PracticePointWorld];
}

// Choose the nearest valid point in the player's dimension. Ties use slot order.
stock PracticeNearestStart(zone, playerid, Float:radius)
{
    new nearest = -1, Float:best = radius * radius, Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
    {
        new slot = PracticeStartSlot(index);
        if (!PracticePoints[zone][slot][PracticePointSet]
            || GetPlayerInterior(playerid) != PracticePoints[zone][slot][PracticePointInterior]
            || GetPlayerVirtualWorld(playerid) != PracticePoints[zone][slot][PracticePointWorld]) continue;
        new Float:dx = x - PracticePoints[zone][slot][PracticePointX];
        new Float:dy = y - PracticePoints[zone][slot][PracticePointY];
        new Float:dz = z - PracticePoints[zone][slot][PracticePointZ];
        new Float:distance = dx*dx + dy*dy + dz*dz;
        if (distance <= best && (nearest < 0 || distance < best))
        { nearest = slot; best = distance; }
    }
    return nearest;
}

stock PracticePointDisplayName(const point[E_PRACTICE_POINT], slot, name[], size = sizeof(name))
{
    if (point[PracticePointName][0]) format(name, size, "%s", point[PracticePointName]);
    else if (PracticeIsStartSlot(slot)) format(name, size, "Punto %d", PracticeStartNumber(slot));
    else format(name, size, "Blanco %d", slot);
    return 1;
}

stock bool:PracticeStartsValid(zone)
{
    new first = PracticeFirstStart(zone);
    if (first < 0) return false;
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
    {
        new slot = PracticeStartSlot(index);
        if (!PracticePoints[zone][slot][PracticePointSet]) continue;
        if (!PracticePointValid(PracticePoints[zone][slot]) || PracticePoints[zone][slot][PracticePointNoShoot]
            || PracticePoints[zone][slot][PracticePointModel]
            || !PracticeSameDimension(PracticePoints[zone][first], PracticePoints[zone][slot])) return false;
    }
    return true;
}

stock bool:PracticeTargetFitsStarts(zone, const target[E_PRACTICE_POINT])
{
    if (!PracticeCountStarts(zone)) return false;
    for (new index = 0; index < PRACTICE_MAX_STARTS; index++)
    {
        new slot = PracticeStartSlot(index);
        if (PracticePoints[zone][slot][PracticePointSet]
            && (!PracticeSameDimension(PracticePoints[zone][slot], target)
                || !PracticeTargetDistanceValid(PracticePoints[zone][slot], target))) return false;
    }
    return true;
}

stock bool:PracticeConfigurationReady(zone, reason[], size = sizeof reason)
{
    if (!PracticeCountStarts(zone))
        format(reason, size, "Falta un punto de inicio. Abre Puntos de inicio en /practicaconfig.");
    else if (!PracticeStartsValid(zone))
        format(reason, size, "Revisa los puntos de inicio. Deben compartir interior y mundo.");
    else if (!PracticeRulesValid(PracticeRules[zone]))
        format(reason, size, "La configuración de armas, puntos o tiempos no es válida.");
    else if (!PracticeCountTargets(zone))
        format(reason, size, "Falta un blanco. Abre Blancos en /practicaconfig y elige Añadir.");
    else
    {
        if (PracticeCountShootTargets(zone) < 1)
        {
            format(reason, size, "Falta un blanco de tiro. Revisa Comportamiento en el menú de Blancos.");
            return false;
        }
        if (PracticeRules[zone][PracticePassPoints] > PracticeCountShootTargets(zone))
        {
            format(reason, size, "Los puntos requeridos superan los blancos de tiro. Revisa Reglas del ejercicio.");
            return false;
        }
        if (PracticeRules[zone][PracticeMaxErrors] > PracticeCountNoShootTargets(zone))
        {
            format(reason, size, "Los errores permitidos superan los blancos protegidos. Revisa Reglas del ejercicio.");
            return false;
        }
        for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        {
            if (!PracticePoints[zone][slot][PracticePointSet]) continue;
            if (!PracticePointValid(PracticePoints[zone][slot]) || PracticeModelIndex(PracticePoints[zone][slot][PracticePointModel]) < 0
                || !PracticeTargetFitsStarts(zone, PracticePoints[zone][slot]))
            {
                format(reason, size, "Revisa el blanco %d: mismo interior/mundo y entre 3 y 100 metros de cada punto de inicio.", slot);
                return false;
            }
        }
        reason[0] = EOS;
        return true;
    }
    return false;
}

// Typed host/repository ports are declared before registry/editor consumers.
forward bool:PracticeHostIsAdministrator(playerid);
forward bool:PracticeHostCanUseZone(zone, playerid);
forward bool:PracticeRepositorySave();
