MENG4030 Embedded Parallel Programming Sprint Project- SPRINT Summaries Ruber Castro N01592974 2026/09/21 - 2026/10/23
--------------------------------
SPRINT 1
--------------------------------
The objective of Sprint 1 was to get familiarity with the STM32 coding enviroment, including CubeMX and
stm32CubeIDE. This sprint was separated into 5 different sections:
1. Install stm32 software
2. Create a basic CubeMX GPIO pheripheral configuation and make a basic bare metal code to implement the blinking of an LED.
3. Develop a bare metal code to transfer data through STM32 Uart port, and configure its ports.
4. Configure CubeMX to use RTOS and transfer LED code from Bare metal to an RTOS task.
5. Combine all the knowledge and Keep the same LED RTOS task, but now introduce a new task to transfer LED state as an UART messaga by creating another task.
   
Sprint 1 began with the installation of the required STM32 development software, including STM32CubeIDE and STM32CubeMX. Next, STM32CubeMX was used to configure the GPIO peripherals, followed by the development of a basic bare-metal program using HAL functions to blink the STM32 Nucleo board's onboard LD2 LED, using the resorce video as a guide provided by the faculty. Furthermore, I configured the UART periphial in CubeMX to stablich serial communication between the microcontroller and a PC, this meaning stablishing baud rate, stop bits, and pararity, allowing the text "Hello World" to be transmitted. Then, the next step was to transfer the bare-metal code to an RTOS-based code,  configuring FreeRTOS and transferring the LED blinking functionality into a dedicated task, again following initial RTOS guide to create RTOS task from CubeMX. Finally, I introduced a second task to monitor the LED state and transmit corresponding "LED ON" and "LED OFF" messages through UART while the original LED task continued operating independently. 

###Testing and verification
Testing and verification for this sprint was to observe the onboard LD2 LED to confirm the blinking functionality and using Tera Term as a serial terminal to receive and monitor UART messages from the STM32.
