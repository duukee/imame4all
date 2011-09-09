/*

  GP2X minimal library v0.A by rlyeh, (c) 2005. emulnation.info@rlyeh (swap it!)

  Thanks to Squidge, Robster, snaff, Reesy and NK, for the help & previous work! :-)

  License
  =======

  Free for non-commercial projects (it would be nice receiving a mail from you).
  Other cases, ask me first.

  GamePark Holdings is not allowed to use this library and/or use parts from it.

*/


#include "minimal.h"
#include "wiimote.h"
#include "driver.h"

#include <unistd.h>
#include <fcntl.h>

#import <AudioToolbox/AudioQueue.h>
#import <AudioToolbox/AudioToolbox.h>
#import <pthread.h>

/* Audio Resources */
//SQ minimum required buffers for iOS AudioQueue
#define AUDIO_BUFFERS 3


extern "C" void app_MuteSound(void);
extern "C" void app_DemuteSound(void);
extern "C" void iphone_UpdateScreen(void);
extern "C" void iphone_Reset_Views(void);
int sound_close_AudioQueue();

int iOS_video_width = 320;
int iOS_video_height = 240;

typedef struct AQCallbackStruct {
    AudioQueueRef queue;
    UInt32 frameCount;
    AudioQueueBufferRef mBuffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription mDataFormat;
} AQCallbackStruct;

typedef unsigned char byte;
extern unsigned short *screenbuffer;

extern int soundcard;

AQCallbackStruct in;
int soundInit = 0;
struct timeval ptv;
float __audioVolume = 1.0;


unsigned long 			gp2x_dev[3];
static unsigned long 		gp2x_ticks_per_second;
unsigned char 			*gp2x_screen8;
unsigned short 			*gp2x_screen15;
unsigned short 			*gp2x_logvram15[2];
unsigned long 			gp2x_physvram[2];
unsigned int			gp2x_nflip;
unsigned short 	gp2x_palette[512];
int				gp2x_sound_rate=44100;
int				gp2x_sound_stereo=0;
int 				gp2x_pal_50hz=0;
int				gp2x_ram_tweaks=0;
int 				rotate_controls=0;

unsigned char  			*gp2x_dualcore_ram;
unsigned long  			 gp2x_dualcore_ram_size;

unsigned long gp2x_pad_status = 0;

void gp2x_video_flip(void)
{
	iphone_UpdateScreen();
}

void gp2x_video_flip_single(void)
{
	iphone_UpdateScreen();
}


void gp2x_video_setpalette(void)
{
}

/*
int master_volume;
*/

unsigned long gp2x_joystick_read(int n)
{
  unsigned long res=0;
	res = gp2x_pad_status;
		
	/* GP2X F200 Push Button */
	if ((res & GP2X_VOL_UP) && (res & GP2X_VOL_DOWN))
	  		res |= GP2X_PUSH;

	if (num_of_joys>n)
	{
	  	/* Check USB Joypad */
		//printf("%d %d\n",num_of_joys,n);
		res |= iOS_wiimote_check(&joys[n]);
	}
  	
	return res;
}

unsigned long gp2x_joystick_press (int n)
{
	unsigned long ExKey=0;
	/*
	while(gp2x_joystick_read(n)&0x8c0ff55) { gp2x_timer_delay(150); }
	while(!(ExKey=gp2x_joystick_read(n)&0x8c0ff55)) { }
	*/
	while(gp2x_joystick_read(n)==0) { gp2x_timer_delay(150); }
	//while(!(ExKey=gp2x_joystick_read(n))) { }


    //while(!(ExKey & (JOY_BUTTON_X|JOY_BUTTON_B))) {nKeys = joystick_read();gp2x_timer_delay(150);} // para quieto!!
    //while(ExKey & (JOY_BUTTON_X|JOY_BUTTON_B)) {nKeys = joystick_read(); gp2x_timer_delay(150);} // para quieto!!

	return ExKey;
}

void gp2x_sound_volume(int l, int r)
{
	//__audioVolume = l;
}


void gp2x_timer_delay(unsigned long ticks)
{
	unsigned long long ini=gp2x_timer_read();
	//printf("ini %ld %ld %d\n",gp2x_timer_read(),ini,(gp2x_timer_read()-ini));
	while (gp2x_timer_read()-ini<ticks){/*nope*/};
}


unsigned long long gp2x_timer_read(void)
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	
	return ((unsigned long long)current_time.tv_sec * 1000LL + (current_time.tv_usec / 1000LL));
}

unsigned long long gp2x_timer_read_real(void)
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	
	return (((unsigned long long)current_time.tv_sec * 1000000LL + current_time.tv_usec));
}

unsigned long gp2x_timer_read_scale(void)
{
 	return gp2x_ticks_per_second;
}

void gp2x_timer_profile(void)
{
}



#if defined(__cplusplus)
extern "C" {
#endif
extern int fcloseall(void);
#if defined(__cplusplus)
}
#endif



void gp2x_set_clock(int mhz)
{
}

void gp2x_set_video_mode(int bpp,int width,int height)
{

	//if(iOS_video_width!=width || iOS_video_height!=height)
	//{
	  iOS_video_width = width;
	  iOS_video_height = height;

	  iphone_Reset_Views();
	//}

	memset(gp2x_screen15, 0, iOS_video_width*iOS_video_height*2);

  	gp2x_video_flip();

  	gp2x_video_flip_single();

	gp2x_pal_50hz=0;
}

void set_ram_tweaks(void)
{
}

static unsigned char fontdata8x8[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x3C,0x42,0x99,0xBD,0xBD,0x99,0x42,0x3C,0x3C,0x42,0x81,0x81,0x81,0x81,0x42,0x3C,
	0xFE,0x82,0x8A,0xD2,0xA2,0x82,0xFE,0x00,0xFE,0x82,0x82,0x82,0x82,0x82,0xFE,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x64,0x74,0x7C,0x38,0x00,0x00,
	0x80,0xC0,0xF0,0xFC,0xF0,0xC0,0x80,0x00,0x01,0x03,0x0F,0x3F,0x0F,0x03,0x01,0x00,
	0x18,0x3C,0x7E,0x18,0x7E,0x3C,0x18,0x00,0xEE,0xEE,0xEE,0xCC,0x00,0xCC,0xCC,0x00,
	0x00,0x00,0x30,0x68,0x78,0x30,0x00,0x00,0x00,0x38,0x64,0x74,0x7C,0x38,0x00,0x00,
	0x3C,0x66,0x7A,0x7A,0x7E,0x7E,0x3C,0x00,0x0E,0x3E,0x3A,0x22,0x26,0x6E,0xE4,0x40,
	0x18,0x3C,0x7E,0x3C,0x3C,0x3C,0x3C,0x00,0x3C,0x3C,0x3C,0x3C,0x7E,0x3C,0x18,0x00,
	0x08,0x7C,0x7E,0x7E,0x7C,0x08,0x00,0x00,0x10,0x3E,0x7E,0x7E,0x3E,0x10,0x00,0x00,
	0x58,0x2A,0xDC,0xC8,0xDC,0x2A,0x58,0x00,0x24,0x66,0xFF,0xFF,0x66,0x24,0x00,0x00,
	0x00,0x10,0x10,0x38,0x38,0x7C,0xFE,0x00,0xFE,0x7C,0x38,0x38,0x10,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x1C,0x1C,0x18,0x00,0x18,0x18,0x00,
	0x6C,0x6C,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x7C,0x28,0x7C,0x28,0x00,0x00,
	0x10,0x38,0x60,0x38,0x0C,0x78,0x10,0x00,0x40,0xA4,0x48,0x10,0x24,0x4A,0x04,0x00,
	0x18,0x34,0x18,0x3A,0x6C,0x66,0x3A,0x00,0x18,0x18,0x20,0x00,0x00,0x00,0x00,0x00,
	0x30,0x60,0x60,0x60,0x60,0x60,0x30,0x00,0x0C,0x06,0x06,0x06,0x06,0x06,0x0C,0x00,
	0x10,0x54,0x38,0x7C,0x38,0x54,0x10,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x04,0x08,0x10,0x20,0x40,0x00,0x00,
	0x38,0x4C,0xC6,0xC6,0xC6,0x64,0x38,0x00,0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00,
	0x7C,0xC6,0x0E,0x3C,0x78,0xE0,0xFE,0x00,0x7E,0x0C,0x18,0x3C,0x06,0xC6,0x7C,0x00,
	0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x00,0xFC,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00,
	0x3C,0x60,0xC0,0xFC,0xC6,0xC6,0x7C,0x00,0xFE,0xC6,0x0C,0x18,0x30,0x30,0x30,0x00,
	0x78,0xC4,0xE4,0x78,0x86,0x86,0x7C,0x00,0x7C,0xC6,0xC6,0x7E,0x06,0x0C,0x78,0x00,
	0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x18,0x18,0x30,
	0x1C,0x38,0x70,0xE0,0x70,0x38,0x1C,0x00,0x00,0x7C,0x00,0x00,0x7C,0x00,0x00,0x00,
	0x70,0x38,0x1C,0x0E,0x1C,0x38,0x70,0x00,0x7C,0xC6,0xC6,0x1C,0x18,0x00,0x18,0x00,
	0x3C,0x42,0x99,0xA1,0xA5,0x99,0x42,0x3C,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00,
	0xFC,0xC6,0xC6,0xFC,0xC6,0xC6,0xFC,0x00,0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00,
	0xF8,0xCC,0xC6,0xC6,0xC6,0xCC,0xF8,0x00,0xFE,0xC0,0xC0,0xFC,0xC0,0xC0,0xFE,0x00,
	0xFE,0xC0,0xC0,0xFC,0xC0,0xC0,0xC0,0x00,0x3E,0x60,0xC0,0xCE,0xC6,0x66,0x3E,0x00,
	0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,
	0x06,0x06,0x06,0x06,0xC6,0xC6,0x7C,0x00,0xC6,0xCC,0xD8,0xF0,0xF8,0xDC,0xCE,0x00,
	0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0x00,
	0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0xFC,0xC6,0xC6,0xC6,0xFC,0xC0,0xC0,0x00,0x7C,0xC6,0xC6,0xC6,0xDE,0xCC,0x7A,0x00,
	0xFC,0xC6,0xC6,0xCE,0xF8,0xDC,0xCE,0x00,0x78,0xCC,0xC0,0x7C,0x06,0xC6,0x7C,0x00,
	0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,
	0xC6,0xC6,0xC6,0xEE,0x7C,0x38,0x10,0x00,0xC6,0xC6,0xD6,0xFE,0xFE,0xEE,0xC6,0x00,
	0xC6,0xEE,0x3C,0x38,0x7C,0xEE,0xC6,0x00,0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00,
	0xFE,0x0E,0x1C,0x38,0x70,0xE0,0xFE,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,
	0x60,0x60,0x30,0x18,0x0C,0x06,0x06,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,
	0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x06,0x3E,0x66,0x66,0x3C,0x00,
	0x60,0x7C,0x66,0x66,0x66,0x66,0x7C,0x00,0x00,0x3C,0x66,0x60,0x60,0x66,0x3C,0x00,
	0x06,0x3E,0x66,0x66,0x66,0x66,0x3E,0x00,0x00,0x3C,0x66,0x66,0x7E,0x60,0x3C,0x00,
	0x1C,0x30,0x78,0x30,0x30,0x30,0x30,0x00,0x00,0x3E,0x66,0x66,0x66,0x3E,0x06,0x3C,
	0x60,0x7C,0x76,0x66,0x66,0x66,0x66,0x00,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x00,
	0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x0C,0x38,0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00,
	0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0xEC,0xFE,0xFE,0xFE,0xD6,0xC6,0x00,
	0x00,0x7C,0x76,0x66,0x66,0x66,0x66,0x00,0x00,0x3C,0x66,0x66,0x66,0x66,0x3C,0x00,
	0x00,0x7C,0x66,0x66,0x66,0x7C,0x60,0x60,0x00,0x3E,0x66,0x66,0x66,0x3E,0x06,0x06,
	0x00,0x7E,0x70,0x60,0x60,0x60,0x60,0x00,0x00,0x3C,0x60,0x3C,0x06,0x66,0x3C,0x00,
	0x30,0x78,0x30,0x30,0x30,0x30,0x1C,0x00,0x00,0x66,0x66,0x66,0x66,0x6E,0x3E,0x00,
	0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0xC6,0xD6,0xFE,0xFE,0x7C,0x6C,0x00,
	0x00,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x06,0x3C,
	0x00,0x7E,0x0C,0x18,0x30,0x60,0x7E,0x00,0x0E,0x18,0x0C,0x38,0x0C,0x18,0x0E,0x00,
	0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00,0x70,0x18,0x30,0x1C,0x30,0x18,0x70,0x00,
	0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x10,0x28,0x10,0x54,0xAA,0x44,0x00,0x00,
};

static void gp2x_text(unsigned short *screen, int x, int y, char *text, int color)
{
	unsigned int i,l;
	screen=screen+x+y*320;

	for (i=0;i<strlen(text);i++) {
		
		for (l=0;l<8;l++) {
			screen[l*320+0]=(fontdata8x8[((text[i])*8)+l]&0x80)?gp2x_palette[color]:screen[l*320+0];
			screen[l*320+1]=(fontdata8x8[((text[i])*8)+l]&0x40)?gp2x_palette[color]:screen[l*320+1];
			screen[l*320+2]=(fontdata8x8[((text[i])*8)+l]&0x20)?gp2x_palette[color]:screen[l*320+2];
			screen[l*320+3]=(fontdata8x8[((text[i])*8)+l]&0x10)?gp2x_palette[color]:screen[l*320+3];
			screen[l*320+4]=(fontdata8x8[((text[i])*8)+l]&0x08)?gp2x_palette[color]:screen[l*320+4];
			screen[l*320+5]=(fontdata8x8[((text[i])*8)+l]&0x04)?gp2x_palette[color]:screen[l*320+5];
			screen[l*320+6]=(fontdata8x8[((text[i])*8)+l]&0x02)?gp2x_palette[color]:screen[l*320+6];
			screen[l*320+7]=(fontdata8x8[((text[i])*8)+l]&0x01)?gp2x_palette[color]:screen[l*320+7];
		}
		screen+=8;
	} 
}

void gp2x_gamelist_text_out(int x, int y, char *eltexto)
{
	char texto[33];
	strncpy(texto,eltexto,32);
	texto[32]=0;
	if (texto[0]!='-')
		gp2x_text(gp2x_screen15,x+1,y+1,texto,0);
	gp2x_text(gp2x_screen15,x,y,texto,255);
}

/* Variadic functions guide found at http://www.unixpapa.com/incnote/variadic.html */
void gp2x_gamelist_text_out_fmt(int x, int y, char* fmt, ...)
{
	char strOut[128];
	va_list marker;
	
	va_start(marker, fmt);
	vsprintf(strOut, fmt, marker);
	va_end(marker);	

	gp2x_gamelist_text_out(x, y, strOut);
}

static int pflog=0;

void gp2x_printf_init(void)
{
	pflog=0;
}

static void gp2x_text_log(char *texto)
{
	if (!pflog)
	{
		memset(gp2x_screen15,0,320*240*2);
	}
	gp2x_text(gp2x_screen15,0,pflog,texto,255);
	pflog+=8;
	if(pflog>239) pflog=0;
}

/* Variadic functions guide found at http://www.unixpapa.com/incnote/variadic.html */
void gp2x_printf(char* fmt, ...)
{
	int i,c;
	char strOut[4096];
	char str[41];
	va_list marker;
	
	va_start(marker, fmt);
	vsprintf(strOut, fmt, marker);
	va_end(marker);	

	fprintf(stderr, "%s\n", strOut);
	
	c=0;
	for (i=0;i<strlen(strOut);i++)
	{
		str[c]=strOut[i];
		if (str[c]=='\n')
		{
			str[c]=0;
			gp2x_text_log(str);
			c=0;
		}
		else if (c==39)
		{
			str[40]=0;
			gp2x_text_log(str);
			c=0;
		}		
		else
		{
			c++;
		}
	}
}


void gp2x_sound_thread_mute(void)
{
}

void gp2x_sound_thread_start(int len)
{
	//fprintf(stderr,"audio buffer size %d \n", audioBufferSize);
	gp2x_sound_thread_mute();
}

void gp2x_sound_thread_stop(void)
{
    sound_close_AudioQueue();
}

void gp2x_sound_set_rate(int rate)
{
}

void gp2x_sound_set_stereo(int stereo)
{
}

void gp2x_video_wait_vsync()
{
}

extern "C" void set_save_state()
{
}


////////////////////////////////////

static int lib_inited = 0;

pthread_mutex_t mut;

void gp2x_init(int ticks_per_second, int bpp, int rate, int bits, int stereo, int Hz)
{

    if (!lib_inited )
    {

       gp2x_ticks_per_second=1000;

	   //gp2x_screen15=screenbuffer;
	   //gp2x_screen8=(unsigned char *)screenbuffer;
	   gp2x_nflip=0;

	   gp2x_set_video_mode(bpp,320,240);

	   gp2x_video_color8(0,0,0,0);
	   gp2x_video_color8(255,255,255,255);
	   gp2x_video_setpalette();

	   /* atexit(gp2x_deinit); */
	   //app_DemuteSound();

	   pthread_mutex_init (&mut,NULL);
   	   lib_inited = 1;
    }
}

void gp2x_deinit(void)
{
    if (lib_inited )
    {
	   //memset(gp2x_screen15, 0, 320*240*2); gp2x_video_flip();

    	pthread_mutex_destroy(&mut);
        lib_inited = 0;
    }
}

/// QUEUE
//TODO quidado.. que va a 60hz no a 50
//SQ buffers for sound between MAME and iOS AudioQueue. AudioQueue
//SQ callback reads from these.
//SQ Size: (44100/30fps) * bytesize * stereo * (3 buffers)
#define TAM (1470 * 2 * 2 * 3)
unsigned char ptr_buf[TAM];
unsigned head = 0;
unsigned tail = 0;

inline int fullQueue(unsigned short size){

    if(head < tail)
	{
		return head + size >= tail;
	}
	else if(head > tail)
	{
		return (head + size) >= TAM ? (head + size)- TAM >= tail : false;
	}
	else return false;
}

inline int emptyQueue(){
	return head == tail;
}

void queue(unsigned char *p,unsigned size){
        unsigned newhead;
		if(head + size < TAM)
		{
			memcpy(ptr_buf+head,p,size);
			newhead = head + size;
		}
		else
		{
			memcpy(ptr_buf+head,p, TAM -head);
			memcpy(ptr_buf,p + (TAM-head), size - (TAM-head));
			newhead = (head + size) - TAM;
		}
		pthread_mutex_lock(&mut);

		head = newhead;

		pthread_mutex_unlock(&mut);
}

unsigned short dequeue(unsigned char *p,unsigned size){

    	unsigned real;

		if(emptyQueue())
		{
	    	memset(p,0,size);//TODO ver si quito para que no petardee
			return size;
		}

		pthread_mutex_lock(&mut);

		unsigned datasize = head > tail ? head - tail : (TAM - tail) + head ;
		real = datasize > size ? size : datasize;

		if(tail + real < TAM)
		{
			memcpy(p,ptr_buf+tail,real);
			tail+=real;
		}
		else
		{
			memcpy(p,ptr_buf + tail, TAM - tail);
			memcpy(p+ (TAM-tail),ptr_buf , real - (TAM-tail));
			tail = (tail + real) - TAM;
		}

		pthread_mutex_unlock(&mut);

        return real;
}

int stereo_cached = 0;
int bits_cached = 16;
int rate_cached = 44100;


void checkStatus(OSStatus status){}

static void AQBufferCallback(void *userdata,
							 AudioQueueRef outQ,
							 AudioQueueBufferRef outQB)
{
	unsigned char *coreAudioBuffer;
	coreAudioBuffer = (unsigned char*) outQB->mAudioData;

	int res = dequeue(coreAudioBuffer, in.mDataFormat.mBytesPerFrame * in.frameCount);
	outQB->mAudioDataByteSize = in.mDataFormat.mBytesPerFrame * in.frameCount;

	AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);
}


int sound_close_AudioQueue(){

	if( soundInit == 1 )
	{

		AudioQueueDispose(in.queue, true);
		soundInit = 0;
        head = 0;
        tail = 0;
	}
}

int sound_open_AudioQueue(int rate, int bits, int stereo){

    Float64 sampleRate = 44100.0;
    int i;
    int fps;
    
    stereo_cached = stereo;
    rate_cached = rate;
    bits_cached = bits;

    if(rate==32000)
    	sampleRate = 32000.0;
    else if(rate==22050)
    	sampleRate = 22050.0;
    else if(rate==11025)
    	sampleRate = 11025.0;

	//SQ Roundup for games like Galaxians
    fps = ceil(Machine->drv->frames_per_second);

    if( soundInit == 1 )
    {
    	sound_close_AudioQueue();
    }

    soundInit = 0;
    memset (&in.mDataFormat, 0, sizeof (in.mDataFormat));
    in.mDataFormat.mSampleRate = sampleRate;
    in.mDataFormat.mFormatID = kAudioFormatLinearPCM;
    in.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger  | kAudioFormatFlagIsPacked;
    in.mDataFormat.mBytesPerPacket =  (stereo == 1 ? 4 : 2 );
    in.mDataFormat.mFramesPerPacket = 1;
    in.mDataFormat.mBytesPerFrame = (stereo ==  1? 4 : 2);
    in.mDataFormat.mChannelsPerFrame = (stereo == 1 ? 2 : 1);
    in.mDataFormat.mBitsPerChannel = 16;
	in.frameCount = rate / fps;
/*
    printf("Sound format %f %d %d %d %d %d %d\n",
            in.mDataFormat.mSampleRate,
    		in.mDataFormat.mBytesPerPacket,
    		in.mDataFormat.mFramesPerPacket,
    		in.mDataFormat.mBytesPerFrame,
    		in.mDataFormat.mChannelsPerFrame,
    		in.mDataFormat.mBitsPerChannel,
            in.frameCount);
*/
    /* Pre-buffer before we turn on audio */
    UInt32 err;
    err = AudioQueueNewOutput(&in.mDataFormat,
							  AQBufferCallback,
							  NULL,
							  NULL,
							  kCFRunLoopCommonModes,
							  0,
							  &in.queue);

    //printf("res %ld",err);

    unsigned long bufferSize;
    bufferSize = in.frameCount * in.mDataFormat.mBytesPerFrame;
    
	for (i=0; i<AUDIO_BUFFERS; i++)
	{
		err = AudioQueueAllocateBuffer(in.queue, bufferSize, &in.mBuffers[i]);
		in.mBuffers[i]->mAudioDataByteSize = bufferSize;
		AudioQueueEnqueueBuffer(in.queue, in.mBuffers[i], 0, NULL);
	}

	soundInit = 1;
	err = AudioQueueStart(in.queue, NULL);

	return 0;

}

extern "C" void app_MuteSound(void) {
	if( soundInit == 1 )
	{
		sound_close_AudioQueue();
	}
}

extern "C" void app_DemuteSound(void) {
	if( soundInit == 0 && soundcard!=0)
	{
		  sound_open_AudioQueue(gp2x_sound_rate, bits_cached, gp2x_sound_stereo);
	}
}

void gp2x_sound_play(void *buff, int len)
{
	queue((unsigned char *)buff,len);
}

///END QUEUE
