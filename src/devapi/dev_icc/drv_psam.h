#ifndef __DRV_PSAM_H
#define __DRV_PSAM_H


typedef struct _STR_ICC_INFO
{
    u8 m_resettype;     //��¼����λ����
                        //0:û�и�λ
                        //1:�Ѹ�λ
    u8 m_exist;         //��Ƭ�Ƿ����
                        //����PSAM����һֱ��Ϊ�п�����1
    u8 m_power;         //��������ѹ
                        //0:û���ϵ�
                        //1:1.8v
                        //2:3.0v
                        //3:5.0v
    u8 m_protocolType;  //��¼��Ƭ��Э��
                        //0: T0��
                        //1: T1��
    u8 m_Nbit;          //��¼IC���ļ���
                        //0: ��ʾ������
                        //1: ��ʾ������
    u8 m_icfd;          //IC����FDֵ
                        //0: ��ʾF/D = 1
                        //1: ��ʾF/D = 2
                        //2: ��ʾF/D = 3
    u8 m_protecttime;   //����ʱ��
    u8 m_ifsi;          
    u8 m_ipcb;
    u8 m_nrpcb;
    u32 m_gt;
    u32 m_bwt;
    u32 m_cwt;
}str_icc_info_t;


typedef struct _icc_content{
    u16 m_lc;
    u16 m_le;
    u16 m_txlen;
    u16 m_txcnt;
    u16 m_rxlen;
//    u16 m_rxmax;                  //���ջ������󳤶�
    u16 m_rxcnt;                  //
    u8 m_txhead[5];               //����ͷ�� 
    u8 m_txheadlen;             //ͷ�򳤶�
    u8 m_txbuf[260];              //��������
    u8 m_rxbuf[260];              //�������� 
    u8 m_sw[2];
//    u8 m_swbak[2];      
    volatile u8 m_busy;         //��¼��ǰ����״̬
 //   volatile u8 m_opttype;      //��������:b7~4:������, 0:�ȴ�������wait
                                //                      1:��λ
                                //                      2:����T0��
                                //                      3:����T1��
                                //         b3~0:С����  �������;���ֲ�����
//    u8 m_casetype;              //��¼R-APDU����,1~4�ֱ�Ϊcase1,2,3,4
    u8 m_result;                //��¼���ؽ��
    u8 m_step;                  //��¼ִ�в���
    u8 m_gpcnt1user;            //0:��ʱ��,1:��ʱ��
    u8 m_curformat;             //��¼��ǰ��Э���ʽ
    u8 m_protocoltmp;  
    u8 m_parityerr;             //��¼��żУ���1:��żУ���
    u8 m_lasterrtype;
    u8 m_rfu[2];
}icc_content_t;

typedef enum ICC_T1_LASTSTATUS
{
    IC_T1_LASTOK       = 0 ,  //0:û��
    IC_T1_LASTEDCERR   = 1 ,  //1:У���������飬EDC����ʱ��
    IC_T1_LASTNODERR   = 2 ,  //2:NAD��                 
    IC_T1_LASTIERR     = 3 ,  //3:I����Ŵ��򳤶ȴ�0xFF��,��I��
    IC_T1_LASTRNOERR   = 4 ,  //4:R����Ŵ�
    IC_T1_LASTROTHERR  = 5 ,  //5:R��������
    IC_T1_LASTSLENERR  = 6 ,  //6:S�鳤�ȴ�
    IC_T1_LASTSINFERR  = 7 ,  //7:S��INF��
    IC_T1_LASTSNULLERR = 8 ,  //8:S��Ƿ�Ӧ��
    IC_T1_LASTSNREQERR = 9 ,  //9:S��Unknown����
    IC_T1_LASTTXERR    = 10,  //10:��һ�鴫����ش���һ��
    IC_T1_LASTABORTERR = 11,  //11:���������
}ICC_T1_LASTSTATUS_t;


typedef enum ICC_PROC_STATUS
{
    ICC_PROC_OK      = 0x00,    //дû�д���
    //ICC_PROC_READOK       = 0x01,    //��û�д���
    ICC_PROC_CWTOVER      = 0x02,    //�������ֽڳ�ʱ
    ICC_PROC_BWTOVER      = 0x03,    //��һ֡��ʱ
    ICC_PROC_T0READERR    = 0x05,    //ͬһ���ֽ�����5�δ���        
    ICC_PROC_T1PARITYERR  = 0x06,    //λУ�����,��Э�鲻ΪT0
    ICC_PROC_T1CRCERR     = 0x07,    //һ֡У�����
    ICC_PROC_T0WEITEERR   = 0x08,    //ͬһ���ֽ���д3�δ���
    ICC_PROC_SWERR        = 0x09,    //IC�����ع��̻�״̬�ִ���
    ICC_PROC_TSERR        = 0x0A,    //��λTS����
    ICC_PROC_BLOCKERR     = 0x0B,    //T1��ο��ش�����
    ICC_PROC_RESETERR     = 0x0C,    //��λ���󣨲�����EMV�淶��
    ICC_PROC_EXCHGINT     = 0x0D,    //���������ݱ��ж�  //20060828
        
    ICC_PROC_PARAERR      = 0x20,    //IC����������
    ICC_PROC_INFOERR      = 0x21,    //û��������IC������Ϣ
    ICC_PROC_NORESET      = 0x22,    //��������IC�Ŀ�û�и�λ
    ICC_PROC_NORESPOND    = 0x23,    //51û����Ӧ
    ICC_PROC_CANNCEL      = 0x24,    //���յ���λ����canncel����
    ICC_PROC_ABNORAL      = 0x25,    //IC�������쳣
    ICC_PROC_NOCARD       = 0x26,    //û��IC��
    
}ICC_PROC_STATUS_t;

//###########IC_LastERRType###############
#define ICC_CWT_DEFAULT     10180    //������B��ʱ����
#define ICC_BWT_DEFAULT     20160    //һ֡��ʱ����    

//=========================================
s32 drv_psam_init(void);
s32 drv_psam_open(s32 psamno);
s32 drv_psam_close(s32 psamno);
s32 drv_psam_poweroff(s32 psamno);
s32 drv_psam_reset(s32 psamno, u8 *lpAtr);
s32 drv_psam_exchange_apdu(s32 psamno, u8 *wbuf, u32 wlen, u8 *rbuf, u32 *rlen, u32 rxmax);



#endif
