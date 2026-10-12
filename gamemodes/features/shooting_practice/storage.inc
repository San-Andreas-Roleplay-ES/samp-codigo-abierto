#if defined SP_STORAGE_INCLUDED
    #endinput
#endif
#define SP_STORAGE_INCLUDED

#define PRACTICE_CONFIG_TEMP "shooting_practice.cfg.tmp"
#define PRACTICE_CONFIG_BACKUP "shooting_practice.cfg.bak"
// File backend v9 stores the entire registry. Attempts remain transient.
// FILE implementation behind integration.inc repository ports. SQL substitution belongs
// at that boundary, preserving full version/start/indicator snapshots and async acknowledgements.

stock bool:PracticeWriteLine(File:output, const line[])
{
    return fwrite(output, line) == strlen(line);
}

stock bool:PracticeReadLine(File:input, line[], size = sizeof line)
{
    if (!fread(input, line, size)) return false;
    new length = strlen(line);
    while (length && (line[length - 1] == '\r' || line[length - 1] == '\n')) line[--length] = EOS;
    return true;
}

stock bool:PracticeReadLayout(File:input, version, points[PRACTICE_POINT_COUNT][E_PRACTICE_POINT],
    rules[E_PRACTICE_RULE], const terminator[], bool:multipleStarts = false)
{
    new line[256], legacyNoShootCount;
    new bool:valid = true;
    if (valid)
    {
        valid = PracticeReadLine(input, line);
        // Legacy layouts keep their coordinates and rules. New fields default to
        // zero (all shoot targets, fixed order/distribution, no identification errors).
        if (valid && version == 1)
            valid = !sscanf(line, "iiiii", rules[PracticeWeaponMask], rules[PracticePassPoints],
                rules[PracticePreparation], rules[PracticeExposure], rules[PracticeInterval]);
        else if (valid && version == 2)
            valid = !sscanf(line, "iiiiiiiii", rules[PracticeWeaponMask], rules[PracticePassPoints],
                rules[PracticePreparation], rules[PracticeExposure], rules[PracticeInterval],
                legacyNoShootCount, rules[PracticeMaxErrors], rules[PracticeRandomOrder], rules[PracticeRandomTypes])
                && legacyNoShootCount >= 0 && legacyNoShootCount < PRACTICE_MAX_TARGETS
                && rules[PracticeMaxErrors] <= legacyNoShootCount;
        else if (valid)
            valid = !sscanf(line, "iiiiiiii", rules[PracticeWeaponMask], rules[PracticePassPoints],
                rules[PracticePreparation], rules[PracticeExposure], rules[PracticeInterval],
                rules[PracticeMaxErrors], rules[PracticeRandomOrder], rules[PracticeRandomTypes]);
        // v1-v4 store one weapon ID; v5 stores the set of permitted weapons.
        if (valid && version < 5)
        {
            valid = PracticeIsSupportedWeapon(rules[PracticeWeaponMask]);
            if (valid) rules[PracticeWeaponMask] = 1 << (rules[PracticeWeaponMask] - PRACTICE_WEAPON_FIRST);
        }
        // v1-v5 store exposure/interval in seconds. Validate before multiplication
        // so an overflowing legacy value cannot turn into a plausible duration.
        if (valid && version < 6)
        {
            valid = rules[PracticeExposure] >= 1 && rules[PracticeExposure] <= 60
                && rules[PracticeInterval] >= 0 && rules[PracticeInterval] <= 30;
            if (valid)
            {
                rules[PracticeExposure] *= 1000;
                rules[PracticeInterval] *= 1000;
            }
        }
        valid = valid && PracticeRulesValid(rules);
    }

    new pointCount = multipleStarts ? PRACTICE_POINT_COUNT : PRACTICE_MAX_TARGETS + 1;
    for (new slot = 0; valid && slot < pointCount; slot++)
    {
        new marked, noShoot;
        valid = PracticeReadLine(input, line);
        if (valid && version >= 4)
        {
            valid = !sscanf(line, "iffffiiiiff", marked,
                points[slot][PracticePointX], points[slot][PracticePointY], points[slot][PracticePointZ],
                points[slot][PracticePointAngle], points[slot][PracticePointInterior], points[slot][PracticePointWorld], noShoot,
                points[slot][PracticePointModel], points[slot][PracticePointRotX], points[slot][PracticePointRotY]);
            if (valid)
            {
                valid = PracticeReadLine(input, line) && strfind(line, "NAME ") == 0
                    && strlen(line) < 5 + PRACTICE_NAME_SIZE;
                if (valid) strmid(points[slot][PracticePointName], line, 5, strlen(line), PRACTICE_NAME_SIZE);
            }
        }
        else if (valid && version == 3)
            valid = !sscanf(line, "iffffiii", marked,
                points[slot][PracticePointX], points[slot][PracticePointY], points[slot][PracticePointZ],
                points[slot][PracticePointAngle], points[slot][PracticePointInterior], points[slot][PracticePointWorld], noShoot);
        else if (valid)
            valid = !sscanf(line, "iffffii", marked,
                points[slot][PracticePointX], points[slot][PracticePointY], points[slot][PracticePointZ],
                points[slot][PracticePointAngle], points[slot][PracticePointInterior], points[slot][PracticePointWorld]);
        if (!valid || (marked != 0 && marked != 1) || (noShoot != 0 && noShoot != 1) || (PracticeIsStartSlot(slot) && noShoot))
        {
            valid = false;
            break;
        }
        points[slot][PracticePointSet] = bool:marked;
        points[slot][PracticePointNoShoot] = bool:noShoot;
        if (version < 4 && slot && marked) points[slot][PracticePointModel] = PRACTICE_TARGET_MODEL;
        valid = PracticePointValid(points[slot]);
        if (!PracticeIsStartSlot(slot) && points[slot][PracticePointSet])
            valid = valid && PracticeModelIndex(points[slot][PracticePointModel]) >= 0
                && (multipleStarts || (points[0][PracticePointSet] && PracticeSameDimension(points[0], points[slot])));
        else valid = valid && !points[slot][PracticePointModel];
    }
    if (valid && multipleStarts)
    {
        // Partial configurations may retain targets after the last start is deleted.
        // Marked points must still share one dimension, with or without a start.
        new first = -1;
        for (new slot = 0; valid && slot < pointCount; slot++)
        {
            if (!points[slot][PracticePointSet]) continue;
            if (first < 0) first = slot;
            else valid = PracticeSameDimension(points[first], points[slot]);
        }
    }
    if (valid) valid = PracticeReadLine(input, line) && !strcmp(line, terminator);
    if (version == 2)
        for (new slot = PRACTICE_MAX_TARGETS; slot > 0 && legacyNoShootCount; slot--)
            if (points[slot][PracticePointSet])
            {
                points[slot][PracticePointNoShoot] = true;
                legacyNoShootCount--;
            }
    return valid;
}

// Staging records are static: loading a bounded registry does not consume the VM stack.
// No partial publication, and validation-only reads never mutate live state or UI.
stock bool:PracticeReadRegistry(const path[], bool:publish = false)
{
    new File:input = fopen(path, io_read);
    if (!input) return false;
    static points[PRACTICE_MAX_ZONES][PRACTICE_POINT_COUNT][E_PRACTICE_POINT];
    static rules[PRACTICE_MAX_ZONES][E_PRACTICE_RULE];
    static ids[PRACTICE_MAX_ZONES], names[PRACTICE_MAX_ZONES][PRACTICE_ZONE_NAME_SIZE];
    static bool:enabled[PRACTICE_MAX_ZONES], bool:archived[PRACTICE_MAX_ZONES], bool:indicators[PRACTICE_MAX_ZONES];
    new empty[E_PRACTICE_POINT];
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        ids[zone] = 0; names[zone][0] = EOS; enabled[zone] = archived[zone] = indicators[zone] = false;
        rules[zone] = PracticeDefaultRules;
        for (new slot = 0; slot < PRACTICE_POINT_COUNT; slot++) points[zone][slot] = empty;
    }
    new line[256], expected[80], prefix[24], version, count = 1, nextId = 2;
    new bool:valid = PracticeReadLine(input, line) && !sscanf(line, "s[24]i", prefix, version);
    if (valid)
    {
        format(expected, sizeof(expected), "SARP_PRACTICE %d", version);
        valid = version >= 1 && version <= 9 && !strcmp(line, expected);
    }
    if (valid && version >= 7)
    {
        valid = PracticeReadLine(input, line) && strfind(line, "REGISTRY ") == 0
            && !sscanf(line[9], "ii", nextId, count) && nextId > 0
            && count >= 0 && count <= PRACTICE_MAX_ZONES;
        if (valid)
        {
            format(expected, sizeof(expected), "REGISTRY %d %d", nextId, count);
            valid = !strcmp(line, expected);
        }
        for (new zone = 0; valid && zone < count; zone++)
        {
            new active, retired, marker;
            valid = PracticeReadLine(input, line) && strfind(line, "ZONE ") == 0;
            if (valid && version >= 9)
                valid = !sscanf(line[5], "iiii", ids[zone], active, retired, marker);
            else if (valid)
                valid = !sscanf(line[5], "iii", ids[zone], active, retired);
            valid = valid && ids[zone] > 0 && ids[zone] < nextId
                && (active == 0 || active == 1) && (retired == 0 || retired == 1)
                && (marker == 0 || marker == 1) && !(active && retired);
            if (valid)
            {
                if (version >= 9)
                    format(expected, sizeof(expected), "ZONE %d %d %d %d", ids[zone], active, retired, marker);
                else format(expected, sizeof(expected), "ZONE %d %d %d", ids[zone], active, retired);
                valid = !strcmp(line, expected);
            }
            for (new earlier = 0; valid && earlier < zone; earlier++)
                if (ids[zone] == ids[earlier]) valid = false;
            if (valid)
            {
                enabled[zone] = bool:active; archived[zone] = bool:retired; indicators[zone] = bool:marker;
                valid = PracticeReadLine(input, line) && strfind(line, "ZONE_NAME ") == 0
                    && strlen(line) < 10 + PRACTICE_ZONE_NAME_SIZE;
                if (valid)
                {
                    strmid(names[zone], line, 10, strlen(line), PRACTICE_ZONE_NAME_SIZE);
                    valid = PracticeZoneNameValid(names[zone]);
                }
            }
            if (valid) valid = PracticeReadLayout(input, 6, points[zone], rules[zone], "END_ZONE", version >= 8);
        }
        if (valid) valid = PracticeReadLine(input, line) && !strcmp(line, "END");
    }
    else if (valid)
    {
        ids[0] = 1;
        format(names[0], PRACTICE_ZONE_NAME_SIZE, "Campo de tiro");
        enabled[0] = true; // Preserve the legacy public station, if playable.
        valid = PracticeReadLayout(input, version, points[0], rules[0], "END");
    }
    while (valid && PracticeReadLine(input, line))
        if (line[0]) valid = false;
    fclose(input);
    if (!valid || !publish) return valid;
    // Runtime reloads may not replace an occupied registry or an editor's draft.
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
        if (PracticeIsBusy(zone) || PracticeConfigurationEditing(zone)) return false;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) PracticeResetUI(playerid);
    PracticeResetDiscovery();
    PracticeRegistrySavePending = false;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        PracticeZoneID[zone] = ids[zone];
        format(PracticeZoneName[zone], PRACTICE_ZONE_NAME_SIZE, "%s", names[zone]);
        PracticeZoneEnabled[zone] = enabled[zone]; PracticeZoneArchived[zone] = archived[zone];
        PracticeZoneIndicators[zone] = indicators[zone];
        PracticeRules[zone] = rules[zone];
        for (new slot = 0; slot < PRACTICE_POINT_COUNT; slot++) PracticePoints[zone][slot] = points[zone][slot];
        PracticeRevision[zone]++;
        PracticeSavePending[zone] = false;
        new reason[144];
        if (enabled[zone] && !PracticeConfigurationReady(zone, reason))
        {
            PracticeZoneEnabled[zone] = false;
            if (version >= 7) PracticeSavePending[zone] = PracticeRegistrySavePending = true;
        }
    }
    PracticeNextZoneID = nextId;
    PracticeRegistryRevision++;
    PracticeStorageBlocked = false;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        PracticeSelectedZone[playerid] = PracticeFindZone(PracticeSelectedZoneID[playerid]);
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++) PracticeSynchronizeIndicators(zone);
    return true;
}

stock bool:PracticeCopyConfiguration(const source[], const destination[])
{
    new File:input = fopen(source, io_read);
    if (!input) return false;
    new File:output = fopen(destination, io_write);
    if (!output) { fclose(input); return false; }
    new line[256], bool:success = true;
    while (fread(input, line, sizeof(line)))
        if (!PracticeWriteLine(output, line)) { success = false; break; }
    fclose(input);
    if (!fclose(output)) success = false;
    return success;
}

stock bool:PracticeSaveConfiguration()
{
    if (PracticeStorageBlocked) return false;
    new File:output = fopen(PRACTICE_CONFIG_TEMP, io_write);
    if (!output) return false;
    new line[256], count;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++) if (PracticeZoneID[zone]) count++;
    new bool:success = PracticeWriteLine(output, "SARP_PRACTICE 9\r\n");
    format(line, sizeof(line), "REGISTRY %d %d\r\n", PracticeNextZoneID, count);
    if (!PracticeWriteLine(output, line)) success = false;
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
    {
        if (!PracticeZoneID[zone]) continue;
        format(line, sizeof(line), "ZONE %d %d %d %d\r\n", PracticeZoneID[zone], PracticeZoneEnabled[zone], PracticeZoneArchived[zone], PracticeZoneIndicators[zone]);
        if (!PracticeWriteLine(output, line)) success = false;
        format(line, sizeof(line), "ZONE_NAME %s\r\n", PracticeZoneName[zone]);
        if (!PracticeWriteLine(output, line)) success = false;
        format(line, sizeof(line), "%d %d %d %d %d %d %d %d\r\n", PracticeRules[zone][PracticeWeaponMask],
            PracticeRules[zone][PracticePassPoints], PracticeRules[zone][PracticePreparation], PracticeRules[zone][PracticeExposure],
            PracticeRules[zone][PracticeInterval], PracticeRules[zone][PracticeMaxErrors], PracticeRules[zone][PracticeRandomOrder], PracticeRules[zone][PracticeRandomTypes]);
        if (!PracticeWriteLine(output, line)) success = false;
        for (new slot = 0; slot < PRACTICE_POINT_COUNT; slot++)
        {
            format(line, sizeof(line), "%d %.6f %.6f %.6f %.6f %d %d %d %d %.6f %.6f\r\n",
                PracticePoints[zone][slot][PracticePointSet], PracticePoints[zone][slot][PracticePointX], PracticePoints[zone][slot][PracticePointY],
                PracticePoints[zone][slot][PracticePointZ], PracticePoints[zone][slot][PracticePointAngle], PracticePoints[zone][slot][PracticePointInterior],
                PracticePoints[zone][slot][PracticePointWorld], PracticePoints[zone][slot][PracticePointNoShoot], PracticePoints[zone][slot][PracticePointModel],
                PracticePoints[zone][slot][PracticePointRotX], PracticePoints[zone][slot][PracticePointRotY]);
            if (!PracticeWriteLine(output, line)) success = false;
            format(line, sizeof(line), "NAME %s\r\n", PracticePoints[zone][slot][PracticePointName]);
            if (!PracticeWriteLine(output, line)) success = false;
        }
        if (!PracticeWriteLine(output, "END_ZONE\r\n")) success = false;
    }
    if (!PracticeWriteLine(output, "END\r\n")) success = false;
    if (!fclose(output)) success = false;
    if (!success || !PracticeReadRegistry(PRACTICE_CONFIG_TEMP)) return false;
    // SA-MP has no native atomic rename. Keep the previous validated registry before
    // copying the validated stage. Startup can recover the backup after a torn copy.
    if (fexist(PRACTICE_CONFIG_FILE) && PracticeReadRegistry(PRACTICE_CONFIG_FILE))
        if (!PracticeCopyConfiguration(PRACTICE_CONFIG_FILE, PRACTICE_CONFIG_BACKUP)
            || !PracticeReadRegistry(PRACTICE_CONFIG_BACKUP)) return false;
    if (!PracticeCopyConfiguration(PRACTICE_CONFIG_TEMP, PRACTICE_CONFIG_FILE)
        || !PracticeReadRegistry(PRACTICE_CONFIG_FILE)) return false;
    fremove(PRACTICE_CONFIG_TEMP);
    return true;
}

stock bool:PracticeLoadConfiguration()
{
    // Refusing an in-use reload is not evidence of file corruption.
    for (new zone = 0; zone < PRACTICE_MAX_ZONES; zone++)
        if (PracticeIsBusy(zone) || PracticeConfigurationEditing(zone)) return false;
    if (PracticeReadRegistry(PRACTICE_CONFIG_FILE, true)) return true;
    if (PracticeReadRegistry(PRACTICE_CONFIG_BACKUP, true))
    {
        PracticeRegistrySavePending = true;
        print("[Shooting Practice] Recovered backup. Save the registry to repair shooting_practice.cfg.");
        return true;
    }
    if (!fexist(PRACTICE_CONFIG_FILE) && PracticeReadRegistry(PRACTICE_CONFIG_TEMP, true))
    {
        PracticeRegistrySavePending = true;
        print("[Shooting Practice] Recovered initial staging file. Save the registry.");
        return true;
    }
    if (fexist(PRACTICE_CONFIG_FILE) || fexist(PRACTICE_CONFIG_BACKUP) || fexist(PRACTICE_CONFIG_TEMP))
    {
        PracticeStorageBlocked = true;
        print("[Shooting Practice] Invalid registry and no valid recovery file. Writes blocked.");
    }
    return false;
}

// TODO(SP-DB-ACK): asynchronous repositories acknowledge committed revisions,
// retaining pending state and requester identity until a safe callback completes.
stock PracticeSaveAndNotify(playerid, zone = -1)
{
    if (zone < 0) zone = PracticeSelectedZone[playerid];
    new bool:success = PracticeRepositorySave();
    PracticeRegistrySavePending = !success;
    if (success)
    {
        for (new index = 0; index < PRACTICE_MAX_ZONES; index++) PracticeSavePending[index] = false;
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Configuración guardada. Se recuperará al reiniciar.");
    }
    else
    {
        if (PracticeZoneExists(zone)) PracticeSavePending[zone] = true;
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "No se pudo guardar. Cambios en memoria: usa Reintentar guardado antes de reiniciar.");
    }
    return 1;
}
