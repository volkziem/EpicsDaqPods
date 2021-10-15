TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = daqpod
# daqpod.dbd will be created and installed
DBD += daqpod.dbd

# daqpod.dbd will be made up from these files:
daqpod_DBD += base.dbd

# Include dbd files from all support applications:
daqpod_DBD += asyn.dbd
daqpod_DBD += stream.dbd
daqpod_DBD += drvAsynIPPort.dbd

# Add all the support libraries needed by this IOC
#daqpod_LIBS += xxx

# daqpod_registerRecordDeviceDriver.cpp derives from daqpod.dbd
daqpod_SRCS += daqpod_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
daqpod_SRCS_DEFAULT += daqpodMain.cpp
daqpod_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#daqpod_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
daqpod_LIBS += $(EPICS_BASE_IOC_LIBS)
daqpod_LIBS += asyn
daqpod_LIBS += stream

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

