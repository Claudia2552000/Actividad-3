# Actividad 3
Entrega actividad 3 de equipos e instrumentación electrónica. Grupo: Claudia Betancor Soca, Leticia Esther Santana Robaina y María Almudena Jiménez Suárez

## 1. Introducción

Este proyecto implementa un sistema completo de medición, control, actuación y presentación del clima utilizando un microcontrolador Arduino.  

El sistema simula una boya climática avanzada, con capacidades de control remoto, autodiagnóstico y supervisión inteligente. Todos los componentes se integran en un entorno simulado con Wokwi.

## 2. Objetivos del sistema

- Medir condiciones ambientales (temperatura, humedad, luz, viento, calidad del aire).
- Controlar actuadores de forma automática (calefacción, ventilación, iluminación).
- Ajustar parámetros por control remoto (mando IR).
- Mostrar información en una interfaz HMI (pantalla LCD y monitor serie).
- Realizar autodiagnóstico del sensor de temperatura.
- Implementar comportamiento adaptativo (modo noche).

## 3. Componentes lógicos y físicos

### 3.1 Sensores y entradas

| Dispositivo        | Función                                    |
|--------------------|-------------------------------------------|
| DHT22 (x2)         | Medición de temperatura y humedad ambiente|
| LDR (A0)           | Medición de iluminación                    |
| Potenciómetro (A1) | Simulación de velocidad del viento (0-200 km/h) |
| Potenciómetro (A2) | Simulación de calidad del aire (0-100%)  |
| Botón (pin 4) | Activación del modo de visualización |
| Receptor IR (pin 12) | Recepción de comandos del mando a distancia |



### 3.2 Actuadores

| Actuador          | Simula                                    |
|-------------------|-------------------------------------------|
| LED FRIO (pin 8)  | Encendido de calefacción                  |
| LED CALOR (pin 9) | Encendido de ventilador                   |
| LED LUZ (pin 10)  | Iluminación artificial activada           |
| LEDs RGB (5-7)    | Estado de la calidad del aire             |
| LCD I2C           | Interfaz HMI para mostrar datos           |

## 4. Lógica de funcionamiento del sistema

### 4.1 Lectura de datos

- Se lee la temperatura y humedad del sensor DHT22 principal (dht1).
- Si este falla (retorna NaN), se activa el sensor secundario (dht2).
- Se mide la iluminación mediante el sensor LDR, convirtiendo el valor analógico a lux.
- La calidad del aire se determina con un potenciómetro y se clasifica en buena, regular o mala.
- El viento se simula con otro potenciómetro, mapeado a 0-200 km/h.

### 4.2 Autodiagnóstico

- Si dht1 devuelve un valor inválido, el sistema activa dht2 como sensor de respaldo.
- Se notifica en LCD cuando el sensor principal ha fallado.

### 4.3 Setpoint y control de temperatura

- El **setpointTemp** es el valor de referencia deseado para la temperatura, ajustable con el mando IR (+ y -).
- Se aplica una zona muerta de ±5 °C:
  - Si `temp < setpoint - 5` → enciende calefacción (LED rojo).
  - Si `temp > setpoint + 5` → enciende ventilación (LED naranja).
  - Dentro de la zona muerta, los sistemas están apagados.

### 4.4 Control de iluminación y modo noche

El modo noche es un mecanismo adaptativo que ajusta el sistema ante condiciones de baja iluminación para optimizar confort y ahorro energético.

#### Activación automática

- Se activa automáticamente cuando la iluminación cae por debajo de 100 lux.
- Se desactiva automáticamente cuando la iluminación supera los 300 lux (zona de histéresis para evitar cambios frecuentes).
- En modo noche automático, se reduce el setpoint de temperatura en 2 °C.
- El LED de luz artificial se enciende si la iluminación está por debajo de 300 lux.

#### Activación manual

- El usuario puede alternar el modo noche con el mando IR (comando 104).
- Al activarlo manualmente, el modo noche se activa y el LED de luz artificial se enciende inmediatamente.
- Al desactivarlo, modo noche y LED de luz se apagan, incluso si la iluminación sigue baja.
- Esta función anula temporalmente la lógica automática, permitiendo control directo.

#### Setpoint de temperatura

- Mientras el modo noche esté activo (automático o manual), el setpoint efectivo se reduce en 2 °C.
- Ejemplo: setpoint ajustado 24 °C → modo noche 22 °C.

#### Consideraciones sobre el LED de luz

- En modo noche automático, el LED depende del nivel de iluminación (< 300 lux).
- En modo noche manual, el LED se controla directamente por el usuario.
- Al desactivar modo noche manual, si la iluminación sigue baja, el LED puede encenderse por lógica automática, generando superposición.

### 4.5 Control remoto (IR)

| Botón | Comando IR | Acción                               |
|-------|------------|------------------------------------|
| +     | 2          | Aumenta `setpointTemp` en 1 °C     |
| -     | 152        | Disminuye `setpointTemp` en 1 °C   |
| POWER | 162        | Activa/desactiva todo el sistema    |
| PLAY  | 168        | Muestra datos completos en LCD      |
| 0     | 104        | Alterna modo noche manual y luz     |
| 1     | 48         | Enciende LED de luz manual por 5 s |
| 2     | 24         | Muestra temperatura y humedad actual|
| C     | 176        | Resetea valores y modos             |

### 4.6 Visualización HMI

- LCD 16x2 que muestra mensajes breves:
  - Temperatura y humedad
  - Luz y viento
  - Calidad del aire
  - Estados de actuadores
- Consola serial que imprime cada 3 segundos:
  - Temperatura actual
  - Setpoint
  - Estado del modo noche

## 5. Comportamiento esperado

**Escenario 1: Calefacción**  
- Temperatura: 10 °C  
- Setpoint: 20 °C  
- Resultado: LED FRIO encendido, mensaje "Calefacción encendida"

**Escenario 2: Ventilación**  
- Temperatura: 30 °C  
- Setpoint: 20 °C  
- Resultado: LED CALOR encendido, mensaje "Ventilador encendido"

**Escenario 3: Luz baja**  
- Luz: 50 lux  
- Resultado: modo noche activado automáticamente, setpoint ajustado, luz artificial encendida

**Escenario 4: Modo noche manual activado**  
- Pulsar botón IR (comando 104, boton 0)  
- Resultado: modo noche activado manualmente, luz artificial encendida forzada

**Escenario 5: Modo noche manual desactivado**  
- Pulsar botón IR (comando 104, boton 0)  
- Resultado: modo noche manual desactivado, luz artificial apagada

## 6. Conclusión

Este sistema es una simulación funcional y ampliada de una boya climática inteligente. Permite ajustar, visualizar y supervisar variables críticas del entorno, con actuación automática basada en condiciones reales. El control remoto mediante IR, la gestión dual del modo noche (manual y automático) y el autodiagnóstico por sensores redundantes son ejemplos de aplicación de técnicas modernas de instrumentación y control.
