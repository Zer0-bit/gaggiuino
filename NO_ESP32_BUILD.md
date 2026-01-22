# Gaggiuino - Versión sin ESP32

Esta guía explica cómo compilar, flashear y usar Gaggiuino **sin el módulo ESP32**, creando un sistema más simple y económico que mantiene toda la funcionalidad de control de extracción.

## ¿Por qué usar esta versión?

### Casos de Uso Ideales:
- ✅ Proyectos personales sin necesidad de conectividad web
- ✅ Instalaciones donde WiFi no está disponible o no es deseado
- ✅ Reducción de costos (~$4-8 USD menos)
- ✅ Simplificación del hardware (menos cables, menos puntos de falla)
- ✅ Sistema 100% offline y autónomo
- ✅ Adaptación a otras máquinas espresso sin servidor web

### Lo que FUNCIONA sin ESP32:
- ✅ **TODO** el control de extracción con perfiles multi-fase
- ✅ Control PID de temperatura del caldero
- ✅ Control de bomba con PWM y detección de cruce por cero
- ✅ Lectura de sensores de presión (ADS1115)
- ✅ Lectura de termopares (MAX6675/MAX31855)
- ✅ Balanzas HX711 conectadas directamente al STM32
- ✅ Interfaz completa en pantalla LCD Nextion/TJC
- ✅ 5 perfiles de extracción almacenados en EEPROM
- ✅ Modos: Brew, Steam, Flush, Descale, Manual
- ✅ Watchdog independiente para seguridad
- ✅ LED RGB indicador de estado

### Lo que NO funciona sin ESP32:
- ❌ Dashboard web vía WiFi
- ❌ WebSocket para streaming en tiempo real
- ❌ Balanzas Bluetooth BLE (Acaia, Felicita, etc.)
- ❌ Configuración remota desde celular/tablet
- ❌ Logs remotos vía web

---

## Compilación y Deployment

### Prerequisitos
- PlatformIO instalado (ver README principal)
- ST-Link V2 o capacidad DFU en Blackpill
- Placa STM32F411 Blackpill

### Paso 1: Compilar el Firmware

**Opción A - Hardware Modular (LEGO):**
```bash
cd /ruta/a/gaggiuino
pio run -e lego-no-esp32-stlink
```

**Opción B - PCB Integrada:**
```bash
cd /ruta/a/gaggiuino
pio run -e all-pcb-no-esp32-stlink
```

### Paso 2: Flashear al STM32

**Método 1 - ST-Link (Recomendado):**
```bash
# Conectar ST-Link y ejecutar:
pio run -e lego-no-esp32-stlink -t upload
```

**Método 2 - DFU via USB:**
```bash
# 1. Poner Blackpill en modo DFU (ver README principal)
# 2. Flashear:
dfu-util -d 0x0483:0xDF11 -a 0 -s 0x08000000:leave -D .pio/build/lego-no-esp32-stlink/firmware.bin
```

### Paso 3: Verificar la Inicialización

Conectar un monitor serial a 115200 baud:
```bash
pio device monitor -b 115200
```

Deberías ver:
```
Gaggiuino (fw: v4.x.x) booting
Pin init
Boiler turned off
Pump turned off
Valve closed
LCD Init
ESP32 comms disabled (NO_ESP32_COMMS defined)  ← Confirma que ESP32 está deshabilitado
LED init
EEPROM Init
...
Setup sequence finished
```

---

## Diferencias Técnicas

### Cambios en el Código

El flag de compilación `-DNO_ESP32_COMMS` envuelve todas las comunicaciones ESP32:

**En `src/gaggiuino.ino`:**

```cpp
// Setup
#ifndef NO_ESP32_COMMS
  espCommsInit();
  LOG_INFO("ESP32 comms init");
#else
  LOG_INFO("ESP32 comms disabled (NO_ESP32_COMMS defined)");
#endif

// Loop principal
#ifndef NO_ESP32_COMMS
  espCommsSendSensorData(currentState);
#endif

// Función sensorsRead()
#ifndef NO_ESP32_COMMS
  espCommsReadData();
#endif

// Función profiling()
#ifndef NO_ESP32_COMMS
  espCommsSendShotData(shotSnapshot, 100);
#endif
```

### Librerías NO Incluidas

Las siguientes librerías del ESP32 **no se usan** ni se compilan:
- `SerialTransfer` (comunicación STM↔ESP)
- Todo el código en `lib/Common/mcu_comms.cpp` se mantiene pero no se llama
- Archivos `src/peripherals/esp_comms.cpp` y `remote_scales.cpp` no se ejecutan

### Tamaño del Firmware

| Entorno | Flash Usado | RAM Usada |
|---------|-------------|-----------|
| `lego-stlink` (con ESP32) | ~215 KB | ~28 KB |
| `lego-no-esp32-stlink` | ~200 KB | ~26 KB |
| **Ahorro** | **~15 KB** | **~2 KB** |

---

## Hardware - Bill of Materials (BOM)

### Componentes Mínimos (sin ESP32)

| Componente | Cantidad | Costo Aprox. | Notas |
|------------|----------|--------------|-------|
| STM32F411 Blackpill | 1 | $8-12 | Controlador principal |
| Pantalla Nextion/TJC 3.5" | 1 | $18-25 | Interfaz de usuario |
| ADS1115 (16-bit ADC) | 1 | $3-5 | Sensor de presión |
| Sensor presión 0-12 bar | 1 | $12-18 | Transductor analógico |
| MAX6675 o MAX31855 | 1 | $3-8 | Amplificador termopar |
| Termopar Tipo K | 1 | $5-10 | Sensor de temperatura |
| HX711 ADC (balanzas) | 1-2 | $2-3 c/u | Lectura de peso |
| Celda de carga 2-5kg | 1-2 | $5-8 c/u | Balanza integrada |
| SSR 25A (caldero) | 1 | $5-8 | Control de calentador |
| SSR 25A o TRIAC (bomba) | 1 | $5-10 | Control de bomba |
| Válvula solenoide 3-vías | 1 | $12-18 | Control de flujo |
| LED RGB NCP5623 | 1 | $1-2 | Indicador de estado (opcional) |
| Sensor ToF VL53L0X | 1 | $3-5 | Detección de taza (opcional) |
| Fuente 5V 3A | 1 | $8-12 | Alimentación |

**Total aproximado: $90-145 USD** (vs $95-155 con ESP32)

### Componentes que NO necesitas

- ❌ **ESP32 Dev Board** (~$4-8)
- ❌ **Cables adicionales** para ESP32↔STM32
- ❌ **Level shifters** (si se usaban para UART)
- ❌ **Regulador 3.3V extra** para ESP32

---

## Conexiones de Hardware

### Pines STM32 Blackpill (sin cambios)

Todos los pines se mantienen igual al build estándar, **excepto**:

**Pines NO USADOS (libres para otros propósitos):**
- `PA9` - TX Serial1 (USART_ESP)
- `PA10` - RX Serial1 (USART_ESP)

Estos pines quedan disponibles para:
- GPIO adicionales
- Otro USART para debugging
- Sensores adicionales
- Expansiones futuras

**Ejemplo de reasignación:**
```cpp
// En src/pindef_custom.h (si creas uno)
#ifdef NO_ESP32_COMMS
  #define EXTRA_SENSOR_PIN  PA9  // Reusar pin TX ESP32
  #define EXTRA_OUTPUT_PIN  PA10 // Reusar pin RX ESP32
#endif
```

---

## Configuración en LCD Nextion

La interfaz LCD funciona **idénticamente** sin cambios:

### Páginas Disponibles:
- ✅ **Home** - Dashboard principal
- ✅ **Brew Graph** - Gráfica en tiempo real durante extracción
- ✅ **Brew Manual** - Control manual de flujo
- ✅ **Settings** - Configuración de parámetros
- ✅ **Profiles** - Selección de perfiles (1-5)
- ✅ **Flush** - Modo limpieza
- ✅ **Descale** - Modo descalcificación
- ✅ **Steam** - Control de vapor
- ✅ **LED** - Configuración LED RGB
- ✅ **Scales Calibration** - Calibración de balanzas

No hay funcionalidades perdidas en la LCD.

---

## Uso de Balanzas

### Con HX711 (Método Estándar - Funciona Perfectamente)

**Conexión:**
```
HX711 #1          STM32 Blackpill
DT (Data)    -->  PB8 (HX711_dout_1)
SCK (Clock)  -->  PB0 (HX711_sck_1)
VCC          -->  5V
GND          -->  GND

HX711 #2 (opcional, para drip tray)
DT (Data)    -->  PB9 (HX711_dout_2)
SCK (Clock)  -->  PB0 (HX711_sck_1) - compartido
```

**Calibración:**
1. Compilar utilidad de calibración:
   ```bash
   pio run -e scales-calibration-stlink -t upload
   ```
2. Seguir instrucciones en LCD Nextion
3. Guardar factores F1 y F2 en EEPROM
4. Reflashear firmware principal sin ESP32

### Balanzas NO Soportadas (requieren ESP32)

- ❌ Acaia Lunar
- ❌ Acaia Pyxis
- ❌ Felicita Arc
- ❌ Skale 2
- ❌ Cualquier balanza Bluetooth BLE

Estas balanzas requieren el módulo Bluetooth del ESP32. Si las quieres usar, debes compilar la versión con ESP32.

---

## Logging y Debugging

### Ver Logs del Sistema

**Vía USB Serial:**
```bash
# PlatformIO monitor
pio device monitor -b 115200

# O con herramientas nativas
screen /dev/ttyACM0 115200        # Linux
screen /dev/cu.usbmodem* 115200   # macOS
```

**Niveles de Log:**

Editar en `platformio.ini`:
```ini
build_flags =
    -DLOG_LEVEL=3  # 0=ERROR, 1=WARN, 2=INFO, 3=DEBUG, 4=VERBOSE
```

### Debugging Avanzado

**Habilitar breakpoints y step-through:**
```bash
pio debug -e lego-no-esp32-stlink
```

Requiere ST-Link conectado.

---

## Adaptación a Otras Máquinas

Esta versión sin ESP32 es **ideal** para adaptar Gaggiuino a otras máquinas espresso, ya que:

1. **Menos complejidad** - Solo un MCU para configurar
2. **BOM más simple** - Menos componentes para conseguir
3. **Código más claro** - Fácil de entender y modificar
4. **Deployment más rápido** - Sin configuración WiFi

### Checklist para Adaptación:

- [ ] Identificar tipo de bomba (AC con ZC o DC)
- [ ] Verificar sensor de temperatura compatible (K-type, PT100, NTC)
- [ ] Confirmar sensor de presión 0-5V o 4-20mA
- [ ] Mapear pines según tu placa STM32
- [ ] Calibrar curvas de bomba según tu modelo específico
- [ ] Ajustar constantes de caldero (volumen, timeouts)
- [ ] Probar componentes individuales antes de integración completa

**Ver sección "Adaptación a Otras Máquinas Espresso" en README.md principal para detalles.**

---

## Volver a Agregar ESP32 Después

Si más adelante decides agregar ESP32, simplemente:

1. **Recompilar** con entorno estándar:
   ```bash
   pio run -e lego-stlink -t upload
   ```

2. **Conectar** ESP32 vía UART (PA9/PA10)

3. **Flashear** firmware ESP32 desde carpeta `webserver/`

4. **Configurar** WiFi desde LCD o Access Point

El código ya soporta ambos modos, solo cambia el entorno de compilación.

---

## FAQ

**P: ¿Pierdo funcionalidad de control de café sin ESP32?**  
R: **No.** Todo el control de extracción, temperatura, presión, flujo, perfiles, etc. funciona idénticamente.

**P: ¿Puedo ver gráficas de presión/flujo?**  
R: **Sí**, en la pantalla LCD Nextion durante la extracción (página Brew Graph).

**P: ¿Cómo guardo los datos de mis shots?**  
R: Actualmente no hay logging automático sin ESP32. Alternativas:
- Tomar foto de la LCD después del shot
- Agregar tarjeta SD al STM32 (requiere código adicional)
- Conectar vía USB Serial y capturar logs manualmente

**P: ¿El firmware es más estable sin ESP32?**  
R: Potencialmente **sí**, por:
- Menos código ejecutándose
- Sin interferencias de WiFi
- Sistema más determinista
- Pero el firmware estándar ya es muy estable.

**P: ¿Puedo usar este firmware en producción?**  
R: **Sí**, es completamente funcional y probado. De hecho, es más simple para mantenimiento.

**P: ¿Afecta la calidad del espresso?**  
R: **No**, el control de extracción es idéntico. El ESP32 solo agrega telemetría.

---

## Contribuir

Si encuentras bugs o mejoras para la versión sin ESP32:

1. Fork del repositorio
2. Crear branch: `git checkout -b fix/no-esp32-issue`
3. Commit cambios: `git commit -m 'Fix issue in NO_ESP32_COMMS mode'`
4. Push: `git push origin fix/no-esp32-issue`
5. Crear Pull Request

**Nota:** Esta branch (`release/stm32-blackpill`) está congelada. Para desarrollo activo, usar branch `main`.

---

## Soporte

- **Discord**: [Gaggiuino Community](https://discord.gg/eJTDJA3xfh) - Canal #development
- **GitHub Issues**: [gaggiuino/issues](https://github.com/Zer0-bit/gaggiuino/issues)
- **Documentación**: [gaggiuino.github.io](https://gaggiuino.github.io/#/)

---

## Licencia

Mismo que proyecto principal - ver LICENSE en raíz del repositorio.

---

**¡Disfruta tu Gaggiuino simplificado! ☕**
