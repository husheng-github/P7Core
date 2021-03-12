


#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"


static s32 g_trng_fd = -1;


/*******************************************************************
Author:   
Function Name: s32 dev_trng_open (void)
Function Purpose:open trng
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
s32 dev_trng_open (void)
{
    if(g_trng_fd < 0)
    {
//        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TRNG, ENABLE);
        //TRNG->RNG_AMA |= TRNG_RNG_AMA_PD_TRNG0_Mask;
//        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TRNG, ENABLE);
//        mh_crypt_it_clear();
        g_trng_fd = 0;
    }
    return DEVSTATUS_SUCCESS;
}

void dev_trng_close (void)
{
    if(g_trng_fd >=0)
    {
//        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TRNG, DISABLE);
        g_trng_fd = -1;
    }
}

static s32 dev_trng_get(u32 *rand)
{
    u32 timerid;
    s32 ret;
    u8 flg = 0;

    /*if(g_trng_fd < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }*/
    //TRNG->RNG_AMA &= ~TRNG_RNG_AMA_PD_TRNG0_Mask;
    //TRNG->RNG_CSR &= ~TRNG_RNG_CSR_S128_TRNG0_Mask;
    timerid = dev_user_gettimeID();
    TRNG_Start(TRNG0);
    while(1)
    {
        if (0 == TRNG_Get(rand, TRNG0))
        {
            flg = 1;
            break;
        }

        if(dev_user_querrytimer(timerid, 10))
        {
            //³¬Ê±
            break;
        }
    }
    TRNG_Stop(TRNG0);

    if(flg)
    {
        return DEVSTATUS_SUCCESS;
    }
    else
    {
        return DEVSTATUS_ERR_FAIL;
    }
}
/*******************************************************************
Author:   
Function Name: s32 dev_trng_open (void)
Function Purpose:open trng
Input Paramters: 
Output Paramters:N/A
Return Value:      N/A
Remark: N/A
Modify: N/A
********************************************************************/
u32 dev_trng_read(void *trngdata,u32 trngdatalen)
{
    u32 i = 0;
    u32 j;
    //u8 rand[16];
    u32 rand[4];
    u8 *ap;

    u8 *randdata = (u8 *)trngdata;

    if(dev_trng_open() == DEVSTATUS_SUCCESS)
    {
        //SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TRNG, ENABLE);
        ap = (u8*)rand;
        for(i = 0;i< trngdatalen;)
        {
            j = 0;
            memset(ap, 0, 16);
            dev_trng_get(rand);
            while(j < 16)
            {
                if(ap[j] != 0)
                {
                    randdata[i++] = ap[j];
                }
                j++;
                if(i >= trngdatalen)
                {
                    //return i;
                    break;
                } 

            }
            
        }
        //SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TRNG, DISABLE);

    }
    dev_trng_close();
    /*else
    {
        return i;
    } */   

    return i;

}



void test_trng(void)
{
    
    u8 rng[16];
    u8 i;
    
    dev_debug_printf("\r\ntest trng:");
    dev_trng_read(rng,16);
    for(i = 0;i < 16;i++)
    {
        dev_debug_printf("%02x ",rng[i]);
    }
    dev_debug_printf("\r\n");
    
    
}


