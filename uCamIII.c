/* ****************************************************************************/
/* Copyright 2019-2019 National Space Science and Technology Center, UAE      */
/*                                                                            */
/* Project: ucamIII Demo			                              */
/*                                                                            */
/* www.nsstc.ae 							      */
/* ****************************************************************************/


#include <uCamIII.h>

/* local function */
void printBuffer(uint8_t *buffer, int32_t len);
void configureUart();

/* main function */
int32_t main(int32_t argc, char *argv[])
{
	uint8_t ucamDevPath[20] =  UART_DEVICE; // make sure that you put ucamiii.rules file in /etc/udev/rules.d else change /dev/ucam to /dev/ttyUSB0 or /dev/ttyUSB1 based on the how the ucamIII gets mounted on your system
	ucam_t ucam;
	uint8_t isSynced=FALSE;
	int8_t syncStatus;

	//configure uart
	configureUart();

	//initialize the camera
	uCamInit(&ucam, ucamDevPath);

	//sync the camera
	syncStatus=uCamSync(&ucam);

	/* if camera synced take JPEG pics */
	if (NO_ERROR==syncStatus)
	{
		sleep(2); // As per manual after sync, it requires 2-3 seconds before taking the picture
	
		// read https://github.com/mukesh-nsstc/uCamIII-example#how-to-click-different-modes-and-resolution-of-image to select or create proper function to take raw or jpg image	
		// Note: Call only one instance of functions (takeJpegPic or takeRawPic) to get Image; calling more than one function won't work 

		// to take jpeg picture
		takeJpegPic(&ucam, JPEG_RES_160_128, "jpeg160x128.jpg");
		//takeJpegPic(&ucam, JPEG_RES_320_240, "jpeg320x240.jpg");
		//takeJpegPic(&ucam, JPEG_RES_640_480, "jpeg640x480.jpg");
		// To take raw image un-comment one line below, or  create similar function with proper parameters
		// takeRawPic(&ucam, COLOR16BIT_RGB_FORMAT, RAW_RES_128_128, 38400 /* buffer size corresponding to mode 16-bit Colour (RAW, 565(RGB)) and resolution  128 x 128 */, "rgb128x128.raw");
		// takeRawPic(&ucam, <mode>, <RAW_RES_XXX_XXX>, <buffer size corresponding mode and resolution in above table>, "rawFileName.raw");
	}
 	return 0;
}

/* function definition */
int8_t uCamInit(ucam_t *ucam, uint8_t *devName)
{
	int8_t status=NO_ERROR;
	/* initialize the ucam with all 0s */
	memset(ucam, 0, sizeof(ucam_t));

	/* Copy the device name */
	strncpy(&ucam->devName[0], devName, DEVNAME_LENGTH);
	/* Start with assumption that the initialization failed */
	ucam->isInitialized=FALSE;

	/* open the file handlers */
	ucam->fhWrite=fopen(ucam->devName,"w");
	ucam->fhRead=fopen(ucam->devName, "r");
	ucam->fdWrite=fileno(ucam->fhWrite);
	ucam->fdRead=fileno(ucam->fhRead);

	/* make read option non-blocking */
	int32_t flags = fcntl(ucam->fdRead, F_GETFL, 0);
	fcntl(ucam->fdRead, F_SETFL, flags | O_NONBLOCK);

	//todo:
	ucam->fhRead_blocking=fopen(ucam->devName, "r");
	ucam->fdRead_blocking=fileno(ucam->fhRead_blocking);

	/* if opening of file handler failed, initialization failed */
	if((NULL == ucam->fhWrite) || (NULL == ucam->fhRead) || (ERROR == ucam->fdWrite) || (ERROR == ucam->fdRead) || (NULL == ucam->fhRead_blocking) || (ERROR == ucam->fdRead_blocking))
	{
		status=ERROR;
		ucam->isInitialized=FALSE;
	}
	else
	{
		ucam->isInitialized=TRUE;
	}

	/* initialized commands */
	uCamInitCommands(ucam);

	return status;
}

int8_t uCamSync(ucam_t *ucam)
{

	int32_t status=NO_ERROR;
	int32_t syncAttemptCount = 0;
	uint32_t size_read;
	uint8_t isSynced=FALSE;
	uint8_t ack_cmd_from_camera[SIZE_OF_CAM_RESPONSE] =  {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};

	while(FALSE == isSynced && syncAttemptCount <  SYNC_MAX_ATTEMPT)// todo: change this
	{
		printf("\n *********************** Sync Attempt: %d *************************** \n", syncAttemptCount+1);

		status =  writeCmdToCamAndCheckResponse(ucam, "SYNC", &ucam->commands.sync[0], "ACK", &ack_cmd_from_camera[0], 3, 5000+syncAttemptCount*1000);

		syncAttemptCount++;

		if(NO_ERROR == status)
		{

			/* As per manual, the ack to be after reading sync should have same 4th byte i.e. 3rd index */
			ucam->commands.ack[3]=ucam->responseBuffer[3];

			/* read sync from camera */
			size_read = read(ucam->fdRead, &ucam->responseBuffer[0], SIZE_OF_CAM_RESPONSE);
			printf("Reading: SYNC : ");
			printBuffer(&ucam->responseBuffer[0],SIZE_OF_CAM_RESPONSE);
			if(ucam->responseBuffer[0] != 0xAA || ucam->responseBuffer[1] != 0x0D)
			{
				printf("\nSYNC from camera is incorrect: ");
			}
			else
			{
				writeCommandToCamera(ucam, "Send ACK : ", &ucam->commands.ack[0], 5000);
				usleep(100);

				printf("\n *********************** Sync Is Done: %d *************************** \n\n", syncAttemptCount);
				isSynced=TRUE;
				break;
			}
		}
	}

	// set the return value based on status of sync
	if(FALSE==isSynced)
	{
		status=ERROR;
	}

	return status;
}

int8_t takeJpegPic(ucam_t *ucam, uint8_t jpegResolution, uint8_t *jpegFileName) // Parameter4 corresponds to the resolution
{
	int8_t status=NO_ERROR;
	int32_t getPicTry=0;
	int32_t getPicStatus=ERROR;

	int32_t noOfPackets=0;
	int32_t imageSize=0;
	int32_t fpImageFile;
	int32_t size_write;
	uint8_t imageBuffer[IMAGE_BUFFER_BYTES];
	uint16_t pktSize;

	/* expected responses of commands */
	uint8_t init_cmd_response[6] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00}; 	//Init response
	uint8_t set_package_response[6]= {0xAA, 0x0E, 0x06, 0x00, 0x00, 0x00};
	uint8_t snapshot_cmd_response[6] = {0xAA, 0x0E, 0x05, 0x00, 0x00, 0x00}; 	//get a snapshot immediately
	uint8_t getpic_cmd_response[6] = {0xAA, 0x0E, 0x04, 0x00, 0x00, 0x00}; 	//get picture


	if((JPEG_RES_160_128 != jpegResolution) &&  (JPEG_RES_320_240 != jpegResolution) &&  (JPEG_RES_640_480 != jpegResolution))
	{
		printf("\n The given JPEG resolution: %02X is invalid. Setting the default resolution to: %02X\n", jpegResolution, JPEG_RES_160_128);
		jpegResolution = JPEG_RES_160_128;
	}

	/* initialize init command */
	setCommandParameters(&ucam->commands.initial[0], 0x00, JPEG_MODE, 0x00, jpegResolution);

	//initialize
	status =  writeCmdToCamAndCheckResponseBlocking(ucam, "Init", &ucam->commands.initial[0], "Init Response", &init_cmd_response[0], 3, 20000);

	if(NO_ERROR == status)
	{
		//set package size to IMAGE_BUFFER_BYTES i.e. 512 = 0x0200 i.e. param2  (lower byte) = 0x00 param2  (higher byte) = 0x02 and param1= 0x08 as per manual
		setCommandParameters(&ucam->commands.setPackageSize[0], 0x08, 0x00, 0x02, 0x00);
		status =  writeCmdToCamAndCheckResponseBlocking(ucam, "Set Pkg", &ucam->commands.setPackageSize[0], "Set Pkg Response", &set_package_response[0], 3, 5000);
	}

	if(NO_ERROR == status)
	{
		ucam->commands.snapshot[2]=0x00; // as per manual, for compressed picture, parameter1 is 0x00
		status =  writeCmdToCamAndCheckResponseBlocking(ucam, "Snapshot", &ucam->commands.snapshot[0], "Snapshot Response", &snapshot_cmd_response[0], 3, 5000);
		sleep(1); //todo: less sleep results in failure of next command
	}

	if(NO_ERROR == status)
	{
		while(getPicTry < GET_PIC_MAX_ATTEMPT && ERROR == getPicStatus)
		{
			//get picture
			printf("\n getPicTry : %d\n", getPicTry);
			ucam->commands.getPicture[2]=0x05; //JPEG Picture Mode
			getPicStatus =  writeCmdToCamAndCheckResponse(ucam, "Get Pic", &ucam->commands.getPicture[0], "GetPic Response", &getpic_cmd_response[0], 3, 5000);
			getPicTry++;
			usleep(100);
		}

		if(NO_ERROR == getPicStatus)
		{
			sleep(2);
			readFromCam(ucam, "Data SnapShot");
			// calculate image size
			imageSize=ucam->responseBuffer[3]+ucam->responseBuffer[4]*256;
			// calculate packet size
			noOfPackets=ceil(imageSize*1.0/(IMAGE_BUFFER_BYTES*1.0 - 6.0)); //as per the manual
			printf("\nImage size is: %d, number of packets: %d \n", imageSize, noOfPackets);

			// open file to write the jpg image data
			fpImageFile = open(jpegFileName, O_WRONLY | O_CREAT, 0777);
			if(-1 != fpImageFile)
			{
				for (int16_t pktIx=0; pktIx < noOfPackets; pktIx++)
				{
					// set ack command with packet number
					ucam->commands.ack[4]=(pktIx & 0x00FF);
					ucam->commands.ack[5]=((pktIx & 0xFF00) >> 8);
					printf("\n ack_cmd[4]:%02X , ack_cmd[5]: %02X", ucam->commands.ack[4], ucam->commands.ack[5]);
					writeCommandToCamera(ucam, "Send ACK", &ucam->commands.ack[0], 5000);
					memset(&imageBuffer[0], 0, IMAGE_BUFFER_BYTES);
					usleep(100000);//todo mj
					read(ucam->fdRead, &imageBuffer[0], IMAGE_BUFFER_BYTES);

					pktSize=imageBuffer[2]+imageBuffer[3]*256;
					printf(" PktSize: %02X, %d \n",pktSize, pktSize);
					write(fpImageFile, &imageBuffer[4], pktSize);
				}
				// send the last ack packet
				ucam->commands.ack[4]=noOfPackets & 0x00FF;
				ucam->commands.ack[5]=(noOfPackets & 0xFF00) >> 8;
				writeCommandToCamera(ucam, "Send ACK", &ucam->commands.ack[0], 5000);
				close (fpImageFile);
			}
			else
			{
				printf("\nUnable to open the jpeg file: %s\n", jpegFileName);
				status=ERROR;
			}
		}
		else
		{
			printf("\ngetPicStatus FAILED\n");
			status=ERROR;
		}
	}
	return status;
}

int8_t takeRawPic(ucam_t *ucam, uint8_t rawMode, uint8_t rawResolution, uint16_t sizeOfRawData, uint8_t *rawFileName)
{
	int8_t status=ERROR;
	int32_t getPicTry=0;
	int32_t getPicStatus=ERROR;

	uint8_t imageBuffer[RAW_IMAGE_MAX_BUFFER_BYTES];

	int32_t noOfPackets=0;
	int32_t imageSize=0;
	uint8_t init_cmd_response[6] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00}; 		//Init response
	uint8_t snapshot_cmd_response[6] = {0xAA, 0x0E, 0x05, 0x00, 0x00, 0x00}; 	//get a snapshot immediately
	uint8_t getpic_cmd_response[6] = {0xAA, 0x0E, 0x04, 0x00, 0x00, 0x00}; 		//get picture

	int32_t fpRawImageFile;
	int32_t size_write;
	uint32_t totalReadBytes=0;
	uint8_t readout;
	int8_t nReadout;
	uint32_t readIx=0;

	if( (GRAYSCALE_MODE != rawMode) && (COLOR16BIT_CrYCbY_MODE != rawMode) && (COLOR16BIT_RGB_MODE != rawMode))
	{
		printf("\nspecified raw mode: %02X is invalid, reverting to raw mode: %02X\n", rawMode, GRAYSCALE_MODE);
		rawMode=GRAYSCALE_MODE;
	}

	if( (RAW_RES_80_60 != rawResolution) && (RAW_RES_160_120 != rawResolution) && (RAW_RES_128_128 != rawResolution)  &&  (RAW_RES_128_96 != rawResolution))
	{
		printf("\nspecified raw resolution: %02X is invalid, reverting to raw resolution: %02X\n", rawMode, RAW_RES_80_60);
		rawResolution=RAW_RES_80_60;
	}

	/* initialize init command */
	setCommandParameters(&ucam->commands.initial[0], 0x00, rawMode, rawResolution, 0x00);

	//initialize
	status =  writeCmdToCamAndCheckResponseBlocking(ucam, "Init", &ucam->commands.initial[0], "Init Response", &init_cmd_response[0], 3, 20000);

	if(NO_ERROR == status)
	{
		ucam->commands.snapshot[2]=0x01; // as per manual, for uncompressed picture, parameter1 is 0x01
		status =  writeCmdToCamAndCheckResponseBlocking(ucam, "Snapshot", &ucam->commands.snapshot[0], "Snapshot Response", &snapshot_cmd_response[0], 3, 5000);
		sleep(1);
	}

	if(NO_ERROR == status)
	{
		while(getPicTry < GET_PIC_MAX_ATTEMPT && ERROR == getPicStatus)
		{
			//get picture
			ucam->commands.getPicture[2]=0x02; //RAW Picture Mode
			getPicStatus =  writeCmdToCamAndCheckResponse(ucam, "Get Pic", &ucam->commands.getPicture[0], "GetPic Response", &getpic_cmd_response[0], 3, 5000);
			getPicTry++;
			usleep(100);
			sleep(1);
		}


		if(NO_ERROR == getPicStatus)
		{
			readFromCam(ucam, "Data SnapShot");
			// calculate image size
			imageSize=ucam->responseBuffer[3]+ucam->responseBuffer[4]*256;
			// calculate packet size
			noOfPackets=ceil(imageSize*1.0/(IMAGE_BUFFER_BYTES*1.0 - 6.0));
			printf("\nImage size is: %d, number of packets: %d \n", imageSize, noOfPackets);

			// send command to get data packets
			fpRawImageFile = open(rawFileName, O_RDWR | O_CREAT, 0777);

			memset(&imageBuffer[0], 0, sizeOfRawData);
			usleep(100000); //todo: fine tune this

			for(uint32_t readIx1=0; readIx1<sizeOfRawData*100; readIx1++) //todo: change this logic
			{
				nReadout=read(ucam->fdRead, &readout, 1);
				if (nReadout>0)
				{
					readIx++;
					imageBuffer[readIx++]=readout;
					write(fpRawImageFile, &readout, 1);
				}
				if (readIx>=sizeOfRawData) break;
			}

			// send last ack command to camera
			setCommandParameters(&ucam->commands.ack[0], 0x0A, 0x00, 0x01, 0x00); // as per manual
			writeCommandToCamera(ucam, "Send ACK", &ucam->commands.ack[0], 5000);
			close (fpRawImageFile);
		}
		else
		{
			printf("\ngetPicStatus FAILED\n");
			status=ERROR;
		}
	}
	return status;
}

int8_t writeCmdToCamAndCheckResponse(ucam_t *ucam, char *commandName, uint8_t *command, char *expectedResponseName, uint8_t *expectedResponse, int32_t noOfBytesTocompare, int32_t waitMiliSeconds)
{
	int8_t status=NO_ERROR;
	int32_t size_read=0;
	int32_t nwrite=0;

	memset(ucam->responseBuffer,0,SIZE_OF_CAM_RESPONSE);
	printf("\nWriting: %s : ", commandName);
	printBuffer(command,SIZE_OF_CAM_RESPONSE);
	nwrite = write(ucam->fdWrite, command, SIZE_OF_CAM_RESPONSE);

	usleep(waitMiliSeconds);
	printf("\nReading: %s : ", expectedResponseName);
	size_read = read(ucam->fdRead, ucam->responseBuffer, SIZE_OF_CAM_RESPONSE);
	if(size_read > 0)
	{
		printBuffer(ucam->responseBuffer,SIZE_OF_CAM_RESPONSE);
		printf("\n");
		for(int32_t responseIx=0; responseIx<noOfBytesTocompare;	responseIx++)
		{
			if(ucam->responseBuffer[responseIx] != expectedResponse[responseIx])
			{
				printf("Expected response does not match at index: %d\n", responseIx);
				printf("Response Expected : ");
				printBuffer(expectedResponse,SIZE_OF_CAM_RESPONSE);
				printf("\nResponse Received : ");
				printBuffer(ucam->responseBuffer,SIZE_OF_CAM_RESPONSE);
				printf("\n");
				status=ERROR;
				break;
			}
		}
	}
	else
	{
		status=ERROR;
		printf(" Failed\n");
	}
	return status;
}

int8_t writeCmdToCamAndCheckResponseBlocking(ucam_t *ucam, char *commandName, uint8_t *command, char *expectedResponseName, uint8_t *expectedResponse, int32_t noOfBytesTocompare, int32_t waitMiliSeconds)
{
	int8_t status=NO_ERROR; //todo
	int32_t size_read=0;
	int32_t nwrite=0;

	memset(ucam->responseBuffer,0,SIZE_OF_CAM_RESPONSE);
	printf("\nWriting: %s : ", commandName);
	printBuffer(command,SIZE_OF_CAM_RESPONSE);
	nwrite = write(ucam->fdWrite, command, SIZE_OF_CAM_RESPONSE);

	usleep(waitMiliSeconds);
	printf("\nReading: %s : ", expectedResponseName);
	size_read = read(ucam->fdRead_blocking, ucam->responseBuffer, SIZE_OF_CAM_RESPONSE);
	if(size_read > 0)
	{
		printBuffer(ucam->responseBuffer,SIZE_OF_CAM_RESPONSE);
		printf("\n");
		for(int32_t responseIx=0; responseIx<noOfBytesTocompare;	responseIx++)
		{
			if(ucam->responseBuffer[responseIx] != expectedResponse[responseIx])
			{
				printf("Expected response does not match at index: %d\n", responseIx);
				printf("Response Expected : ");
				printBuffer(expectedResponse,SIZE_OF_CAM_RESPONSE);
				printf("\nResponse Received : ");
				printBuffer(ucam->responseBuffer,SIZE_OF_CAM_RESPONSE);
				printf("\n");
				status=ERROR;
				break;
			}
		}
	}
	else
	{
		status=ERROR;
		printf(" Failed\n");
	}
	return status;
}

int8_t readFromCam(ucam_t *ucam, char *expectedResponseName)
{
	int8_t status=NO_ERROR;
	int32_t size_read=0;

	printf("\nReading: %s : ", expectedResponseName);
	memset(&ucam->responseBuffer[0], 0, SIZE_OF_CAM_RESPONSE);
	size_read = read(ucam->fdRead, &ucam->responseBuffer[0], SIZE_OF_CAM_RESPONSE);
	if (size_read>0)
	{
		printBuffer(&ucam->responseBuffer[0],SIZE_OF_CAM_RESPONSE);
	}
	else
	{
		printf(" Failed\n");
		status=ERROR;
	}
	return status;
}

void writeCommandToCamera(ucam_t *ucam, char *commandName, uint8_t *command, int32_t waitMiliSeconds)
{
	int32_t status=NO_ERROR;
	int32_t nwrite=0;

	printf("\nWriting: %s ", commandName);
	printBuffer(command,SIZE_OF_CAM_RESPONSE);
	nwrite = write(ucam->fdWrite, command, SIZE_OF_CAM_RESPONSE);
	usleep(waitMiliSeconds);
}

void uCamInitCommands(ucam_t *cam)
{
	memset(&cam->commands, 0, sizeof(cam->commands));
	setCommand(&cam->commands.initial[0], ID_INITIAL, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.getPicture[0], ID_GET_PICTURE, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.snapshot[0], ID_SNAPSHOT, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.setPackageSize[0], ID_SET_PACKAGE_SIZE, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.setBaudRate[0], ID_SET_BAUD_RATE, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.reSet[0], ID_RESET, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.data[0], ID_DATA, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.sync[0], ID_SYNC, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.ack[0], ID_ACK, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.nak[0], ID_NCK, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.light[0], ID_LIGHT, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.contrastBrightnessExposure[0], ID_CONTRAST_BRIGHTNESS_EXPOSURE, 0x00, 0x00, 0x00, 0x00);
	setCommand(&cam->commands.sleep[0], ID_SLEEP, 0x00, 0x00, 0x00, 0x00);
 }

void setCommand(uint8_t command[], uint8_t idNumber, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4)
{
	command[0]=0xAA; // first byte is always 0xAA as per the manual
	command[1]=idNumber;
	setCommandParameters(&command[0], param1,  param2,  param3, param4);
}

void setCommandParameters(uint8_t command[], uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4)
{
	command[2]=param1;
	command[3]=param2;
	command[4]=param3;
	command[5]=param4;
}

void printBuffer(uint8_t *buffer, int32_t len)
{
	for(int32_t bufIx=0; bufIx<len; bufIx++)
	{
		printf(" %02X ",buffer[bufIx]);
	}
	printf("\n");
}

void configureUart()
{
	uint8_t commandString[COMMAND_LENGHT]={0};
	snprintf(&commandString[0], COMMAND_LENGHT, "%s %s %s","sudo stty -F ", UART_DEVICE, "  raw");
	system(&commandString[0]);

	memset(&commandString[0], 0, COMMAND_LENGHT);
	snprintf(&commandString[0],  COMMAND_LENGHT, "%s %s %s","sudo stty -F ", UART_DEVICE, "  -echo -echoe -echok");
	system(&commandString[0]);

	memset(&commandString[0], 0, COMMAND_LENGHT);
	snprintf(&commandString[0],  COMMAND_LENGHT, "%s %s %s","sudo stty -F ", UART_DEVICE, " 115200");
	system(&commandString[0]);
}
