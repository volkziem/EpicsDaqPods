# EpicsDaqPods
We describe a system, based on connecting various integrated circuits
to an ESP01 microcontroller, to flexibly interface analog and digital
channels to an EPICS control system.

Detailed explanations and schematics are available from
 - V. Ziemann , *EPICS Data Acquisition Pods*, FREIA Report 2021-04, October 2021,
   [(link)](http://urn.kb.se/resolve?urn=urn%3Anbn%3Ase%3Auu%3Adiva-456234).
  

# The pods with sensor connected to ESP01
Contents of the different subdirectories

- **esp01-18b20:** Temperature pod with DS18b20

- **esp01-MCP3302:** ADC pod with MCP3302

- **esp01-MCP4922:** DAC pod with MCP4922

- **esp01-MPU9250:** Magnetic, accelerometer and gyro pod with  a MPU-9250 

- **esp01-MLX90393:** Magnetic sensor with a MLX90393

- **esp01-BME680:** Environmental pod with BME280 or BME680

- **esp01-MCP23017:** Digital-IO pod with MCP23017

- **esp01-bare-trigger:** Asynchronously triggered IO pin with a bare ESP01

# The base station with the IOC controller
Uses a Raspberry Pi that spans its own WLAN network to which the pods connect. The Raspi also
runs the EPICS base package and publishes the process variables with the data from the pods on
its wired network interface.

The **daqpod** subdirectory contains the EPICS IOC with all prototype and database files, as well as st.cmd. 
