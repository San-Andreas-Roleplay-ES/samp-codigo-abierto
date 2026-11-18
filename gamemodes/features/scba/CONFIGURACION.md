# Configuración del SCBA de LSFD

El SCBA admite puntos fijos y vehículos de la flota. Cada uno puede entregar y recibir equipo, recargar cilindros o prestar ambos servicios. La configuración incluida deja la sede y la flota desactivadas hasta que LSFD defina sus ubicaciones y unidades. [Configuración del módulo](config.inc)

## Servicios disponibles

| Uso | Valor de `acciones` en Pawn | Comandos que habilita |
| --- | --- | --- |
| Vestuario o depósito de equipos | `_:SCBA_EQUIP` | `/scba equipar` y `/scba guardar` |
| Banco de carga o camión de recarga | `_:SCBA_REFILL` | `/scba recargar` |
| Puesto o vehículo con ambos servicios | `SCBA_ALL_ACTIONS` | Los tres comandos anteriores |

El equipo se puede devolver en cualquier ubicación que lo entregue. Cada interacción se hace a pie. Para recargar, el jugador también debe llevar el equipo, tener la máscara retirada y haber consumido algo de aire.

## Registrar un punto fijo

1. En el juego, ponte donde estará el jugador al usar el servicio. Usa `/marcarcoord scba_vestuario` y luego `/listarcoords` para leer `X`, `Y` y `Z`. El comando también las escribe en `scriptfiles/coordenadas.txt`.
2. Anota el interior y el mundo virtual del punto. Puedes consultarlos en el código con `GetPlayerInterior(playerid)` y `GetPlayerVirtualWorld(playerid)`. El archivo de coordenadas guarda el nombre y las tres coordenadas.
3. En `ScbaSupplyPoints`, añade una fila por ubicación antes de `SCBA_UNUSED_POINT_ROWS`. Los campos, en orden, son `activo`, `nombre`, `X`, `Y`, `Z`, `interior`, `mundo virtual`, `radio` y `acciones`. Usa un radio positivo en unidades del juego. Para retirar un punto, cambia `activo` a `false`.
4. Recompila el gamemode y carga el AMX actualizado en el servidor donde lo vayas a probar.

Este ejemplo muestra un vestuario dentro de un edificio y dos puestos exteriores. Sustituye las coordenadas y las dimensiones por las de tu mapeo.

```pawn
new ScbaSupplyPoints[SCBA_SUPPLY_POINT_LIMIT][E_SCBA_POINT] =
{
    {true, "Vestuario", 508.7362, -87.4335, 998.9609, 11, 7, SCBA_INTERACTION_RADIUS, _:SCBA_EQUIP},
    {true, "Banco de carga", 1552.0, -1675.0, 13.5583, 0, 0, SCBA_INTERACTION_RADIUS, _:SCBA_REFILL},
    {true, "Puesto mixto", 1544.3810, -1675.4711, 13.5583, 0, 0, SCBA_INTERACTION_RADIUS, SCBA_ALL_ACTIONS},
    SCBA_UNUSED_POINT_ROWS
};
```

Se mide la distancia 3D al punto. El jugador debe estar dentro del radio y compartir el interior y el mundo virtual. Si varios servicios se solapan, se elige la primera fila fija que permite la acción. Si ninguno de los puntos fijos la ofrece, se usa el primer vehículo válido en el orden de su tabla. La recarga permanece vinculada al punto o vehículo elegido al iniciarla.

## Registrar un vehículo de la flota

Cada unidad ocupa una fila propia en `ScbaSupplyVehicles`. La autorización vincula el `vehicleid` que asigna el servidor **y** el modelo esperado. Así pueden circular otros camiones del modelo 407 sin prestar servicio. La posición y el mundo virtual se leen del vehículo en ese momento.

**1. Reserva la fila.** En `config.inc`, indica `activo`, `vehicleid`, `modelo esperado`, `interior`, `radio` y `acciones`. El ejemplo reserva la primera fila para una autobomba exterior que solo recarga. `LSFD_AUTOBOMBA_1` es el nombre que damos a esa fila en el ejemplo.

```pawn
#define LSFD_AUTOBOMBA_1 (0)
new ScbaSupplyVehicles[SCBA_SUPPLY_VEHICLE_LIMIT][E_SCBA_VEHICLE] =
{
    {false, INVALID_VEHICLE_ID, 407, 0, SCBA_INTERACTION_RADIUS, _:SCBA_REFILL},
    SCBA_UNUSED_VEHICLE_ROWS
};
```

**2. Vincula el camión.** En el código que crea esa unidad de LSFD, usa el `vehicleid` devuelto por `CreateVehicle`. Hazlo cada vez que la flota vuelva a crearla. Si la flota cambia su interior, actualiza también `ScbaVehicleInterior`.

```pawn
if (vehicleid > 0 && vehicleid < MAX_VEHICLES
    && GetVehicleModel(vehicleid) == ScbaSupplyVehicles[LSFD_AUTOBOMBA_1][ScbaVehicleExpectedModel])
{
    ScbaSupplyVehicles[LSFD_AUTOBOMBA_1][ScbaVehicleId] = vehicleid;
    ScbaSupplyVehicles[LSFD_AUTOBOMBA_1][ScbaVehicleEnabled] = true;
}
```

**3. Retira el registro antes de destruir o sustituir la unidad.** Este bloque interrumpe las recargas de esa fila aunque el servidor reutilice inmediatamente el mismo ID para otro vehículo del mismo modelo.

```pawn
if (ScbaSupplyVehicles[LSFD_AUTOBOMBA_1][ScbaVehicleId] == vehicleid)
{
    ScbaSupplyVehicles[LSFD_AUTOBOMBA_1][ScbaVehicleEnabled] = false;
    ScbaSupplyVehicles[LSFD_AUTOBOMBA_1][ScbaVehicleId] = INVALID_VEHICLE_ID;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (ScbaPlayer[playerid][ScbaSupplySource] == SCBA_SOURCE_VEHICLE
            && ScbaPlayer[playerid][ScbaSupplyIndex] == LSFD_AUTOBOMBA_1)
            ScbaCancelSupply(playerid, true);
    }
}
```

Para prestar servicio, el vehículo debe tener salud positiva y estar detenido dentro de la tolerancia configurada. El radio se mide desde el centro del vehículo. Si sale de alcance, cambia de mundo o se pone en marcha durante la recarga, el jugador conserva el aire que tenía.

Con tres o más plazas libres, `SCBA_UNUSED_POINT_ROWS` y `SCBA_UNUSED_VEHICLE_ROWS` completan la tabla con filas desactivadas. Pawn extrapola a partir de las dos últimas filas, por eso ambas macros las repiten. Si quedan una o dos plazas, sustituye la macro por tantas filas explícitas como plazas queden. Usa `{false}` en los puntos y `{false, INVALID_VEHICLE_ID}` en la flota. Con la tabla llena, termina con la última fila configurada.

## Ajustes y valores por defecto

Las duraciones terminadas en `_MS` se expresan en milisegundos. Las distancias se miden en unidades del juego. [Valores en `config.inc`](config.inc)

| Ajuste | Valor por defecto | Efecto |
| --- | --- | --- |
| `SCBA_FULL_AIR_SECONDS` | 1200 s (20 min) | Autonomía de un cilindro lleno con el regulador conectado. El tiempo en espera conserva la reserva. Admite de 1 a 86 400 segundos. |
| `SCBA_LOW_AIR_PERCENT` y `SCBA_CRITICAL_AIR_PERCENT` | 25 % y 10 % | Avisos al llegar a cada porcentaje. El umbral crítico debe ser mayor que cero e inferior al de reserva, y el de reserva inferior al 100 %. |
| `SCBA_DAMAGE_AMOUNT` y `SCBA_DAMAGE_INTERVAL_MS` | 5 HP cada 2000 ms | Daño con el cilindro agotado, la máscara puesta y el jugador en un interior. Su intervalo sigue corriendo si se desconecta el regulador y es independiente de la autonomía. |
| `SCBA_SUPPLY_MIN_MS` y `SCBA_SUPPLY_MAX_MS` | 2000 y 8000 ms | Tiempo mínimo de manipulación y duración con el cilindro vacío. El mínimo debe ser positivo y el máximo puede ser igual o mayor. |
| `SCBA_INTERACTION_RADIUS` | 3.0 | Radio común que las filas pueden usar o sustituir por un valor propio. |
| `SCBA_SERVICE_MOVEMENT_RADIUS` | 0.75 | Desplazamiento que admite la recarga desde donde empezó el jugador. |
| `SCBA_VEHICLE_MAX_SPEED` | 0.05 | Magnitud máxima de la velocidad que devuelve `GetVehicleVelocity`, en unidades nativas del juego. |
| `SCBA_SUPPLY_POINT_LIMIT` y `SCBA_SUPPLY_VEHICLE_LIMIT` | 16 puntos y 32 vehículos | Capacidad de cada lista, incluidas las filas desactivadas. |
| `SCBA_EVENT_RADIUS` y `SCBA_ALARM_INTERVAL_MS` | 10.0 y 5000 ms | Alcance de sonidos y entornos en la misma dimensión, e intervalo mínimo entre pitidos de un equipo. |
| `SCBA_PULSE_INTERVAL_MS` y `SCBA_HUD_INTERVAL_MS` | 250 y 1000 ms | Frecuencia de comprobación del equipo y de actualización periódica del manómetro. |
| `SCBA_ANIM_RETRY_MS` y `SCBA_ANIM_START_TIMEOUT_MS` | 500 y 3000 ms | Espera entre intentos y plazo para que el cliente empiece la animación. |

En ambos tipos de fuente, la duración es `mínimo + (máximo − mínimo) × fracción de aire que falta`, redondeada al milisegundo. Se fija al iniciar el servicio y el reloj arranca cuando el cliente confirma la animación. Con media carga son 5000 ms, tanto con el cilindro de prueba de 60 segundos como con el de 20 minutos. Un cilindro lleno queda fuera del servicio.

| Otros ajustes | Dónde se cambian |
| --- | --- |
| Síntomas por nivel de salud | `ScbaSymptoms` reúne, en orden descendente de salud, el umbral, intervalo de jadeos, balanceo inicial y final y mensaje privado. Los jadeos se repiten cada 12 s por encima de 60 HP, cada 6 s por encima de 30 y hasta 60 HP y cada 4 s con 30 HP o menos. |
| Alarmas y efectos | `SCBA_ALARM_BASE_URL` acepta la URL pública de los cuatro MP3, sin barra final. Con la URL vacía, el cliente reproduce los sonidos nativos. `SCBA_*_CLIP` selecciona los archivos, `SCBA_FALLBACK_*_SOUND` los sonidos nativos y `SCBA_DISTRESS_SOUND` el jadeo. `SCBA_AUDIO_CLIP_WINDOW_MS` reserva 1100 ms por oyente para evitar solapamientos. Los MP3 de reserva y aire crítico tienen picos de aproximadamente −10,4 dBFS. El volumen de los sonidos nativos depende del cliente. |
| Animaciones de recarga | `SCBA_SERVICE_ANIM_LIBRARY` y `SCBA_SERVICE_ANIM_NAME` seleccionan la animación del puesto. `SCBA_TRUCK_ANIM_LIBRARY` y `SCBA_TRUCK_ANIM_NAME` usan por defecto la misma animación agachada, `BOMBER/BOM_Plant_Loop`. Pueden configurarse por separado. Ambas usan `SCBA_SERVICE_ANIM_SPEED`. |
| Manómetro y colores | `SCBA_GAUGE_LEFT`, `SCBA_GAUGE_TOP`, `SCBA_GAUGE_WIDTH` y `SCBA_GAUGE_HEIGHT` sitúan y dimensionan el indicador. El resto de `SCBA_GAUGE_*` ajusta la esfera, la aguja, la franja y las cifras. `ScbaStatusArt` asocia textura, rótulo y color por estado. `SCBA_COLOUR_*` reúne la paleta. |
| Modelos y ajuste por skin | `ScbaModels` contiene ID personalizado, modelo base, DFF, TXD, hueso y CRC de cada pieza. `ScbaAttachmentFits` define desplazamiento, rotación y escala por skin y pieza. La skin `-1` proporciona el ajuste común, calibrado sobre la 277 de bombero. |

Si cambias el dibujo de la esfera, los fotogramas de la aguja o los rótulos de estado, regenera las texturas y actualiza el CRC de la máscara. [Modelos y servidor](README.md#modelos-y-servidor)

## Ajuste personal en los stats del personaje

Cada personaje puede tener un ajuste por pieza. La clave de guardado es el **ID persistente del personaje** junto con `SCBA_BACKPACK` (0) o `SCBA_MASK` (1). Para cada pieza se guardan seis valores decimales, en orden de desplazamiento X, Y y Z con respecto al hueso y rotación X, Y y Z en grados. La ausencia de registro usa la calibración de `ScbaAttachmentFits`. Una fila con los seis valores a cero representa un ajuste explícito. El modelo, hueso y escala los determina el servidor.

Para conectar el almacenamiento oficial, define `SCBA_CUSTOM_FIT_STATS` antes de incluir `features/scba/header` e implementa las dos funciones `ScbaHostSaveFit` y `ScbaHostResetFits`. El servidor de pruebas usa los adaptadores de sesión incluidos.

- **Al cargar el personaje**, pasa cada fila a `ScbaSetCharacterFit(playerid, part, pose)`, con un arreglo `pose[E_SCBA_POSE]`. Sus campos son `ScbaOffsetX`, `ScbaOffsetY`, `ScbaOffsetZ`, `ScbaRotationX`, `ScbaRotationY` y `ScbaRotationZ`. Esta llamada también actualiza una pieza ya puesta y evita una segunda escritura en los stats.
- **Al confirmar una edición**, el módulo llama a `ScbaHostSaveFit(playerid, part, pose)` en `host.inc`. Actualiza solo esa pieza en los stats del PJ activo. Devuelve `true` cuando los stats acepten el cambio. Si devuelves `false`, el editor restaura el ajuste anterior. Al cancelar con Esc se omite la escritura.
- **Al restablecer**, `/scba ajustar reset` llama a `ScbaHostResetFits(playerid)`. Elimina en una sola operación los ajustes de ambas piezas de ese personaje. Devuelve `true` también si ya estaban ausentes. Así, al volver a equipar se elige la calibración vigente de la skin. Si el adaptador rechaza el cambio, el módulo recupera los ajustes anteriores.
- **Al cambiar de PJ dentro de una conexión**, llama a `ScbaCleanup(playerid)` y `ScbaClearCharacterFits(playerid)` antes de cargar las filas del nuevo PJ. Si la consulta es asíncrona, comprueba que el personaje que solicitó las filas sigue activo antes de aplicarlas. Al desconectar, el módulo limpia la sesión automáticamente.

La base pública conserva los ajustes en memoria durante la conexión. El servidor oficial aporta el ID del personaje y su almacenamiento permanente. `SCBA_FIT_OFFSET_LIMIT` permite configurar el desplazamiento máximo por eje desde el hueso (2.0 unidades por defecto). [Adaptador de stats](host.inc) · [Editor de accesorios de SA-MP](https://open.mp/docs/scripting/callbacks/OnPlayerEditAttachedObject)

## Integración y prueba en el mapeo

- **Acceso LSFD.** `ScbaHostCanUse` acepta a los jugadores conectados que sean personas. Integra aquí el permiso de facción si el servidor debe limitar el uso a sus miembros. [Adaptación al servidor](host.inc)
- **Edificios.** `ScbaHostIsIndoors` considera interior un ID mayor que cero. Para edificios mapeados en el interior `0`, conecta la consulta de propiedades o zonas del servidor. Esto afecta al daño por falta de aire. Un puesto de servicio sigue comprobando el interior y el mundo registrados en su fila.
- **Puntos fijos.** En el ejemplo, `/irpos 508.7362 -87.4335 998.9609 7 11` lleva al vestuario. `/irpos` recibe **mundo y luego interior**, mientras que la fila guarda **interior y luego mundo**. Comprueba que puedes equiparte y guardar allí. En el banco, prueba a recargar con la máscara retirada y el cilindro parcialmente usado.
- **Flota.** Prueba el camión registrado y otro del mismo modelo fuera del radio de cualquier puesto fijo. Solo el registrado debe ofrecer la acción. Mueve o retira el camión durante una recarga y comprueba que se interrumpe y se conserva la reserva anterior. [Pruebas automáticas](README.md#pruebas)
