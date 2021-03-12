#include "hid_keyboard.h"
#include "hid_keycode.h"

static USB_OTG_CORE_HANDLE* USBDevHandle;

void Keyboard_Configuration(USB_OTG_CORE_HANDLE* pdev)
{
    USBDevHandle = pdev;
}

void Keyboard_SendKeys(char* keys)
{
    char*   key        = keys;
    uint8_t KeyBuff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    char    lastKey    = 0;
    do
    {
        for (int i = 0; i < sizeof(KeyCodeArray) / sizeof(KeyCodeArray[0]); i++)
        {
            if (*key == KeyCodeArray[i][0])
            {
                if (KeyCodeArray[i][2] == lastKey)
                {
                    memset(KeyBuff, 0x00, sizeof(KeyBuff));
                    USBD_HID_SendReport(USBDevHandle, KeyBuff, 8);
                }
                KeyBuff[0] = KeyCodeArray[i][1];
                KeyBuff[2] = KeyCodeArray[i][2];
                lastKey    = KeyBuff[2];
                break;
            }
        }
        USBD_HID_SendReport(USBDevHandle, KeyBuff, 8);
        USB_OTG_BSP_mDelay(2);
    } while (*(++key));

    memset(KeyBuff, 0x00, sizeof(KeyBuff));
    USBD_HID_SendReport(USBDevHandle, KeyBuff, 8);
}

void Keyboard_SendKey(char key)
{
    uint32_t i = 0;

    uint8_t KeyBuff[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    for (i = 0; i < sizeof(KeyCodeArray) / sizeof(KeyCodeArray[0]); i++)
    {
        if (key == KeyCodeArray[i][0])
        {
            KeyBuff[0] = KeyCodeArray[i][1];
            KeyBuff[2] = KeyCodeArray[i][2];
            break;
        }
    }

    USBD_HID_SendReport(USBDevHandle, KeyBuff, 8);
    USB_OTG_BSP_mDelay(2);

    memset(KeyBuff, 0x00, sizeof(KeyBuff));
    USBD_HID_SendReport(USBDevHandle, KeyBuff, 8);
}
