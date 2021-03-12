

#include "bootglobal.h"


//lcd display
#ifdef PROMPT_LCDDISPLAY_SUPPORT
void bootdownload_prompt_cleardisprow(u8 rowno)
{
    if(rowno == BIOSSECLINE1)
    {
        dev_lcd_clear_rowram(BIOSSECLINE1);
    }
    else if(rowno == BIOSSECLINE2)
    {
        dev_lcd_clear_rowram(BIOSSECLINE2);
        
    }
}


void bootdownload_prompt_fill_row(u8 pageid, u32 colid, u8 *str, u8 atr)
{
    dev_lcd_fill_rowram(pageid, colid, str, atr);
	
}

void bootdownload_prompt_brush()
{
    dev_lcd_brush_screen();
	
}
#endif



void bootdownload_prompt(u8 prompttype,u8 *promptlen ,u8 *prompt)
{
    
	s8 executionprompt[20];
	
    memset(executionprompt,0,sizeof(executionprompt));
    
    if(prompttype == EXECUTION_MEMORY)
    {
        strcpy(executionprompt,PROMPT_EXE_MEMORY);
    }
    else if(prompttype == EXECUTION_HARDWAREVERSION)
    {
        strcpy(executionprompt,PROMPT_EXE_HARDWAREVERSION);
    }
    else if(prompttype == EXECUTION_DOWNFILENUM)
    {
        strcpy(executionprompt,PROMPT_EXE_DOWNFILENUM);
    }
    else if(prompttype == EXECUTION_DOWNFILEOPEN)
    {
        strcpy(executionprompt,PROMPT_EXE_DOWNFILEOPEN);
    }
    else if(prompttype == EXECUTION_DOWNFILEOPS)
    {
        strcpy(executionprompt,PROMPT_EXE_DOWNFILEOPS);
    }
    else if(prompttype == EXECUTION_MACHINEMODE)
    {
        strcpy(executionprompt,PROMPT_EXE_MACHINEMODE);
    }
    else if(prompttype == EXECUTION_DISKNOTENOUGH)
    {
        strcpy(executionprompt,PROMPT_EXE_DISKNOTENOUGH);
    }
    else if(prompttype == EXECUTION_DOWNFILENAME)
    {
        strcpy(executionprompt,PROMPT_EXE_DOWNFILENAME);
    }
    else if(prompttype == EXECUTION_CRCERR)
    {
        strcpy(executionprompt,PROMPT_EXE_CRCERR);
    }
    else if(prompttype == DOWNLOADFILESUCCESS)
    {
        strcpy(executionprompt,PROMPT_DOWNLOADFILESUCCESS);
    }
    else if(prompttype == DOWNLOADAUTHENTICATE)
    {
        strcpy(executionprompt,PROMPT_DOWNLOADAUTHENTICATE);
    }
    else if(prompttype == AUTHENTICATEFAIL)
    {
        strcpy(executionprompt,PROMPT_AUTHENTICATEFAIL);
    }
    else if(prompttype == EXECUTION_SAVEINFOERROR)
    {
        strcpy(executionprompt,PROMPT_SAVEINFOERROR);
    }
    else if(prompttype == EXECUTION_TRENDITPKNOTEXIST)
    {
        strcpy(executionprompt,PROMPT_TRENDITPKNOTEXIST);
    }
    else if(prompttype == EXECUTION_ACQUIRERPKAUTHENFAIL)
    {
        strcpy(executionprompt,PROMPT_ACQUIRERPKAUTHENFAIL);
    }
    else if(prompttype == EXECUTION_APPVERIFYFAIL)
    {
        strcpy(executionprompt,PROMPT_APPVERIFYFAIL);
    }
    else if(prompttype == EXECUTION_PKNOTSUPPORT)
    {
        strcpy(executionprompt,PROMPT_PKNOTSUPPORT);
    }
    else
    {
        strcpy(executionprompt,PROMPT_EXE_OTHER);
    }

  #ifdef PROMPT_LCDDISPLAY_SUPPORT
	bootdownload_prompt_cleardisprow(BIOSSECLINE2);
    bootdownload_prompt_fill_row(BIOSSECLINE2,0,(u8 *)executionprompt,BIOSSECFDISP|BIOSSECCDISP);
	bootdownload_prompt_brush();
  #endif	

   
	if(prompt != NULL&&promptlen != NULL)
	{
	     
		strcpy(prompt,executionprompt);
		*promptlen = strlen(executionprompt);
		
	}

	
}


void bootdownload_prompt_downloading(u8 *filename)
{
  #ifdef PROMPT_LCDDISPLAY_SUPPORT
	bootdownload_beep(BOOTDOWNLOAD_BEEP_NORMAL);

	bootdownload_prompt_cleardisprow(BIOSSECLINE1);
    bootdownload_prompt_cleardisprow(BIOSSECLINE2);
    bootdownload_prompt_fill_row(BIOSSECLINE1,0,(u8 *)PROMPT_DOWNLOADING,BIOSSECFDISP|BIOSSECCDISP);
    bootdownload_prompt_fill_row(BIOSSECLINE2,0,filename,BIOSSECFDISP|BIOSSECCDISP);
	bootdownload_prompt_brush();
	
  #endif
}

