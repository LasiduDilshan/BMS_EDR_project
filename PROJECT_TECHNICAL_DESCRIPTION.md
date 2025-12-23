# Battery Management System (BMS) for Electric Dirt Racing (EDR) - Technical Description

## Project Overview
Custom-designed Battery Management System (BMS) for high-performance electric vehicle applications, specifically developed for Electric Dirt Racing. This is a comprehensive embedded systems project encompassing hardware design, firmware development, and system integration with advanced monitoring and control capabilities.

---

## System Architecture

### Multi-Tier Distributed Architecture
- **Modular Cell Monitoring System**: Distributed architecture with individual monitoring modules for each battery cell
- **Centralized ESP32 Controller**: Main control unit with web-based interface, cloud connectivity, and real-time monitoring
- **Serial Communication Bus**: Custom isolated RS-485 serial protocol for reliable inter-module communication (2400-10000 baud configurable)
- **Scalability**: Supports up to 128 individual cell modules per controller
- **Fail-Safe Design**: Independent cell-level monitoring with watchdog timers and redundant safety mechanisms

---

## Hardware Design & Electronics

### 1. Cell Module PCB (ATTINY-based)
**Multiple Hardware Revisions Designed:**
- V4.0, V4.1, V4.2, V4.21, V4.3, V4.4, V4.5 with progressive improvements
- V4.21 LTO variant for Lithium Titanate batteries (round PCB design)

**Microcontroller Options:**
- ATTINY841 (8-bit AVR @ 2MHz, early versions)
- ATTINY1624 (12-bit ADC, latest V4.5 design @ 5MHz)

**Technical Specifications:**
- High-precision voltage measurement (±2mV accuracy with calibration)
- Dual temperature sensing (internal & external NTC thermistors)
- Active cell balancing with PWM-controlled bypass resistors
- Load resistance: 3.30Ω to 4.96Ω (depending on version)
- Power dissipation management with temperature-controlled PID loop
- Multiple baud rate support (2400, 5000, 9600, 10000 baud)

**Key Features:**
- 12-bit ADC for voltage measurement (V4.5)
- Hardware sample averaging
- Brown-out detection (1.8V)
- Fuse protection and TVS diodes (V4.4+)
- AZ432ANTR-E1 voltage reference for precision measurements
- External crystal oscillator for reliable communications (V4.4)

### 2. Main Controller PCB (ESP32-based)
**Core Platform:**
- ESP32 DevKit-C (Dual-core Xtensa LX6 @ 240MHz)
- 4MB Flash memory with custom partition scheme
- LittleFS filesystem for web interface storage

**Integrated Peripherals:**
- ILI9341 TFT Display (320x240, SPI @ 40MHz with DMA)
- INA229 High-Precision Current/Power Monitor (20-bit, 85V, SPI interface)
- CAN Bus interface (TWAI protocol)
- RS-485 transceiver for cell module communication
- MicroSD card slot for data logging
- Multiple relay outputs for load/charge control
- IO expander chips (TCA6408A, TCA9534, TCA6416A) for GPIO expansion

**Communication Interfaces:**
- WiFi (802.11 b/g/n)
- Ethernet support
- CAN Bus (250k/500k baud)
- RS-485 serial bus
- UART for debugging (115200 baud)

### 3. External Temperature Module PCB
- Additional temperature monitoring capability
- Daisy-chain compatible with cell modules
- NTC thermistor-based measurement

### 4. Mechanical Design (SolidWorks)
- Custom enclosures for cell modules
- Thermal management considerations
- Assembly drawings and 3D models (.SLDASM, .SLDPRT)
- STEP file exports for manufacturing
- Injection molding designs

---

## Firmware & Software Development

### 1. Cell Module Firmware (C/C++)
**Development Environment:**
- PlatformIO with Arduino framework
- Platform: Atmel megaAVR / AVR
- ~1000 lines of embedded C/C++ code

**Core Functionality:**
- Real-time voltage monitoring with oversampling
- Dual temperature measurement (internal PCB & external sensor)
- Active balancing algorithm with PID control
- Serial packet processing with CRC validation
- EEPROM storage for configuration
- Watchdog timer implementation
- Low-power optimization

**Key Algorithms:**
- FastPID library integration for thermal management
- Temperature-dependent balancing control
- B-coefficient thermistor calculations (3950K-4150K)
- Voltage calibration routines
- Bypass current calculation and monitoring

### 2. ESP32 Controller Firmware (C/C++)
**Development Environment:**
- PlatformIO + ESP-IDF 2.0.14
- Arduino framework for ESP32
- Platform: Espressif32 @ 6.4.0
- ~15,400 lines of C/C++ code

**Major Modules:**
- **PacketReceiveProcessor**: Handles incoming data from cell modules
- **PacketRequestGenerator**: Manages polling and data requests
- **Rules Engine**: Safety rule evaluation and enforcement
- **Web Server**: Full-featured HTTP server with REST API
- **MQTT Client**: IoT integration and cloud connectivity
- **InfluxDB Integration**: Time-series data logging
- **CAN Bus Protocols**: Victron, Pylontech, Pylon Force H2 emulation
- **Current Monitoring**: INA229 driver with energy calculation
- **TFT Display**: Real-time status visualization
- **AVRISP Programmer**: Over-the-air firmware updates for cell modules

**Key Features:**
- Multi-bank battery support (up to 16 banks)
- Real-time rule-based safety system (16 configurable rules)
- Web-based configuration interface with multi-language support (EN, DE, ES, FR, HR, NL, PT, RU)
- Data visualization with ECharts library
- OTA (Over-The-Air) firmware updates
- SD card data logging
- NTP time synchronization
- mDNS service discovery
- RESTful API for external integration
- Home Assistant integration support

### 3. Web Interface
**Technologies:**
- HTML5/CSS3/JavaScript
- jQuery for DOM manipulation
- ECharts for real-time graphing
- Responsive design
- AJAX-based communication
- Multi-language support

**Features:**
- Real-time cell voltage monitoring
- Temperature visualization
- State-of-Charge (SoC) calculation and display
- Battery statistics and history
- Configuration management
- Rule configuration and status
- Current/power monitoring dashboard
- Alert and notification system

### 4. Build System & Automation
**Python Build Scripts:**
- Automated versioning (`buildscript_versioning.py`)
- Web asset compression (`prebuild_compress.py`)
- Integrity hash generation (`prebuild_generate_integrity_hash.py`)
- Embedded file generation (`prebuild_generate_embedded_files.py`)
- Binary file generation for bootloader
- BMP to array conversion for graphics

---

## Communication Protocols

### 1. Cell Module Serial Protocol
- Custom binary packet format with SerialEncoder library
- CRC error detection
- Daisy-chain topology (controller → module 1 → module 2 → ... → controller)
- Configurable baud rates (2400-10000 baud)
- Packet size: 8 bytes + PacketStruct
- Maximum 16 modules per packet
- Watchdog timeout detection

### 2. CAN Bus Protocols
**Supported Standards:**
- **Victron Energy**: VE.Can protocol emulation with DVCC support
- **Pylontech**: Battery protocol compatibility
- **Pylon Force H2**: High-voltage battery protocol
- **Generic Inverter**: Standard battery management protocol
- **DEYE Inverter**: Specific inverter compatibility

**CAN Bus Features:**
- Configurable baud rate (250k/500k)
- Battery voltage and current reporting
- SoC (State of Charge) broadcasting
- Charge/discharge limits communication
- Alarm and warning propagation
- DVCC (Distributed Voltage and Current Control) support

### 3. Network Protocols
- **MQTT**: Publish/subscribe for IoT integration
- **HTTP/HTTPS**: Web server and REST API
- **mDNS**: Service discovery
- **NTP**: Network time synchronization
- **InfluxDB**: Time-series database integration

---

## Advanced Features & Algorithms

### 1. Battery Management
- **Cell Balancing**: Active PWM-controlled resistive balancing with temperature feedback
- **SoC Calculation**: Coulomb counting with voltage-based correction
- **SoH (State of Health) Monitoring**: Capacity tracking over time
- **Temperature Compensation**: Voltage readings adjusted for temperature
- **Charge Profile Management**: Standard, Absorb, Float, Dynamic charging modes
- **Current Integration**: Real-time Ah counting with configurable efficiency

### 2. Safety Systems
**16 Configurable Safety Rules:**
1. Emergency Stop
2. BMS Error Detection
3. Overcurrent Protection
4. Module Overvoltage Protection
5. Module Undervoltage Protection
6. Over-temperature (Internal)
7. Under-temperature (Internal)
8. Over-temperature (External)
9. Under-temperature (External)
10. System Overvoltage
11. System Undervoltage
12. Bank Overvoltage
13. Bank Undervoltage
14. Bank Voltage Range Monitoring
15. Timer-based Controls (2 timers)

**Additional Safety Features:**
- Watchdog timers at multiple levels
- CRC validation on all serial communications
- Brown-out detection
- Module count verification
- Zero-volt module detection
- Communication error detection
- Inconsistency detection (voltage, temperature, firmware version)
- Relay control for charge/discharge isolation

### 3. Monitoring & Diagnostics
- **Real-time Metrics**: Voltage, current, power, temperature, SoC
- **Historical Data**: Min/max values, charge/discharge cycles
- **Error Logging**: Comprehensive error codes and warnings
- **Performance Metrics**: Module response times, CRC error counts
- **Thermal Management**: Temperature-based derating and protection

### 4. Power Management
- **INA229 Current Monitor**: 20-bit precision, ±85V range
- **Shunt Calibration**: Configurable shunt resistance and max current
- **Energy Calculation**: Wh consumed/generated
- **Power Limits**: Configurable charge/discharge power limits
- **Voltage Limits**: Per-cell and system-level limits
- **Temperature Derating**: Automatic current limiting at high temperatures

---

## Development Tools & Workflow

### Version Control & Collaboration
- Git version control
- GitHub repository hosting
- Multiple hardware revisions tracked
- Incremental firmware development

### Programming & Debugging
- JTAG2UPDI programmer for ATTINY (UPDI interface)
- USB-ASP programmer support
- ESP32 USB serial programming
- Remote firmware updates via AVRISP protocol
- Serial debugging with ESP32 exception decoder

### Testing & Validation
- Multiple test programs for hardware validation
- PID tuning utilities
- Blinky test programs for hardware verification
- ESP32 board test suite

---

## PCB Manufacturing Details

### Design Tools
- Altium Designer (.PrjPcb, .PcbDoc, .SchDoc files)
- KiCad (reference designs)
- JLCPCB manufacturing-ready designs

### Manufacturing Specifications
- 2-layer PCB design
- SMD components (0603, 0805 package sizes)
- Machine assembly compatible
- Design Rule Check (DRC) files included
- Bill of Materials (BOM) documentation
- Gerber files generation
- STEP file exports for 3D verification

---

## Technical Specifications Summary

### Cell Module
- **Voltage Range**: 0-5V (configurable with resistor divider for LTO cells)
- **Voltage Accuracy**: ±2mV after calibration
- **Temperature Range**: -40°C to +85°C (measurement)
- **Balancing Current**: Up to ~1A (depending on cell voltage and load resistance)
- **Communication**: RS-485, 2.4k-10k baud
- **Power Consumption**: <50mW idle, variable during balancing
- **Microcontroller**: ATTINY841/1624
- **ADC Resolution**: 10-bit (841) / 12-bit (1624)

### Controller Module
- **Supported Cells**: Up to 128 cell modules
- **Banks**: Up to 16 independent battery banks
- **Current Measurement**: ±320A typical (configurable with shunt)
- **Voltage Range**: 0-85V (INA229 monitor)
- **Current Accuracy**: 20-bit (±0.0001%)
- **Display**: 320x240 TFT touchscreen
- **WiFi**: 802.11 b/g/n
- **CAN Bus**: 250k/500k baud
- **Storage**: MicroSD card support + 4MB flash
- **Logging Rate**: Configurable (seconds to minutes)
- **Web Server**: Embedded HTTP server with WebSocket support

### System Performance
- **Scan Rate**: All modules polled every 2-5 seconds
- **Response Time**: <100ms per module
- **Data Retention**: EEPROM storage on both controller and modules
- **Update Rate**: Display updates 1-2 Hz, web interface real-time
- **Reliability**: CRC error checking, watchdog timers, redundant safety checks

---

## Project Deliverables

### Documentation
- Design documentation (PDF format)
- Photographic evidence of builds
- Individual contribution reports
- Programming hardware guides
- Home Assistant API integration guide
- Schematic diagrams (multiple formats)
- Datasheet references

### Source Code
- Complete firmware source code for ATTINY modules
- Complete firmware source code for ESP32 controller
- Build scripts and automation tools
- Web interface source files
- Configuration files

### Hardware Files
- PCB design files (multiple versions)
- Schematic captures
- BOM (Bill of Materials)
- Assembly drawings
- 3D mechanical models (SolidWorks)
- Manufacturing files (Gerber, drill files)

---

## Development Skills Demonstrated

### Embedded Systems
- Bare-metal AVR programming
- ESP32/FreeRTOS development
- Real-time systems design
- Interrupt-driven programming
- Watchdog timer implementation
- Power management optimization

### Electronics & Hardware Design
- Analog circuit design (precision voltage measurement)
- Digital circuit design
- PCB layout (2-layer, SMD)
- Component selection and specification
- Thermal management design
- Signal integrity considerations
- EMI/EMC compliance design

### Firmware Development
- C/C++ embedded programming (16K+ LOC)
- PID control algorithms
- State machine implementation
- Serial communication protocols
- CRC error detection
- EEPROM data management
- Bootloader development

### Software Engineering
- Object-oriented design
- Modular architecture
- RESTful API design
- Web server implementation
- Database integration (InfluxDB)
- MQTT protocol implementation
- Multi-threaded programming

### Build Systems & DevOps
- PlatformIO build system
- Python build automation
- Version control (Git)
- Continuous integration concepts
- OTA update mechanisms

### Communication Protocols
- RS-485 serial communication
- CAN Bus (TWAI)
- I2C peripheral communication
- SPI high-speed interfaces
- UART debugging
- Network protocols (TCP/IP, HTTP, MQTT, mDNS)

### User Interface Development
- HTML/CSS/JavaScript
- Responsive web design
- Real-time data visualization
- Multi-language support
- AJAX/REST integration

### Testing & Validation
- Hardware validation procedures
- Firmware testing methodologies
- Integration testing
- System-level testing
- Performance benchmarking

---

## Key Technical Achievements

1. **Distributed Monitoring System**: Successfully designed and implemented a scalable, daisy-chained monitoring architecture supporting up to 128 cells

2. **High-Precision Measurement**: Achieved ±2mV voltage measurement accuracy across wide temperature ranges using calibration algorithms

3. **Active Balancing with Thermal Management**: Implemented PID-controlled cell balancing with real-time temperature monitoring to prevent thermal runaway

4. **Multi-Protocol CAN Bus Integration**: Developed compatibility layers for multiple industry-standard inverter protocols (Victron, Pylontech)

5. **Web-Based Monitoring**: Created full-featured web interface with real-time visualization accessible from any device

6. **Comprehensive Safety System**: Implemented 16 configurable safety rules with fail-safe defaults and emergency stop capabilities

7. **Over-the-Air Updates**: Built AVRISP programmer functionality allowing remote firmware updates to distributed cell modules

8. **Multi-Bank Support**: Designed system to independently monitor and manage up to 16 separate battery banks simultaneously

9. **IoT Integration**: Implemented MQTT, InfluxDB, and Home Assistant integration for cloud connectivity and automation

10. **Power Monitoring**: Integrated 20-bit precision current monitoring with energy calculation and SoC tracking

---

## Application Domain
- **Electric Vehicles**: Primary application for Electric Dirt Racing
- **Energy Storage Systems**: Grid-tied and off-grid battery systems
- **Solar Energy Storage**: Integration with solar inverters via CAN bus
- **Electric Boats**: Marine battery management applications
- **Portable Power Systems**: High-capacity portable battery packs
- **Research & Development**: Battery testing and characterization platforms

---

## Future Enhancement Possibilities
- CAN FD support for higher bandwidth
- Wireless cell module communication
- Machine learning for SoH prediction
- Advanced fault diagnosis algorithms
- Cell chemistry detection and adaptation
- Battery thermal modeling
- Cloud-based fleet management
- Mobile application development

---

## Technical Complexity Indicators
- **Lines of Code**: 16,000+ (firmware only)
- **Hardware Revisions**: 7+ versions designed and tested
- **Supported Protocols**: 8+ communication protocols implemented
- **Safety Features**: 16 configurable protection rules
- **Scalability**: 128 cell modules per controller
- **Languages**: Embedded C/C++, Python, JavaScript, HTML/CSS
- **Platforms**: AVR 8-bit, ESP32 32-bit dual-core
- **Development Time**: Multi-year iterative development
- **Documentation**: Comprehensive technical documentation with schematics, datasheets, and user guides

---

## Project Impact
This project demonstrates comprehensive embedded systems engineering capabilities spanning:
- Complete product development lifecycle (concept → design → implementation → testing)
- Hardware-software co-design methodology
- Safety-critical systems design
- Real-time embedded systems
- IoT and cloud connectivity
- User interface development
- Manufacturing-ready PCB design
- Professional documentation practices

The system is production-ready and demonstrates real-world application in high-performance electric vehicle racing environments where reliability, safety, and performance are critical.
