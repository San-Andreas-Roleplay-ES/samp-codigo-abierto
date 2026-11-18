#if defined SCBA_CONFIG_INCLUDED
    #endinput
#endif
#define SCBA_CONFIG_INCLUDED

#define SCBA_SECOND_MS (1000)
#define SCBA_SUPPLY_POINT_LIMIT (16)
#define SCBA_SUPPLY_VEHICLE_LIMIT (32)
#if !defined SCBA_FULL_AIR_SECONDS
    #define SCBA_FULL_AIR_SECONDS (20 * 60)
#endif
#define SCBA_FULL_AIR_MS (SCBA_FULL_AIR_SECONDS * SCBA_SECOND_MS)
#define SCBA_LOW_AIR_PERCENT (25)
#define SCBA_CRITICAL_AIR_PERCENT (10)
#define SCBA_PULSE_INTERVAL_MS (250)
#define SCBA_HUD_INTERVAL_MS (1000)
#define SCBA_DAMAGE_INTERVAL_MS (2 * SCBA_SECOND_MS)
#define SCBA_DAMAGE_AMOUNT (5.0)
#define SCBA_SUPPLY_DURATION_MS (8000)
#define SCBA_ANIM_RETRY_MS (500)
#define SCBA_ANIM_START_TIMEOUT_MS (3000)
#define SCBA_ALARM_INTERVAL_MS (5000)
#define SCBA_INTERACTION_RADIUS (3.0)
#define SCBA_SERVICE_MOVEMENT_RADIUS (0.75)
#define SCBA_VEHICLE_MAX_SPEED (0.05)
#define SCBA_EVENT_RADIUS (10.0)
// Public directory for positional SCBA audio, without a trailing slash.
#if !defined SCBA_ALARM_BASE_URL
    #define SCBA_ALARM_BASE_URL ""
#endif
#define SCBA_RESERVE_CLIP "reserva.mp3"
#define SCBA_CRITICAL_CLIP "critica.mp3"
#define SCBA_STATION_CLIP "valvula.mp3"
#define SCBA_TRUCK_CLIP "acople.mp3"
#define SCBA_AUDIO_CLIP_WINDOW_MS (1100)
new ScbaAudioBaseUrl[192] = SCBA_ALARM_BASE_URL;
#define SCBA_FALLBACK_RESERVE_SOUND (1056)
#define SCBA_FALLBACK_CRITICAL_SOUND (1057)
#define SCBA_FALLBACK_STATION_SOUND (1134)
#define SCBA_FALLBACK_TRUCK_SOUND (1133)
#define SCBA_DISTRESS_SOUND (19024)
#define SCBA_EXTERNAL_DIZZINESS (-1)
#define SCBA_SERVICE_ANIM_LIBRARY "BOMBER"
#define SCBA_SERVICE_ANIM_NAME "BOM_Plant_Loop"
#define SCBA_TRUCK_ANIM_LIBRARY "CAR"
#define SCBA_TRUCK_ANIM_NAME "Fixn_Car_Loop"
#define SCBA_SERVICE_ANIM_SPEED (4.1)
#define SCBA_EMPTY_SLOT (-1)
#define SCBA_ACTION_LENGTH (16)

#define SCBA_COLOUR_TEXT (0xE4E9EDFF)
#define SCBA_COLOUR_MUTED (0xAAB8C2FF)
#define SCBA_COLOUR_NORMAL (0xB8D69DFF)
#define SCBA_COLOUR_LOW (0xE8B756FF)
#define SCBA_COLOUR_CRITICAL (0xEB745EFF)
#define SCBA_COLOUR_ROLEPLAY (0xC2A2DAFF)
#define SCBA_GAUGE_LEFT (537.0)
#define SCBA_GAUGE_TOP (314.0)
#define SCBA_GAUGE_WIDTH (88.0)
#define SCBA_GAUGE_HEIGHT (114.0)
#define SCBA_GAUGE_ART_SIZE (256.0)
#define SCBA_GAUGE_DIAL_X (128.0)
#define SCBA_GAUGE_DIAL_Y (88.0)
#define SCBA_GAUGE_DIAL_SIZE (148.0)
#define SCBA_GAUGE_STATUS_LEFT (54.0)
#define SCBA_GAUGE_STATUS_TOP (220.0)
#define SCBA_GAUGE_STATUS_WIDTH (148.0)
#define SCBA_GAUGE_STATUS_HEIGHT (21.0)
#define SCBA_GAUGE_START_ANGLE (145.0)
#define SCBA_GAUGE_SWEEP_ANGLE (250.0)
#define SCBA_GAUGE_NEEDLE_STEPS (50)
#define SCBA_GAUGE_INK (0x090908FF)
#define SCBA_GAUGE_DIGITS (3)
#define SCBA_GAUGE_FONT_WIDTH (0.21)
#define SCBA_GAUGE_FONT_HEIGHT (0.90)
#define SCBA_GAUGE_TEXT_X(%0) (SCBA_GAUGE_LEFT + (%0) * SCBA_GAUGE_WIDTH / SCBA_GAUGE_ART_SIZE)
#define SCBA_GAUGE_TEXT_Y(%0) (SCBA_GAUGE_TOP + (%0) * SCBA_GAUGE_HEIGHT / SCBA_GAUGE_ART_SIZE)

enum E_SCBA_GAUGE_LAYER
{
    PlayerText:SCBA_DIAL_FACE, PlayerText:SCBA_DIAL_NEEDLE,
    PlayerText:SCBA_AIR_CAPTION, PlayerText:SCBA_TIME_CAPTION,
    PlayerText:SCBA_AIR_READING, PlayerText:SCBA_TIME_READING, PlayerText:SCBA_BAND_READING
}

enum E_SCBA_GAUGE_STATUS
{
    SCBA_STATUS_DISCONNECTED, SCBA_STATUS_FILLING, SCBA_STATUS_NORMAL,
    SCBA_STATUS_LOW, SCBA_STATUS_CRITICAL, SCBA_STATUS_EMPTY
}
enum E_SCBA_STATUS_ART
{
    ScbaStatusSprite[16], ScbaStatusCaption[16], ScbaStatusColour
}
new const ScbaStatusArt[E_SCBA_GAUGE_STATUS][E_SCBA_STATUS_ART] =
{
    {"air_s_off", "DESCONECTADO", SCBA_COLOUR_MUTED},
    {"air_s_fill", "RECARGANDO", SCBA_COLOUR_TEXT},
    {"air_s_on", "EN SERVICIO", SCBA_COLOUR_NORMAL},
    {"air_s_low", "RESERVA", SCBA_COLOUR_LOW},
    {"air_s_crit", "AIRE MUY BAJO", SCBA_COLOUR_CRITICAL},
    {"air_s_empty", "SIN AIRE", SCBA_COLOUR_CRITICAL}
};

enum E_SCBA_ACTION
{
    SCBA_EQUIP = 1,
    SCBA_REFILL = 2
}
#define SCBA_ALL_ACTIONS (_:SCBA_EQUIP | _:SCBA_REFILL)

enum E_SCBA_SYMPTOM
{
    Float:ScbaHealthAbove,
    ScbaGaspIntervalMs,
    ScbaSwayOnset,
    ScbaSwayLevel,
    ScbaSymptomMessage[80]
}
new const ScbaSymptoms[][E_SCBA_SYMPTOM] =
{
    {60.0, 12 * SCBA_SECOND_MS, 0, 0, "Te falta el aire y te cuesta respirar."},
    {30.0, 6 * SCBA_SECOND_MS, 2100, 2350, "Te sientes mareado y te cuesta mantener el equilibrio."},
    { 0.0, 4 * SCBA_SECOND_MS, 3900, 3900, "La vista te da vueltas. Sientes que vas a desvanecerte."}
};

enum E_SCBA_POINT
{
    bool:ScbaPointEnabled,
    ScbaPointName[40],
    Float:ScbaPointX,
    Float:ScbaPointY,
    Float:ScbaPointZ,
    ScbaPointInterior,
    ScbaPointWorld,
    Float:ScbaPointRadius,
    ScbaPointActions
}
#define SCBA_UNUSED_POINT_ROWS {false}, {false}, ...
new ScbaSupplyPoints[SCBA_SUPPLY_POINT_LIMIT][E_SCBA_POINT] =
{
    // TODO(SCBA-HQ): set the agreed HQ coordinates, then enable this entry.
    {false, "HQ LSFD (pendiente)", 0.0, 0.0, 0.0, 0, 0, SCBA_INTERACTION_RADIUS, SCBA_ALL_ACTIONS},
    SCBA_UNUSED_POINT_ROWS
};

enum E_SCBA_VEHICLE
{
    bool:ScbaVehicleEnabled,
    ScbaVehicleId,
    ScbaVehicleExpectedModel,
    ScbaVehicleInterior,
    Float:ScbaVehicleRadius,
    ScbaVehicleActions
}
#define SCBA_UNUSED_VEHICLE_ROWS {false, INVALID_VEHICLE_ID}, {false, INVALID_VEHICLE_ID}, ...
new ScbaSupplyVehicles[SCBA_SUPPLY_VEHICLE_LIMIT][E_SCBA_VEHICLE] =
{
    // Vehicle IDs belong to the host's fleet. A matching model alone grants nothing.
    {false, INVALID_VEHICLE_ID, 407, 0, SCBA_INTERACTION_RADIUS, SCBA_ALL_ACTIONS},
    SCBA_UNUSED_VEHICLE_ROWS
};

enum E_SCBA_PART
{
    SCBA_BACKPACK,
    SCBA_MASK,
    SCBA_PART_COUNT
}
enum E_SCBA_MODEL
{
    ScbaCustomModel,
    ScbaBaseModel,
    ScbaModelDff[40],
    ScbaModelTxd[40],
    ScbaModelBone,
    ScbaModelDffCrc,
    ScbaModelTxdCrc
}
new const ScbaModels[SCBA_PART_COUNT][E_SCBA_MODEL] =
{
    {-21001, 19300, "scba/scba.dff", "scba/scba.txd", 1, 0xEA75767C, 0xD55595F4},
    {-21002, 19300, "scba/mask.dff", "scba/mask.txd", 2, 0x5D138AF1, 0xC31AE780}
};

enum E_SCBA_FIT
{
    ScbaFitSkin,
    E_SCBA_PART:ScbaFitPart,
    Float:ScbaFitX,
    Float:ScbaFitY,
    Float:ScbaFitZ,
    Float:ScbaFitRotX,
    Float:ScbaFitRotY,
    Float:ScbaFitRotZ,
    Float:ScbaFitScaleX,
    Float:ScbaFitScaleY,
    Float:ScbaFitScaleZ
}
new const ScbaAttachmentFits[][E_SCBA_FIT] =
{
    // Shared GTA skeleton. Fitted against the LSFD 277 mesh and its head and upper spine bind matrices.
    {-1, SCBA_BACKPACK, -0.079, 0.050, -0.020, 0.0, 90.0, 180.0, 1.0, 1.12, 1.0},
    {-1, SCBA_MASK, 0.0394, 0.0793, 0.0045, -9.46, -1.27, -164.10, 1.07, 1.07, 1.07}
};

#if SCBA_FULL_AIR_SECONDS <= 0 || SCBA_FULL_AIR_SECONDS > 86400
    #error SCBA_FULL_AIR_SECONDS must be between 1 and 86400
#endif
#if SCBA_CRITICAL_AIR_PERCENT <= 0 || SCBA_CRITICAL_AIR_PERCENT >= SCBA_LOW_AIR_PERCENT || SCBA_LOW_AIR_PERCENT >= 100
    #error SCBA reserve thresholds must satisfy 0 < critical < low < 100
#endif
#if SCBA_DAMAGE_INTERVAL_MS <= 0 || SCBA_SUPPLY_DURATION_MS <= 0 || SCBA_ALARM_INTERVAL_MS <= 0
    #error SCBA effect and supply intervals must be positive
#endif
#if SCBA_ANIM_RETRY_MS <= 0 || SCBA_ANIM_START_TIMEOUT_MS <= SCBA_ANIM_RETRY_MS
    #error SCBA animation retry must fit within the start timeout
#endif
#if SCBA_PULSE_INTERVAL_MS <= 0 || SCBA_HUD_INTERVAL_MS <= 0 || SCBA_SUPPLY_POINT_LIMIT <= 0 || SCBA_SUPPLY_VEHICLE_LIMIT <= 0
    #error SCBA pulse intervals and source limits must be positive
#endif
#if SCBA_GAUGE_NEEDLE_STEPS < 1 || SCBA_GAUGE_NEEDLE_STEPS > 99
    #error SCBA needle textures must use two-digit frame numbers
#endif
