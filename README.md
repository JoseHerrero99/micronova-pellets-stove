# 🔥 Micronova Pellet Stove Controller

<div align="center">

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![IoT: Blynk](https://img.shields.io/badge/IoT-Blynk-00D4FF.svg)](https://blynk.io/)

**Advanced remote control and automation system for Micronova-based pellet stoves**

[Features](#-features) • [Hardware](#-hardware) • [Installation](#-installation) • [Documentation](#-project-structure) • [License](#-license)

---

### 🌐 Language / Idioma

**[🇪🇸 Español](#-documentación-en-español)** | **[🇬🇧 English](#-english-documentation)**

</div>

---

# 📖 Documentación en Español

## 🎯 Características

### Control Remoto y Local
- ✅ **Control WiFi mediante Blynk** - Interfaz móvil y web para control desde cualquier lugar
- ✅ **Terminal Serial Interactivo** - Consola VT100 para control y diagnóstico local
- ✅ **Encendido/Apagado Inteligente** - Con protección de tiempo mínimo de funcionamiento
- ✅ **Ajuste de Potencia** - 5 niveles de potencia con retroalimentación en tiempo real

### Automatización Avanzada
- ⏰ **Programador Semanal** - Hasta 8 entradas programables por día/hora
- ⏲️ **Temporizador de Apagado** - Apagado automático después de X minutos
- 🛡️ **Protecciones de Seguridad** - Tiempo mínimo de encendido configurable
- 📊 **Monitoreo de Estado** - Lectura continua de temperatura y estado operativo

### Características Técnicas
- 🔧 **Acceso Directo a Memoria** - Lectura/escritura RAM y EEPROM del controlador
- 🧪 **Modo Simulación** - Testing sin hardware físico
- 🔄 **Multitarea con FreeRTOS** - Operación concurrente y eficiente
- 📝 **Código Modular** - Arquitectura profesional fácilmente extensible

## 🔌 Hardware

### Componentes Principales
- **Microcontrolador**: ESP32 (cualquier modelo con WiFi)
- **Estufa**: Controlador Micronova (protocolo UART 1200 baud, 8N2)
- **Interfaz**: Circuito optoacoplado para aislamiento y adaptación de niveles

### Circuito de Interfaz

El circuito utiliza optoacopladores para adaptar los niveles de voltaje (la estufa usa lógica de 5V y el ESP32 3.3V). Los optoacopladores también implementan cancelación de eco. Para leer valores desde la estufa, es necesario poner el optoacoplador ENABLE_RX en bajo, pero entonces no se enviará más datos al TX de la estufa.

El diseño está basado en el circuito de [philibertc/micronova_controller](https://github.com/philibertc/micronova_controller).

#### Lista de Materiales (BOM)
| Cantidad | Componente | Especificación |
|----------|------------|----------------|
| 2 | Resistencias | 150Ω |
| 1 | Resistencia | 680Ω |
| 1 | Resistencia | 4K7Ω |
| 3 | Optoacopladores | PC817 o EL817 |
| 1 | Módulo Step-Down | Entrada 20V, Salida 5V |

#### Diagrama del Circuito
![Esquema de conexión (Schematic.png)](./Schematic.png)

> Advertencia: Mi esquema es diferente al del repositorio original. Sospecho que el esquemático publicado allí no está correctamente conectado para este caso concreto. Es posible que tengas que ajustar los valores de las resistencias según tu hardware. En mi montaje con PC817, los valores que funcionaron son los que aparecen en la lista de materiales anterior.

Se ha probado y funcionado con la estufa de pellets **Bronpi**; debería funcionar con otras estufas de pellets, siempre que se verifiquen y, en caso necesario, se actualicen las direcciones RAM y los valores de los componentes relevantes.

#### Conexiones ESP32
```
ESP32          Circuito
GPIO 33   -->  RX (desde estufa)
GPIO 32   -->  TX (hacia estufa)
GPIO 27   -->  ENABLE_RX (control RS485)
GND       -->  GND común
```

### Esquema de Conexión
```
┌──────────────┐        ┌──────────────┐        ┌──────────────┐
│              │        │  Optocoupler │        │              │
│    ESP32     │◄──────►│   Circuit    │◄──────►│   Micronova  │
│   (3.3V)     │        │ (Level Shift)│        │   Stove (5V) │
│              │        │              │        │              │
└──────────────┘        └──────────────┘        └──────────────┘
```

## 📥 Instalación

### Requisitos Previos
- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) (recomendado) o Arduino IDE
- ESP32 board support instalado
- Cuenta en [Blynk.io](https://blynk.io/) (plan gratuito disponible)

### Pasos de Instalación

1. **Clonar el repositorio**
   ```bash
   git clone https://github.com/JoseHerrero99/micronova-pellets-stove.git
   cd micronova-pellets-stove
   ```

2. **Configurar credenciales en `src/Config.h`**
   ```cpp
   // WiFi
   static const char* WIFI_SSID = "Tu_SSID";
   static const char* WIFI_PASS = "Tu_Password";
   
   // Blynk
   #define BLYNK_AUTH_TOKEN "Tu_Token_Blynk"
   ```

3. **Ajustar pines GPIO si es necesario**
   ```cpp
   #define HW_RX_PIN_DEFAULT    33
   #define HW_TX_PIN_DEFAULT    32
   #define HW_EN_RX_PIN_DEFAULT 27
   ```

4. **⚠️ IMPORTANTE: Verificar direcciones RAM para tu modelo de estufa**
   
   Las direcciones RAM pueden variar según el modelo de estufa. Este proyecto usa:
   ```cpp
   #define RAM_ADDR_POWER_FEEDBACK 0xB9  // Feedback de potencia
   #define RAM_ADDR_STATE          0x21  // Estado de la estufa
   #define RAM_ADDR_COMMAND        0x58  // Dirección de comandos
   ```
   
   Si tu estufa no responde correctamente:
   - Prueba cambiar `RAM_ADDR_POWER_FEEDBACK` a `0x34` (usado en otros modelos)
   - Usa el comando de terminal `ram <dirección>` para explorar
   - Consulta la sección [Protocolo Micronova](#protocolo-micronova) más abajo

5. **Compilar y cargar**
   ```bash
   pio run --target upload
   ```

   O desde PlatformIO IDE: `PlatformIO: Upload`

### Configuración de Blynk

1. Crear nuevo template con el ID: `BLYNK_TEMPLATE_ID`
2. Configurar los siguientes Virtual Pins:
   - V0: Stove State (Display - Numeric)
   - V1: Ambient Temperature (Display - °C)
   - V2: Power Level Read (Display - Numeric)
   - V3: Power Level Write (Slider 1-5)
   - V4: On/Off Switch (Switch)
   - V6: Set Timer (Input - Minutes)
   - V7: State String (Display - Text)
   - V8-V20: Scheduler controls (ver Config.h para detalles)

## 🖥️ Uso

### Control por Terminal Serial

Conectar a 115200 baud y usar comandos:

```
Comandos Principales:
  help          - Mostrar ayuda completa
  status        - Estado actual de la estufa
  on            - Encender estufa
  off           - Apagar estufa
  power <1-5>   - Establecer nivel de potencia
  temp          - Mostrar temperatura
  
Scheduler:
  sched_list    - Listar programaciones
  sched_set <idx> <active> <day> <hour> <min> <power>
  
Timer:
  timer set <minutos>
  timer cancel
  timer status
```

### Control por Blynk

Usar la aplicación móvil de Blynk para:
- Encender/apagar la estufa remotamente
- Ajustar nivel de potencia
- Configurar temporizadores
- Programar encendidos automáticos
- Monitorear temperatura y estado

## 📚 Estructura del Proyecto

```
micronova-pellets/
├── src/
│   ├── main.cpp              # Punto de entrada y tareas FreeRTOS
│   ├── Config.h              # Configuración global
│   ├── StoveController.{h,cpp}   # Lógica principal de control
│   ├── StoveComm.{h,cpp}         # Comunicación hardware UART
│   ├── SimStoveComm.{h,cpp}      # Simulador para testing
│   ├── BlynkInterface.{h,cpp}    # Interfaz Blynk IoT
│   ├── Scheduler.{h,cpp}         # Programador semanal
│   ├── Terminal.{h,cpp}          # Terminal interactivo
│   └── IStoveComm.h              # Interfaz abstracta
├── platformio.ini            # Configuración PlatformIO
├── LICENSE                   # GPL-3.0
└── README.md                 # Este archivo
```

## 🔧 Desarrollo

### Modo Simulación

Para desarrollo sin hardware real, habilitar en `platformio.ini`:

```ini
build_flags = -DSIMULATION_MODE
```

Comandos de simulación disponibles:
```
sim_state <0-6>      # Forzar estado
sim_power <1-5>      # Forzar potencia
sim_temp <°C>        # Forzar temperatura
sim_fail             # Activar modo falla
sim_recover          # Recuperar de falla
```

### Protocolo Micronova

El sistema implementa el protocolo Micronova estándar:
- **Baud Rate**: 1200
- **Configuración**: 8 bits, sin paridad, 2 bits de parada (8N2)
- **Checksum**: Implementado según especificación Micronova

#### Direcciones RAM Específicas de Esta Estufa

Este proyecto ha sido probado y configurado para un modelo específico de estufa Micronova. Las direcciones pueden **diferir** de otros proyectos (como [philibertc/micronova_controller](https://github.com/philibertc/micronova_controller)).

**Direcciones utilizadas en esta estufa:**

| Dirección | Función | Descripción |
|-----------|---------|-------------|
| `0x21` | Estado | Byte de estado de la estufa (OFF=0x21, WORKING=0x04, etc.) |
| `0x01` | Temperatura | Temperatura ambiente en grados Celsius |
| `0xB9` | Potencia (feedback) | Nivel de potencia actual (1-5) ⚠️ **Difiere del original** |
| `0x58` | Comandos | Dirección donde se escriben los comandos |

**Comandos de Control (escritos en dirección 0x58):**

| Comando | Valor | Acción |
|---------|-------|--------|
| Power + | `0x54` | Incrementar nivel de potencia |
| Power - | `0x50` | Decrementar nivel de potencia |
| ON/OFF | `0x5A` | Encender/Apagar estufa |
| Temp + | `0x52` | Incrementar temperatura objetivo (no usado) |
| Temp - | `0x58` | Decrementar temperatura objetivo (no usado) |

⚠️ **Nota Importante**: La dirección de feedback de potencia (`0xB9`) es **diferente** a la del proyecto original (`0x34`). Si tienes otra estufa Micronova y no funciona, prueba:
1. Cambiar `RAM_ADDR_POWER_FEEDBACK` de `0xB9` a `0x34` en `Config.h`
2. Usar comandos de terminal `ram <addr>` para descubrir las direcciones de tu modelo
3. Verificar que los comandos respondan correctamente

Para más detalles, consulta la documentación completa en `src/Config.h`.

## 🤝 Contribuciones

Las contribuciones son bienvenidas! Por favor:

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## 📄 Licencia

Este proyecto está licenciado bajo la **Licencia GPL-3.0** - ver el archivo [LICENSE](LICENSE) para más detalles.

**Autor**: Jose Herrero Ruiz

Este es software libre: puedes redistribuirlo y/o modificarlo bajo los términos de la Licencia Pública General de GNU publicada por la Free Software Foundation, versión 3.
Por favor, **mantén esta nota y el copyright**.

## 🙏 Reconocimientos

- Basado en el trabajo de [philibertc/micronova_controller](https://github.com/philibertc/micronova_controller)
- Comunidad ESP32 y Arduino
- Equipo de Blynk IoT

---

# 📖 English Documentation

## 🎯 Features

### Remote and Local Control
- ✅ **WiFi Control via Blynk** - Mobile and web interface for control from anywhere
- ✅ **Interactive Serial Terminal** - VT100 console for local control and diagnostics
- ✅ **Smart On/Off Control** - With minimum runtime protection
- ✅ **Power Adjustment** - 5 power levels with real-time feedback

### Advanced Automation
- ⏰ **Weekly Scheduler** - Up to 8 programmable entries per day/time
- ⏲️ **Shutdown Timer** - Automatic shutdown after X minutes
- 🛡️ **Safety Protections** - Configurable minimum on-time
- 📊 **State Monitoring** - Continuous temperature and operational state reading

### Technical Features
- 🔧 **Direct Memory Access** - Read/write controller RAM and EEPROM
- 🧪 **Simulation Mode** - Testing without physical hardware
- 🔄 **FreeRTOS Multitasking** - Concurrent and efficient operation
- 📝 **Modular Code** - Professional architecture, easily extensible

## 🔌 Hardware

### Main Components
- **Microcontroller**: ESP32 (any WiFi-enabled model)
- **Stove**: Micronova controller (UART protocol 1200 baud, 8N2)
- **Interface**: Optocoupler circuit for isolation and level shifting

### Interface Circuit

The circuit uses optocouplers to perform level-shifting (stove uses 5V logic and ESP32 uses 3.3V). The optocouplers also implement echo cancellation. To read values from the stove, the ENABLE_RX optocoupler must be pulled low, but then no data will be sent to the stove's TX anymore.

The circuit is based on the design from [philibertc/micronova_controller](https://github.com/philibertc/micronova_controller).

#### Bill of Materials (BOM)
| Quantity | Component | Specification |
|----------|-----------|---------------|
| 2 | Resistencias | 150Ω |
| 1 | Resistencia | 680Ω |
| 1 | Resistencia | 4K7Ω |
| 3 | Optocouplers | PC817 or EL817 |
| 1 | Step-Down Module | 20V input, 5V output |

#### Circuit Diagram
![Connection schematic (Schematic.png)](./Schematic.png)

> Warning: My schematic differs from the original repository’s schematic. I suspect the one published there may not be correctly connected for this specific setup. You may need to tune the resistor values for your hardware. With PC817, the values that worked for me are the ones listed above in the BOM.

This project has been tested and worked with a **Bronpi** pellet stove; it should work with other pellet stoves provided you verify and, if necessary, update the RAM addresses and relevant component values.

#### ESP32 Connections
```
ESP32          Circuit
GPIO 33   -->  RX (from stove)
GPIO 32   -->  TX (to stove)
GPIO 27   -->  ENABLE_RX (RS485 control)
GND       -->  Common GND
```

### Connection Schematic
```
┌──────────────┐        ┌──────────────┐        ┌──────────────┐
│              │        │  Optocoupler │        │              │
│    ESP32     │◄──────►│   Circuit    │◄──────►│   Micronova  │
│   (3.3V)     │        │ (Level Shift)│        │   Stove (5V) │
│              │        │              │        │              │
└──────────────┘        └──────────────┘        └──────────────┘
```

## 📥 Installation

### Prerequisites
- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) (recommended) or Arduino IDE
- ESP32 board support installed
- [Blynk.io](https://blynk.io/) account (free plan available)

### Installation Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/JoseHerrero99/micronova-pellets-stove.git
   cd micronova-pellets-stove
   ```

2. **Configure credentials in `src/Config.h`**
   ```cpp
   // WiFi
   static const char* WIFI_SSID = "Your_SSID";
   static const char* WIFI_PASS = "Your_Password";
   
   // Blynk
   #define BLYNK_AUTH_TOKEN "Your_Blynk_Token"
   ```

3. **Adjust GPIO pins if necessary**
   ```cpp
   #define HW_RX_PIN_DEFAULT    33
   #define HW_TX_PIN_DEFAULT    32
   #define HW_EN_RX_PIN_DEFAULT 27
   ```

4. **⚠️ IMPORTANT: Verify RAM addresses for your stove model**
   
   RAM addresses may vary by stove model. This project uses:
   ```cpp
   #define RAM_ADDR_POWER_FEEDBACK 0xB9  // Power feedback
   #define RAM_ADDR_STATE          0x21  // Stove state
   #define RAM_ADDR_COMMAND        0x58  // Command address
   ```
   
   If your stove doesn't respond correctly:
   - Try changing `RAM_ADDR_POWER_FEEDBACK` to `0x34` (used in other models)
   - Use terminal command `ram <address>` to explore
   - Check the [Micronova Protocol](#micronova-protocol-1) section below

5. **Build and upload**
   ```bash
   pio run --target upload
   ```

   Or from PlatformIO IDE: `PlatformIO: Upload`

### Blynk Configuration

1. Create new template with ID: `BLYNK_TEMPLATE_ID`
2. Configure the following Virtual Pins:
   - V0: Stove State (Display - Numeric)
   - V1: Ambient Temperature (Display - °C)
   - V2: Power Level Read (Display - Numeric)
   - V3: Power Level Write (Slider 1-5)
   - V4: On/Off Switch (Switch)
   - V6: Set Timer (Input - Minutes)
   - V7: State String (Display - Text)
   - V8-V20: Scheduler controls (see Config.h for details)

## 🖥️ Usage

### Serial Terminal Control

Connect at 115200 baud and use commands:

```
Main Commands:
  help          - Show complete help
  status        - Current stove status
  on            - Turn stove on
  off           - Turn stove off
  power <1-5>   - Set power level
  temp          - Show temperature
  
Scheduler:
  sched_list    - List schedules
  sched_set <idx> <active> <day> <hour> <min> <power>
  
Timer:
  timer set <minutes>
  timer cancel
  timer status
```

### Blynk Control

Use the Blynk mobile app to:
- Turn stove on/off remotely
- Adjust power level
- Set timers
- Schedule automatic starts
- Monitor temperature and status

## 📚 Project Structure

```
micronova-pellets/
├── src/
│   ├── main.cpp              # Entry point and FreeRTOS tasks
│   ├── Config.h              # Global configuration
│   ├── StoveController.{h,cpp}   # Main control logic
│   ├── StoveComm.{h,cpp}         # Hardware UART communication
│   ├── SimStoveComm.{h,cpp}      # Simulator for testing
│   ├── BlynkInterface.{h,cpp}    # Blynk IoT interface
│   ├── Scheduler.{h,cpp}         # Weekly scheduler
│   ├── Terminal.{h,cpp}          # Interactive terminal
│   └── IStoveComm.h              # Abstract interface
├── platformio.ini            # PlatformIO configuration
├── LICENSE                   # GPL-3.0
└── README.md                 # This file
```

## 🔧 Development

### Simulation Mode

For development without real hardware, enable in `platformio.ini`:

```ini
build_flags = -DSIMULATION_MODE
```

Available simulation commands:
```
sim_state <0-6>      # Force state
sim_power <1-5>      # Force power
sim_temp <°C>        # Force temperature
sim_fail             # Enable failure mode
sim_recover          # Recover from failure
```

### Micronova Protocol

The system implements the standard Micronova protocol:
- **Baud Rate**: 1200
- **Configuration**: 8 bits, no parity, 2 stop bits (8N2)
- **Checksum**: Implemented according to Micronova specification

#### Specific RAM Addresses for This Stove

This project has been tested and configured for a specific Micronova stove model. Addresses may **differ** from other projects (such as [philibertc/micronova_controller](https://github.com/philibertc/micronova_controller)).

**RAM addresses used in this stove:**

| Address | Function | Description |
|---------|----------|-------------|
| `0x21` | State | Stove state byte (OFF=0x21, WORKING=0x04, etc.) |
| `0x01` | Temperature | Ambient temperature in Celsius |
| `0xB9` | Power (feedback) | Current power level (1-5) ⚠️ **Differs from original** |
| `0x58` | Commands | Address where commands are written |

**Control Commands (written to address 0x58):**

| Command | Value | Action |
|---------|-------|--------|
| Power + | `0x54` | Increase power level |
| Power - | `0x50` | Decrease power level |
| ON/OFF | `0x5A` | Turn stove on/off |
| Temp + | `0x52` | Increase target temperature (not used) |
| Temp - | `0x58` | Decrease target temperature (not used) |

⚠️ **Important Note**: The power feedback address (`0xB9`) is **different** from the original project (`0x34`). If you have another Micronova stove and it doesn't work, try:
1. Change `RAM_ADDR_POWER_FEEDBACK` from `0xB9` to `0x34` in `Config.h`
2. Use terminal commands `ram <addr>` to discover addresses for your model
3. Verify commands respond correctly

For more details, see the complete documentation in `src/Config.h`.

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the **GPL-3.0 License** - see the [LICENSE](LICENSE) file for details.

**Author**: Jose Herrero Ruiz

This is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
Please **keep this notice and copyright**.

## 🙏 Acknowledgments

- Based on work by [philibertc/micronova_controller](https://github.com/philibertc/micronova_controller)
- ESP32 and Arduino community
- Blynk IoT team

---

<div align="center">

**Made with ❤️ for the pellet stove automation community**

⭐ Star this repo if you find it useful!

</div>