# Endeavr AV hardware

Repository for Endeavr AV external hardware containing CAD files, firmware, electrical diagrams, and schematics.

## Wiring
___
The Endeavr hardware consists of xxx major components:

### CAN Junction

This unit connects the rest of the hardware and vehicle CAN bus to the Panda OBD2 interface. It contains a 3-pin power connector with pins labeled A, B, and C. There are also 2 loose wires for OBD CAN connection. The following diagram shows their connections:

Three-Pin connector:

| LABEL | CONNECTION |
|-------|------------|
| A     |+12V Battery|
| B     |    GND     |
| C     |  +12V IGN  |

Loose wires:

| COLOR |   CONNECTION   |
|-------|----------------|
|  RED  |OBD PIN6 (CANH) |
| BLACK |OBD PIN14 (CANL)|

There are also four RJ45 Ethernet jacks, which connect CAN and ignition signals to the various actuators and control devices

### Steering Actuator

This unit controls the steering motor. Internally, it consists of an Arduino Uno, a CAN shield, an 80A Relay, a 43A H-Bridge, and a few other components. It contains a 3-pin power connector, a 2-pin DC motor connector, and a RJ45 Ethernet connector.

Three-Pin connector:

| LABEL | CONNECTION |
|-------|------------|
| A     |+12V Battery|
| B     |    GND     |
| C     |  +12V IGN  |

Two-Pin connector

| Label |  CONNECTION  |
|-------|--------------|
|   A   |    Motor+    |
|   B   |    Motor -   |

Ethernet connects to one of the Ethernet inputs on the CAN Junction.

### Pedal interceptor
The Pedal inteceptor controls the throttle by spoofing the signal between the PCM and the accelerator pedal.Ethernet connects to the CAN junction.

TODO: document connections to throttle

### Brake Booster Controller
This device sends park assist commands to the installed Tesla Model X iBooster, which replaces the factory vacuum brake booster assembly.

| COLOR  | ETHERNET COLOR |
|--------|----------------|
| YELLOW |   CAN H (Gr)   |
|  BLUE  |  CAN L (GrWh)  |

TODO: document the other connections

### Radar

TODO: document the radar connections