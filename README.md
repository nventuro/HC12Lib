HC12Lib
=======

A C library for Freescale's CodeWarrior containing interrupt-driven drivers for different peripherals of the MC9S12XDP512 microcontroller and some devices that use them.
It offers support for:
  - RTI (Real Time Interrupt)
  - I2C
  - SPI
  - Input Capture / Output Compare
  - PWM
  - Analog to Digital Conversion
  - A Hitachi HD44780 based LCD
  - An MPU-6050 motion sensor (gyroscope + accelerometer)
  - An HC-SR04 ultrasonic distance sensor
  - A DS1307 RTC (Real Time Clock)
  - Generic RF transmitters and receivers (315 / 433 MHz)
  - Nordic Semiconductor's nRF24L01+ 2.4 GHz transceivers
  - An infrared remote control using the RC-5 protocol
  
  There's also specialized modules that use some of the above peripherals, such as a Battery module that samples a number of batteries voltages and informs of low battery conditions, an analog 4-axis joystick, and an interface for remotely controlling a quadrotor using said joystick, printing information on an LCD screen and using the nRF to transmit the sampled data.