# PR: sistema de prácticas de tiro (MVP)

Actualizado: 2026-10-12. Documento de entrega para revisión del primer MVP y de la preparación de integración posterior.

**Título sugerido:** Añadir sistema configurable de prácticas de tiro (MVP)

**Origen:** `SouthViking/samp-codigo-abierto:feat/shooting-practice-mvp`

**Destino:** `San-Andreas-Roleplay-ES/samp-codigo-abierto:main`

## Resumen

Añade un sistema de prácticas de tiro configurable dentro del juego. Permite preparar campos con varias
casetas, colocar y orientar blancos, definir cuándo disparar o abstenerse y ajustar las reglas del ejercicio.
El participante recibe cuenta regresiva, señales sonoras, acciones de rol y un resumen privado.

[Ver la demostración del MVP en YouTube](https://www.youtube.com/watch?v=cpVQbs0igC8).

El video acompaña el MVP jugable. La preparación posterior de funciones de integración se verifica con
compilación y pruebas separadas. El modelo SQL describe una integración futura.

## Funcionalidades incluidas

- Hasta **64 zonas independientes** por defecto, cada una con nombre e ID estable, activación,
  desactivación, reinicio y archivo. Los registros archivados cuentan para el límite y no reutilizan IDs.
- Hasta **10 puestos y 10 blancos por zona**. Una sola práctica activa por zona, compartida entre sus puestos.
  Zonas distintas pueden funcionar a la vez. Un jugador participa en una única ronda.
- Configuración mediante diálogos, con cierre temporal para caminar y continuar después.
  La autoridad administrativa actual es RCON. Las zonas están abiertas a los jugadores.
- Editor visual nativo para posición y orientación. Blancos con nombre, objeto y comportamiento
  independientes, duplicación, eliminación, guardado explícito y vista previa privada.
- Catálogo de **9 objetos** con tarjetas, filtros y páginas: siluetas armadas/civiles, caja,
  botellas, lata y cono. Sin nuevos modelos descargables.
- Una o varias armas permitidas, puntos mínimos, errores máximos y tiempos con ayuda contextual.
  Exposición e intervalo admiten décimas. Orden y distribución pueden ser fijos o aleatorios.
- Indicadores flotantes opcionales en todos los puestos de una zona, **desactivados por defecto**.
  El aviso de uso aparece en el chat al acercarse, con independencia del indicador.
- Cuenta regresiva visual y auditiva, señal por aparición y al finalizar, acciones ambientales
  y aviso de abandono del puesto.
- Resultado privado con aciertos, blancos sin impactar, protegidos respetados, errores,
  disparos, precisión porcentual, puntuación neta y veredicto.
- Persistencia de la configuración completa en **archivo v9**, migración de formatos 1-8,
  validación, respaldo recuperable y reintento de guardado.
- Modelo SQL de **16 tablas** y funciones de integración llamadas desde el flujo real de
  autorización, carga/guardado, inicio, cierre y apagado.

## Comandos

| Comando | Uso actual |
| --- | --- |
| `/practicaconfig` | Listado y administración de zonas. Requiere RCON. |
| `/practicaconfig <ID>` | Configuración directa de una zona existente. Requiere RCON. |
| `/practica` | Iniciar desde un puesto cercano. Si coinciden zonas, elegir una. |
| `/practica <ID>` | Iniciar en esa zona, junto a uno de sus puestos. |
| `/practica ayuda` | Consultar las reglas del ejercicio cercano. |
| `/practica cancelar` | Interrumpir solo la ronda propia. |
| `/irpruebas` | Atajo RCON de desarrollo a la ubicación de pruebas, estando vivo y a pie. |

## Reglas de uso y puntuación

Se inicia vivo, a pie, con un arma permitida y a un máximo de **1.5 m** del puesto. La zona debe estar
activa, configurada y libre de participante/editor. Alejarse más de **3 m** del origen seleccionado
cancela la ronda. Los puestos comparten interior/mundo y geometría válida. Los blancos quedan a
**3-100 m de cada puesto**.

Un impacto válido en un blanco de tiro suma **1 punto**. Un disparo durante la exposición de un
protegido resta **1 punto** y cuenta como error, incluso al aire. No disparar ante un protegido es correcto.
La puntuación neta es aciertos menos errores. Aprobar exige alcanzar los puntos mínimos y respetar
el máximo de errores. Los tiros fallados, repetidos y del intervalo reducen la precisión, sin añadir
por sí solos otro error de identificación. Los tiros de preparación no se cuentan.

Los blancos del ejercicio son públicos en su interior/mundo. El contador y el resultado son del
participante. El equipo del jugador no se entrega ni repone automáticamente.

## Persistencia e integración

| Área | Estado entregado |
| --- | --- |
| Zonas, puestos, blancos, reglas e indicadores | Guardado real en archivo v9. |
| Rondas, resultados y borradores de edición | Datos temporales. Sin historial persistente. |
| Administración | Adaptador RCON operativo. Preparado para reemplazarlo por permisos del host. |
| Acceso a prácticas | Zonas públicas. El acceso por grupos/facciones está diseñado, pendiente de integrar. |
| Identidad persistente | Puerto explícito no disponible. Sin usar nickname o `playerid` como identidad de BD. |
| SQL | DDL de referencia probado en una BD temporal. No es una migración productiva ni backend activo. |
| Evaluaciones y supervisión | Modelo/contratos futuros. `/evaluaciontiro` no está habilitado. |

El modelo incluye zonas versionadas, puestos y blancos, armas, grupos/rangos, evaluaciones y
supervisión. Una evaluación futura referencia el puesto de la misma versión y conserva el contexto
histórico. El host deberá aportar identidad, pertenencia, permisos y conexión compartida.
Las operaciones asíncronas deberán confirmar el guardado efectivo e implementar reintentos.
Las funciones placeholder rechazan la evaluación no disponible. No simulan un registro exitoso.

## TODOs de funcionalidades futuras

- `SP-ACCESS-UI` y `SP-ACCESS`: permitir al staff seleccionar acceso libre o restringido
  y asignar uno o varios grupos/facciones a cada campo mediante el panel.
- `SP-SUPERVISION`: permitir asignar supervisores individuales o reglas por facción/rango,
  con consulta de evaluaciones y permiso opcional para añadir observaciones.
  Estos permisos serán independientes del acceso para practicar y de la configuración.

Son ampliaciones propuestas para una etapa posterior, pendientes de integrar con los servicios del host.

## Documentación

| Documento | Contenido |
| --- | --- |
| [Sistema y arquitectura](shooting-practice-mvp.md) | Módulos, editor, sesiones, puntuación, audio, persistencia y límites. |
| [Configuración y pruebas](shooting-practice-testing.md) | Guía desde cero, menús, ejemplos, comandos y casos manuales. |
| [Modelo de datos](shooting-practice-data-model.md) | Relaciones, versiones, integridad, permisos, historial y contratos de integración. |
| [DDL SQL](sql/shooting-practice.sql) | Propuesta MySQL 8.0.16+ de 16 tablas. |
| [Servidor Windows](local-windows-server.md) | Dependencias fijadas, instalación, compilación, arranque, RCON y conexión local. |
| [Pruebas automatizadas](../tests/README.md) | Cobertura, ejecución reproducible, resultados y límites de los adaptadores. |

## Validación realizada

- Gamemode completo y pruebas compilados con **Pawn 3.10.10**, usando `-;+`, `-(+`, `-d3` y `-E`.
- **1.291 comprobaciones Pawn, 0 fallos**, en un servidor aislado SA-MP 0.3.DL-R1 Windows con sscanf 2.13.1.
- **54 comprobaciones SQL aprobadas** sobre MySQL 8.0.34, en una instancia temporal independiente.
  Cubren límites, puestos múltiples, claves foráneas de versión/origen y conservación histórica.
- MVP jugable arrancado localmente el 11 de octubre a petición del autor. Cargó sus siete plugins,
  respondió en `127.0.0.1:7777` y conservó el hash de la configuración durante ese arranque.
  El autor realizó pruebas en el cliente y aceptó esta iteración.
- Tras consultar `upstream/main` el 12 de octubre, no hay commits nuevos del destino respecto
  a la base de esta rama. La entrega se prepara sobre esa base, con las funciones de integración
  y la documentación incluidas en la rama del fork.

Para reproducir las pruebas, después de preparar las dependencias locales:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/test-shooting-practice.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/test-shooting-practice-sql.ps1
```

Las comprobaciones SQL requieren los ejecutables de MySQL instalados. Se puede indicar su carpeta
con `-MySqlBin`. Los runners usan datos propios e ignorados por Git. No sustituyen la prueba manual
del renderizado, las colisiones, el audio y los callbacks reales. Los registros locales completos y
el comando de compilación están descritos en [tests/README.md](../tests/README.md).

## Cambios en el host y compatibilidad

El módulo se incluye desde `gamemodes/SARP.pwn`. Las guardas en el editor administrativo de objetos
evitan que otro comando sustituya el editor activo de un blanco. El archivo de teletransportes contiene
el atajo de desarrollo `/irpruebas`.

La firma compartida `ShowModelSelectionMenuEx` conserva sus llamadas anteriores y admite una etiqueta
opcional para volver. El selector actual del campo usa sus propios PlayerTextDraws.
El template `server.cfg` explicita `lagcompmode 1` para los callbacks de disparo.
Los scripts de instalación se encargan únicamente del entorno de desarrollo local.

Las fuentes Pawn y el README heredado mantienen **Windows-1252** para las tildes del cliente.
Los documentos del feature y SQL usan UTF-8. Esta actualización documental mantiene finales de línea CRLF.
Las credenciales, binarios, configuraciones de campos y archivos generados del entorno local no forman
parte de la entrega.

## Límites conocidos y próximos pasos

- El acceso por facción/grupo, los permisos reales del staff, SQL, historial y supervisión requieren
  los servicios del servidor principal. Son una etapa posterior al MVP.
- Una zona con varias casetas mantiene una sola ronda activa para compartir los blancos públicos.
  No hay rondas simultáneas independientes entre casetas de la misma zona.
- El guardado por archivo usa staging validado y respaldo. No dispone de renombrado atómico nativo.
- Los impactos reportados sobre objetos de vista previa se reparan. Puede verse brevemente la rotura
  nativa y no se garantiza protección frente a golpes o explosiones.
- La ejecución reproducible y el arranque documentados corresponden a Windows SA-MP 0.3.DL-R1.
  Otro entorno requiere sus plugins compatibles y una comprobación de arranque.
- La geometría de zonas distintas puede superponerse. La selección por ID resuelve el acceso,
  pero conviene separar físicamente los campos para evitar confusión visual entre sus blancos.

## Entrega para revisión

- [x] Alcance del MVP, guía, arquitectura, modelo y contratos documentados.
- [x] Video del autor referenciado.
- [x] Compilación y pruebas registradas.
- [x] Integraciones futuras y límites identificados.

La entrega reúne el MVP, la preparación de integración y la documentación en la rama del fork.
La PR se dirige a `main` del repositorio original y acompaña esta descripción y el video.

La revisión del equipo y la comprobación en su entorno forman parte del proceso de la PR.
La integración SQL/facciones no impide presentar el MVP con este alcance.
