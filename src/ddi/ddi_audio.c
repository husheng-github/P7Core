
#include "devglobal.h"
#include "ddi_audio.h"

#ifdef DDI_AUDIO_ENABLE

s32 ddi_audio_open (void)
{
    return dev_audio_open();
}



s32 ddi_audio_close (void)
{
    return dev_audio_close();
}



s32 ddi_audio_play(const u8 * lpFile)
{
    return dev_audio_play(lpFile);
}

s32 ddi_audio_playcharacters(str_audio_tts audio_tts)
{
    if(!drv_systick_get_run_switch())
    {
        TRACE_INFO("ignore the audio play");
        return DDI_EBUSY;
    }
    
    return dev_audio_playcharacters(audio_tts.m_texttype, audio_tts.m_text);
}

s32 ddi_audio_stop (void)
{
    return dev_audio_stop();
}

s32 ddi_audio_pause (void)
{
    return dev_audio_pause();
}


s32 ddi_audio_resumeplay(void)
{
    return dev_audio_replay();
}

s32 ddi_audio_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    return dev_audio_ioctl(nCmd, lParam, wParam);
}


#else


s32 ddi_audio_open (void)
{
	return DDI_ENODEV;
}



s32 ddi_audio_close (void)
{
	return DDI_ENODEV;
}



s32 ddi_audio_play(const u8 * lpFile)
{
	return DDI_ENODEV;
}


s32 ddi_audio_stop (void)
{
	return DDI_ENODEV;
}

s32 ddi_audio_pause (void)
{
	return DDI_ENODEV;
}


s32 ddi_audio_replay (void)
{
	return DDI_ENODEV;
}

s32 ddi_audio_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}



#endif

