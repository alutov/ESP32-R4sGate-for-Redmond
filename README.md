# ESP32 Ready4Sky (R4S) Gateway for Redmond Kettle

The file fr4sGate.bin in build folder is already assembled binary for esp32 with 4Mb memory is flashed with a single file from the address 0x0000 to a clean esp32.<br> 
You can also use three standard files for flashing instead: bootloader.bin (addr 0x1000), partitions.bin (addr 0x8000) and r4sGate.bin (addr 0x10000).
Then you need to create a guest wifi network in the router with the ssid "r4s" and password "12345678", wait until esp32 connects to it then enter esp32<br>
IP address in web browser and set the remaining parameters. After which the guest network is no longer needed.<br>
Then you must enter Redmond Kettle name to connect to device.
Mqtt topics:<br>
 r4s/devaddr/cmd/state <-- 0/off/false - switch off, 1/on/true - boil, 2...100 - boil&heat;<br>
 r4s/devaddr/cmd/heat_temp <-- 0 - switch off, 1...100 heat;<br>
 r4s/devaddr/cmd/nightlight  <-- 0/off/false - switch off, 1/on/true - switch on;<br>
 r4s/devaddr/rsp/ - current state, temperature, rssi etc.;<br>


# Rebuild notes
Use Espressif IoT Development Framework(https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) to build binary files.<br>
