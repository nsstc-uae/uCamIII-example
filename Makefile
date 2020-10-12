# ****************************************************************************/
# Copyright 2019-2020 National Space Science and Technology Center, UAE      */
#                                                                            */
# Project: ucamIII                                                           */
#                                                                            */
# ****************************************************************************/

CC=gcc
OUTPUT=output
UCAM_PROJECT_PATH=.

UCAM_PROJECT_INCLUDE_PATH=-I$(UCAM_PROJECT_PATH)/include
UCAM_PROJECT_OBJ_NAME=uCamIII

UCAM_PROJECT_SRC=$(UCAM_PROJECT_PATH)/$(UCAM_PROJECT_OBJ_NAME).c
UCAM_PROJECT_PATH_OUTPUT=$(UCAM_PROJECT_PATH)/$(OUTPUT)
UCAM_PROJECT_OBJ=$(UCAM_PROJECT_PATH_OUTPUT)/$(UCAM_PROJECT_OBJ_NAME).o
UCAM_PROJECT_EXE=$(UCAM_PROJECT_PATH_OUTPUT)/$(UCAM_PROJECT_OBJ_NAME)

$(UCAM_PROJECT_PATH_OUTPUT):
	mkdir $@

uCamIIIcompile: $(UCAM_PROJECT_SRC) $(UCAM_PROJECT_PATH_OUTPUT)
	$(CC) $(UCAM_PROJECT_SRC) $(UCAM_PROJECT_INCLUDE_PATH) -o $(UCAM_PROJECT_EXE) -lm

clickImage: uCamIIIcompile
	$(UCAM_PROJECT_EXE)