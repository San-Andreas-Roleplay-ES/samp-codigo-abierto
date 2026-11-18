# Guía de uso del SCBA de LSFD

El SCBA es el equipo de respiración autónoma. Llevas el cilindro en la espalda y conectas la máscara cuando quieres empezar a usar su aire. La autonomía habitual de un cilindro lleno es de **20 minutos de uso**. Puedes consultar el tiempo restante en el manómetro.

## Equiparse y empezar a respirar

1. Ve a pie a un punto de equipamiento o a un vehículo habilitado y usa `/scba equipar`. Aparece el manómetro y marca **DESCONECTADO**. Puedes desplazarte con el equipo a la espalda y conservar el aire.
2. Usa `/scba mascara` para colocarte la máscara y conectar el regulador. El cilindro empieza a consumirse, tanto dentro de los edificios como al aire libre.
3. Para hacer una pausa, vuelve a usar `/scba mascara`. La lectura se mantiene en el aire que te queda. Puedes volver a conectar la máscara mientras quede aire en el cilindro.

## Leer el manómetro

La aguja y el porcentaje indican cuánto aire queda. **TIEMPO** muestra los minutos y segundos de uso restantes. La franja inferior indica el estado del equipo.

![De izquierda a derecha se ven los estados desconectado, en servicio, reserva, aire muy bajo, sin aire y recargando.](media/estados-manometro.png)

| Estado | Qué significa |
| --- | --- |
| **DESCONECTADO** | Llevas el equipo con la máscara retirada. El aire se conserva. |
| **EN SERVICIO** | La máscara está conectada y queda más del 25 % de aire. |
| **RESERVA** | Queda entre el 11 % y el 25 % en el indicador. Suena la alarma de reserva. |
| **AIRE MUY BAJO** | Queda entre el 1 % y el 10 % en el indicador. La alarma se hace más insistente. |
| **SIN AIRE** | El cilindro se agotó con la máscara conectada. En un interior puedes empezar a perder vida. |
| **RECARGANDO** | Estás recargando o cambiando el cilindro. El indicador conserva la lectura anterior hasta que termina. |

Las alarmas y los jadeos se oyen hasta unos 10 metros en el mismo entorno. El aviso en el chat describe lo que se escucha sin identificar a quien lleva el equipo.

## Recargar o cambiar el cilindro

Retírate la máscara y usa `/scba recargar` a pie junto a una fuente habilitada. **En un puesto fijo** se oye abrir la válvula al empezar la carga. **En un camión detenido** se oye el acople al cambiar el cilindro. Algunos vehículos permiten solo recargar.

La operación dura **8 segundos** desde que comienza la animación. Quédate junto al puesto o al camión. Puedes usar `/scba cancelar` para interrumpirla, incluso mientras el personaje está agachado junto al vehículo. Si te alejas del puesto o el camión se mueve, el cilindro conserva el aire que tenía.

Al terminar verás **100 %** y **DESCONECTADO**. Usa `/scba mascara` cuando quieras volver a respirar del cilindro.

## Si se acaba el aire

- **En un interior**, con la máscara conectada y el cilindro vacío, pierdes **5 puntos de vida cada 2 segundos**. El primer descuento llega al cumplir dos segundos en esas condiciones. Recibes un aviso privado y quienes estén cerca pueden oír tus jadeos.
- Los jadeos se vuelven más frecuentes conforme baja tu vida. La primera pérdida mantiene la cámara estable. Cuando te quedan 60 puntos de vida o menos puede aparecer un mareo suave. A 30 o menos se intensifica.
- **Salir al exterior o retirarte la máscara** detiene el daño, los jadeos y el mareo del equipo. Si entras de nuevo con el cilindro vacío y la máscara conectada, empieza un nuevo intervalo de dos segundos. Para volver a ponerte una máscara retirada con el cilindro vacío, primero debes recargar.

## Guardar el equipo

Usa `/scba guardar` a pie donde esté permitido equiparse. Puedes hacerlo con la máscara puesta o retirada. Al guardarlo o al morir desaparecen el equipo y el manómetro. Si vuelves a equiparte durante la misma conexión, recuperas el aire que quedaba. Al volver a conectarte, empiezas sin el equipo y recibes un cilindro lleno la próxima vez que te equipes.

## Comandos

| Comando | Acción |
| --- | --- |
| `/scba` | Ver los comandos disponibles. |
| `/scba equipar` | Recoger el equipo en un lugar habilitado. |
| `/scba mascara` | Ponerte o retirarte la máscara. |
| `/scba recargar` | Recargar en un puesto o cambiar el cilindro junto a un vehículo habilitado. |
| `/scba cancelar` | Interrumpir la recarga o el cambio de cilindro. |
| `/scba guardar` | Devolver el equipo. |
