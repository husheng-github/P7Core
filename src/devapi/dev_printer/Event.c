#include "includes.h"

//======================================================================================================
static volatile uint8_t evtHead;
static uint8_t evtTail;
static uint8_t evtQueue[64];
//======================================================================================================

//======================================================================================================
extern void event_init(void)
{
	evtHead = evtTail = 0;
}
//======================================================================================================
extern void event_post(uint8_t event)
{
	uint32_t head;

	if(event)
	{
         // Disable Interrupts
		head = evtHead;
		evtHead = (head + 1) & (ARRAY_SIZE(evtQueue) - 1);
		// restore flag

		evtQueue[head] = event;
	}
}
//======================================================================================================
//======================================================================================================
extern uint8_t event_pend(void)
{
	uint8_t event;

	if(evtHead == evtTail)
	{
		return evtNULL;
	}
	event = evtQueue[evtTail];
	evtTail = (evtTail + 1) & (ARRAY_SIZE(evtQueue) - 1);
	return event;
}
//======================================================================================================
extern void event_proc(void)
{
	switch(event_pend())
	{
	//-----------------------------------------------------------------
	case evtKeyDownFeed:
        #if defined(PT488)
		#ifdef DEBUG_TEST
		TPSelfTest_1();
		#else
		TPFeedStart();
		#endif
        #endif
		break;
	case evtKeyUpFeed:
        #if defined(PT488)
		TPFeedStop();
        #endif
		break;
	case evtKeyDownHold500msMode:
        break;
	case evtKeyDownMode:
		#if 1
		if(TPPrinterReady())
        TPPrintTestPage();
		#endif
		break;
	case evtKeyDownHold2000msMode:
		if(TPPrinterReady())
        TPPrintTestPage();
		break;
	case evtKeyUpMode:
		break;
	case evtKeyHoldMode:
		break;
	case evtKeyDownHold5000msMode:
		break;
    case evtKeyDownHold7000msMode:
        break;
	case evtPaperOut:
        esc_sts.status4 |= (0x03<<5);
		break;
	case evtPaperIn:
        printf("evtPaperIn\n");
        esc_sts.status4 &= ~(0x03<<5);
  //      Wake_up();
        break;
    case evtBmDetect:
        break;
    case evtGetRealTimeStatus4:
       Putchar(esc_sts.status4);
        break;
	case evtLifetest:
	   #if defined(POWER_SELFTEST)
	   TPSelfTest_1();
	   #endif
	   break;
	default:
		break;
	}
}
//======================================================================================================




