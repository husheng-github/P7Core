#ifndef __DRV_PSAM_H
#define __DRV_PSAM_H


typedef struct _STR_ICC_INFO
{
    u8 m_resettype;     //记录卡复位类型
                        //0:没有复位
                        //1:已复位
    u8 m_exist;         //卡片是否存在
                        //对于PSAM卡，一直认为有卡处理，1
    u8 m_power;         //卡操作电压
                        //0:没有上电
                        //1:1.8v
                        //2:3.0v
                        //3:5.0v
    u8 m_protocolType;  //记录卡片的协议
                        //0: T0卡
                        //1: T1卡
    u8 m_Nbit;          //记录IC卡的极性
                        //0: 表示正极性
                        //1: 表示负极性
    u8 m_icfd;          //IC卡的FD值
                        //0: 表示F/D = 1
                        //1: 表示F/D = 2
                        //2: 表示F/D = 3
    u8 m_protecttime;   //保护时间
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
//    u16 m_rxmax;                  //接收缓冲的最大长度
    u16 m_rxcnt;                  //
    u8 m_txhead[5];               //发送头域 
    u8 m_txheadlen;             //头域长度
    u8 m_txbuf[260];              //发送数据
    u8 m_rxbuf[260];              //接收数据 
    u8 m_sw[2];
//    u8 m_swbak[2];      
    volatile u8 m_busy;         //记录当前工作状态
 //   volatile u8 m_opttype;      //操作类型:b7~4:大类型, 0:等待，用于wait
                                //                      1:复位
                                //                      2:操作T0卡
                                //                      3:操作T1卡
                                //         b3~0:小类型  针打大类型具体分操作。
//    u8 m_casetype;              //记录R-APDU类型,1~4分别为case1,2,3,4
    u8 m_result;                //记录返回结果
    u8 m_step;                  //记录执行步骤
    u8 m_gpcnt1user;            //0:超时用,1:延时用
    u8 m_curformat;             //记录当前的协议格式
    u8 m_protocoltmp;  
    u8 m_parityerr;             //记录奇偶校验错，1:奇偶校验错
    u8 m_lasterrtype;
    u8 m_rfu[2];
}icc_content_t;

typedef enum ICC_T1_LASTSTATUS
{
    IC_T1_LASTOK       = 0 ,  //0:没错
    IC_T1_LASTEDCERR   = 1 ,  //1:校验错误（奇检验，EDC，超时）
    IC_T1_LASTNODERR   = 2 ,  //2:NAD错                 
    IC_T1_LASTIERR     = 3 ,  //3:I块序号错，或长度错（0xFF）,或I块
    IC_T1_LASTRNOERR   = 4 ,  //4:R块序号错
    IC_T1_LASTROTHERR  = 5 ,  //5:R块其它错
    IC_T1_LASTSLENERR  = 6 ,  //6:S块长度错
    IC_T1_LASTSINFERR  = 7 ,  //7:S块INF错
    IC_T1_LASTSNULLERR = 8 ,  //8:S块非法应答
    IC_T1_LASTSNREQERR = 9 ,  //9:S块Unknown请求
    IC_T1_LASTTXERR    = 10,  //10:上一块传输错，重传上一块
    IC_T1_LASTABORTERR = 11,  //11:放弃请求错
}ICC_T1_LASTSTATUS_t;


typedef enum ICC_PROC_STATUS
{
    ICC_PROC_OK      = 0x00,    //写没有错误
    //ICC_PROC_READOK       = 0x01,    //读没有错误
    ICC_PROC_CWTOVER      = 0x02,    //两相邻字节超时
    ICC_PROC_BWTOVER      = 0x03,    //读一帧超时
    ICC_PROC_T0READERR    = 0x05,    //同一个字节连读5次错误        
    ICC_PROC_T1PARITYERR  = 0x06,    //位校验错误,且协议不为T0
    ICC_PROC_T1CRCERR     = 0x07,    //一帧校验错误
    ICC_PROC_T0WEITEERR   = 0x08,    //同一个字节连写3次错误
    ICC_PROC_SWERR        = 0x09,    //IC卡返回过程或状态字错误
    ICC_PROC_TSERR        = 0x0A,    //复位TS错误
    ICC_PROC_BLOCKERR     = 0x0B,    //T1多次块重传错误
    ICC_PROC_RESETERR     = 0x0C,    //复位错误（不满足EMV规范）
    ICC_PROC_EXCHGINT     = 0x0D,    //卡交换数据被中断  //20060828
        
    ICC_PROC_PARAERR      = 0x20,    //IC卡参数错误
    ICC_PROC_INFOERR      = 0x21,    //没有所操作IC卡的信息
    ICC_PROC_NORESET      = 0x22,    //所操作的IC的卡没有复位
    ICC_PROC_NORESPOND    = 0x23,    //51没有响应
    ICC_PROC_CANNCEL      = 0x24,    //接收到上位机的canncel处理
    ICC_PROC_ABNORAL      = 0x25,    //IC卡处理异常
    ICC_PROC_NOCARD       = 0x26,    //没有IC卡
    
}ICC_PROC_STATUS_t;

//###########IC_LastERRType###############
#define ICC_CWT_DEFAULT     10180    //两相邻B超时常数
#define ICC_BWT_DEFAULT     20160    //一帧超时常数    

//=========================================
s32 drv_psam_init(void);
s32 drv_psam_open(s32 psamno);
s32 drv_psam_close(s32 psamno);
s32 drv_psam_poweroff(s32 psamno);
s32 drv_psam_reset(s32 psamno, u8 *lpAtr);
s32 drv_psam_exchange_apdu(s32 psamno, u8 *wbuf, u32 wlen, u8 *rbuf, u32 *rlen, u32 rxmax);



#endif
