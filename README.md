# EpicsDaqPods
We describe a system, based on connecting various integrated circuits
to an ESP01 microcontroller, to flexibly interface analog and digital
channels to an EPICS control system.

# The pods with sensor connected to ESP01

- Temperature pod
with DS18b20

-  ADC pod
with MCP3302

- DAC pod
with MCP4922

- Magnetic, accelerometer and gyro pod
uses a MPU-9250 and another one with a MLX90393

- Environmental pod
with BME280 or BME680

- Digital-IO opd
with MCP23017

- Asynchronously triggered IO pin
with a bare ESP01

# The base station with the IOC controller
Uses a Raspberry Pi that spans its own WLAN network to which the pods connect. The Raspi also
runs the EPICS base package and publishes the process variables with the data from the pods on
its wired network interface.
