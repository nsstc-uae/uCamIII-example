# uCamIII-example
This is an example program to click raw or jpeg image from [uCamIII](https://4dsystems.com.au/ucam-iii) based on the [documentation](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCAM-III_datasheet_R_1_0-1100457.pdf). The given example code i.e. [uCamIII.c](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c) clicks and saves a JPEG image of resolution 160x128 with filename, `jpeg160x128.jpg`; if you want to click and save other mode (jpeg or raw) and other resolution of image (160x128, 320x240, 640x480 etc.), you will have to modify [uCamIII.c](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c). You'll have to replace [`takeJpegPic(&ucam, JPEG_RES_160_128, "jpeg160x128.jpg");`](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L42) with appropriate function corrosponding to the mode and resolution and recompile. The details of which function to call has been explained in section [How to click different modes and resolution of image](https://github.com/nsstc-uae/uCamIII-example#how-to-click-different-modes-and-resolution-of-image)

## Overview:
The example implements functions to sync the camera, and take raw and jpg images according to datasheet. Specifically, it implements functions to:
1. [Sync](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L95)  
2. [Take jpeg pic](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L147) 
3. [Take raw image](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L255) 

## Installation Guidelines 
1. Copy the file [uCamIII.rules](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.rules) in direcory */etc/udev/rules.d* or corrosponding udev rules directory in your linux system. This rule creates a symlink "/dev/ucam" with proper permissions whenever uCamIII is connected via USB to your linux machine.  
**Note**: If you do not want to avoid this step, once you connect the uCamIII usb to your machine, a corresponding */dev/ttyUSB0* or */dev/ttyUSBx* will be created in your linux system. Update the  [#define UART_DEVICE](https://github.com/nsstc-uae/uCamIII-example/blob/main/include/uCamIII.h#L39) in [uCamIII.h](https://github.com/nsstc-uae/uCamIII-example/blob/main/include/uCamIII.h) and re-compile the code.   
2. From the directory where [uCamIII.c](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c) is located, execute  
a. To compile this example:   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; **make uCamIIIcompile**  
b. To click a photo:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; **make clickImage**  
  
## How to click different modes and resolution of image
The given [uCamIII.c](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c) code will give you JPEG with resolution 160x128 in accordance with code [`takeJpegPic(&ucam, JPEG_RES_160_128, "jpeg160x128.jpg");`](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L42). To click image with different mode and resolution, one has to modify the [*main()*](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L17) function and call appropriate function with proper parameters. The function to modify/replace is: [`takeJpegPic(&ucam, JPEG_RES_160_128, "jpeg160x128.jpg");`](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L42) in the file [uCamIII.c](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c).  

### For JPEG, call the following function:  
**takeJpegPic(&ucam, <JPEG_RESOLUTION>, "fileName.jpg");**
e.g.   
1. jpeg corrosponding to 160x128 resolution  
`takeJpegPic(&ucam, JPEG_RES_160_128, "jpeg160x128.jpg");`   
2. jpeg corrosponding to 320x240 resolution  
`takeJpegPic(&ucam, JPEG_RES_320_240, "jpeg320x240.jpg");`   
3. jpeg corrosponding to 640x480 resolution   
`takeJpegPic(&ucam, JPEG_RES_640_480, "jpeg640x480.jpg");`    

### For RAW, call the following function:    
**takeRawPic(&ucam, <MODE_MACRO>, <RAW_RES_XXX_XXX>, <BUFFER SIZE CORROSPONDING TO MODE_MACRO AND RAW_RES_XXX_YYY>, "rawFilename.raw");**

<table>
	<tbody>
		<tr>
			<td> MODE ↓ </td>
			<td> resolution (RAW_RES_XXX_YYY) →  MODE_MACRO ↓ </td>			
			<td>80x60 (RAW_RES_80_60)</td>
			<td>160x120 (RAW_RES_160_120)</td>
			<td>128x128 (RAW_RES_128_128) </td>
			<td>128x96 (RAW_RES_128_96)</td>
		</tr>
		<tr>
			<td> 8-bit Gray Scale (RAW, 8-bit for Y only) </td>
			<td> GRAYSCALE_MODE </td>
			<td>4800</td>
			<td>19200</td>
			<td>16384</td>
			<td>12288</td>
		</tr>
		<tr>
			<td> 16-bit Colour (RAW, CrYCbY) </td>
			<td>  COLOR16BIT_CrYCbY_MODE  </td>
			<td>9600</td>
			<td>38400</td>
			<td>32768</td>
			<td>24576</td>
		</tr>
		<tr>
			<td> 16-bit Colour (RAW, 565(RGB)) </td>
			<td> COLOR16BIT_RGB_MODE  </td>			
			<td>9600</td>
			<td>38400</td>
			<td>32768</td>
			<td>24576</td>
		</tr>
	</tbody>
</table>

e.g.: 
1. MODE: 8-bit Gray Scale (RAW, 8-bit for Y only), MODE_MACRO: `GRAYSCALE_MODE`, resolution: 80x60 ==> `RAW_RES_80_60`, i.e. corrosponding Image buffer 4800   
`takeRawPic(&ucam, GRAYSCALE_MODE, RAW_RES_80_60, 4800, "rgb128x128.raw");`  

2. MODE: 8-bit Gray Scale (RAW, 8-bit for Y only), MODE_MACRO: `GRAYSCALE_MODE`, resolution: 128x128 ==> `RAW_RES_128_128`, i.e. corrosponding Image buffer 16384   
`takeRawPic(&ucam, GRAYSCALE_MODE, RAW_RES_128_128, 16384, "rgb128x128.raw");`       

3. MODE: 16-bit Colour (RAW, CrYCbY), MODE_MACRO: `COLOR16BIT_CrYCbY_MODE`, resolution: 160x120 ==> `RAW_RES_160_120`, i.e. corrosponding Image buffer 38400   
`takeRawPic(&ucam, COLOR16BIT_CrYCbY_MODE, RAW_RES_160_120, 38400, "CrYCbY160x120.raw");`  

4. mode: 16-bit Colour (RAW, 565(RGB)), MODE_MACRO: `COLOR16BIT_RGB_FORMAT`, resolution: 128x128 ==> `RAW_RES_128_128`, i.e. corrosponding Image buffer 32768    
`takeRawPic(&ucam, COLOR16BIT_RGB_FORMAT, RAW_RES_128_128, 32768, "rawRGB128x128.raw");` 

Note: The raw image obtainted from uCamIII requires further processing to convert it into BMP or PNG formats. The raw image won't be displayed properly without further processing.

Note: This [main()](https://github.com/nsstc-uae/uCamIII-example/blob/main/uCamIII.c#L17) shall have only one function call to take image (takeRawPic or takeJpegPic) with valid parameters. 

## Tested
This example has been tested against ubuntu 16.04 LTS.
