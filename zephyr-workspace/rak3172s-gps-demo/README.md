# RAK3172 GNSS Setup

A simple dual-UART setup for RAK3172 (STM32WLE5CC) to communicate with Quectel LC86G GPS module using Zephyr RTOS.

## Description

This project enables GPS/GNSS functionality on the RAK3172 development board by configuring two separate UARTs:
- **USART1 (PB6/PB7)**: Console output for debugging
- **USART2 (PA2/PA3)**: GPS module communication

The setup reads NMEA sentences from the GPS module and displays them on the console for testing and development.

## Prerequisites

### Hardware
- RAK3172 development board (STM32WLE5CC)
- Quectel LC86G GPS module
- USB-to-Serial adapter (FTDI, CP2102, etc.)
- GPS antenna
- Jumper wires

### Software
- [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) v0.17.1 (toolchain)
- [Zephyr OS](https://github.com/zephyrproject-rtos/zephyr) v4.1.99+ 
- [West tool](https://docs.zephyrproject.org/latest/develop/west/index.html)
- Python 3.10+
- Git

## Zephyr OS Setup

### 1. Install Zephyr SDK (Toolchain)
```bash
# Download and install Zephyr SDK v0.17.1 (or compatible version)
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.1/zephyr-sdk-0.17.1_linux-x86_64.tar.gz
tar xvf zephyr-sdk-0.17.1_linux-x86_64.tar.gz
cd zephyr-sdk-0.17.1
./setup.sh
```

### 2. Create Zephyr Workspace
```bash
# Install west
pip3 install west

# Initialize workspace (gets latest Zephyr OS)
west init zephyr-workspace
cd zephyr-workspace

# Update Zephyr and modules  
west update

# Install Python dependencies
pip3 install -r zephyr/scripts/requirements.txt
```

### 3. Set Environment Variables
```bash
# Add to ~/.bashrc or ~/.zshrc
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=/path/to/zephyr-sdk-0.17.1

# Source Zephyr environment (from workspace directory)
source zephyr/zephyr-env.sh
```

## Project Setup

### 1. Clone or Create Project
```bash
# Create new project directory
mkdir rak3172-gps-demo
cd rak3172-gps-demo

# Copy required files:
# - rak3172.overlay (devicetree configuration)
# - prj.conf (Kconfig settings)
# - src/main.c (application code)
```

### 2. Hardware Connections

**Console Connection (USART1)**:
```
RAK3172 PB6 (TX) → USB-to-Serial RX
RAK3172 PB7 (RX) → USB-to-Serial TX
RAK3172 GND      → USB-to-Serial GND
```

**GPS Connection (USART2)**:
```
RAK3172 PA2 (TX) → LC86G RX
RAK3172 PA3 (RX) → LC86G TX
RAK3172 GND      → LC86G GND
RAK3172 3V3      → LC86G VCC
```

## Build and Flash

```bash
# Build for RAK3172
west build -b rak3172 --pristine

# Flash to board
west flash

# Alternative flashing with specific programmer
west flash --runner pyocd
```

## Usage

1. **Connect console**: Open terminal to USB-to-Serial adapter at 115200 baud
2. **Power on**: Connect RAK3172 to power source
3. **Attach GPS**: Connect LC86G module with antenna
4. **Monitor output**: Console will show GPS NMEA sentences when satellites are acquired

### Expected Console Output
```
<inf> gps_test: RAK3172 GPS Test - Console: USART1, GPS: USART2
<inf> gps_test: GPS UART ready. Connect LC86G to PA2/PA3
<inf> gps_test: GPS: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
<inf> gps_test: GPS: $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
```

## Key Configuration Notes

- **STM32WLE5 UART naming**: Use `usart1`, `usart2`, `lpuart1` (not `uart1`)
- **Console reliability**: USART1 (PB6/PB7) works better than LPUART1 for console
- **UART driver**: Use `CONFIG_UART_INTERRUPT_DRIVEN=y` (not async API for STM32)
- **GPS acquisition**: Requires clear sky view and may take 30+ seconds for first fix

## Troubleshooting

**No console output**: Check USB-to-Serial connection to PB6/PB7 at 115200 baud  
**No GPS data**: Verify LC86G power, antenna connection, and clear sky view  
**Build errors**: Ensure devicetree uses correct USART node names  
**Flash errors**: Try different programmer or check SWD connections  

## References

- [Official Zephyr GNSS Sample](https://docs.zephyrproject.org/latest/samples/drivers/gnss/README.html)
- [Jerónimo Agulló's Zephyr-GNSS-Sample](https://github.com/jeronimoagullo/Zephyr-GNSS-Sample)
- [RAK3172 Documentation](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3172-Module/)

## License

This project is open source and available under the [Apache 2.0 License](LICENSE).