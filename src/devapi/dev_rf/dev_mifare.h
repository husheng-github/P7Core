#ifndef __DEV_MIFARE_H
#define __DEV_MIFARE_H


#define MIFARE_AUTHENT_A            (0x60)   /* AUTHENT A command. */
#define MIFARE_AUTHENT_B            (0x61)   /* AUTHENT B command. */
#define MIFARE_READ                 (0x30)   /* READ command. */
#define MIFARE_WRITE                (0xA0)   /* WRITE 16 bytes command. */
#define MIFARE_WRITE4               (0xA2)   /* WRITE 4 bytes command. */
#define MIFARE_INCREMENT            (0xC1)   /* INCREMENT command. */
#define MIFARE_DECREMENT            (0xC0)   /* DECREMENT command.  */
#define MIFARE_RESTORE              (0xC2)   /* RESTORE command. */
#define MIFARE_TRANSFER             (0xB0)   /* TRANSFER command. */
#define MIFARE_NOCOMMAND            (0x00)   /* VOID command (no MIFARE command). */


#define MF_KEY_A        0x60
#define MF_KEY_B        0x61

#define MF_EC_NO_ERROR                  0x00
#define MF_EC_EXECUTE_FAILED            0x01    
#define MF_EC_LENGTH_INCORRECT          0x02      
#define MF_EC_BLOCK_NUM_INCORRECT       0x03
#define MF_EC_DATA_FORMAT_INCORRECT     0x04    
#define MF_EC_AUTH_FAILED               0x05
#define MF_EC_KEY_TYPE_UNKNOW           0x06
#define MF_EC_PARAM_INVALID             0x07


s32 dev_mifare_auth(u8 key_type,u8 *key, u8 *uid, u8 block_num);
s32 dev_mifare_read_binary(u8 block_num,u8 *rdata);
s32 dev_mifare_write_binary(u8 block_num,u8 *wdata);
s32 dev_mifare_read_value(u8 block_num,u32 *rdata);
s32 dev_mifare_write_value(u8 block_num,u32 wdata);
s32 dev_mifare_inc_value(u8 block_num, u32 value);
s32 dev_mifare_dec_value(u8 block_num, u32 value);
s32 dev_mifare_backup_value(u8 src_block_num, u8 dst_block_num);

#endif

