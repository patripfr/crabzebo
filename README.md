<p align="left">
    <img src="https://github.com/patripfr/crabzebo/blob/master/img/crabzebo.png" width="550" />
</p>


# Crabzebo Window Opener
Adapted from Window Blind example at [esp-homekit-demo](https://github.com/maximkulkin/esp-homekit).

## Usage

1. Initialize and sync all submodules (recursively):
```shell
git submodule update --init --recursive
```
2. Copy wifi.h.sample -> wifi.h and edit it with correct WiFi SSID and password.
3. Install [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk), build it with `make toolchain esptool libhal STANDALONE=n`, then edit your PATH and add the generated toolchain bin directory. The path will be something like /path/to/esp-open-sdk/xtensa-lx106-elf/bin. (Despite the similar name esp-open-sdk has different maintainers - but we think it's fantastic!)

4. Install [esptool.py](https://github.com/themadinventor/esptool) and make it available on your PATH. If you used esp-open-sdk then this is done already.
5. Checkout [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos) and set SDK_PATH environment variable pointing to it.
6. Configure settings:
    1. If you use ESP8266 with 4MB of flash (32m bit), then you're fine. If you have
1MB chip, you need to set following environment variables:
    ```shell
    export FLASH_SIZE=8
    export HOMEKIT_SPI_FLASH_BASE_ADDR=0x7a000
    ```
    2. If you're debugging stuff, or have troubles and want to file issue and attach log, please enable DEBUG output:
    ```shell
    export HOMEKIT_DEBUG=1
    ```
    3. Depending on your device, it might be required to change the flash mode:
    ```shell
    export FLASH_MODE=dout
    ```
    (see issue #80)
7. Build example:
```shell
make -C window_covering all
```
8. Set ESPPORT environment variable pointing to USB device your ESP8266 is attached
   to (assuming your device is at /dev/ttyUSB0):
```shell
export ESPPORT=/dev/ttyUSB0
```
9. To prevent any effects from previous firmware (e.g. firmware crashing right at
   start), highly recommend to erase flash:
```shell
    make -C window_covering erase_flash
```
10. Upload firmware to ESP:
```shell
    make -C window_covering test
```
  or
```shell
    make -C window_covering flash
    make -C window_covering monitor
```

