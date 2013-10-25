
#include "generic_sensor.h"
/*
*      Driver Version Note
*v0.0.1: this driver is compatible with generic_sensor
*v0.1.1:
*        add sensor_focus_af_const_pause_usr_cb;
*/
static int version = KERNEL_VERSION(0,1,1);
module_param(version, int, S_IRUGO);

static int debug =1;
module_param(debug, int, S_IRUGO|S_IWUSR);

#define dprintk(level, fmt, arg...) do {			\
	if (debug >= level) 					\
	printk(KERN_WARNING fmt , ## arg); } while (0)

/* Sensor Driver Configuration Begin */
#define SENSOR_NAME RK29_CAM_SENSOR_NT99340
#define SENSOR_V4L2_IDENT V4L2_IDENT_NT99340
#define SENSOR_ID 0x3400
#define SENSOR_BUS_PARAM  (SOCAM_MASTER | SOCAM_PCLK_SAMPLE_RISING|\
                          SOCAM_HSYNC_ACTIVE_HIGH| SOCAM_VSYNC_ACTIVE_HIGH|\
                          SOCAM_DATA_ACTIVE_HIGH | SOCAM_DATAWIDTH_8  |SOCAM_MCLK_24MHZ)

#define SENSOR_PREVIEW_W                     800
#define SENSOR_PREVIEW_H                     600
#define SENSOR_PREVIEW_FPS                   15000     // 15fps 
#define SENSOR_FULLRES_L_FPS                 5000      // 7.5fps
#define SENSOR_FULLRES_H_FPS                 10000      // 7.5fps
#define SENSOR_720P_FPS                      15000 
#define SENSOR_1080P_FPS                     0

#define SENSOR_REGISTER_LEN                  2         // sensor register address bytes
#define SENSOR_VALUE_LEN                     1         // sensor register value bytes

static unsigned int SensorConfiguration = 0;
static unsigned int SensorChipID[] = {SENSOR_ID};

/* Sensor Driver Configuration End */

#define SENSOR_NAME_STRING(a) STR(CONS(SENSOR_NAME, a))
#define SENSOR_NAME_VARFUN(a) CONS(SENSOR_NAME, a)

#define SensorRegVal(a,b) CONS4(SensorReg,SENSOR_REGISTER_LEN,Val,SENSOR_VALUE_LEN)(a,b)
#define sensor_write(client,reg,v) CONS4(sensor_write_reg,SENSOR_REGISTER_LEN,val,SENSOR_VALUE_LEN)(client,(reg),(v))
#define sensor_read(client,reg,v) CONS4(sensor_read_reg,SENSOR_REGISTER_LEN,val,SENSOR_VALUE_LEN)(client,(reg),(v))
#define sensor_write_array generic_sensor_write_array

struct sensor_parameter
{
	unsigned int PreviewDummyPixels;
	unsigned int CaptureDummyPixels;
	unsigned int preview_exposure;
	unsigned short int preview_line_width;
	unsigned short int preview_gain;

	unsigned short int PreviewPclk;
	unsigned short int CapturePclk;
	char awb[6];
};

struct specific_sensor{
	struct generic_sensor common_sensor;
	//define user data below
	struct sensor_parameter parameter;
};

/*
*  The follow setting need been filled.
*  
*  Must Filled:
*  sensor_init_data :               Sensor initial setting;
*  sensor_fullres_lowfps_data :     Sensor full resolution setting with best auality, recommand for video;
*  sensor_preview_data :            Sensor preview resolution setting, recommand it is vga or svga;
*  sensor_softreset_data :          Sensor software reset register;
*  sensor_check_id_data :           Sensir chip id register;
*
*  Optional filled:
*  sensor_fullres_highfps_data:     Sensor full resolution setting with high framerate, recommand for video;
*  sensor_720p:                     Sensor 720p setting, it is for video;
*  sensor_1080p:                    Sensor 1080p setting, it is for video;
*
*  :::::WARNING:::::
*  The SensorEnd which is the setting end flag must be filled int the last of each setting;
*/

/* Sensor initial setting */
static struct rk_sensor_reg sensor_init_data[] ={
	{0x32F0, 0x00},
	{0x32FF, 0x00},
	{0x303E, 0x0C},
	{0x303F, 0x02},
	{0x3040, 0xFF},
	{0x3041, 0x00},
	{0x3042, 0x00},
	{0x3051, 0xFC},
	{0x3052, 0x10},
	{0x3069, 0x03},
	{0x306a, 0x03},
	{0x3118, 0xA7},
	{0x3119, 0xA7},
	{0x311A, 0xA7},
	{0x3100, 0x03},
	{0x3101, 0x80},
	{0x3102, 0x09},
	{0x3103, 0x09},
	{0x3104, 0x01},
	{0x3105, 0x01},
	{0x3106, 0x03},
	{0x3107, 0x20},
	{0x3108, 0x00},
	{0x3109, 0x82},
	{0x310A, 0x04},
	{0x3030, 0x0A},
	{0x306C, 0x0C},
	{0x306D, 0x0C},
	{0x3583, 0x18},
	{0x3587, 0x02},
	{0x3584, 0x00},
	{0x3585, 0x00},
	{0x3580, 0xB2},
	{0x3600, 0x66},
	{0x3601, 0x99},
	{0x3604, 0x15},
	{0x3605, 0x13},
	{0x3614, 0x20},
	{0x361A, 0x10},
	{0x3615, 0x24},
	{0x3610, 0x02},
	{0x3290, 0x01},
	{0x3291, 0x80},
	{0x3296, 0x01},
	{0x3297, 0x60},
	{0x3250, 0x80},
	{0x3251, 0x01},
	{0x3252, 0x27},
	{0x3253, 0x9D},
	{0x3254, 0x00},
	{0x3255, 0xE3},
	{0x3256, 0x81},
	{0x3257, 0x70},
	{0x329B, 0x00},
	{0x32A1, 0x00},
	{0x32A2, 0xEC},
	{0x32A3, 0x01},
	{0x32A4, 0x7A},
	{0x32A5, 0x01},
	{0x32A6, 0x14},
	{0x32A7, 0x01},
	{0x32A8, 0xB3},
	{0x3270, 0x00},
	{0x3271, 0x0B},
	{0x3272, 0x16},
	{0x3273, 0x2B},
	{0x3274, 0x3F},
	{0x3275, 0x51},
	{0x3276, 0x72},
	{0x3277, 0x8F},
	{0x3278, 0xA7},
	{0x3279, 0xBC},
	{0x327A, 0xDC},
	{0x327B, 0xF0},
	{0x327C, 0xFA},
	{0x327D, 0xFE},
	{0x327E, 0xFF},
	{0x3302, 0x00},
	{0x3303, 0x4D},
	{0x3304, 0x00},
	{0x3305, 0x96},
	{0x3306, 0x00},
	{0x3307, 0x1D},
	{0x3308, 0x07},
	{0x3309, 0xC7},
	{0x330A, 0x07},
	{0x330B, 0x0A},
	{0x330C, 0x01},
	{0x330D, 0x30},
	{0x330E, 0x01},
	{0x330F, 0x07},
	{0x3310, 0x06},
	{0x3311, 0xFF},
	{0x3312, 0x07},
	{0x3313, 0xFA},
	{0x3210, 0x1E},
	{0x3211, 0x20},
	{0x3212, 0x20},
	{0x3213, 0x1E},
	{0x3214, 0x18},
	{0x3215, 0x1A},
	{0x3216, 0x1A},
	{0x3217, 0x18},
	{0x3218, 0x18},
	{0x3219, 0x1A},
	{0x321A, 0x1A},
	{0x321B, 0x18},
	{0x321C, 0x16},
	{0x321D, 0x1A},
	{0x321E, 0x17},
	{0x321F, 0x15},
	{0x3234, 0x0F},
	{0x3235, 0x0F},
	{0x3236, 0x0F},
	{0x3237, 0x0F},
	{0x3238, 0x40},
	{0x3239, 0x40},
	{0x323A, 0x40},
	{0x3241, 0x40},
	{0x3243, 0x41},
	{0x32F6, 0x0F},
	{0x32F9, 0x42},
	{0x32FA, 0x24},
	{0x3338, 0x18},
	{0x3339, 0xC6},
	{0x333A, 0x6C},
	{0x3335, 0x60},
	{0x3336, 0x20},
	{0x3337, 0x40},
	{0x333B, 0xC6},
	{0x333C, 0x6C},
	{0x3325, 0xAF},
	{0x3326, 0x02},
	{0x3327, 0x00},
	{0x3331, 0x08},
	{0x3332, 0xFF},
	{0x333F, 0x07},
	{0x334A, 0x34},
	{0x334B, 0x14},
	{0x334C, 0x10},
	{0x3363, 0x33},
	{0x3360, 0x08},
	{0x3361, 0x10},
	{0x3362, 0x18},
	{0x3364, 0x88},
	{0x3365, 0x80},
	{0x3366, 0x68},
	{0x3367, 0x40},
	{0x3368, 0x50},
	{0x3369, 0x40},
	{0x336A, 0x30},
	{0x336B, 0x20},
	{0x336C, 0x00},
	{0x336D, 0x1A},
	{0x336E, 0x16},
	{0x336F, 0x14},
	{0x3370, 0x0C},
	{0x3371, 0x3F},
	{0x3372, 0x3F},
	{0x3373, 0x3F},
	{0x3374, 0x3F},
	{0x3375, 0x0E},
	{0x3376, 0x10},
	{0x3377, 0x14},
	{0x3378, 0x20},
	{0x3012, 0x02},
	{0x3013, 0x58},
	{0x301D, 0x01},
	SensorEnd
};
/* Senor full resolution setting: recommand for capture */
static struct rk_sensor_reg sensor_fullres_lowfps_data[] ={
	{0x32BF, 0x60},
	{0x32C0, 0x84},
	{0x32C1, 0x84},
	{0x32C2, 0x84},
	{0x32C3, 0x00},
	{0x32C4, 0x20},
	{0x32C5, 0x20},
	{0x32C6, 0x20},
	{0x32C7, 0x00},
	{0x32C8, 0x9E},
	{0x32C9, 0x84},
	{0x32CA, 0xA4},
	{0x32CB, 0xA4},
	{0x32CC, 0xA4},
	{0x32CD, 0xA4},
	{0x32DB, 0x73},
	{0x32D0, 0x01},
	{0x3200, 0x3F},
	{0x3201, 0x0F},
	{0x302A, 0x00},
	{0x302B, 0x01},
	{0x302C, 0x0F},
	{0x302D, 0x00},
	{0x302E, 0x00},
	{0x302F, 0x02},
	{0x3022, 0x27},
	{0x3023, 0x24},
	{0x3128, 0x01},
	{0x3002, 0x00},
	{0x3003, 0x04},//x_start=4
	{0x3004, 0x00},
	{0x3005, 0x04},//y_start=4
	{0x3006, 0x08},
	{0x3007, 0x03},//x_end=2051
	{0x3008, 0x06},
	{0x3009, 0x03},//y_end=1539
	{0x300A, 0x0B},
	{0x300B, 0xD0},//pixel=3024
	{0x300C, 0x0C},
	{0x300D, 0x66},//line=3174
	{0x300E, 0x08},
	{0x300F, 0x00},//x=2048
	{0x3010, 0x06},
	{0x3011, 0x00},//y=1536
	{0x3052, 0x10},
	{0x32BB, 0x87},
	{0x32B8, 0x3B},
	{0x32B9, 0x2D},
	{0x32BC, 0x34},
	{0x32BD, 0x38},
	{0x32BE, 0x30},
	{0x3201, 0xBF},
	{0x3109, 0x82},
	{0x3530, 0xC0},
	{0x320A, 0x42},
	{0x3021, 0x06},
	{0x3060, 0x01},
	SensorEnd
};
/* Senor full resolution setting: recommand for video */
static struct rk_sensor_reg sensor_fullres_highfps_data[] ={
	{0x32BF, 0x60},
	{0x32C0, 0x74},
	{0x32C1, 0x74},
	{0x32C2, 0x74},
	{0x32C3, 0x00},
	{0x32C4, 0x20},
	{0x32C5, 0x20},
	{0x32C6, 0x20},
	{0x32C7, 0x00},
	{0x32C8, 0x9E},
	{0x32C9, 0x74},
	{0x32CA, 0x94},
	{0x32CB, 0x94},
	{0x32CC, 0x94},
	{0x32CD, 0x94},
	{0x32DB, 0x73},
	{0x32D0, 0x01},
	{0x3200, 0x3F},
	{0x3201, 0x0F},
	{0x302A, 0x00},
	{0x302B, 0x01},
	{0x302C, 0x0F},
	{0x302D, 0x00},
	{0x302E, 0x00},
	{0x302F, 0x02},
	{0x3022, 0x27},
	{0x3023, 0x24},
	{0x3128, 0x01},
	{0x3002, 0x00},
	{0x3003, 0x04},//x_start=4
	{0x3004, 0x00},
	{0x3005, 0x04},//y_start=4
	{0x3006, 0x08},
	{0x3007, 0x03},//x_end=2051
	{0x3008, 0x06},
	{0x3009, 0x03},//y_end=1539
	{0x300A, 0x0B},
	{0x300B, 0xD0},//pixel=3024
	{0x300C, 0x06},
	{0x300D, 0x33},//line=1587
	{0x300E, 0x08},
	{0x300F, 0x00},//x=2048
	{0x3010, 0x06},
	{0x3011, 0x00},//y=1536
	{0x3052, 0x10},
	{0x32BB, 0x87},
	{0x32B8, 0x3B},
	{0x32B9, 0x2D},
	{0x32BC, 0x34},
	{0x32BD, 0x38},
	{0x32BE, 0x30},
	{0x3201, 0xBF},
	{0x3109, 0x82},
	{0x3530, 0xC0},
	{0x320A, 0x42},
	{0x3021, 0x06},
	{0x3060, 0x01},
	SensorEnd
};
/* Preview resolution setting*/
static struct rk_sensor_reg sensor_preview_data[] =
{
	{0x32BF, 0x60},
	{0x32C0, 0x6A},
	{0x32C1, 0x6A},
	{0x32C2, 0x6A},
	{0x32C3, 0x00},
	{0x32C4, 0x20},
	{0x32C5, 0x20},
	{0x32C6, 0x20},
	{0x32C7, 0x00},
	{0x32C8, 0xF0},
	{0x32C9, 0x6A},
	{0x32CA, 0x8A},
	{0x32CB, 0x8A},
	{0x32CC, 0x8A},
	{0x32CD, 0x8A},
	{0x32DB, 0x7E},
	{0x32D0, 0x01},
	{0x32E0, 0x03},
	{0x32E1, 0x20},
	{0x32E2, 0x02},
	{0x32E3, 0x58},
	{0x32E4, 0x00},
	{0x32E5, 0x48},
	{0x32E6, 0x00},
	{0x32E7, 0x48},
	{0x3200, 0x3F},
	{0x3201, 0x0F},
	{0x302A, 0x00},
	{0x302B, 0x01},
	{0x302C, 0x0F},
	{0x302D, 0x00},
	{0x302E, 0x00},
	{0x302F, 0x02},
	{0x3022, 0x27},
	{0x3023, 0x6E},
	{0x3128, 0x01},
	{0x3002, 0x00},
	{0x3003, 0x04},//x_start=4
	{0x3004, 0x00},
	{0x3005, 0x04},//y_start=4
	{0x3006, 0x08},
	{0x3007, 0x03},//x_end=2051
	{0x3008, 0x06},
	{0x3009, 0x03},//y_end=1539
	{0x300A, 0x07},
	{0x300B, 0xD0},//pixel=2000
	{0x300C, 0x03},
	{0x300D, 0x20},//line=800
	{0x300E, 0x04},
	{0x300F, 0x00},//x=1024
	{0x3010, 0x03},
	{0x3011, 0x00},//y=768
	{0x3052, 0x10},
	{0x32BB, 0x87},
	{0x32B8, 0x3B},
	{0x32B9, 0x2D},
	{0x32BC, 0x34},
	{0x32BD, 0x38},
	{0x32BE, 0x30},
	{0x3201, 0xFF},
	{0x3109, 0x82},
	{0x3530, 0xC0},
	{0x320A, 0x42},
	{0x3021, 0x06},
	{0x3060, 0x01},
	SensorEnd
};
/* 1280x720 */
static struct rk_sensor_reg sensor_720p[]={
	{0x32BF, 0x60},
	{0x32C0, 0x6A},
	{0x32C1, 0x6A},
	{0x32C2, 0x6A},
	{0x32C3, 0x00},
	{0x32C4, 0x20},
	{0x32C5, 0x20},
	{0x32C6, 0x20},
	{0x32C7, 0x40},
	{0x32C8, 0x08},
	{0x32C9, 0x6A},
	{0x32CA, 0x8A},
	{0x32CB, 0x8A},
	{0x32CC, 0x8A},
	{0x32CD, 0x8A},
	{0x32DB, 0x80},
	{0x32D0, 0x01},
	{0x32E0, 0x05},
	{0x32E1, 0x00},
	{0x32E2, 0x02},
	{0x32E3, 0xD0},
	{0x32E4, 0x00},
	{0x32E5, 0x28},
	{0x32E6, 0x00},
	{0x32E7, 0x28},
	{0x3200, 0x3F},
	{0x3201, 0x0F},
	{0x302A, 0x00},
	{0x302B, 0x01},
	{0x302C, 0x0F},
	{0x302D, 0x00},
	{0x302E, 0x00},
	{0x302F, 0x02},
	{0x3022, 0x27},
	{0x3023, 0x24},
	{0x3128, 0x00},
	{0x3002, 0x01},
	{0x3003, 0x20},//x_start=288
	{0x3004, 0x01},
	{0x3005, 0x64},//y_start = 356
	{0x3006, 0x06},
	{0x3007, 0xE7},//x_end = 1767
	{0x3008, 0x04},
	{0x3009, 0xA3},//y_end=1187
	{0x300A, 0x07},
	{0x300B, 0x14},//pixel=1812
	{0x300C, 0x03},
	{0x300D, 0x73},//line=883
	{0x300E, 0x05},
	{0x300F, 0xC8},//x=1480
	{0x3010, 0x03},
	{0x3011, 0x40},//y=832
	{0x3052, 0x10},
	{0x32BB, 0x87},
	{0x32B8, 0x3B},
	{0x32B9, 0x2D},
	{0x32BC, 0x34},
	{0x32BD, 0x38},
	{0x32BE, 0x30},
	{0x3201, 0xFF},
	{0x3109, 0x82},
	{0x3530, 0xC0},
	{0x320A, 0x42},
	{0x3021, 0x06},
	{0x3060, 0x01},
	SensorEnd
};

/* 1920x1080 */
static struct rk_sensor_reg sensor_1080p[]={
	SensorEnd
};


static struct rk_sensor_reg sensor_softreset_data[]={
    SensorRegVal(0x3021,0x61),
	SensorEnd
};

static struct rk_sensor_reg sensor_check_id_data[]={
    SensorRegVal(0x3000,0),
    SensorRegVal(0x3001,0),
	SensorEnd
};
/*
*  The following setting must been filled, if the function is turn on by CONFIG_SENSOR_xxxx
*/
static struct rk_sensor_reg sensor_WhiteB_Auto[]=
{
	//[WB-AUTO]
	{0x3201, 0xFF},
	SensorEnd
};
/* Cloudy Colour Temperature : 6500K - 8000K  */
static	struct rk_sensor_reg sensor_WhiteB_Cloudy[]=
{
	//[WB-CLOUDY]
	{0x3201, 0xEF},
	{0x3290, 0x01},
	{0x3291, 0x51},
	{0x3296, 0x01},
	{0x3297, 0x00},
	SensorEnd
};
/* ClearDay Colour Temperature : 5000K - 6500K	*/
static	struct rk_sensor_reg sensor_WhiteB_ClearDay[]=
{
	//[WB-DAYLIGHT]
	{0x3201, 0xEF},
	{0x3290, 0x01},
	{0x3291, 0x38},
	{0x3296, 0x01},
	{0x3297, 0x68},  
	SensorEnd
};
/* Office Colour Temperature : 3500K - 5000K  */
static	struct rk_sensor_reg sensor_WhiteB_TungstenLamp1[]=
{
	//[WB-INCANDESCENCE]
	{0x3201, 0xEF},
	{0x3290, 0x01},
	{0x3291, 0x30},
	{0x3296, 0x01},
	{0x3297, 0xCB},
	SensorEnd
};
/* Home Colour Temperature : 2500K - 3500K	*/
static	struct rk_sensor_reg sensor_WhiteB_TungstenLamp2[]=
{
	//[WB-FLUORESCENT]
	{0x3201, 0xEF},
	{0x3290, 0x01},
	{0x3291, 0x70},
	{0x3296, 0x01},
	{0x3297, 0xFF},
	SensorEnd
};
static	struct rk_sensor_reg sensor_WhiteB_TungstenLamp3[]=
{
	//[WB-TUNGSTEN]
	{0x3201, 0xEF},
	{0x3290, 0x01},
	{0x3291, 0x00},
	{0x3296, 0x02},
	{0x3297, 0x30},
	SensorEnd
};

static struct rk_sensor_reg *sensor_WhiteBalanceSeqe[] = {sensor_WhiteB_Auto, sensor_WhiteB_TungstenLamp1,sensor_WhiteB_TungstenLamp2,
	sensor_WhiteB_ClearDay, sensor_WhiteB_Cloudy,sensor_WhiteB_TungstenLamp3,NULL,
};

static	struct rk_sensor_reg sensor_Brightness0[]=
{
	// Brightness -2
	SensorEnd
};

static	struct rk_sensor_reg sensor_Brightness1[]=
{
	// Brightness -1

	SensorEnd
};

static	struct rk_sensor_reg sensor_Brightness2[]=
{
	//	Brightness 0

	SensorEnd
};

static	struct rk_sensor_reg sensor_Brightness3[]=
{
	// Brightness +1

	SensorEnd
};

static	struct rk_sensor_reg sensor_Brightness4[]=
{
	//	Brightness +2

	SensorEnd
};

static	struct rk_sensor_reg sensor_Brightness5[]=
{
	//	Brightness +3

	SensorEnd
};
static struct rk_sensor_reg *sensor_BrightnessSeqe[] = {sensor_Brightness0, sensor_Brightness1, sensor_Brightness2, sensor_Brightness3,
	sensor_Brightness4, sensor_Brightness5,NULL,
};

static	struct rk_sensor_reg sensor_Effect_Normal[] =
{
	//[SE-Normal]
	{0x32F1, 0x00},
	{0x32F4, 0x80},
	{0x32F5, 0x80},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Effect_WandB[] =
{
	//[SE-GrayScale]
	{0x32F1, 0x01},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Effect_Sepia[] =
{
	//[SE-SEPIA]
	{0x32F1, 0x02},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Effect_Inverse[] =
{
	//[SE-Inverse] 
	{0x32F1, 0x03},
	SensorEnd
};
static	struct rk_sensor_reg sensor_Effect_Bluish[] =
{
	//[SE-SEPIABlue]
	{0x32F1, 0x05},
	{0x32F4, 0xF0},
	{0x32F5, 0x80},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Effect_Green[] =
{	
	//[SE-SEPIAGreen]
	{0x32F1, 0x05},
	{0x32F4, 0x60},
	{0x32F5, 0x20},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Effect_Solarization[] =
{	
	//[SE-Solarization]
	{0x32F1, 0x04},
	SensorEnd
};

static struct rk_sensor_reg *sensor_EffectSeqe[] = {sensor_Effect_Normal, sensor_Effect_WandB, sensor_Effect_Inverse,sensor_Effect_Sepia,
	sensor_Effect_Bluish, sensor_Effect_Green,sensor_Effect_Solarization,NULL,
};

static	struct rk_sensor_reg sensor_Exposure04[]=
{
	//[EV-4]				
	{0x32F2, 0x40},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure03[]=
{
	//[EV-3]				
	{0x32F2, 0x50},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure02[]=
{
	//[EV-2]				
	{0x32F2, 0x60},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure01[]=
{
	//[EV-1]				
	{0x32F2, 0x70},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure00[]=
{
	//[EV+0]				
	{0x32F2, 0x80},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure11[]=
{
	//[EV+1]              
	{0x32F2, 0x90},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure12[]=
{
	//[EV+2]				
	{0x32F2, 0xA0},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure13[]=
{
	//[EV+3]				
	{0x32F2, 0xB0},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Exposure14[]=
{	
	//[EV+4]				
	{0x32F2, 0xC0},
	SensorEnd
};

static struct rk_sensor_reg *sensor_ExposureSeqe[] = {sensor_Exposure04,sensor_Exposure03, sensor_Exposure02, sensor_Exposure01, sensor_Exposure00,
	sensor_Exposure11, sensor_Exposure12,sensor_Exposure13,sensor_Exposure14,NULL,
};

static	struct rk_sensor_reg sensor_Saturation0[]=
{
	SensorEnd
};

static	struct rk_sensor_reg sensor_Saturation1[]=
{
	SensorEnd
};

static	struct rk_sensor_reg sensor_Saturation2[]=
{
	SensorEnd
};
static struct rk_sensor_reg *sensor_SaturationSeqe[] = {sensor_Saturation0, sensor_Saturation1, sensor_Saturation2, NULL,};

static	struct rk_sensor_reg sensor_Contrast04[]=
{
	//[Contrast : -4]   
	{0x32C2, 0x40},
	{0x32F2, 0x40},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Contrast03[]=
{
	//[Contrast : -3]    
	{0x32C2, 0x30},
	{0x32F2, 0x50},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Contrast02[]=
{
	//[Contrast : -2]	    
	{0x32C2, 0x20},
	{0x32F2, 0x60},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Contrast01[]=
{
	//[Contrast : -1]     
	{0x32C2, 0x10},
	{0x32F2, 0x70},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Contrast00[]=
{
	//[Contrast : 0]		
	{0x32C2, 0x00},
	{0x32F2, 0x80},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Contrast11[]=
{
	//[Contrast : +1] 	
	{0x32C2, 0xF0},
	{0x32F2, 0x90},
	SensorEnd
};


static	struct rk_sensor_reg sensor_Contrast12[]=
{
	//[Contrast : +2]	    
	{0x32C2, 0xE0},
	{0x32F2, 0xA0},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Contrast13[]=
{
	//[Contrast : +3] 	
	{0x32C2, 0xD0},
	{0x32F2, 0xB0},
	SensorEnd
};

static	struct rk_sensor_reg sensor_Contrast14[]=
{
	//[Contrast : +4] 	
	{0x32C2, 0xC0},
	{0x32F2, 0xC0},
	SensorEnd
};

static struct rk_sensor_reg *sensor_ContrastSeqe[] = {sensor_Contrast04, sensor_Contrast03, sensor_Contrast02, sensor_Contrast01,
	sensor_Contrast00, sensor_Contrast11, sensor_Contrast12, sensor_Contrast13, sensor_Contrast14,NULL,
};

static	struct rk_sensor_reg sensor_SceneAuto[] =
{
	SensorEnd
};

static	struct rk_sensor_reg sensor_SceneNight[] =
{
	SensorEnd
};
static struct rk_sensor_reg *sensor_SceneSeqe[] = {sensor_SceneAuto, sensor_SceneNight,NULL,};

static struct rk_sensor_reg sensor_Zoom0[] =
{
	SensorEnd
};

static struct rk_sensor_reg sensor_Zoom1[] =
{
	SensorEnd
};

static struct rk_sensor_reg sensor_Zoom2[] =
{
	SensorEnd
};


static struct rk_sensor_reg sensor_Zoom3[] =
{
	SensorEnd
};
static struct rk_sensor_reg *sensor_ZoomSeqe[] = {sensor_Zoom0, sensor_Zoom1, sensor_Zoom2, sensor_Zoom3, NULL,};


/*
* User could be add v4l2_querymenu in sensor_controls by new_usr_v4l2menu
*/
static struct v4l2_querymenu sensor_menus[] =
{
	//white balance
	new_usr_v4l2menu(V4L2_CID_DO_WHITE_BALANCE,0,"auto",0),
	new_usr_v4l2menu(V4L2_CID_DO_WHITE_BALANCE,1,"incandescent",0),
	new_usr_v4l2menu(V4L2_CID_DO_WHITE_BALANCE,2,"fluorescent",0),
	new_usr_v4l2menu(V4L2_CID_DO_WHITE_BALANCE,3,"daylight",0),
	new_usr_v4l2menu(V4L2_CID_DO_WHITE_BALANCE,4,"cloudy-daylight",0),
	new_usr_v4l2menu(V4L2_CID_DO_WHITE_BALANCE,5,"tungsten",0),

	//speical effect
	new_usr_v4l2menu(V4L2_CID_EFFECT,0,"none",0),
	new_usr_v4l2menu(V4L2_CID_EFFECT,1,"mono",0),
	new_usr_v4l2menu(V4L2_CID_EFFECT,2,"negative",0),
	new_usr_v4l2menu(V4L2_CID_EFFECT,3,"sepia",0),
	new_usr_v4l2menu(V4L2_CID_EFFECT,4,"posterize",0),
	new_usr_v4l2menu(V4L2_CID_EFFECT,5,"aqua",0),
	new_usr_v4l2menu(V4L2_CID_EFFECT,6,"solarize",0),
};
/*
* User could be add v4l2_queryctrl in sensor_controls by new_user_v4l2ctrl
*/
static struct sensor_v4l2ctrl_usr_s sensor_controls[] =
{
	new_user_v4l2ctrl(V4L2_CID_DO_WHITE_BALANCE,V4L2_CTRL_TYPE_MENU,"White Balance Control", 0, 5, 1, 0,sensor_v4l2ctrl_default_cb, sensor_WhiteBalanceSeqe),
	new_user_v4l2ctrl(V4L2_CID_EXPOSURE,V4L2_CTRL_TYPE_INTEGER,"Exposure Control", -4, 4, 1, 0,sensor_v4l2ctrl_default_cb, sensor_ExposureSeqe),
	new_user_v4l2ctrl(V4L2_CID_EFFECT,V4L2_CTRL_TYPE_MENU,"Effect Control", 0, 6, 1, 0,sensor_v4l2ctrl_default_cb, sensor_EffectSeqe),
	new_user_v4l2ctrl(V4L2_CID_CONTRAST,V4L2_CTRL_TYPE_INTEGER,"Contrast Control", -4, 4, 1, 0,sensor_v4l2ctrl_default_cb, sensor_ContrastSeqe),
};

//MUST define the current used format as the first item   
static struct rk_sensor_datafmt sensor_colour_fmts[] = {
	{V4L2_MBUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
	{V4L2_MBUS_FMT_YUYV8_2X8, V4L2_COLORSPACE_JPEG} 
};
static struct soc_camera_ops sensor_ops;


/*
**********************************************************
* Following is local code:
* 
* Please codeing your program here 
**********************************************************
*/
/*
**********************************************************
* Following is callback
* If necessary, you could coding these callback
**********************************************************
*/
/*
* the function is called in open sensor  
*/
static int sensor_activate_cb(struct i2c_client *client)
{
	//struct soc_camera_device *icd = client->dev.platform_data;
	//generic_sensor_ioctrl(icd, Sensor_PowerDown, 0);

    SENSOR_DG("%s",__FUNCTION__);
	
	return 0;
}
/*
* the function is called in close sensor
*/
static int sensor_deactivate_cb(struct i2c_client *client)
{
	struct generic_sensor *sensor = to_generic_sensor(client);
	//struct soc_camera_device *icd = client->dev.platform_data;

    SENSOR_DG("%s",__FUNCTION__);
    
	/* ddl@rock-chips.com : all sensor output pin must switch into Hi-Z */
	if (sensor->info_priv.funmodule_state & SENSOR_INIT_IS_OK) {
		//generic_sensor_ioctrl(icd, Sensor_PowerDown, 1);
		 sensor->info_priv.funmodule_state &= ~SENSOR_INIT_IS_OK;
	}
	
	return 0;
}
/*
* the function is called before sensor register setting in VIDIOC_S_FMT  
*/
static int sensor_s_fmt_cb_th(struct i2c_client *client,struct v4l2_mbus_framefmt *mf, bool capture)
{
    if (capture) {
       // sensor_parameter_record(client);
    }

    return 0;
}
/*
* the function is called after sensor register setting finished in VIDIOC_S_FMT  
*/
static int sensor_s_fmt_cb_bh (struct i2c_client *client,struct v4l2_mbus_framefmt *mf, bool capture)
{
    if (capture) {
       // sensor_ae_transfer(client);
    }
    return 0;
}

static int sensor_try_fmt_cb_th(struct i2c_client *client,struct v4l2_mbus_framefmt *mf)
{
	return 0;
}

static int sensor_softrest_usr_cb(struct i2c_client *client,struct rk_sensor_reg *series)
{
	
	return 0;
}
static int sensor_check_id_usr_cb(struct i2c_client *client,struct rk_sensor_reg *series)
{
	return 0;
}

static int sensor_suspend(struct soc_camera_device *icd, pm_message_t pm_msg)
{
	//struct i2c_client *client = to_i2c_client(to_soc_camera_control(icd));
		
	if (pm_msg.event == PM_EVENT_SUSPEND) {
		SENSOR_DG("Suspend");
		
	} else {
		SENSOR_TR("pm_msg.event(0x%x) != PM_EVENT_SUSPEND\n",pm_msg.event);
		return -EINVAL;
	}
	return 0;
}

static int sensor_resume(struct soc_camera_device *icd)
{

	SENSOR_DG("Resume");

	return 0;

}
static int sensor_mirror_cb (struct i2c_client *client, int mirror)
{
	char val;
	int err = 0;
    
    SENSOR_DG("mirror: %d",mirror);
	if (mirror) {
		err = sensor_read(client, 0x3022, &val);
		if (err == 0) {
			val |= 0x02;
			err = sensor_write(client, 0x3022, val);
		}
	} else {
		err = sensor_read(client, 0x3022, &val);
		if (err == 0) {
			val &= 0xfd;
			err = sensor_write(client, 0x3022, val);
		}
	}

	return err;    
}
/*
* the function is v4l2 control V4L2_CID_HFLIP callback  
*/
static int sensor_v4l2ctrl_mirror_cb(struct soc_camera_device *icd, struct sensor_v4l2ctrl_info_s *ctrl_info, 
                                                     struct v4l2_ext_control *ext_ctrl)
{
	struct i2c_client *client = to_i2c_client(to_soc_camera_control(icd));

    if (sensor_mirror_cb(client,ext_ctrl->value) != 0)
		SENSOR_TR("sensor_mirror failed, value:0x%x",ext_ctrl->value);
	
	SENSOR_DG("sensor_mirror success, value:0x%x",ext_ctrl->value);
	return 0;
}

static int sensor_flip_cb(struct i2c_client *client, int flip)
{
	char val;
	int err = 0;	

    SENSOR_DG("flip: %d",flip);
	if (flip) {
		err = sensor_read(client, 0x3022, &val);
		if (err == 0) {
			val |= 0x01;
			err = sensor_write(client, 0x3022, val);
		}
	} else {
		err = sensor_read(client, 0x3022, &val);
		if (err == 0) {
			val &= 0xfe;
			err = sensor_write(client, 0x3022, val);
		}
	}

	return err;    
}
/*
* the function is v4l2 control V4L2_CID_VFLIP callback  
*/
static int sensor_v4l2ctrl_flip_cb(struct soc_camera_device *icd, struct sensor_v4l2ctrl_info_s *ctrl_info, 
                                                     struct v4l2_ext_control *ext_ctrl)
{
	struct i2c_client *client = to_i2c_client(to_soc_camera_control(icd));

    if (sensor_flip_cb(client,ext_ctrl->value) != 0)
		SENSOR_TR("sensor_flip failed, value:0x%x",ext_ctrl->value);
	
	SENSOR_DG("sensor_flip success, value:0x%x",ext_ctrl->value);
	return 0;
}

/*
AF call back
*/
static int sensor_focus_init_usr_cb(struct i2c_client *client){
	return 0;
}

static int sensor_focus_af_single_usr_cb(struct i2c_client *client){
	return 0;
}

static int sensor_focus_af_near_usr_cb(struct i2c_client *client){
	return 0;
}

static int sensor_focus_af_far_usr_cb(struct i2c_client *client){
	return 0;
}

static int sensor_focus_af_specialpos_usr_cb(struct i2c_client *client,int pos){
	return 0;
}

static int sensor_focus_af_const_usr_cb(struct i2c_client *client){
	return 0;
}
static int sensor_focus_af_const_pause_usr_cb(struct i2c_client *client)
{
    return 0;
}
static int sensor_focus_af_close_usr_cb(struct i2c_client *client){
	return 0;
}

static int sensor_focus_af_zoneupdate_usr_cb(struct i2c_client *client, int *zone_tm_pos)
{
	return 0;
}

/*
face defect call back
*/
static int 	sensor_face_detect_usr_cb(struct i2c_client *client,int on){
	return 0;
}

/*
*   The function can been run in sensor_init_parametres which run in sensor_probe, so user can do some
* initialization in the function. 
*/
static void sensor_init_parameters_user(struct specific_sensor* spsensor,struct soc_camera_device *icd)
{
    return;
}

/*
* :::::WARNING:::::
* It is not allowed to modify the following code
*/

sensor_init_parameters_default_code();

sensor_v4l2_struct_initialization();

sensor_probe_default_code();

sensor_remove_default_code();

sensor_driver_default_module_code();



