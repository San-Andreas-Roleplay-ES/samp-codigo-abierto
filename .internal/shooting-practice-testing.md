# Configurar y probar las prácticas de tiro

Actualizado: 2026-10-12. Hasta 64 zonas independientes por defecto, diez puntos de inicio y diez blancos por zona,
un participante por zona y una ronda por jugador. Las zonas archivadas cuentan para ese límite.
El selector, los modelos nuevos y los ajustes de altura del pickup, separadores y distribución/apertura
están compilados, probados y aplicados al servidor local tras la solicitud de arranque del 11 de octubre.
El usuario aceptó el MVP actual y dio por cerrada esta etapa de desarrollo.
Todas las zonas actuales son públicas. Facciones, historial y supervisión quedan para la siguiente integración.
La preparación posterior de SQL añade puestos versionados y funciones de integración, con el archivo
como backend activo. Está compilada y probada, pendiente de aplicar mediante un reinicio solicitado.

[Demostración del MVP en YouTube](https://www.youtube.com/watch?v=cpVQbs0igC8).
El [resumen para la PR](shooting-practice-pr.md) distingue el alcance implementado y las integraciones futuras.

## Entrar y abrir la configuración

1. Inicia con Iniciar-SARP.cmd, o desde el repositorio ejecuta:
   `powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Start`.
   Si estaba encendido, detenlo primero para aplicar cambios.
2. Conecta SA-MP **0.3.DL-R1** a **127.0.0.1:7777** desde este PC.
3. Accede como RCON con la clave local de .local-server/server.cfg. En el entorno preparado,
   Acceso-RCON.txt contiene el comando de acceso.
4. Estando vivo y a pie, escribe **/practicaconfig**, sin argumentos.

/practicaconfig muestra las zonas existentes, con diez por página. Selecciona una o usa
**/practicaconfig <ID>** para abrirla directamente. Los IDs son estables y no se reutilizan al archivar.
Un archivo anterior se recupera como **zona #1: Campo de tiro**, conservando sus puntos y reglas.
Una instalación nueva empieza sin zonas: elige **Crear zona**, escribe su nombre y configúrala.
Las zonas nuevas quedan **inactivas** hasta que completes la configuración y elijas **Activar zona**.
El comando acepta solo un ID numérico positivo como argumento.

| Opción | Función |
| --- | --- |
| Indicadores de inicio | Activar o desactivar una pistola flotante en cada puesto de la zona. |
| Puntos de inicio | Añadir puestos desde tu posición actual, nombrarlos, reubicarlos y eliminarlos. |
| Blancos | Añadir, nombrar, elegir objeto/comportamiento, mover, girar, duplicar y eliminar. |
| Reglas del ejercicio | Armas permitidas, puntaje, errores, tiempos, orden y distribución. |
| Vista previa | Marcadores de los puestos, objetos y etiquetas privadas. |
| Iniciar práctica | Aparece si el puesto está listo, estás en el inicio y equipas un arma permitida. |
| Cancelar práctica | Aparece durante tu ronda activa. |
| Reiniciar configuración | Borrar todos los puntos de inicio y blancos y restaurar reglas, con confirmación. |
| Reintentar guardado | Aparece si falló la escritura; también está en el listado general. |
| Nombre de la zona | Cambiar el nombre, hasta 48 caracteres, sin códigos de color. |
| Activar / Desactivar zona | Abrir o cerrar la zona para nuevas prácticas; debe estar libre. |
| Archivar zona | Retirar del listado y uso con confirmación; conservar ID y configuración. |

El título principal es **Zona #ID: Nombre**. **Zonas** vuelve al listado.
Iniciar práctica aparece solo si la zona está activa y puedes iniciar.
La vista previa está disponible desde el menú principal. Las condiciones se comprueban de nuevo al seleccionar Iniciar.

## Configurar uno o varios puntos de inicio

1. Crea/abre la zona que quieres configurar. Para borrar su montaje, elige **Reiniciar configuración > Reiniciar**.
   El ID y nombre se conservan. La zona queda inactiva.
2. Camina a la primera caseta y mira hacia los blancos.
3. Abre **Puntos de inicio > Añadir punto**. Se guardan inmediatamente tu posición, orientación, interior y mundo.
4. En **Editar punto de inicio**, elige **Nombre** para identificar la caseta.
   Escribe **-** para recuperar el nombre automático **Punto N**.
5. Vuelve al listado, camina a otra caseta y repite **Añadir punto**. Máximo diez puntos por zona.
6. Para reubicar uno existente, selecciona su fila y elige **Marcar mi posición**.
   Para borrarlo, elige **Eliminar punto > Eliminar**. Los blancos y reglas se conservan.
   Eliminar el último punto desactiva la zona. Puedes añadir otro después sin perder los blancos.

Todos los puntos comparten blancos y reglas, en el mismo interior y mundo.
Un punto adicional debe quedar a menos de 100 metros de los demás puestos.
Los blancos deben quedar entre **3 y 100 metros de cada punto**. Si un nuevo puesto incumple
esas condiciones, no se guarda y se muestra el motivo.

Hay **una sola práctica activa por zona**, aunque tenga varias casetas. Los blancos del ejercicio
son públicos para quienes estén en el mismo interior y mundo. Al terminar o cancelar se libera el campo.
Las zonas distintas conservan su funcionamiento independiente.

Reubicar el único punto a otro interior/mundo pide confirmar la eliminación de los blancos anteriores.
Cancelar conserva todo. Con varios puntos, se rechaza mover uno a otro interior/mundo.
Mover el único punto lejos, dentro del mismo mundo, conserva los blancos y puede dejar la zona inactiva.
Revisa sus distancias y vuelve a activarla cuando sea válida.

## Crear, mover y orientar blancos

1. Acércate a menos de **100 metros de uno de los puntos de inicio**, en el mismo interior y mundo.
   Abre **Blancos > Añadir blanco**. Desde otra ciudad se rechaza la creación antes de abrir el editor.
2. Elige una tarjeta del catálogo. Cada tarjeta muestra el objeto y su nombre, con seis por página.
   Filtra por **Todos**, **Siluetas**, **Botellas**, **Latas** u **Otros**, o usa **Anterior / Siguiente**.
   Puedes elegir **Caja marcada (1271)**, **Silueta armada A/B (1583/1584)**,
   **Silueta civil A/B (1585/1586)**, **Botella de vidrio A/B (1543/1544)**,
   **Lata de bebida (2601)** o **Cono (1238)**.
   **Atrás / ESC** vuelve al blanco que estabas editando. Al añadir uno nuevo, cancela la creación sin guardar.
3. Elige **Blanco de tiro (disparar)** o **Blanco protegido (no disparar)**, independiente del modelo.
   Esto activa distribución fija al guardar. Luego puedes activar la aleatoria en Reglas.
4. Se abre el editor visual sobre un objeto temporal, inicialmente delante del personaje.
   Usa los controles del juego para **mover y girar en los tres ejes**.
5. Colócalo entre **3 y 100 metros de cada punto de inicio**, en su mismo interior/mundo.
   Para probar usa 8-15 metros y orienta la cara hacia el puesto.
6. **Confirmar en el editor guarda todos los cambios**. **Cancelar descarta toda la edición**
   y conserva lo anterior. Si era un blanco nuevo, no se añade.
7. Repite con dos blancos de disparo y uno de no disparo. Ajusta las reglas y elige **Activar zona** en su menú principal.

Para editar uno existente selecciona su fila: Nombre, Objeto, Comportamiento, Mover y orientar,
Duplicar blanco, Eliminar blanco y, al final, Guardar cambios. El nombre admite 32 caracteres. Escribe
un guion (-) para restaurar el nombre automático. Cambios de nombre/modelo/comportamiento quedan
pendientes hasta Guardar cambios o confirmar el editor. Descartar conserva el blanco anterior.

Duplicar abre el editor con una copia. Confirmar usa un espacio libre sin alterar el original.
Eliminar exige confirmación. Máximo diez blancos. Si no aparece el editor, acércate al blanco.
Termina cualquier otro editor de objetos antes de abrirlo. Solo un administrador puede editar
cada zona a la vez. Sus rondas esperan hasta que guarde o descarte. Puedes editar otra zona
mientras hay una ronda en curso; los borradores y bloqueos son independientes.

Al abrir Blancos o Puntos de inicio aparecen automáticamente los blancos guardados y los puestos, solo para ti.
Siguen visibles en los diálogos, el catálogo y el editor. El blanco que estás moviendo sustituye
su copia guardada mientras lo editas. Los demás conservan sus posiciones y permanecen visibles.
Guardar actualiza el blanco cambiado. Cancelar restaura su versión guardada.
Volver al menú principal retira esta vista automática. Si activaste Vista previa previamente,
seguirá activada hasta que la ocultes.

El primer punto se puede marcar sin un inicio previo. Reubicar el único punto conserva el comportamiento
anterior. Los puntos adicionales se validan respecto al campo y sus blancos.

## Probar el selector y los objetos nuevos

Comprobaciones disponibles con los ajustes ya aplicados:

1. Abre **Blancos > Añadir blanco** una sola vez. Tras una pausa breve debe abrirse y permanecer visible.
   Comprueba que el fondo cubra todo el panel, los nombres queden dentro de cada tarjeta
   y los títulos de los botones estén centrados. Prueba también tras volver de una selección anterior.
2. En **Todos**, pasa a la segunda página y vuelve a la primera. Los botones de los extremos deben desactivarse.
3. Revisa los filtros. Siluetas tiene cuatro objetos, Botellas dos, Latas uno y Otros dos.
   Las tildes de **Atrás** y **Página** deben verse correctamente.
4. Elige cada objeto nuevo, configura su comportamiento y colócalo con el editor.
   Botellas y latas son pequeñas. Colócalas sobre una superficie visible y prueba primero a poca distancia.
5. Edita el nombre de un blanco y entra en **Objeto**. Cambia de página o filtro y pulsa **Atrás / ESC**.
   El nombre pendiente debe conservarse. **Descartar** conserva la configuración guardada.
6. Dispara a las botellas, la lata y el cono durante una práctica. Comprueba un acierto por blanco de tiro
   y un error si disparas durante un blanco protegido, incluso al aire.
7. En **Vista previa**, dispara a esos objetos y comprueba que se repongan tras un impacto.
   El renderizado, la colisión y la rotura nativa de cada modelo deben comprobarse en el cliente.
8. Tras guardar y un reinicio solicitado, revisa modelos, posiciones, rotaciones y comportamientos.

## Cerrar el menú y continuar la configuración

Las listas de **Puntos de inicio** y **Blancos**, y los menús de edición de un punto o blanco,
incluyen **Cerrar menú**. El diálogo se cierra para permitir desplazarse y las referencias visibles se conservan.

1. Selecciona **Cerrar menú**.
2. Desplázate a la posición deseada.
3. Usa **/practicaconfig**, sin argumentos, para continuar en la misma sección.
   **/practicaconfig <ID de la misma zona>** también continúa allí.
4. Selecciona **Añadir punto**, **Añadir blanco**, **Marcar mi posición** o **Mover y orientar**, según corresponda.

Cerrar el menú no confirma los cambios pendientes de un blanco. Se conservan hasta guardar,
confirmar el editor o descartar. **Guardar cambios** permanece como última opción.
Una lista cerrada no ocupa el campo. Un borrador de blanco pendiente mantiene su bloqueo de edición.
Cambiar a otra zona descarta el borrador anterior y retira sus referencias, igual que la navegación habitual.
Si otro administrador modifica la zona, se actualiza la sección al continuar.
La pérdida de acceso, muerte, desconexión o salida del rango válido de un borrador limpian su contexto.

## Vista previa

1. Selecciona **Vista previa**.
2. El diálogo se cierra y puedes caminar. Se muestran las etiquetas de todos los puestos y el checkpoint del más cercano, además de los
   objetos con sus modelos, posiciones y rotaciones. Las etiquetas indican nombre y comportamiento. El nombre automático es Blanco N, sin repetir el número.
3. Solo tú ves estos elementos. No hay ronda, sonidos ni puntuación.
   Los impactos se bloquean y el objeto alcanzado se repone para evitar que quede desaparecido.
   La reposición también funciona en la vista automática de configuración.
4. Abre **/practicaconfig <ID de la misma zona>** y selecciona **Ocultar vista previa** para retirarlos.
   Abrir el listado general o cambiar de zona también retira tu vista.

La vista se limpia al desconectarte, morir, entrar en un vehículo o cambiar de interior/mundo.
Guardar cambios actualiza las vistas de esa zona sin retirar los blancos que no cambiaron.
Iniciar una práctica retira las vistas de su zona y la del participante; otras zonas mantienen sus vistas.

## Indicadores de inicio y aviso cercano

1. Abre **/practicaconfig <ID>** y selecciona **Indicadores de inicio**.
2. La opción alterna entre **Desactivados** y **Pistola en cada punto**. Se guarda por zona.
   Está desactivada por defecto, también al cargar una configuración anterior.
3. Con la zona activa, comprueba una pistola flotante por puesto. Es solo una señal visual,
   sin texto flotante, entrega de armas ni inicio automático.
   Se ha bajado un metro respecto a la posición anterior. Comprueba que se vea sutil, cerca de las caderas.
4. Desactiva los indicadores y comprueba que desaparezcan todos. Acércate de nuevo a cualquier
   punto, a menos de 1.5 metros, y comprueba el aviso del chat:
   **Campo de tiro. Usa /practica para iniciar una práctica.**
5. El aviso funciona con y sin pickup, en el mismo interior y mundo. No se repite mientras
   permanezcas cerca. Al alejarte más de 3 metros y regresar, vuelve a aparecer.
6. Reubica o elimina un puesto y comprueba que su pickup se actualice. Desactivar o archivar
   la zona oculta sus indicadores y evita avisos. Reiniciar la configuración restablece Desactivados.
7. Reinicia cuando quieras verificar el guardado y comprueba que cada zona conserve su opción.

## Reglas y primera ronda

Para dos blancos de disparo y uno de no disparo configura:

| Regla | Valor de prueba |
| --- | --- |
| Armas permitidas | Desert Eagle (24), opcionalmente M4 (31) |
| Puntos mínimos | 2 |
| Errores máximos | 0 |
| Preparación / exposición / intervalo | 3 / 5 / 1 segundos |
| Orden | Aleatorio |
| Distribución | Fija por posición |

Abre **Reglas del ejercicio > Armas permitidas**. Selecciona un arma y pulsa **Cambiar**:
alterna entre **Permitida** y **No permitida**. Puedes habilitar varias. Siempre debe quedar al menos una.
Cada cambio se guarda inmediatamente. Pulsa **Volver** al terminar.
Puedes alternar entre las armas habilitadas durante la ronda. Elegir o disparar con otra cancela la práctica.

Los campos numéricos muestran una explicación, el valor actual y el rango permitido:

- **Puntos mínimos:** puntuación neta necesaria para aprobar. Aciertos menos errores.
  Ejemplo: 4 aciertos y 1 error dejan 3 puntos. El máximo alcanzable depende de los blancos de tiro configurados.
- **Errores máximos:** disparar ante un blanco protegido cuenta, incluso al aire. 0 exige no cometer errores.
  Superar el límite impide aprobar, pero el ejercicio continúa hasta el final.
- **Preparación:** de 1 a 30 segundos enteros antes del primer blanco, con cuenta regresiva visual y pitidos.
- **Tiempo por blanco:** exposición máxima. Un blanco de tiro sin impacto desaparece sin sumar puntos;
  ante un blanco protegido hay que esperar sin disparar. Acepta de 0.1 a 60.0 segundos.
- **Intervalo:** pausa entre blancos, de 0.0 a 30.0 segundos. 0 elimina la pausa programada.
  Disparar en ella reduce la precisión.

Tiempo por blanco e Intervalo admiten un decimal, con punto o coma, por ejemplo **0.5** o **0,5**.
Para probar tiempos cortos, usa exposición 0.5 e intervalo 0.2. La exposición no admite 0.
La preparación y los límites de puntos/errores siguen usando enteros.

Cada regla se guarda al confirmarla. Orden aleatorio cambia cuándo aparece cada punto,
conservando su objeto/comportamiento. Distribución aleatoria mueve **objeto y comportamiento juntos**
entre posiciones. Conserva cantidades y no sobrescribe el montaje. Una secuencia puede repetirse.
Las rotaciones pertenecen a las posiciones: usa orientaciones compatibles para distribución aleatoria.

Atajo de desarrollo: con RCON y estando vivo y a pie, usa **/irpruebas** para ir a la zona de pruebas:
XYZ 802.8580, 1674.5205, 5.2813, orientación 181.2324, interior y mundo 0.
El comando mueve al personaje. Los puntos se marcan desde Puntos de inicio > Añadir punto en /practicaconfig.

1. Acércate a cualquier punto de inicio, a menos de 1.5 metros.
2. En este servidor de desarrollo, obtén la Desert Eagle con **/creararma 24 100 0** y equípala.
   Para probar el cambio de arma, permite también M4 y obtén una con **/creararma 31 100 0**.
3. Usa **/practica**, o Iniciar práctica desde el menú. Si coinciden puntos de zonas distintas se abre un listado
   para elegir la zona. Dentro de la misma zona se toma el punto más cercano. También puedes usar **/practica <ID>**, estando junto al inicio de esa zona.
   **/practica ayuda** muestra sus reglas; **/practica cancelar** termina solo tu ronda.
4. Verás **Comienza en 3, 2, 1** en pantalla, con un pitido por segundo. El contador desaparece
   al aparecer el primer blanco. Oirás otro pitido en cada aparición. La cuenta regresiva usa el sonido
   1056 y las apariciones el 1057. Cajas: roja DISPARAR, azul NO DISPARAR. Las siluetas conservan sus texturas:
   el instructor debe explicar el criterio y elegir figuras distinguibles.
5. Permanece a menos de 3 metros del punto desde el que iniciaste. Ir a otra caseta cancela la ronda
   si te alejas de ese punto. Cada blanco aparece una sola vez.
6. Al terminar hay otra señal, una acción ambiental y un resumen privado de ocho líneas.
   El título y los separadores usan azul suave **#A8C4D9**. Los datos son blancos,
   salvo **Aprobado** en verde o **Reprobado** en rojo. «Veredicto:» y la puntuación siguen blancos.

Los pitidos de cuenta regresiva y la primera aparición quedan separados por al menos un segundo.
Si el servidor se retrasa, no recupera los pitidos en ráfaga ni acorta la última pausa.

Un impacto válido en DISPARAR suma **+1**. Cualquier disparo durante NO DISPARAR, incluso al aire,
resta **1** y registra un error. El primero retira el blanco. Dejarlo vencer sin disparar es correcto.
Los disparos del intervalo cuentan para precisión sin volver a penalizar el blanco anterior.
Repetir disparos ante un mismo blanco de tiro también reduce la precisión. Para un ejercicio limpio,
procura un tiro por blanco de tiro y ninguno ante los protegidos. Disparar de más no cambia por sí
solo el veredicto, que sigue dependiendo de los puntos netos y los errores de identificación.
Puntos netos = impactos menos errores, incluso negativos. Se exige alcanzar el mínimo de puntos
y no superar los errores. Los disparos de preparación quedan excluidos.

El resultado agrupa dos campos por línea, sin sangría ni prefijos repetidos:

```text
----------------------------------------------------------------------------------------------
CAMPO DE TIRO
Puntos mínimos: 3 | Errores máximos: 1
Aciertos: 4/5 | Sin impactar a tiempo: 1
Protegidos respetados: 2/3 | Errores de identificación: 1
Disparos: 6 | Precisión: 66.7%
Puntuación neta: 3 | Veredicto: Aprobado
----------------------------------------------------------------------------------------------
```

Sin disparos, la precisión es **no disponible**. Las interrupciones muestran el mismo título
y delimitadores, con una línea blanca de **Incompleta** y el motivo. Los separadores de apertura y cierre son iguales.

/practica cancelar interrumpe tu ronda. /practica ayuda muestra armas, reglas y puntuación.
Si sales del radio de 3 metros, verás **Incompleta: Cancelada automáticamente por salir del puesto de tiro.**
La práctica se cancela y quienes están cerca del puesto ven
**Nombre Apellido abandona el puesto de tiro**, con tu nombre de rol.
Cualquier jugador puede practicar. Configurar requiere RCON. No se entrega ni repone equipo.
Sonidos y acciones de rol se limitan a 10 metros del puesto en su mismo interior/mundo.

## Guardado y comprobaciones

- Archivo local: .local-server/scriptfiles/shooting_practice.cfg. En otro servidor:
  scriptfiles/shooting_practice.cfg, relativo a su directorio de ejecución.
- Formatos 1 a 4 conservan posiciones y reglas, y su arma se mantiene como única permitida.
  Los formatos 1 a 3 usan caja 1271 con rotaciones X/Y cero. El 4 conserva modelos y rotaciones completos.
  El formato 5 conserva las múltiples armas. Los formatos 1 a 5 convierten exposición e intervalo
  de segundos a milisegundos al cargar. El formato 6 conserva las décimas.
  Todos migran a zona #1 pública. Si están incompletos, se conservan inactivos.
  El siguiente guardado usa formato 9 con todas las zonas, puntos, nombres, estados, IDs e indicadores.
  Los formatos 7 y 8 conservan sus zonas y puestos. Los formatos 1-8 cargan los indicadores desactivados. Cargar no reescribe el archivo.
- El guardado valida primero un archivo .tmp, conserva una copia válida .bak y comprueba el archivo final.
  Si el principal está dañado, la carga puede recuperar el respaldo y muestra **Reintentar guardado**
  para reparar el principal. No hay un renombrado atómico nativo: conserva respaldos externos.
  Si ningún archivo es válido, se bloquean la creación y las modificaciones para no sobrescribir los datos.
  Corrige/restaura los archivos con el servidor detenido antes de volver a cargar.
- Si falla la escritura, **Reintentar guardado** permite guardar los cambios que siguen en memoria.
  Está disponible en el listado incluso después de archivar. Reiniciar con cambios pendientes puede perderlos.
- Para que aparezca Iniciar práctica debe existir un blanco de tiro y los puntos y errores deben respetar
  sus cantidades. También debes estar vivo, a pie, cerca del inicio y con un arma permitida.
  El puesto debe estar libre y nadie puede estar editándolo. /practica explica qué falta si intentas iniciar.
  Durante una ronda no se puede cambiar esa zona. Cambiar un inicio o borrar blancos puede dejarla
  incompleta: se desactiva automáticamente; corrige sus reglas/distancias y vuelve a activarla.
- Comprueba en el cliente: catálogo, mover/girar/confirmar/cancelar, checkpoint, etiquetas,
  colisiones de siluetas, tildes, posición del contador y sonidos. Dispara a cada modelo en la vista previa
  y comprueba que se repone sin desaparecer el resto. El motor puede mostrar brevemente su rotura.
  La reparación se aplica a impactos de bala, no garantiza protección ante golpes o explosiones.
  Comprueba título y separadores azul suave, datos blancos y solo Aprobado/Reprobado en verde/rojo.
  Confirma el símbolo % en la precisión. Prueba dos blancos de tiro: falla un intento, acierta,
  dispara durante el intervalo y acierta el segundo. Debe mostrar 2 aciertos, 4 disparos y 50.0%.
  Verifica el espacio de un segundo entre pitidos y primera aparición.
  Prueba exposición 0.5 e intervalo 0.2, guarda y revisa esos valores después de un reinicio solicitado. Con dos jugadores confirma la privacidad del preview y que
  ambos pueden observar el blanco de una ronda real y el aviso de abandono. Solo el participante ve
  el contador y sus resultados. Reinicia y revisa nombres/modelos/rotaciones y armas seleccionadas.
- **1.291 comprobaciones automáticas sin fallos**, ejecutadas en SA-MP 0.3.DL-R1 Windows con sscanf 2.13.1,
  y compilación de pruebas y gamemode completos. Incluyen filtros, páginas, cierre, limpieza,
  guardado de modelos nuevos y rutas de puntuación. Los controles, renderizado, colisiones, audio
  y el despacho real de callbacks del cliente siguen siendo comprobaciones manuales.
  El MVP se aplicó al servidor local tras la solicitud de arranque del 11 de octubre.
  La preparación posterior de integración está compilada, con **54 comprobaciones SQL** en una BD
  temporal independiente. Su modelo describe la conexión futura. No activa un backend SQL ni historial.

## Comprobar varias zonas

1. Crea una segunda zona con otro nombre e inicio, y coloca sus blancos. Activa ambas.
2. Con dos jugadores, inicia una práctica en cada una. Cada participante debe tener su contador,
   secuencia y resumen; acertar, cancelar, morir o desconectarse en una no debe terminar la otra.
3. Con RCON, intenta modificar la zona ocupada: debe bloquearse. Configura la otra cuando quede libre.
   Sus reglas, blancos y vistas no deben cambiar en la primera.
4. Si dos inicios coinciden, usa /practica sin RCON y elige por ID. Debe mantener las comprobaciones de
   distancia, mundo, interior, arma y ocupación. Una misma persona no puede iniciar dos rondas.
5. Archiva una zona libre y comprueba que desaparece del listado y no admite /practica <ID>.
   El ID se conserva en el archivo y la siguiente creación recibe un ID nuevo.
6. Tras un reinicio solicitado, revisa ambos montajes, nombres, estados e IDs. El campo legado
   debe conservarse como #1. Prueba el acceso directo a la configuración por ID.

Alcance actual: múltiples zonas públicas y administración RCON, sin BD, historial, facciones/rangos,
certificaciones ni assets propios. Las evaluaciones y la supervisión están descritas en el modelo SQL;
/evaluaciontiro aún no está habilitado.

## Prueba de varias casetas

1. Conserva los blancos actuales y añade un segundo punto, por ejemplo a 6 metros del primero.
2. Revisa ambos puestos desde Vista previa. Los blancos deben servir desde los dos.
3. Inicia con /practica desde el segundo punto. La cuenta atrás y la distancia de abandono se anclan ahí.
4. Con otro jugador, intenta iniciar desde el primer punto. Debe avisar que hay una práctica activa.
   Ambos jugadores deben poder ver el blanco público del ejercicio en el mismo interior y mundo.
5. Cancela o termina. El segundo jugador ya puede iniciar desde cualquier punto.
6. Solicita un reinicio cuando quieras aplicarlo y comprueba que todos los puntos conservan posición,
   orientación y nombre. El archivo anterior migra su único inicio a Punto 1 sin borrarlo.
