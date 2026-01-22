<div align="center">
<b>THIS BRANCH IS FROZEN NO FURTHER DEVELOPMENT HAPPENING OR PULL REQUESTS ACCEPTED</b>

[![Gaggiuino](/images/GAGGIUINO_LOGO_transp.png)](https://gaggiuino.github.io/#/)
  
[![Compile Sketch](https://github.com/Zer0-bit/gaggiuino/actions/workflows/compile-sketch.yml/badge.svg)](https://github.com/Zer0-bit/gaggiuino/actions/workflows/compile-sketch.yml)
[![Discord Chat](https://img.shields.io/discord/890339612441063494)](https://discord.gg/eJTDJA3xfh "Join Discord Help Chat")
</div>



## Intro
**Gaggiuino started as an idea to improve an already capable coffee machine while keeping the machine appearance and button functionality as close as possible to the original. An important part is that no internal cables/connectors were modified; all the connections were made by creating splitters using the purchased spade connectors.**
***
**For install instructions head to the project [documentation](https://gaggiuino.github.io/#/) section.**

*For project related help join us on [discord](https://discord.gg/eJTDJA3xfh).*

</div>

## Resumen Técnico del Proyecto

### Visión General
Gaggiuino es un sistema de control avanzado basado en microcontroladores STM32 (Blackpill) que transforma máquinas de café espresso Gaggia en sistemas inteligentes de alta precisión. El proyecto implementa control automático de presión, temperatura y flujo para optimizar la extracción de espresso mediante perfiles personalizables.

### Arquitectura del Sistema

#### Microcontroladores
- **STM32 Principal (Blackpill)**: Controlador principal ejecutando Arduino Framework
- **ESP32 (Opcional)**: Servidor web para monitoreo remoto y configuración WiFi
- **Comunicación**: I2C, UART, y comunicación bilateral STM-ESP

#### ¿Por qué STM32 en lugar de ESP32 para el Control Principal?

El proyecto utiliza una **arquitectura de doble MCU** con roles claramente diferenciados. Esta decisión no es arbitraria, sino fundamentada en requisitos técnicos de control en tiempo real:

**Razones Técnicas para STM32 como Controlador Principal:**

1. **Determinismo en Tiempo Real**
   - **STM32**: Bucle principal sin interferencias, latencia de interrupción < 1µs
   - **ESP32**: Stack WiFi/Bluetooth ejecutándose en segundo plano causa jitter impredecible
   - **Crítico para**: Detección de cruce por cero (ZC) a 50/60Hz requiere timing de microsegundos
   ```cpp
   // Control de bomba requiere respuesta inmediata en cada ciclo AC
   attachInterrupt(digitalPinToInterrupt(zcPin), zeroCrossISR, FALLING);
   // Cualquier retraso > 100µs causa disparo incorrecto del TRIAC
   ```

2. **FPU Hardware Dedicada (sin sobrecarga)**
   - **STM32F411**: FPU Cortex-M4 siempre disponible, instrucciones de punto flotante en 1 ciclo
   - **ESP32**: FPU disponible pero compartida con tareas de WiFi/BT, puede bloquearse
   - **Uso intensivo**: Filtros Kalman, cálculos PID, interpolación de curvas ejecutados cada 10-50ms
   ```cpp
   // Ejecutados continuamente sin degradación de performance
   SimpleKalmanFilter smoothPressure(0.6f, 0.6f, 0.1f);
   float getPumpFlow(const float cps, const float pressure); // Polinomios de 7mo orden
   ```

3. **Timers Hardware Dedicados**
   - **STM32**: Hasta 11 timers independientes, configuración flexible
   - **ESP32**: Timers compartidos con stack de red, pueden causar conflictos
   - **Necesario para**: PWM de bomba, watchdog, scheduling de sensores, medición de pulsos

4. **Consumo de Recursos Predecible**
   - **STM32**: ~30-40% CPU en operación normal, picos controlados
   - **ESP32**: Baseline 60-70% CPU solo con WiFi activo, picos >95% durante scanning/conexión
   - **Consecuencia**: ESP32 podría perder eventos críticos durante reconexión WiFi

5. **Manejo de Interrupciones Críticas**
   - **STM32**: Sin competencia por prioridades, interrupciones anidadas bien definidas
   - **ESP32**: Interrupciones WiFi/BT tienen alta prioridad, pueden retrasar aplicación
   - **Ejemplo**: Conteo de clicks de bomba (50-60 clicks/seg) no puede perder ni una muestra

6. **Confiabilidad y Reseteo**
   - **STM32**: Watchdog hardware independiente (IWDG), reset limpio
   - **ESP32**: Watchdogs de tareas FreeRTOS pueden fallar si WiFi se bloquea
   - **Seguridad**: Máquina manejando 9+ bar de presión y 130°C no puede quedarse en estado indefinido

7. **Sin Overhead de Red**
   - **STM32**: Sin stack TCP/IP, DNS, TLS, etc. Código más simple y auditable
   - **ESP32**: Miles de líneas de código de red ejecutándose incluso sin WiFi configurado
   - **Tamaño**: Firmware STM32 ~200KB, firmware ESP32 equivalente sería >1MB

**División de Responsabilidades:**

| Función | MCU | Razón |
|---------|-----|-------|
| Control de bomba (PWM + ZC) | **STM32** | Timing crítico de microsegundos |
| Lectura presión (ADS1115 I2C) | **STM32** | 100 muestras/seg con filtrado Kalman |
| Control térmico PID | **STM32** | Cálculos FPU cada 70-250ms |
| Interfaz LCD Nextion | **STM32** | UART dedicado sin interrupciones |
| Procesamiento de perfiles | **STM32** | Decisiones en tiempo real sin latencia |
| Watchdog de seguridad | **STM32** | Independiente de cualquier software |
| **Servidor Web** | **ESP32** | No crítico, puede tener latencia |
| **WebSocket streaming** | **ESP32** | Datos para visualización solamente |
| **Balanzas Bluetooth** | **ESP32** | Conveniencia, no esencial |
| **API REST** | **ESP32** | Configuración remota opcional |

**Comunicación STM32 ↔ ESP32:**
- **Protocolo**: SerialTransfer (UART bidireccional a 115200 baud)
- **Datos STM→ESP**: Snapshots de sensores cada 50-100ms (solo lectura)
- **Datos ESP→STM**: Comandos de configuración, datos de balanzas BLE
- **Diseño**: STM32 funciona completamente autónomo aunque ESP32 falle o no esté presente

**Comparativa de Latencias Típicas:**

```
Evento                          STM32         ESP32 (con WiFi)
─────────────────────────────────────────────────────────────
Interrupción ZC → Disparo PWM   2-5 µs        50-200 µs ❌
Lectura I2C completa            ~800 µs       1-5 ms
Cálculo Kalman + PID           50-100 µs      100-500 µs
Actualización LCD              2-3 ms         5-15 ms
Detección botón → Respuesta    <1 ms          1-10 ms
```

**¿Cuándo sería viable ESP32 solo?**

Posible **SOLO si** se elimina control de bomba AC (usar bomba DC con PWM simple) y se aceptan:
- Latencias variables (50-500ms)
- Posibles glitches durante actividad WiFi
- Pérdida de precisión en control de flujo
- Mayor complejidad de código (particionado de tareas FreeRTOS)

**Conclusión**: La arquitectura dual-MCU permite lo mejor de ambos mundos - **control determinista STM32** + **conectividad moderna ESP32** - por un costo marginal de ~$4 USD adicionales y comunicación UART sencilla.

#### ¿Es Necesario el ESP32 si Solo Uso LCD Touch?

**Respuesta corta: NO, el ESP32 es completamente opcional.**

El STM32 opera de manera **totalmente autónoma** con solo la pantalla LCD Nextion/TJC. El ESP32 únicamente proporciona funcionalidades **adicionales de conveniencia**, no esenciales para el funcionamiento básico.

**Funcionalidades que proporciona el ESP32:**

1. **Servidor Web + WiFi** (opcional)
   - Dashboard en tiempo real desde celular/tablet/PC
   - Gráficas de presión/flujo/temperatura durante extracción
   - Configuración remota de parámetros
   - **Alternativa**: Todo configurable desde LCD touch

2. **Balanzas Bluetooth BLE** (opcional)
   - Soporte para balanzas comerciales: Acaia Lunar/Pyxis, Felicita, etc.
   - **Alternativa**: Balanzas con HX711 conectadas directamente al STM32 (método estándar)

3. **Logging Remoto** (conveniencia)
   - Ver logs del sistema vía WebSocket
   - **Alternativa**: Logs por USB-Serial desde STM32

**¿Qué funciona SIN el ESP32?**
✅ Todo el control de extracción (perfiles multi-fase)
✅ Control de bomba con PWM y detección ZC
✅ Control PID de temperatura
✅ Balanzas HX711 cableadas (lectura de peso)
✅ Sensores de presión y temperatura
✅ Todos los modos: brew, steam, flush, descale
✅ Interfaz completa en LCD Nextion/TJC
✅ Almacenamiento de 5 perfiles en EEPROM
✅ Watchdog y seguridad

**Configuraciones Posibles:**

| Configuración | Componentes | Funcionalidad | Costo Aprox. |
|---------------|-------------|---------------|--------------|
| **Mínima** | STM32 + LCD Nextion + Sensores | 100% funcional, sin conectividad | ~$50-70 |
| **Estándar** | + Balanzas HX711 | Control por peso preciso | +$10-15 |
| **Completa** | + ESP32 | Web dashboard + BLE scales | +$4-8 |

**Modificaciones de Código para Omitir ESP32:**

Si decides no usar ESP32, el código STM32 funcionará sin cambios, pero puedes optimizar:

```cpp
// En src/gaggiuino.ino - Comentar inicialización ESP
void setup(void) {
  // ... resto del código ...
  
  // espCommsInit();  // ← Comentar esta línea
  
  // ... continuar con el resto ...
}

// En loop() - Comentar lectura de datos ESP
void loop(void) {
  // ... código existente ...
  
  // espCommsReadData();  // ← Comentar esta línea
  
  // ... resto del loop ...
}
```

El firmware compilará sin errores y simplemente ignorará la comunicación UART hacia ESP32.

**Ventajas de Omitir ESP32:**

- ✅ **Ahorro de costos**: ~$4-8 USD
- ✅ **Simplificación**: Menos cables, menos puntos de falla
- ✅ **Menor consumo**: ~150mA menos
- ✅ **No requiere configuración WiFi**: Setup más rápido
- ✅ **Sin dependencias de red**: Sistema 100% offline

**Desventajas de Omitir ESP32:**

- ❌ No dashboard web (solo LCD local)
- ❌ No balanzas Bluetooth comerciales (solo HX711 cableadas)
- ❌ No visualización remota de shots
- ❌ No configuración desde celular

**Recomendación para Tu Caso:**

Dado que mencionas usar **solo LCD touch sin WiFi/Bluetooth**, el ESP32 es **completamente innecesario**. Puedes:

1. **Opción 1 - Build Mínimo**: No incluir ESP32 en tu hardware
   - Elimina componente del BOM
   - No flashear firmware ESP32
   - Comentar llamadas `espComms*()` en código STM32
   
2. **Opción 2 - Dejar Header Preparado**: Incluir conector UART en PCB
   - Por si en el futuro decides agregar ESP32
   - Costo: ~$0.20 en conectores
   - Flexibilidad futura sin rediseño de PCB

**Balanzas sin ESP32:**

Si usas balanzas con HX711 (método estándar del proyecto):
```cpp
// Conexión directa STM32 ↔ HX711
#define HX711_sck_1   PB0
#define HX711_dout_1  PB8
#define HX711_dout_2  PB9  // Segundo HX711 opcional (dual scale)
```

Las balanzas funcionarán **idénticamente** sin ESP32. Solo las balanzas BLE comerciales (Acaia, Felicita) requieren el ESP32 para Bluetooth.

**Conclusión para Tu Proyecto:**

Si tu objetivo es adaptar Gaggiuino a otra máquina espresso con:
- Control local únicamente (LCD touch)
- Sin necesidad de dashboard web
- Balanzas HX711 tradicionales

Entonces puedes **omitir completamente el ESP32** y tendrás un sistema:
- Más simple
- Más barato  
- Igualmente funcional
- Con todo el control de perfiles y precisión

El ESP32 es un "nice-to-have", no un "must-have". 🎯

### Compilando una Versión sin ESP32

El proyecto incluye entornos de compilación específicos que eliminan completamente el código relacionado con ESP32, reduciendo el tamaño del firmware y simplificando el sistema.

**Ventajas de compilar sin ESP32:**
- ✅ Firmware más pequeño (~15KB menos)
- ✅ Sin dependencia de librería SerialTransfer
- ✅ Código más limpio y fácil de auditar
- ✅ Ideal para producción cuando no se necesita conectividad

**Compilar y Flashear:**

```bash
# Para hardware modular LEGO
pio run -e lego-no-esp32-stlink -t upload

# Para PCB integrada
pio run -e all-pcb-no-esp32-stlink -t upload
```

**¿Qué hace el flag NO_ESP32_COMMS?**

El flag de compilación `-DNO_ESP32_COMMS` envuelve todas las llamadas a funciones ESP32 en condicionales:

```cpp
#ifndef NO_ESP32_COMMS
  espCommsInit();              // No se ejecuta
  espCommsReadData();          // No se ejecuta
  espCommsSendSensorData();    // No se ejecuta
  espCommsSendShotData();      // No se ejecuta
#endif
```

El compilador eliminará completamente este código del binario final, resultando en:
- Menos uso de Flash
- Sin overhead de comunicación UART
- Sin inicialización de puerto Serial1

**Verificar la Compilación:**

```bash
# Ver tamaño del firmware
pio run -e lego-stlink          # CON ESP32
pio run -e lego-no-esp32-stlink # SIN ESP32

# Comparar output:
# Con ESP32:    ~215KB
# Sin ESP32:    ~200KB
```

**Logs del Sistema:**

Al iniciar con `NO_ESP32_COMMS`, verás en el serial:
```
Gaggiuino (fw: v4.x.x) booting
Pin init
...
ESP32 comms disabled (NO_ESP32_COMMS defined)
...
Setup sequence finished
```

Esto confirma que el sistema está corriendo sin código ESP32.

**📄 Documentación Completa:** Ver [NO_ESP32_BUILD.md](NO_ESP32_BUILD.md) para guía detallada de build sin ESP32.

#### Componentes de Hardware
- **Sensores de Presión**: ADS1115/ADS1015 ADC para medición de presión de alta precisión
- **Termopares**: MAX6675 o MAX31855 para control térmico del caldero
- **Balanzas**: HX711 con soporte para balanzas integradas o Bluetooth (predictivas)
- **Sensor ToF**: Time-of-Flight para detección de taza
- **LED RGB**: NCP5623 para indicadores de estado
- **Pantalla LCD**: Nextion o TJC HMI para interfaz de usuario

#### Funcionalidades Principales

**Control de Extracción:**
- Sistema de perfilado multi-fase con transiciones programables (instantánea, lineal, personalizada)
- Modos de control: presión constante, flujo constante, o combinaciones híbridas
- Hasta 5 perfiles de extracción personalizables almacenados en EEPROM
- Condiciones de parada configurables: tiempo, peso, presión, flujo, volumen de agua

**Gestión Térmica:**
- Control PID de temperatura del caldero
- Modos de operación: preparación (brew), vapor (steam), agua caliente
- DreamSteam: gestión inteligente de vapor con umbrales configurables
- Protección contra sobrecalentamiento (límite 105°C)

**Control de Bomba:**
- Modulación PWM con detección de cruce por cero (ZC)
- Control adaptativo basado en frecuencia de línea (50/60Hz)
- Calibración de flujo y presión
- Compensación automática según curva característica de la bomba

**Procesamiento de Datos:**
- Filtros Kalman para suavizado de lecturas (presión, flujo, peso)
- Sistema de medición dual: flujo de bomba vs flujo de balanza
- Predicción de peso final mediante algoritmos adaptativos
- Logging en tiempo real de todos los parámetros de extracción

**Interfaz de Usuario:**
- HMI táctil con múltiples páginas de configuración
- Visualización en tiempo real de gráficas presión/flujo/temperatura
- Calibración guiada de sensores y balanzas
- Modo descalcificación integrado

**Conectividad (módulo ESP32):**
- Servidor web con interfaz React/TypeScript (Vite)
- WebSocket para streaming de datos en tiempo real
- API REST para configuración remota
- Soporte para balanzas Bluetooth BLE
- Sistema de archivos LittleFS para configuración web

### Estructura del Código

```
src/
├── gaggiuino.ino           # Firmware principal STM32
├── eeprom_data/            # Gestión de configuración persistente y perfiles
├── functional/             # Lógica de control (extracción, descalcificación)
├── peripherals/            # Drivers de hardware (bomba, sensores, escalas)
└── lcd/                    # Comunicación con pantalla Nextion/TJC

lib/Common/                 # Bibliotecas compartidas
├── profiling_phases.cpp    # Motor de perfiles multi-fase
├── measurements.cpp        # Sistema de medición y filtrado
└── mcu_comms.cpp          # Protocolo de comunicación STM-ESP

webserver/                  # Firmware ESP32
├── src/                    # Backend C++ (FreeRTOS)
└── web-interface/          # Frontend React/TypeScript

test/                       # Suite de pruebas unitarias
```

### Configuraciones de Compilación

**Entornos PlatformIO:**
- `lego-stlink`: Hardware modular con válvula separada
- `all-pcb-stlink`: PCB integrado todo-en-uno
- `*-forced-predictive`: Forzar uso de balanzas predictivas
- `scales-calibration`: Utilidad de calibración de balanzas

### Características Técnicas
- **Frecuencia de muestreo**: Tiempo real con filtrado Kalman
- **Precisión de presión**: ±0.1 bar (con calibración)
- **Precisión de temperatura**: ±0.5°C (termopar tipo K)
- **Precisión de peso**: ±0.1g (según balanza)
- **Watchdog**: Protección contra bloqueos del sistema
- **Compatibilidad**: Gaggia Classic, Classic Pro y modelos similares

### Seguridad
- Watchdog independiente (IWDG)
- Protección térmica multi-nivel
- Timeouts configurables para operaciones críticas
- Apagado automático de elementos calefactores en condiciones anormales
- Reset automático del bus I2C en caso de bloqueo

### Desarrollo y Testing
- Integración continua con GitHub Actions
- Suite de pruebas unitarias con mocks de hardware
- Análisis estático con Cppcheck y Clang-Tidy
- Versionado automático en cada compilación
- Soporte para debugging con breakpoints (DBG)

---

## Ambiente de Desarrollo

### Requisitos del Sistema

#### Software Obligatorio
- **PlatformIO Core/IDE** (recomendado) o Arduino IDE con soporte STM32
- **Visual Studio Code** con extensión PlatformIO (método recomendado)
- **Git** para control de versiones
- **Python 3.x** (para scripts de versionado automático)

#### Para Deployment con ST-Link
- **ST-Link V2** o compatible (programador/debugger)
- **Drivers ST-Link**: 
  - Windows: [STSW-LINK009](https://www.st.com/en/development-tools/stsw-link009.html)
  - Linux: `stlink-tools` via gestor de paquetes
  - macOS: `brew install stlink`

#### Para Deployment con DFU (USB)
- **dfu-util** 0.11 o superior
  - Linux: `sudo apt install dfu-util`
  - macOS: `brew install dfu-util`
  - Windows: Descargar binarios + [Zadig](https://zadig.akeo.ie/) para drivers

### Especificaciones de Hardware

#### Placa Oficial del Proyecto
- **Modelo**: STM32F411CEU6 "Blackpill"
- **MCU**: ARM Cortex-M4 a 100MHz
- **Flash**: 512KB
- **RAM**: 128KB
- **FPU**: Si (hardware de punto flotante)
- **USB**: Si (soporte CDC para debugging vía USB)
- **Pines**: 40 pines GPIO, 3x USART, 2x I2C, 3x SPI

#### Placas STM32 Alternativas Compatibles
El proyecto puede adaptarse a otras placas STM32F4xx con las siguientes características mínimas:

**Requisitos Mínimos:**
- MCU: STM32F4xx con FPU (recomendado F401/F411 o superior)
- Flash: ≥256KB (recomendado 512KB para futuras funcionalidades)
- RAM: ≥64KB (recomendado 128KB)
- Timers: Mínimo 2 timers hardware disponibles
- USART: Mínimo 2 puertos (LCD + ESP32/Debug)
- I2C: 1 bus (sensor de presión ADS1115)
- SPI: 1 bus (termopar MAX6675/MAX31855)
- ADC: No requerido (se usa sensor I2C)
- GPIO: ~20 pines mínimo

**Placas Probadas por la Comunidad:**
- STM32F401CCU6 (256KB Flash) - requiere optimización de código
- STM32F411CEU6 (512KB Flash) - **Recomendada (oficial)**
- STM32F405RGT6 (1MB Flash) - sobrada para el proyecto
- STM32 Nucleo F411RE - requiere adaptación de pines

### Configuración del Ambiente PlatformIO

#### Instalación Inicial
```bash
# Clonar el repositorio
git clone https://github.com/Zer0-bit/gaggiuino.git
cd gaggiuino

# Abrir con VS Code + PlatformIO
code .
```

#### Estructura de Configuración
- **platformio.ini**: Configuración principal de compilación
- **extra_defines.ini** (opcional): Configuraciones personalizadas no versionadas
  
**Ejemplo de `extra_defines.ini`:**
```ini
[extra]
build_flags =
    -DDREAM_STEAM_DISABLED  ; Deshabilitar DreamSteam para calderos pequeños
    -DSINGLE_HX711_BOARD    ; Solo una balanza HX711
    -DDEBUG_ENABLED         ; Habilitar output de debugging
```

#### Entornos de Compilación Disponibles

**Para Hardware Modular (LEGO build):**
```bash
pio run -e lego-stlink              # Build estándar con ESP32
pio run -e lego-no-esp32-stlink     # Build SIN ESP32 (recomendado para proyectos simplificados)
pio run -e lego-forced-predictive-stlink  # Con balanzas predictivas forzadas
```

**Para PCB Integrado:**
```bash
pio run -e all-pcb-stlink           # Build release con ESP32
pio run -e all-pcb-no-esp32-stlink  # Build release SIN ESP32
pio run -e all-pcb-forced-predictive-stlink  # Build debug con predictivas
```

**Herramientas de Calibración:**
```bash
pio run -e scales-calibration-stlink  # Utilidad de calibración de balanzas
```

### Proceso de Deployment

#### Método 1: ST-Link (Recomendado para Desarrollo)

**Ventajas:**
- Debugging en tiempo real con breakpoints
- No requiere entrar en modo DFU
- Más rápido para iteraciones de desarrollo
- Permite lectura de memoria y registros

**Pasos:**
1. Conectar ST-Link V2 al Blackpill:
   ```
   ST-Link    Blackpill
   SWDIO   -> SWDIO
   SWCLK   -> SWCLK  
   GND     -> GND
   3.3V    -> 3.3V (opcional, solo si no hay otra alimentación)
   ```

2. Compilar y flashear:
   ```bash
   # Compilar el entorno deseado
   pio run -e lego-stlink
   
   # Flashear directamente
   pio run -e lego-stlink --target upload
   
   # O todo en un comando
   pio run -e lego-stlink -t upload
   ```

3. Para debugging:
   ```bash
   # Iniciar sesión de debug
   pio debug -e lego-stlink
   ```

#### Método 2: DFU via USB (Deployment en Producción)

**Ventajas:**
- No requiere hardware adicional
- Útil para actualizaciones en máquinas instaladas
- Más portable

**Pasos:**

1. **Entrar en modo DFU:**
   - Presionar y mantener ambos botones: **NRST** + **BOOT0**
   - Soltar **NRST**
   - Esperar 1 segundo
   - Soltar **BOOT0**
   - El dispositivo aparecerá como "STM32 BOOTLOADER"

2. **Verificar detección (opcional):**
   ```bash
   # Linux/Mac
   dfu-util -l
   
   # Windows
   dfu-util-static.exe -l
   ```
   
   Debe aparecer: `[0483:df11]`

3. **Compilar firmware:**
   ```bash
   pio run -e lego-stlink
   ```
   
   El archivo .bin estará en: `.pio/build/lego-stlink/firmware.bin`

4. **Flashear via DFU:**
   ```bash
   # Linux/Mac
   dfu-util -d 0x0483:0xDF11 -a 0 -s 0x08000000:leave -D .pio/build/lego-stlink/firmware.bin
   
   # Windows
   dfu-util-static.exe -d 0x0483:0xDF11 -a 0 -s 0x08000000:leave -D .pio\build\lego-stlink\firmware.bin
   ```

5. La placa se reiniciará automáticamente después del flash

#### Método 3: Serial Bootloader (No recomendado)
Posible mediante USART pero requiere configuración adicional no cubierta en este proyecto.

### Adaptación a Otras Máquinas Espresso

#### Consideraciones de Hardware

**Control de Bomba:**
- El código actual usa detección de cruce por cero (ZC) para control de fase
- **Pin requerido:** `zcPin` (PA0) - entrada digital con interrupción
- **Pin PWM:** `dimmerPin` (PA1) - salida con timer hardware
- **Alternativa:** Para bombas DC, modificar `src/peripherals/pump.cpp` eliminando lógica ZC

**Control de Temperatura:**
- **Termopar Type-K** vía MAX6675 o MAX31855
- **Pines SPI:** definidos en `src/pindef.h`
- Para **sensores resistivos (PT100)**: reemplazar driver en `src/peripherals/thermocouple.h`
- Para **termistores NTC**: agregar código de linearización Steinhart-Hart

**Sensor de Presión:**
- Actualmente: **ADS1115** (ADC I2C de 16-bit) con transductor 0-5V
- **Pines I2C:** SDA (PB7), SCL (PB6) - definidos en hardware STM32
- Para otros sensores: modificar `src/peripherals/pressure_sensor.cpp`

**Actuadores Específicos:**
```cpp
// Revisar/modificar en src/pindef.h
#define relayPin      PA15  // Caldero principal
#define valvePin      PC13  // Válvula solenoide 3-vías
#define steamBoilerRelayPin PB13  // Caldero de vapor (opcional)
#define steamValveRelayPin  PB12  // Válvula de vapor (opcional)
```

#### Pasos para Adaptación

**1. Mapeo de Pines Personalizado**
   
Crear un archivo `src/pindef_custom.h`:
```cpp
#ifndef PINDEF_CUSTOM_H
#define PINDEF_CUSTOM_H

// Adaptar según tu placa STM32
#define thermoDO      PB4
#define thermoCS      PA6
#define thermoCLK     PA5

#define zcPin         PA0   // Si usas bomba AC
#define brewPin       PC14  // Botón de brew
#define relayPin      PA15  // SSR caldero
#define dimmerPin     PA1   // Control TRIAC/SSR bomba
// ... resto de pines
#endif
```

Luego incluir condicionalmente en `pindef.h`:
```cpp
#ifdef CUSTOM_BOARD
  #include "pindef_custom.h"
#else
  // Pines originales Blackpill
#endif
```

**2. Crear Entorno de Compilación Personalizado**

Agregar a `platformio.ini`:
```ini
[env:mi-maquina-stlink]
extends = blackpill-core
board = tu_placa_stm32  ; e.g., nucleo_f411re, genericSTM32F401CB
lib_deps =
	${blackpill-core.lib_deps}
	; Agregar/quitar dependencias según necesites
build_flags =
	${blackpill-core.build_flags}
	-DCUSTOM_BOARD
	-DMI_MAQUINA_ESPECIFICA
	; Flags específicos de tu hardware
```

**3. Ajustar Constantes de Calibración**

En `src/gaggiuino.h` o mediante defines:
```cpp
// Ajustar según características de tu máquina
#define BOILER_FILL_TIMEOUT     12000UL  // Calderos más grandes
#define SYS_PRESSURE_IDLE       0.5f     // Presión residual específica
#define GET_KTYPE_READ_EVERY    100      // Según tu termopar
```

**4. Modificar Curvas de Bomba**

En `src/peripherals/pump.cpp` - función `getPumpFlow()`:
```cpp
// Calibrar según curva presión-flujo de TU bomba específica
// Tomar mediciones y ajustar coeficientes
```

**5. Testing Incremental**

Probar componentes individualmente:
```cpp
// Crear test/test_custom_hardware.cpp
void test_pressure_sensor() {
  // Verificar lecturas de presión
}

void test_thermocouple() {
  // Verificar lecturas de temperatura
}

void test_pump_control() {
  // Verificar PWM de bomba
}
```

#### Ejemplo: Adaptación para Rancilio Silvia

```ini
[env:silvia-stlink]
extends = blackpill-core
build_flags =
	${blackpill-core.build_flags}
	-DDREAM_STEAM_DISABLED  ; Caldero más grande, no necesita DreamSteam
	-DBOILER_FILL_TIMEOUT=15000UL  ; Caldero de 300ml vs 127ml Gaggia
	-DMAX_WATER_TEMP=110.0f  ; Mayor capacidad térmica
```

### Debugging y Troubleshooting

#### Habilitar Logs Detallados
```cpp
// En platformio.ini
build_flags = 
    -DLOG_LEVEL=4  ; 0=ERROR, 1=WARN, 2=INFO, 3=DEBUG, 4=VERBOSE
    -DDEBUG_ENABLED
```

#### Monitoring Serial
```bash
# PlatformIO
pio device monitor -e lego-stlink -b 115200

# O herramientas nativas
screen /dev/ttyACM0 115200        # Linux
screen /dev/cu.usbmodem* 115200   # macOS  
putty COM3 -serial -sercfg 115200 # Windows
```

#### Errores Comunes

**"No DFU capable USB device found"**
- Verificar drivers con Zadig (Windows)
- Verificar modo DFU (LED debe parpadear diferente)
- Probar otro cable USB (preferir cables con datos)

**"Multiple stlink devices detected"**
- Desconectar otros ST-Links
- Especificar serial: `pio run -e lego-stlink -t upload --upload-port <serial>`

**"Flash write error"**
- Flash protegido: usar STM32CubeProgrammer para desproteger
- Verificar voltaje de alimentación (mínimo 3.3V estable)

**Watchdog constante reiniciando:**
- Bucle bloqueante en el código
- Deshabilitar temporalmente: comentar `iwdcInit()` en `gaggiuino.ino`

### Recursos Adicionales

#### Documentación Oficial
- [STM32F411 Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)
- [STM32F411 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00119316.pdf)
- [PlatformIO STM32 Platform](https://docs.platformio.org/en/latest/platforms/ststm32.html)

#### Esquemáticos y PCB
- [Gaggiuino Hardware Repository](https://github.com/GAGGIUINO/gaggiuino-hardware)
- Revisar esquemáticos antes de adaptar hardware

#### Comunidad
- [Discord Oficial](https://discord.gg/eJTDJA3xfh) - Canal #development para consultas técnicas
- [Issues de GitHub](https://github.com/Zer0-bit/gaggiuino/issues) - Reportar bugs o consultar problemas conocidos
