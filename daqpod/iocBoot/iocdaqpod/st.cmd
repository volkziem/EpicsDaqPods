#!../../bin/linux-arm/daqpod

## You may have to change daqpod to something else
## everywhere it appears in this file

< envPaths

epicsEnvSet(STREAM_PROTOCOL_PATH,"../../daqpodApp/Db")

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/daqpod.dbd"
daqpod_registerRecordDeviceDriver pdbbase

## Load record instances

# MCP23017 multi-IO
# drvAsynIPPortConfigure("SOCKET55","192.168.20.55:1137",0,0,0)
# dbLoadRecords("db/MCP23017.db","PORT='SOCKET55',USER='MCP23017'")

# MPU9250 Accelerometer+Gyro+magnetic field
drvAsynIPPortConfigure("SOCKET55","192.168.20.55:1137",0,0,0)
dbLoadRecords("db/MPU9250.db","PORT='SOCKET55',USER='MPU9250'")

# BME680 Environmental sensor
# drvAsynIPPortConfigure("SOCKET56","192.168.20.56:1137",0,0,0)
# dbLoadRecords("db/BME680.db","PORT='SOCKET56',USER='BME680'")

# MCP4922 DAC
#drvAsynIPPortConfigure("SOCKET178","192.168.20.178:1137",0,0,0)
#dbLoadRecords("db/MCP4922.db","PORT='SOCKET178',USER='MCP4922'")

# MCP4922 ADC
# drvAsynIPPortConfigure("SOCKET105","192.168.20.105:1137",0,0,0)
# dbLoadRecords("db/MCP3302.db","PORT='SOCKET105',USER='MCP3302'")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=pi"
