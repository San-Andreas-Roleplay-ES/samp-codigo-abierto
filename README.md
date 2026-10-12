<div align="center">
  <a href="https://sarp.es/" target="_blank">
    <img style="width: 200px;" src="https://i.imgur.com/EFK15nk.png" alt="San Andreas Roleplay"/>
  </a>
</div>

![Discord Shield](https://discord.com/api/guilds/1118327104908570654/widget.png?style=shield)
[![License: MIT](https://img.shields.io/badge/License-MIT%20License-yellow.svg)](https://opensource.org/licenses/MIT) 

# Modo de juego de código abierto de SARP.es
Este repositorio representa la base de desarrollo de código abierto para la creación y mejora de scripts utilizados en SARP.es, el servidor de rol en Grand Theft Auto: San Andreas. Está pensado como un espacio colaborativo en el que cualquier miembro de la comunidad, sin importar su nivel de experiencia, puede participar activamente en el crecimiento técnico del proyecto.

Invitamos a todos los desarrolladores interesados a contribuir con ideas, sistemas o mejoras. Toda propuesta será revisada y sometida a pruebas por el equipo de desarrollo oficial. Aquellos sistemas que cumplan con los estándares de calidad, rendimiento y compatibilidad, serán integrados al script principal del servidor, permitiendo que sus autores vean su trabajo reflejado directamente en el juego.

El objetivo de este proyecto de código abierto es democratizar el conocimiento, fomentar el aprendizaje mutuo y brindar una oportunidad real a quienes siempre han soñado con aportar en el desarrollo de un proyecto serio y colaborativo dentro de su juego favorito de Grand Theft Auto - SAMP.

## Requerimientos
### SA-MP 0.3.DL-R1
- [SA-MP 0.3.DL-R1 Windows Server](https://sampforum.blast.hk/files/03DL/samp03DL_svr_R1_win32.zip)
- [SA-MP 0.3.DL-R1 Linux Server](https://sampforum.blast.hk/files/03DL/samp03DLsvr_R1.tar.gz)

### Pawn Community Compiler:
[Release 3.10.10](https://github.com/pawn-lang/compiler/releases/tag/v3.10.10)

### Instrucciones
1. Descargar Windows Server o Linux Server y pegar ejecutables en el repositorio.
2. Descomprimir el Pawn Compiler en la carpeta /pawno.
3. Compilar en tu editor favorito y ejecutar samp-server.exe.

## Servidor local en Windows
La preparación reproducible y los comandos de inicio, estado y apagado están en [.internal/local-windows-server.md](.internal/local-windows-server.md).
El entorno se instala en `.local-server/` (ignorado por Git) y permite conectar desde el mismo PC a `127.0.0.1:7777`.

## Prácticas de tiro (MVP)

Sistema configurable dentro del juego con hasta 64 zonas independientes por defecto, diez puestos y diez blancos por zona. `/practicaconfig` permite administrarlas mediante RCON. `/practica` inicia un ejercicio desde un puesto cercano, con una sola práctica activa por zona y blancos visibles para quienes estén en su interior y mundo.

Incluye editor visual, catálogo de nueve objetos, blancos de tiro y protegidos, varias armas permitidas, reglas y tiempos configurables, aleatoriedad, vista previa privada, indicadores de inicio opcionales, cuenta regresiva, sonidos, acciones de rol y resultados con precisión y veredicto. La configuración se guarda en archivo v9. Las sesiones y resultados son temporales.

- [Demostración del MVP en YouTube](https://www.youtube.com/watch?v=cpVQbs0igC8).
- [Resumen para revisar la PR](.internal/shooting-practice-pr.md).
- [Documentación técnica del sistema](.internal/shooting-practice-mvp.md).
- [Guía de configuración y pruebas](.internal/shooting-practice-testing.md).
- [Modelo de datos y contratos de integración](.internal/shooting-practice-data-model.md), con [DDL SQL propuesto](.internal/sql/shooting-practice.sql).
- [Pruebas automatizadas y evidencia](tests/README.md).

El backend SQL, los permisos del staff del servidor principal, el acceso por facción/grupo y el historial de evaluaciones corresponden a la integración futura. El modelo y las funciones de integración están preparados, pero esas funciones futuras no están habilitadas.

## Licencia
Este proyecto está licenciado bajo los términos de la [Licencia MIT](https://opensource.org/licenses/MIT). Eres libre de usar, modificar y distribuir este software con pocas restricciones.
