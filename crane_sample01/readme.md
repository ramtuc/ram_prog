# STM32U5G9J-DK2 TBS

The default IDE is set to STM32CubeIDE, to change IDE open the STM32U5G9J-DK2.ioc with STM32CubeMX and select from the supported IDEs (EWARM, MDK-ARM and STM32CubeIDE). Supports flashing of the STM32U5G9J-DK2 board directly from TouchGFX Designer using GCC and STM32CubeProgrammer. Flashing the board requires STM32CubeProgrammer which can be downloaded from the ST webpage.

This TBS is configured for 800 x 480 pixels 16 bpp screen resolution.

Performance testing can be done using the GPIO pins designated with the following signals in Spare GPIO connector on the underside of the board:

- VSYNC_FREQ  - PC0  (CN1 pin 14)
- RENDER_TIME - PC10 (CN1 pin 16)
- FRAME_RATE  - PC11 (CN1 pin 18)
- MCU_ACTIVE  - PC12 (CN1 pin 20)
