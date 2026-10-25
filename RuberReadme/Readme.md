MENG4030 Embedded Parallel Programming Sprint Project- SPRINT Summaries Ruber Castro N01592974 2026/09/21 - 2026/10/23
--------------------------------
SPRINT 1
--------------------------------
The objective of Sprint 1 was to get familiarity with the STM32 coding enviroment, including CubeMX and
stm32CubeIDE. This sprint was separated into 5 different sections:
1. Install stm32 software.
2. Create a basic CubeMX GPIO pheripheral configuation and make a basic bare metal code to implement the blinking of an LED.
3. Develop a bare metal code to transfer data through STM32 Uart port, and configure its ports.
4. Configure CubeMX to use RTOS and transfer LED code from Bare metal to an RTOS task.
5. Combine all the knowledge and Keep the same LED RTOS task, but now introduce a new task to transfer LED state as an UART messaga by creating another task.
   
Sprint 1 began with the installation of the required STM32 development software, including STM32CubeIDE and STM32CubeMX. Next, STM32CubeMX was used to configure the GPIO peripherals, followed by the development of a basic bare-metal program using HAL functions to blink the STM32 Nucleo board's onboard LD2 LED, using the resorce video as a guide provided by the faculty. Furthermore, I configured the UART periphial in CubeMX to stablich serial communication between the microcontroller and a PC, this meaning stablishing baud rate, stop bits, and pararity, allowing the text "Hello World" to be transmitted. Then, the next step was to transfer the bare-metal code to an RTOS-based code,  configuring FreeRTOS and transferring the LED blinking functionality into a dedicated task, again following initial RTOS guide to create RTOS task from CubeMX. Finally, I introduced a second task to monitor the LED state and transmit corresponding "LED ON" and "LED OFF" messages through UART while the original LED task continued operating independently. 

***Testing and verification***

Testing and verification for this sprint was to observe the onboard LD2 LED to confirm the blinking functionality and using Tera Term as a serial terminal to receive and monitor UART messages from the STM32. No complications or challenges at this point


--------------------------------
SPRINT 2
--------------------------------
The objective of Sprint 2 was to gain familiarity with sensor integration, data acquisition over a communication protocol in our case I2C, and real-time data processing using FreeRTOS. This sprint was separated into 5 different sections:

1. Read sensor documentation, to determine register configuration and required parameters needed for our application for both the temperature sensor and our load cell sensor ADC. 
2. Implement bare metal code and an RTOS task to read sensor data and transmit the measurements through UART for monitoring, this time by manually creating them in the code and not using CubeMX to generate code for us.
3. Create a project code but this with the project's 1 kg load cell and NAU7802 ADC, requiring additional I2C communication and device configuration. Get Readings from sensor and transmitt then over UART.

Sprint 2 began with the integration of a basic TMP3 temperature sensor to become familiar with sensor data acquisition and peripheral configuration in STM32CubeMX. This sensor was initially selected due to its simpler implementation, which required basic I2C configuration such as ID and register number for data acquisition, I implemented the code in bare metal using simple HAL functions, and then trasnsition them to RTOS Task. Moreover, I transitioned to the sensor selected for our capstone project, a 1 kg load cell connected to an NAU7802 24-bit ADC. This required additional configuration since the NAU7802 requires specific register settings for initialization and data acquisition, this requiered further register configuration such as resolution, sensor reset upon power up, calibration registers, etc. To simplify the implementation, I developed a dedicated sensor driver consisting of 'nau7802.h' and 'nau7802.c', containing reusable functions for I2C register communication, ADC initialization, and reading raw sensor values. This allowed the main program to acquire measurements by calling predefined functions without repeatedly handling low-level register operations. Finally, I integrated the sensor reading function with UART and made both of them an RTOS task and tested sensor response, following similar steps from the temperature sensor code.

***Testing and verification***

We tested the sensors reading by appliying preassure to the load cell and observe its change through the UART port using teraterm as our visual terminal. This sprint required more documentation reading and understading what each register did in order to understand how to handle our sensor, unlike the temperature sensor which had only 2 registers to configure this new NAU sensor had about 20 parameters the could be configured. We initially tried to recive data using regualr lines of code, but soon realized that it was very unefficent and messy, hence why we created a driver to avoid having to repeat the same 10 lines over and over again. At the end, we learned about how importand documentation is and clearly appreciated the use of developing drivers to have an easier maintanable code.

--------------------------------
SPRINT 3
--------------------------------
The objective of Sprint 3 was to expand the previous sensor acquisition system by introducing a dedicated control task responsible for processing sensor measurements and generating control decisions based on the current system state. Additionally, this sprint focused on understanding RTOS multitasking and context switching. This sprint was separated into 2 different sections:

1. Continue with previous sprint2 code and create a new control task using the manual FreeRTOS.
2. Implement FreeRTOS context-switching diagnostics using a trace hook and UART messages to identify which task is being executed by the microcontroller.

Sprint 3 began by cloning the previous sprint's project to retain the existing load-cell driver, I2C communication. Next, I introduced a new FreeRTOS task called 'MonitorTask', which acts as the control task of our system, this task processed the weight measurements acquired by 'LoadCellTask' through a shared global variable and compares them against a reference weight recorded using an external push-button. Based on this comparison, the control logic determines the current inventory status, assigning state 0 for normal operation, state 1 when the remaining weight falls below 20% of the reference value, and state 2 when it falls below 10%. In addition, I modified the existing 'sendmessagetask', the task responsable to communicate sensor reading data to UART to respond to these states, functioning as a temporary representation of the system's actuator. At this stage, instead of controlling a physical actuator, the task generated corresponding UART messages, including the current weight, "RUNNING LOW", or "RAN OUT", depending on the control decision. Laslty, I implemented a temporary 'tracetaskswitch' function and the help of RTOS middleware to capture the name of the currently scheduled task throught the same 'sendmessagetask' function using UART to visually capture the change between the scheduled task in our teraterm terminal, giving us context switching of tasks.

***Testing and verification***

The testing for Sprint 3 was initially performed by using an external push-button to record a reference weight for the inventory monitoring system. However, slight imbalances and fluctuations in the load-cell readings made it difficult to accurately detect the percentage thresholds. To address this issue, we added an additional push-button (On board tactile button) to tare the sensor, allowing the weight reading to be reset to zero before applying and recording a target weight. The system was then tested by applying varying amounts of pressure to the load cell to simulate changes in inventory. Since the control logic was based on percentages of the reference weight, the corresponding 20% and 10% threshold values were calculated beforehand and confirmed using Tera Term to monitor the UART output, we confirmed that the system correctly transitioned between normal operation, running low, and empty states according to the programmed percentage thresholds.

--------------------------------
SPRINT 4
--------------------------------
