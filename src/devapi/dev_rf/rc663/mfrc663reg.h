#ifndef __MFRC663REG_H
#define __MFRC663REG_H

/*! \name Register definitions 
 *  \ingroup reg
 *  Following all register defintions of the RC631.
 */
/*@{*/
#define J663REG_Command             0x00        /*   Starts and stops command execution.                         */
#define J663REG_HostCtrl            0x01        /*!< Host control register.                                    */
#define J663REG_FIFOControl         0x02        /*!<  Control register of the FIFO.                            */
#define J663REG_WaterLevel          0x03        /*!< Level of the FIFO underflow and overflow warning.       */
#define J663REG_FIFOLength          0x04        /*!< Length of the FIFO.                          */
#define J663REG_FIFOData            0x05        /*!<  Data In/Out exchange register of FIFO buffer.          */
                                         
#define J663REG_IRQ0                0x06        /*!< Interrupt register 0.                       */
#define J663REG_IRQ1                0x07        /*!< Interrupt register 1.                       */
#define J663REG_IRQ0En              0x08        /*!< IRQ0En Interrupt enable register 0.                */
#define J663REG_IRQ1En              0x09        /*!< GIRQ1En Interrupt enable register 1.              */
#define J663REG_Error               0x0A        /*!< Error Error bits showing the error status of the last command execution.                  */
#define J663REG_Status              0x0B        /*!<  Status Contains status of the communication         */
#define J663REG_RxBitCtrl           0x0C        /*!< Control register for anticollision adjustments for bit oriented protocols.                          */
#define J663REG_RxColl              0x0D        /*!< Collision position register*/
        
#define J663REG_TControl            0x0E        /*!< Control of Timer 0..3   */
#define J663REG_T0Control           0x0F        /*  Control of Timer0.                                     */
#define J663REG_T0ReloadHi          0x10        /*  High register of the reload value of Timer0.                */
#define J663REG_T0ReloadLo          0x11        /*!< Low register of the reload value of Timer0    */
#define J663REG_T0CounterValHi      0x12        /*!< Counter value high register of Timer0.                             */
#define J663REG_T0CounterValLo      0x13        /*!< Counter value low register of Timer0.                  */
                                         
#define J663REG_T1Control           0x14        /*!< Control of Timer1.                        */
#define J663REG_T1ReloadHi          0x15        /* High register of the reload value of Timer1*/
#define J663REG_T1ReloadLo          0x16        /*!< Low register of the reload value of Timer1.      */
#define J663REG_T1CounterValHi      0x17        /*!< Counter value high register of Timer1           */
#define J663REG_T1CounterValLo      0x18        /*!< Counter value low register of Timer1.          */
                                         
#define J663REG_T2Control           0x19        /*!< Control of Timer2.                                */
#define J663REG_T2ReloadHi          0x1A        /* High byte of the reload value of Timer2*/
#define J663REG_T2ReloadLo          0x1B        /* Low byte of the reload value of Timer2*/
#define J663REG_T2CounterValHi      0x1C        /* Counter value high byte of Timer2*/
#define J663REG_T2CounterValLo      0x1D        /* Counter value low byte of Timer2*/
                                         
#define J663REG_T3Control          0x1E        /*    Control of Timer3.           */
#define J663REG_T3ReloadHi         0x1F        /*!<High byte of the reload value of Timer3    */
#define J663REG_T3ReloadLo         0x20        /*  Low byte of the reload value of Timer3                      */
#define J663REG_T3CounterValHi     0x21        /*!<Counter value high byte of Timer3                        */
#define J663REG_T3CounterValLo     0x22        /*!<Counter value low byte of Timer3                        */
                            
#define J663REG_T4Control           0x23        /*   Control of Timer4                                     */
#define J663REG_T4ReloadHi          0x24        /*!< High byte of the reload value of Timer4 .                      */
#define J663REG_T4ReloadLo          0x25        /*   Low byte of the reload value of Timer4.                           */
#define J663REG_T4CounterValHi      0x26        /*!< Counter value high byte of Timer4               */
#define J663REG_T4CounterValLo      0x27        /*!<  Counter value low byte of Timer4 */
                                          
#define J663REG_DrvMod              0x28        /*!< Driver mode register.      */
#define J663REG_TxAmp               0x29        /*!< Transmitter amplifier register.  */
#define J663REG_DrvCon              0x2A        /*!< Driver configuration register                */
#define J663REG_Txl                 0x2B        /*!< Transmitter register.               */
#define J663REG_TxCrcPreset         0x2C        /*!< Transmitter CRC control register, preset value.       */
#define J663REG_RxCrcPreset          0x2D        /* Receiver CRC control register, preset value*/
#define J663REG_TxDataNum              0x2E        /* Transmitter data number register*/
#define J663REG_TxModWidth            0x2F        /*Transmitter modulation width register */
                                   
#define J663REG_TxSym10BurstLen       0x30        /*Transmitter symbol 1 + symbol 0 burst length register */
#define J663REG_TXWaitCtrl              0x31        /*Transmitter wait control */
#define J663REG_TxWaitLo            0x32        /* Transmitter wait low*/
#define J663REG_FrameCon              0x33        /*Transmitter frame control */
#define J663REG_RxSofD               0x34        /* Receiver start of frame detection*/
#define J663REG_RxCtrl               0x35        /* Receiver control register*/
#define J663REG_RxWait               0x36        /* Receiver wait register*/
#define J663REG_RxThreshold           0x37        /*Receiver threshold register*/
#define J663REG_Rcv                   0x38        /*Receiver register */
#define J663REG_RxAna               0x39        /*Receiver analog register */
#define J663REG_RFU3A               0x3A        /* RFU*/
#define J663REG_SerialSpeed          0x3B        /*Serial speed register */
#define J663REG_LFO_Trimm           0x3C        /* Low-power oscillator trimming register*/
#define J663REG_PLL_Ctrl            0x3D        /* IntegerN PLL control register, for microcontroller clock output adjustment*/
#define J663REG_PLL_DivOut          0x3E        /*IntegerN PLL control register, for microcontroller clock output adjustment */
#define J663REG_LPCD_QMin           0x3F        /*Low-power card detection Q channel minimum threshold */
                                         
#define J663REG_LPCD_QMax           0x40        /*Low-power card detection Q channel maximum threshold */
#define J663REG_LPCD_IMin           0x41        /* Low-power card detection I channel minimum threshold*/
#define J663REG_LPCD_I_Result       0x42        /*Low-power card detection I channel result register */
#define J663REG_LPCD_Q_Result       0x43        /*Low-power card detection Q channel result register */
#define J663REG_PadEn               0x44        /* PIN enable register*/
#define J663REG_PadOut              0x45        /* PIN out register*/
#define J663REG_PadIn               0x46        /* PIN in register*/
#define J663REG_SigOut              0x47        /*Enables and controls the SIGOUT Pin*/
/**--->以下是RC663独有**/                       
#define J663REG_TxBitMod            0x48        /*Transmitter bit modus register*/
#define J663REG_RFU                 0x49        /* */
#define J663REG_TxDataCon           0x4A        /*Transmitter data configuration register */
#define J663REG_TxDataMod           0x4B        /*Transmitter data modulation register */
#define J663REG_TxSymFreq           0x4C        /*Transmitter symbol frequency */
#define J663REG_TxSym0H             0x4D        /*Transmitter symbol 0 high register */
#define J663REG_TxSym0L             0x4E        /*Transmitter symbol 0 low register */
#define J663REG_TxSym1H             0x4F        /*Transmitter symbol 1 high register */
#define J663REG_TxSym1L             0x50        /*Transmitter symbol 1 low register */
#define J663REG_TxSym2              0x51        /*Transmitter symbol 2 register */
#define J663REG_TxSym3              0x52        /*Transmitter symbol 3 register */
#define J663REG_TxSym10Len          0x53        /*Transmitter symbol 1 + symbol 0 length register */
#define J663REG_TxSym32Len          0x54        /*Transmitter symbol 3 + symbol 2 length register */
#define J663REG_TxSym10BurstCtrl    0x55        /*Transmitter symbol 1 + symbol 0 burst control register */
#define J663REG_TxSym10Mod          0x56        /*Transmitter symbol 1 + symbol 0 modulation register */
#define J663REG_TxSym32Mod          0x57        /*Transmitter symbol 3 + symbol 2 modulation register */
#define J663REG_RxBitMod            0x58        /*Receiver bit modulation register */
#define J663REG_RxEofSym            0x59        /*Receiver end of frame symbol register */
#define J663REG_RxSyncValH          0x5A        /*Receiver synchronisation value high register */
#define J663REG_RxSyncValL          0x5B        /*Receiver synchronisation value low register */
#define J663REG_RxSyncMod           0x5C        /*Receiver synchronisation mode register */
#define J663REG_RxMod               0x5D        /*Receiver modulation register */
#define J663REG_RxCorr              0x5E        /*Receiver correlation register */
#define J663REG_FabCal              0x5F        /*Fab calibration register of the receiver */
/**<----**/
#define J663REG_Version             0x7F        /* Version and subversion register*/


/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Possible commands
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name RC631 Command definitions
 *  \ingroup reg
 *  Following all commands of the RC631.
 */
/*@{*/
#define J663CMD_MASK            0x0F  
#define J663CMD_IDLE            0x00  /*!< No action: cancel current command
                                        or home state. \n */
#define J663CMD_LPCD            0x01  /*low-power card detection*/
#define J663CMD_LoadKey         0x02  /*reads a MIFARE key (size of 6 bytes) from FIFO buffer 
                                        ant puts it into Key buffer*/
#define J663CMD_MFAuthent       0x03  /*!< performs the MIFARE standard authentication in 
                                         MIFARE read/write mode only */
        
#define J663CMD_Receive         0x05  /* activates the receive circuit */
#define J663CMD_Transmit        0x06  /*  transmits data from the FIFO buffer*/
#define J663CMD_Transceive      0x07  /*  transmits data from the FIFO buffer and automatically 
                                        activates the receiver after transmission finished*/
#define J663CMD_WriteE2         0x08  /* gets one byte from FIFO buffer and writes it to the 
                                        internal EEPROM, valid address range are the 
                                        addresses of the MIFARE Key area */
#define J663CMD_WriteE2Page     0x09  /* gets up to 64 bytes (one EEPROM page) from the FIFO 
                                        buffer and writes it to the EEPROM, valid page address 
                                        range are the pages of the MIFARE Key Area */
#define J663CMD_ReadE2             0x0A  /* reads data from the EEPROM and copies it into the 
                                         FIFO buffer, valid address range are the addresses of 
                                         the MIFARE Key area */
#define J663CMD_LoadReg         0x0C//0x0B  
                                      /* reads data from the internalEEPROM and initializes the 
                                         MFRC631 registers. EEPROM address needs to be 
                                         within EEPROM sector 2 */
#define J663CMD_LoadProtocol    0x0D//0x0C  
                                      /* reads data from the internalEEPROM and initializes the 
                                        MFRC631 registers needed for a Protocol change */
#define J663CMD_LoadKeyE2       0x0E  /* copies a key of the EEPROM into the key buffer */
#define J663CMD_StoreKeyE2      0x0F  /* stores a MIFARE key (size of6 bytes) into the EEPROM */
#define J663CMD_ReadRNR         0x1C  /*Copies bytes from the Random Number generator into 
                                        the FIFO until the FiFo is full  */
#define J663CMD_SoftReset       0x1F  /* resets the MFRC631 */

//#define J663CMD_Standby         0x80  /*Set to 1, the IC is entering power-down mode */

/*@}*/
/* /////////////////////////////////////////////////////////////////////////////
 * Possible commands parameter（bytes）
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name RC631 Command definitions
 *  \ingroup reg
 *  Following all commands of the RC631.
 */
/*@}*/

#define J663CMD_LoadProtocol_Rec_paraA1     0x00  /*ISO/IEC14443 A  106kbits/s  Manchester SubC*/
#define J663CMD_LoadProtocol_Tra_paraA1     0x00  /*ISO/IEC14443 A  106kbits/s  Miller*/
                                            
#define J663CMD_LoadProtocol_Rec_paraB1     0x04  /*ISO/IEC14443 B  106kbits/s  BPSK*/
#define J663CMD_LoadProtocol_Tra_paraB1     0x04  /*ISO/IEC14443 B  106kbits/s  NRZ*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bit Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name RC631 Bit definitions 
 *  \ingroup reg
 *  Below there are useful bit definition of the RC631 register set of Page 0.
 */
/*@{*/
/* Command Register                            (01) */
#define J663BIT_Standby             0x80   /*!<  the IC is entering power-down mode. */
#define J663BIT_ModemOff            0x40   /*!<  the receiver and the transmitter circuit is powering down. */


/* CommIEn Register                            (02) */
#define J663BIT_FIFOFlash           0x10

/* DivIEn Register                            (03) */
/* CommIEn Register        */
/* CommIrq0 Register                (08H)*/
#define J663BIT_IRQINV              0x80   /*!< Inverts the output of IRQ Pin. */
#define J663BIT_HiAlerIrq0          0x40   /*!< Set, when bit HiAlert in register Status1Reg isset.. */
#define J663BIT_LoAlertIrq0         0x20   /*!< when bit LoAlert in registerStatus1 is set. . */
#define J663BIT_IdleIrq0            0x10   /*!<  when a command terminates byitsel. */
#define J663BIT_TxIrq0              0x08   /*!<when data transmission is completed,. */
#define J663BIT_RxIrq0              0x04   /*!<when the receiver detects the end of a data stream. */
#define J663BIT_ErrIrq0             0x02   /*!Set, when the one of the following errors is set:
FifoWrErr, FiFoOvl, ProtErr, NoDataErr, IntegErr. 
Can only be reset if Set is cleared.<*/
#define J663BIT_RxSOFIrq0           0x01   /*!<Set, when a SOF or a subcarrier is detected. Can only be reset if Set is 
cleared. */

/* CommIrq1 Register                (09H)*/
#define J663BIT_IRQPUSHPULL         0x80   /*!< Sets the IRQ pin to Push Pull mode. */
#define J663BIT_GlobalIrq1          0x40   /*!< Bit position for Transmit Interrupt Enable/Request. */
#define J663BIT_LPCD_Irq1           0x20   /*!< Bit position for Receive Interrupt Enable/Request. */
#define J663BIT_Timer4Irq1          0x10   /*!< Set to logic 1 when Timer4 has an underflow. */
#define J663BIT_Timer3Irq1          0x08   /*!<Set to logic 1 when Timer3 has an underflow. */
#define J663BIT_Timer2Irq1          0x04   /*!< Set to logic 1 when Timer2 has an underflow. */
#define J663BIT_Timer1Irq1          0x02   /*!Set to logic 1 when Timer1 has an underflow<*/
#define J663BIT_Timer0Irq1          0x01   /*!<Set to logic 1 when Timer0 has an underflow. */

/* DivIEn and DivIrq Register           (03, 05) */
#define J663BIT_SIGINACTI           0x10   /*!< Bit position for SiginAct Interrupt Enable/Request. */
#define J663BIT_CRCI                0x04   /*!< Bit position for CRC Interrupt Enable/Request. */

/* CommIrq and DivIrq Register          (04, 05) */
#define J663BIT_SET                 0x80   /*!< Bit position to set/clear dedicated IRQ bits. */

/* Error Register                             (06) */

#define J663BIT_EE_Err              0x80    /*An error appeared during the last EEPROM command.*/
#define J663BIT_FIFOWrErr           0x40    /*Data was written into the FIFO, during a transmission of a possible CRC,
                                              during "RxWait", "Wait for data" or "Receiving" state, or during an
                                              authentication command. The Flag is cleared when a new CL command is
                                              started.*/
#define J663BIT_FIFOOvl             0x20    /*Data is written into the FIFO when it is already full. The data that is already in
                                              the FIFO will remain untouched. All data that is written to the FIFO after this
                                              Flag is set to 1 will be ignored.*/
#define J663BIT_MinFrameErr         0x10    /*A valid SOF was received, but afterwards less then 4 bits of data were received. */
#define J663BIT_NoDataErr           0x08    /*Data should be sent, but no data is in FIFO*/
#define J663BIT_CollDet             0x04    /*A collision has occurred. The position of the first collision is shown in the register RxColl. */
#define J663BIT_ProtErr             0x02    /*A protocol error has occurred. A protocol error can be a wrong stop bit, a
                                              missing or wrong ISO/IEC14443B EOF or SOF or a wrong number of
                                              received data bytes. When a protocol error is detected, data reception is stopped.*/
#define J663BIT_IntegErr            0x01    /*A data integrity error has been detected. Possible cause can be a wrong
                                              parity or a wrong CRC. In case of a data integrity error the reception is continued.*/            
            


/* Status 1 Register                         (07) */
#define J663BIT_CRCOK               0x40   /*!< Bit position for status CRC OK. */
#define J663BIT_CRCREADY            0x20   /*!< Bit position for status CRC Ready. */
#define J663BIT_IRQ                 0x10   /*!< Bit position for status IRQ is active. */
#define J663BIT_TRUNNUNG            0x08   /*!< Bit position for status Timer is running. */
#define J663BIT_HIALERT             0x02   /*!< Bit position for status HiAlert. */
#define J663BIT_LOALERT             0x01   /*!< Bit position for status LoAlert. */

/* Status 2 Register                            (08) */
#define J663BIT_TEMPSENSOFF         0x80   /*!< Bit position to switch Temperture sensors on/off. */
#define J663BIT_I2CFORCEHS          0x40   /*!< Bit position to forece High speed mode for I2C Interface. */
#define J663BIT_TARGETACTIVATED     0x10
#define J663BIT_CRYPTO1ON           0x08   /*!< Bit position for reader status Crypto is on. */

/* Rcv Register                            (38) */
#define J663BIT_Rx_ADCmode          0x40   /*!< 1： LPCD mode for ADC
                            0: normal reception mode for ADC*/

/* Control Register                                (0C) */
#define J663BIT_TSTOPNOW            0x80   /*!< Stops timer if set to 1. */
#define J663BIT_TSTARTNOW           0x40   /*!< Starts timer if set to 1. */
#define J663BIT_WRNFCIDTOFIFO       0x20
#define J663BIT_INITIATOR           0X10

/* BitFraming Register                        (0D) */
#define J663BIT_STARTSEND           0x80   /*!< Starts transmission in transceive command if set to 1. */

/* BitFraming Register                        (0E) */
#define J663BIT_VALUESAFTERCOLL     0x80   /*!< Activates mode to keep data after collision. */
/*@}*/

/*! \name RC522 Bit definitions of Page 1
 *  \ingroup reg
 *  Below there are useful bit definition of the RC522 register set of Page 1.
 */
/*@{*/
/* Mode Register                            (11) */
#define J663BIT_TXWAITRF            0x20   /*!< Tx waits until Rf is enabled until transmit is startet, else
                                            transmit is started immideately. */
#define J663BIT_POLSIGIN            0x08   /*!< Inverts polarity of SiginActIrq, if bit is set to 1 IRQ occures
                                                when Sigin line is 0. */

/* TxMode Register                            (12) */
#define J663BIT_INVMOD              0x08   /*!< Activates inverted transmission mode. */

/* RxMode Register                            (13) */
#define J663BIT_RXNOERR             0x08   /*!< If 1, receiver does not receive less than 4 bits. */

/* Definitions for Tx and Rx            (12, 13) */
#define J663BIT_106KBPS             0x00   /*!< Activates speed of 106kbps. */
#define J663BIT_212KBPS             0x10   /*!< Activates speed of 212kbps. */
#define J663BIT_424KBPS             0x20   /*!< Activates speed of 424kbps. */
        
#define J663BIT_CRCEN               0x01//0x80   /*!< the received CRC byte(s) are copied to the FIFO. */
#define J663BIT_RxForcecWrite       0x80  /*Set to 1, the received CRC byte(s) are written to FiFo. */     
#define J663BIT_ISO14443B           0x03   /*!< Define the ISO14443B framing used for data transmission. */

/* TxControl Register                        (14) */
#define J663BIT_INVTX2ON            0x80   /*!< Inverts the Tx2 output if drivers are switched on. */
#define J663BIT_INVTX1ON            0x40   /*!< Inverts the Tx1 output if drivers are switched on. */
#define J663BIT_INVTX2OFF           0x20   /*!< Inverts the Tx2 output if drivers are switched off. */
#define J663BIT_INVTX1OFF           0x10   /*!< Inverts the Tx1 output if drivers are switched off. */
#define J663BIT_TX2CW               0x08   /*!< Does not modulate the Tx2 output, only constant wave. */
#define J663BIT_TX2RFEN             0x02   /*!< Switches the driver for Tx2 pin on. */
#define J663BIT_TX1RFEN             0x01   /*!< Switches the driver for Tx1 pin on. */
        
/* Demod Register                             (19) */
#define J663BIT_FIXIQ               0x20   /*!< If set to 1 and the lower bit of AddIQ is set to 0, the receiving is fixed to I channel.
                                                If set to 1 and the lower bit of AddIQ is set to 1, the receiving is fixed to Q channel. */
/*@}*/

/*! \name RC522 Bit definitions of Page 2
 *  \ingroup reg
 *  Below there are useful bit definition of the RC522 register set.
 */
/*@{*/
/* DrvMode Register*/
#define J663BIT_TXEN                0x08
/* TMode Register                             (2A) */
#define J663BIT_TAUTO               0x80   /*!< Sets the Timer start/stop conditions to Auto mode. */
#define J663BIT_TAUTORESTART        0x10   /*!< Restarts the timer automatically after finished
                                                counting down to 0. */

#define J663BIT_EMDSup              0x80  
/*@}*/


/* /////////////////////////////////////////////////////////////////////////////
 * Bitmask Definitions
 * ////////////////////////////////////////////////////////////////////////// */
/*! \name RC522 Bitmask definitions
 *  \ingroup reg
 *  Below there are some useful mask defintions for the RC522. All specified
 *  bits are set to 1.
 */
/*@{*/

/* Command register                 (0x01)*/
#define J663MASK_COMMAND            0x0F   /*!< Bitmask for Command bits in Register JREG_COMMAND. */

/* Waterlevel register              (0x0B)*/
#define J663MASK_WATERLEVEL         0x3F   /*!< Bitmask for Waterlevel bits in register JREG_WATERLEVEL. */

/* Control register                 (0x0C)*/
#define J663MASK_RXBITS             0x07   /*!< Bitmask for RxLast bits in register JREG_CONTROL. */

/* Mode register                    (0x11)*/
#define J663MASK_CRCPRESET          0x03   /*!< Bitmask for CRCPreset bits in register JREG_MODE. */

/* TxMode register                  (0x12, 0x13)*/
#define J663MASK_SPEED              0x70   /*!< Bitmask for Tx/RxSpeed bits in register JREG_TXMODE and JREG_RXMODE. */

/* TxSel register                   (0x16)*/
#define J663MASK_DRIVERSEL          0x30   /*!< Bitmask for DriverSel bits in register JREG_TXSEL. */
#define J663MASK_SIGOUTSEL          0x0F   /*!< Bitmask for SigoutSel bits in register JREG_TXSEL. */

/* RxSel register                   (0x17)*/
#define J663MASK_UARTSEL            0xC0   /*!< Bitmask for UartSel bits in register JREG_RXSEL. */
#define J663MASK_RXWAIT             0x3F   /*!< Bitmask for RxWait bits in register JREG_RXSEL. */

/* RxThreshold register             (0x18)*/
#define J663MASK_MINLEVEL           0xF0   /*!< Bitmask for MinLevel bits in register JREG_RXTHRESHOLD. */
#define J663MASK_COLLEVEL           0x07   /*!< Bitmask for CollLevel bits in register JREG_RXTHRESHOLD. */

/* Demod register                   (0x19)*/
#define J663MASK_ADDIQ              0xC0   /*!< Bitmask for ADDIQ bits in register JREG_DEMOD. */
#define J663MASK_TAURCV             0x0C   /*!< Bitmask for TauRcv bits in register JREG_DEMOD. */
#define J663MASK_TAUSYNC            0x03   /*!< Bitmask for TauSync bits in register JREG_DEMOD. */

/* RFCfg register                   (0x26)*/
#define J663MASK_RXGAIN             0x70   /*!< Bitmask for RxGain bits in register JREG_RFCFG. */

/* GsN register                     (0x27)*/
#define J663MASK_CWGSN              0xF0   /*!< Bitmask for CWGsN bits in register JREG_GSN. */
#define J663MASK_MODGSN             0x0F   /*!< Bitmask for ModGsN bits in register JREG_GSN. */

/* CWGsP register                   (0x28)*/
#define J663MASK_CWGSP              0x3F   /*!< Bitmask for CWGsP bits in register JREG_CWGSP. */

/* ModGsP register                  (0x29)*/
#define J663MASK_MODGSP             0x3F   /*!< Bitmask for ModGsP bits in register JREG_MODGSP. */

/* TMode register                   (0x2A)*/
#define J663MASK_TGATED             0x60   /*!< Bitmask for TGated bits in register JREG_TMODE. */
#define J663MASK_TPRESCALER_HI      0x0F   /*!< Bitmask for TPrescalerHi bits in register JREG_TMODE. */
 
#endif

