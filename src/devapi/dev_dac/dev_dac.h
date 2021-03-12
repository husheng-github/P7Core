#ifndef __DEV_DAC_H
#define __DEV_DAC_H
#include "devapi/dev_audio/dev_audio.h"
typedef enum _DAC_DEVICE
{
    DEV_DAC = 0,  //DAC …Ë±∏
    DEV_DAC_MAX = 1
}DAC_DEVICE_t;


s32 dev_dac_open(void);
s32 dev_dac_close(void);
s32 dev_dac_tts_play(strttsData_t *ttsdata, s32 ttslen);
s32 dev_dac_play(u32 waveaddr);
s32 dev_dac_pause(void);
s32 dev_dac_replay(void);
s32 dev_dac_set_volume_level(int level);
s32 dev_dac_get_volume_level(void);
s32 dev_dac_is_playing(void);


#endif

