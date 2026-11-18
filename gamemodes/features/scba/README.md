# SCBA para LSFD

El gamemode incorpora el SCBA con un solo include en `SARP.pwn`. La guía para jugadores explica cómo usar el equipo, reconocer las alarmas y leer los seis estados del manómetro. [Guía de uso del SCBA](GUIA.md)

## Organización del módulo

| Área | Dónde encontrarla |
| --- | --- |
| Entrada y eventos de SA-MP | `header.inc` reúne los archivos y `hooks.inc` enlaza el ciclo del jugador con el módulo. |
| Configuración e integración | `config.inc` define las reglas. `host.inc` adapta el acceso de LSFD, los interiores, la flota y las operaciones nativas de modelos y animaciones. |
| Equipo y puntos de servicio | `equipment.inc` coloca y retira los accesorios. `sources.inc` comprueba el punto o vehículo tanto al iniciar como durante el abastecimiento. |
| Ajuste personal de accesorios | `fit_editor.inc` abre el editor nativo, aplica la posición y la rotación de la pieza y conserva el ajuste confirmado. [Integración con los stats del personaje](CONFIGURACION.md#ajuste-personal-en-los-stats-del-personaje) |
| Estado y reglas de juego | `state.inc` conserva la reserva por jugador. `runtime.inc` gestiona el equipo, la recarga y el consumo. `symptoms.inc` aplica los avisos y el mareo. |
| Interacción con el jugador | `commands.inc` recibe `/scba`. `presentation.inc` dibuja el manómetro y distribuye los eventos de rol y el sonido. |

El módulo reutiliza `GetPlayerRoleplayName` del chat para las acciones del personaje. Los eventos de movimiento y mareo permiten respetar los bloqueos o efectos que otro sistema aplique al mismo jugador.

## Configuración

La referencia técnica contiene los campos y permisos de cada tipo de puesto, ejemplos de alta y baja de vehículos y los valores configurables con sus unidades. Incluye cómo tomar coordenadas y comprobar el mapeo dentro del juego. [Configurar puntos, vehículos y reglas del SCBA](CONFIGURACION.md)

## Funcionamiento

El manómetro aparece al equiparse y permanece visible hasta guardar o perder el equipo.
La aguja, el porcentaje y la autonomía muestran el aire del cilindro. `/scba mascara` coloca o retira la máscara.
Con la máscara puesta, `/scba aire` conecta el regulador y empieza el consumo. Repetirlo desconecta el regulador y conserva la reserva.
La franja muestra «DESCONECTADO» en espera y «EN SERVICIO», «RESERVA» o «AIRE MUY BAJO» al consumir. Con la máscara puesta y el cilindro vacío muestra «SIN AIRE».
La reserva inicial dura veinte minutos de consumo, tanto dentro como fuera. Al retirar la máscara o guardar el equipo, el regulador se desconecta.

El personaje pierde 5 HP cada 2 segundos mientras está en un interior con la máscara puesta y el cilindro vacío. Desconectar el regulador en esas condiciones conserva el intervalo de daño.
El primer descuento llega tras 2 segundos en esas condiciones. Salir al exterior o quitarse la máscara detiene el daño,
los jadeos y el mareo del SCBA. Al volver a entrar con la máscara puesta y el cilindro vacío empieza un intervalo nuevo.
Con el regulador conectado, el aire y las alarmas de reserva siguen su curso en el exterior. La regla se aplica a cualquier interior reconocido,
haya o no un incendio. La duración del cilindro, el intervalo y la cantidad de daño se configuran por separado.

Para recargar hay que estar a pie con la máscara retirada. En un punto fijo, alejarse interrumpe la recarga.
En el camión, el personaje se orienta hacia el vehículo y permanece en la animación de cambio de cilindro
durante los ocho segundos configurados. El contador empieza cuando se confirma la animación.
Si el cliente aún está cargando la biblioteca, el sistema reintenta la animación dentro de la misma recarga.
La franja indica «RECARGANDO». Al completar la recarga vuelve a «DESCONECTADO» y marca el 100 % de aire.
El jugador puede interrumpirla con `/scba cancelar`.
Al terminar o interrumpirse se libera el bloqueo de movimiento de SCBA. Si el camión se mueve, desaparece
o cambia de dimensión, el jugador conserva el aire anterior.

Los /me describen acciones del personaje. Los avisos de alarma y jadeos son entornos
anónimos, sin nombre ni firma. Texto y sonido llegan solo al radio configurado, en el
mismo interior y mundo. El sonido se emite desde el portador y tiene un intervalo mínimo.
El entorno de reserva aparece al cambiar de nivel. Los jadeos se anuncian con el primer
daño. Por encima de 60 HP se oyen cada 12 segundos, entre 31 y 60 HP cada 6 y a 30 HP o menos cada 4.
Los jadeos posteriores solo se oyen. El entorno se escribe una vez por episodio.

La primera pérdida de vida también avisa en privado de la falta de aire y mantiene la cámara estable.
En pérdidas posteriores, el jugador recibe un aviso nuevo al entrar en un tramo más grave.
Al llegar a 60 HP empieza un balanceo suave que aumenta conforme baja la vida. A 30 HP se intensifica.
`ScbaSymptoms` reúne los límites, la frecuencia de los jadeos y la intensidad inicial y final de cada tramo. El balanceo se aplica a pie,
se retira al quitar la máscara y respeta los efectos de cámara que gestione el servidor.

El estado es temporal por conexión. La muerte limpia los accesorios y la interfaz,
conservando la reserva dentro de la conexión. Una conexión nueva comienza sin equipo.
El reloj usa tiempo transcurrido. Una pausa larga no reproduce ráfagas de daño o sonidos.

## Sonidos del equipo

Los archivos de `audio/scba/` se reproducen en la posición del personaje.

| Sonido | Cuándo se oye |
| --- | --- |
| `reserva.mp3` | Dos pitidos al entrar en reserva. |
| `critica.mp3` | Tres pulsos más rápidos con aire muy bajo. |
| `valvula.mp3` | Una válvula abre y se oye un soplo corto de aire en el puesto de carga. |
| `acople.mp3` | El encaje metálico y un escape breve al cambiar el cilindro junto al camión. |

La recarga emite un solo efecto cuando el cliente confirma la animación. Una alarma de reserva
tiene prioridad si coincide con un acople. Para activar los cuatro efectos originales, sirve
el directorio y configura `SCBA_ALARM_BASE_URL` con su URL, sin barra final, antes de compilar.
Con la URL vacía, el juego utiliza sonidos nativos de respaldo. [Sonidos de SA-MP](https://open.mp/docs/scripting/resources/sound-ids#old-list-from-jbeta)

Para una prueba en el mismo PC, desde la raíz del repositorio:

~~~powershell
py -3 -m http.server 8784 --bind 127.0.0.1 --directory audio/scba
~~~

La URL local es `http://127.0.0.1:8784`. En un servidor compartido, la URL debe ser accesible
desde los equipos de los jugadores. SA-MP usa el canal de audio por internet del cliente para estas señales.
Si el cliente muestra una línea con la URL en el chat, `/audiomsg` permite ocultarla. [Comandos del cliente](https://open.mp/docs/client/ClientCommands#audiomsg)

## Modelos y servidor

Los cuatro archivos del equipo están en models/scba, en la raíz del repositorio.
Distribuye esa carpeta como models/scba del directorio de ejecución del servidor,
mantén useartwork 1 y utiliza SA-MP 0.3.DL-R1.

La tabla ScbaModels registra los IDs, archivos, huesos y CRC de ambos modelos.
Si se reemplaza un archivo, actualiza su CRC en esa tabla. El registro del primer
modelo puede devolver cero en SA-MP 0.3.DL aunque tenga éxito. El módulo comprueba
los nombres mediante el registro CRC y espera la finalización de descarga del jugador.

Los dos DFF tienen el pivote raíz normalizado. El diccionario de la máscara incluye las texturas
originales, la esfera, 51 posiciones de aguja y los seis rótulos de estado. Las letras visibles de cada rótulo
quedan centradas dentro de la franja. El indicador utiliza siete TextDraws por jugador y las cifras del HUD de San Andreas.
Para regenerar los gráficos, ejecuta `python scripts/build-scba-gauge.py` con Pillow 13 y actualiza
el CRC impreso en ScbaModels. La opción `--check` comprueba que el arte empaquetado coincide.

## Pruebas

Desde la raíz, con Pawn 3.10.10 y SA-MP 0.3.DL-R1 disponibles:

~~~powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/test-scba.ps1
~~~

El runner busca pawno/pawncc.exe y .local-server/samp-server.exe, o samp-server.exe
en la raíz. Se pueden indicar otras rutas mediante -CompilerExecutable y -ServerExecutable.
Compila el gamemode completo y el arnés con -;+ -(+ -d3 -E. Utiliza un directorio nuevo
bajo .local-server/scba-tests y un puerto libre de loopback entre 7781 y 7791.

Para comprobar además los accesorios con natives reales, mediante un NPC aislado:

~~~powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/test-scba.ps1 -NativeAttachments
~~~

Esta opción requiere FCNPC-DL 2.0.11. Busca .local-server/plugins/FCNPC-DL.dll o acepta
-NpcPlugin. No necesita datos de colisión para esta comprobación de accesorios.

El arnés comprueba la primera recarga, el audio del puesto y del camión, la devolución del equipo,
la duración del cilindro y los estados del manómetro. Simula colocar la máscara en espera, conectar y desconectar el regulador entre pulsos y reanudar desde la reserva anterior. También verifica la frecuencia de los jadeos, la progresión
del mareo y el alcance de las alarmas. Comprueba que el daño se detiene al salir y espera un intervalo completo al volver a entrar.
Compara la secuencia de daño dentro de un interior con la autonomía normal y con un cilindro de 60 segundos. Los resultados quedan en
`core-server.log`, `short-cylinder.log` y, al usar la opción nativa, `native-server.log`.

El arnés ejecuta las funciones de producción. Simula jugadores, reloj, vehículos, audio
y TextDraws. El registro de modelos, búsquedas CRC y ejecución Pawn son reales.
La comprobación opcional usa los callbacks reales de descarga, el control de movimiento y los accesorios sobre la skin 277.
Las pruebas del editor cubren guardado y cancelación por pieza, el rechazo de una escritura, el cambio de skin y la carga de ajustes en otra conexión mediante stats simulados. La prueba nativa también comprueba que el callback oficial de edición llegue al módulo.
Las pruebas de juego usan sus propias ubicaciones en memoria y se pueden ejecutar con puntos del servidor ya configurados.
Los ejecutables de pruebas y el AMX candidato no sustituyen el servidor jugable.

## QA en cliente

El ajuste se contrastó de frente, de perfil y de espaldas con la malla y los huesos de la skin 277.
Para la prueba local, equípate en el puesto, ponte la máscara con `/scba mascara` y conecta el regulador con `/scba aire`. Con el cilindro vacío, comprueba que la vida se mantiene
en la calle. Usa `/irpos 508.7362 -87.4335 998.9609 0 11` para entrar en el interior que utiliza la selección de personaje.
A los dos segundos empieza el daño. Con `/irpos 1544.3810 -1675.4711 13.5583 0 0` vuelves al puesto y el daño se detiene.
El primer descuento deja la cámara estable incluso si el personaje ya estaba herido.
Para revisar el resto del equipo, retira la máscara y escucha una recarga en el puesto y otra en el camión con un compañero cerca.
El sonido de falta de aire es un recurso nativo de tos/ahogo, configurable por ID.

Definir la HQ, autorizar la flota y conectar la membresía real son los puntos de
configuración e integración pendientes para usarlo en el servidor principal.
