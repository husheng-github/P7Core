#ifndef __TTSBIN_HANDLE_H
#define __TTSBIN_HANDLE_H

#define READ_ONCE_BYTES     4096

typedef struct{
    u8 *m_text;//�ַ���
    u8 *m_audio_data;//��Ŷ�Ӧ�ַ�������Ƶ����
    u32 m_audio_data_len;//��Ƶ���ݳ���
    u32 m_file_start_addr;//��Ƶ������tts.source����ʼλ��
}str_audio_file_node;

typedef struct{
    s32 m_num;                                    //�ļ�����
    s32 m_crc;
    str_audio_file_node *m_audio_file_node;      //��Ƶ�ļ����

    s32 (*init)(void);
    void (*exit)(void);
    void (*info_print)(void);
    s8*  (*get_audio_filetext)(s32 index);
    s32  (*get_audio_filelen)(s32 index);
    s32  (*get_audio_filedata)(s32 index, u8 *data_buff, s32 data_buff_len, s32 file_offset);
    s32 (*get_audiostring_index)(s8 *audio_string, u16 *index_tab, s32 *index_tab_num);
    s32 (*get_crc)(void);
}tts_bin_instance_t;

tts_bin_instance_t *ttsbin_instance(void);

#endif