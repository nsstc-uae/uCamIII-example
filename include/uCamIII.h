/* ****************************************************************************/
/* Copyright 2019-2019 National Space Science and Technology Center, UAE      */
/*                                                                            */
/* Project: ucamIII Demo			                              */
/*                                                                            */
/* www.nsstc.ae 							      */
/* ****************************************************************************/


#ifndef INCLUDE_UCAMIII_H
#define INCLUDE_UCAMIII_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include<math.h>
#include <stdint.h>

#define BAUNDRATE 	B115200
#define ERROR 		(-1)
#define NO_ERROR 	(0)
#define TRUE 		(1)
#define FALSE 		(0)

#define UART_DEVICE 		"/dev/ucam" //todo: write udev rules corresponding to this, else change this to corresponding /dev/ttyUSBx and take care of permissions
#define DEVNAME_LENGTH 		(100)
#define COMMAND_LENGHT 		(200)

// IDs of command from the manual
#define ID_INITIAL 				(0x01)
#define ID_GET_PICTURE 			(0x04)
#define ID_SNAPSHOT 			(0x05)
#define ID_SET_PACKAGE_SIZE 	(0x06)
#define ID_SET_BAUD_RATE 		(0x07)
#define ID_RESET 				(0x08)
#define ID_DATA 				(0x0A)
#define ID_SYNC 				(0x0D)
#define ID_ACK 					(0x0E)
#define ID_NCK 					(0x0F)
#define ID_LIGHT 				(0x13)
#define ID_SLEEP 				(0x05)
#define ID_CONTRAST_BRIGHTNESS_EXPOSURE (0x14)

// maximum attempts
#define SYNC_MAX_ATTEMPT 		(60) //as per manual
#define GET_PIC_MAX_ATTEMPT 	(10)

// image modes
#define GRAYSCALE_MODE 			(0x03)
#define COLOR16BIT_CrYCbY_MODE 	(0x08)
#define COLOR16BIT_RGB_MODE 	(0x06)
#define JPEG_MODE 				(0x07)

// resolution options for raw image
#define RAW_RES_80_60 		(0x01)
#define RAW_RES_160_120		(0x03)
#define RAW_RES_128_128 	(0x09)
#define RAW_RES_128_96 		(0x0B)

// resolution options for jpeg image
#define JPEG_RES_160_128 (0x03)
#define JPEG_RES_320_240 (0x05)
#define JPEG_RES_640_480 (0x07)

// buffer sizes
#define IMAGE_BUFFER_BYTES 			(512)
#define RAW_IMAGE_MAX_BUFFER_BYTES 	(38400) // COLOR16BIT_CrYCbY_MODE in RAW_RES_160_120 gives the maximum raw image buffer bytes
#define SIZE_OF_CAM_RESPONSE 		(6)
#define SIZE_OF_COMMAND		 		(6)


typedef struct _ucamCommands
{
	uint8_t initial[SIZE_OF_COMMAND];
	uint8_t getPicture[SIZE_OF_COMMAND];
	uint8_t snapshot[SIZE_OF_COMMAND];
	uint8_t setPackageSize[SIZE_OF_COMMAND];
	uint8_t setBaudRate[SIZE_OF_COMMAND];
	uint8_t reSet[SIZE_OF_COMMAND];
	uint8_t data[SIZE_OF_COMMAND];
	uint8_t sync[SIZE_OF_COMMAND];
	uint8_t ack[SIZE_OF_COMMAND];
	uint8_t nak[SIZE_OF_COMMAND];
	uint8_t light[SIZE_OF_COMMAND];
	uint8_t contrastBrightnessExposure[SIZE_OF_COMMAND];
	uint8_t sleep[SIZE_OF_COMMAND];
} ucamCommands;

typedef struct _ucam_t
{
	char devName[DEVNAME_LENGTH];
	FILE *fhRead;
	FILE *fhRead_blocking;
	FILE *fhWrite;
	int32_t fdRead;
	int32_t fdRead_blocking;
	int32_t fdWrite;
	ucamCommands commands;
	uint8_t responseBuffer[SIZE_OF_CAM_RESPONSE];
	uint8_t isInitialized; //todo: make it boolean
}ucam_t;


int8_t uCamInit(ucam_t *cam, uint8_t *devName);
int8_t uCamSync(ucam_t *cam);
int8_t takeJpegPic(ucam_t *ucam, uint8_t jpegResolution, uint8_t *jpegFileName);
int8_t takeRawPic(ucam_t *ucam, uint8_t rawMode, uint8_t rawResolution, uint16_t sizeOfRawData, uint8_t *rawFileName);
int8_t writeCmdToCamAndCheckResponse(ucam_t *ucam, char *commandName, uint8_t *command, char *expectedResponseName, uint8_t *expectedResponse, int32_t noOfBytesTocompare, int32_t waitMiliSeconds);
int8_t writeCmdToCamAndCheckResponseBlocking(ucam_t *ucam, char *commandName, uint8_t *command, char *expectedResponseName, uint8_t *expectedResponse, int32_t noOfBytesTocompare, int32_t waitMiliSeconds);
int8_t readFromCam(ucam_t *ucam, char *expectedResponseName);
void writeCommandToCamera(ucam_t *ucam, char *commandName, uint8_t *command, int32_t waitMiliSeconds);
void uCamInitCommands(ucam_t *cam);
void setCommand(uint8_t command[], uint8_t idNumber, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);
void setCommandParameters(uint8_t command[], uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);


#endif
