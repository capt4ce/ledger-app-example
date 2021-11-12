/*******************************************************************************
*   Ledger Blue
*   (c) 2016 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "os.h"
#include "cx.h"
#include "ux.h"
#include <string.h>
#include <inttypes.h>
#include <os_io_seproxyhal.h>
#include "int64.h"
#include "crypto.h"
#include "tx.h"
#include "zoobc.h"

#define TEXT_TOP_SIZE 20
#define TEXT_BOTTOM_SIZE 50
#define MENU_STATE_SIZE 2
#define UNSIGNED_TX_SIZE 30

// UX Flow state definitions
#define MENU_ENTRY 0
#define MENU_SEND_MONEY 1
#define MENU_REGISTER_NODE 2

#define MENU_ENTRY_STEP_STATUS 0
#define MENU_ENTRY_STEP_VERSION 1
#define MENU_ENTRY_STEP_QUIT 2

#define MENU_SEND_MONEY_ENTRY 0
#define MENU_SEND_MONEY_SENDER 1
#define MENU_SEND_MONEY_RECIPIENT 2
#define MENU_SEND_MONEY_AMOUNT 3
#define MENU_SEND_MONEY_FEE 4
#define MENU_SEND_MONEY_ACCEPT 5
#define MENU_SEND_MONEY_REJECT 6

#define MENU_REGISTER_NODE_ENTRY 0
#define MENU_REGISTER_NODE_PUBLIC_KEY_NODE 1
#define MENU_REGISTER_NODE_ACCOUNT_ADDRESS_OWNER 2
#define MENU_REGISTER_NODE_LOCKED_BALANCE 3
#define MENU_REGISTER_NODE_FEE 4
#define MENU_REGISTER_NODE_ACCEPT 5
#define MENU_REGISTER_NODE_REJECT 6

// APDU
#define CLA 0x80
#define INS_RESET 0x00
#define INS_DEBUG 0x01
#define INS_GET_PUBLIC_KEY 0x02
#define INS_SIGN_TRANSACTION 0x03
#define INS_SIGN_DATA 0x04
#define INS_HASH_DATA 0x05

// data offset
#define APDU_OFFSET_CLA 0
#define APDU_OFFSET_INS 1
#define APDU_OFFSET_P1 2
#define APDU_OFFSET_P2 3
#define APDU_OFFSET_DATA_LENGTH 4
#define APDU_OFFSET_DATA_START 8
#define APDU_OFFSET_TX_DATA_START 12

// APDU parameters
#define P1_FIRST 0x00
#define P1_MORE 0x80
#define P2_LAST 0x00
#define P2_MORE 0x80

// Transaction Type
#define TRANSACTION_TYPE_SEND_MONEY 1
#define TRANSACTION_TYPE_REGISTER_NODE 2

// transaction data offset
#define TRANSACTION_DATA_OFFSET_SENDER_ADDRESS 13

// data type length
#define INT32_LENGTH 4 // bytes
#define INT64_LENGTH 8 // bytes

#define PUBLIC_KEY_LENGTH 32
#define HASH256_LENGTH 32

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e);

unsigned int ux_step;
uint8_t ux_loop_over_curr_element; // Nano S only

static char textTop[TEXT_TOP_SIZE];
static char textBottom[TEXT_BOTTOM_SIZE];

// UX Flow state
static int menuState[MENU_STATE_SIZE];
static int menuStateNext[MENU_STATE_SIZE];
static int currentMenuMaxStep = 0;

// transactions related variable
static char unsignedTx[UNSIGNED_TX_SIZE];
static char breakChar[1] = "\0";
static int derivationPathIndex;
static char txFee[TEXT_BOTTOM_SIZE];

// send money transaction related
static unsigned char txSenderAddress[TEXT_BOTTOM_SIZE];
static unsigned char txRecipientAddress[TEXT_BOTTOM_SIZE];
static char txAmount[TEXT_BOTTOM_SIZE];

// register node transaction related
static unsigned char nodePublicKey[TEXT_BOTTOM_SIZE];
static unsigned char ownerAccountAddress[TEXT_BOTTOM_SIZE];
static unsigned char lockedBalance[TEXT_BOTTOM_SIZE];

static void ui_screen_menu_entry(void);
static void ui_screen_send_money(void);
static void ui_render(void);
static void render_menu_text_top(char text[TEXT_TOP_SIZE]);
static void render_menu_text_bottom(char text[TEXT_BOTTOM_SIZE]);
static unsigned int
bagl_ui_menu_template_nanos_button(unsigned int button_mask,
                                   unsigned int button_mask_counter);
static const bagl_element_t *io_seproxyhal_touch_ui_rerender(const bagl_element_t *e);
void cancelAction(void);

static void process_get_public_key(void);
static void process_sign_transaction(void);
static void process_sign_data(void);
static void process_hash_data(void);
static void process_more_tx();
static void parse_transaction_data(uint8_t *txBytes);

ux_state_t ux;

// ********************************************************************************
// Ledger Nano S specific UI
// ********************************************************************************

static const bagl_element_t bagl_ui_menu_template_nanos[] = {
    // {
    //     {type, userid, x, y, width, height, stroke, radius, fill, fgcolor,
    //      bgcolor, font_id, icon_id},
    //     text,
    //     touch_area_brim,
    //     overfgcolor,
    //     overbgcolor,
    //     tap,
    //     out,
    //     over,
    // },
    {
        // background
        {BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000,
         0xFFFFFF, 0, 0},
        NULL,
#ifdef TARGET_BLUE
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
#endif /* TARGET_BLUE */
    },
    {
        // text line 1
        {BAGL_LABELINE, 0x01, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
         BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
        textTop,
#ifdef TARGET_BLUE
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
#endif /* TARGET_BLUE */
    },
    {
        // text line 2
        {BAGL_LABELINE, 0x83, 15, 26, 97, 12, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000,
         BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 26},
        textBottom,
#ifdef TARGET_BLUE
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
#endif /* TARGET_BLUE */
    },
    {
        // left arrow
        {BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
         BAGL_GLYPH_ICON_LEFT},
        NULL,
#ifdef TARGET_BLUE
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
#endif /* TARGET_BLUE */
    },
    {
        // right arrow
        {BAGL_ICON, 0x00, 117, 12, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
         BAGL_GLYPH_ICON_RIGHT},
        NULL,
#ifdef TARGET_BLUE
        0,
        0,
        0,
        NULL,
        NULL,
        NULL,
#endif /* TARGET_BLUE */
    },
};

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len)
{
    switch (channel & ~(IO_FLAGS))
    {
    case CHANNEL_KEYBOARD:
        break;

    // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
    case CHANNEL_SPI:
        if (tx_len)
        {
            io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

            if (channel & IO_RESET_AFTER_REPLIED)
            {
                reset();
            }
            return 0; // nothing received from the master so far (it's a tx
                      // transaction)
        }
        else
        {
            return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                          sizeof(G_io_apdu_buffer), 0);
        }

    default:
        THROW(INVALID_PARAMETER);
    }
    return 0;
}

static void process_get_public_key(void)
{
    unsigned char tempInt32Byte[INT32_LENGTH];
    int derivationPathIndex;
    os_memmove(tempInt32Byte, G_io_apdu_buffer + APDU_OFFSET_DATA_START, INT32_LENGTH);
    derivationPathIndex = *(int *)tempInt32Byte;

    cx_ecfp_public_key_t publicKey;
    deriveKeypair(derivationPathIndex, NULL, &publicKey);
    extractPubkeyBytes(G_io_apdu_buffer, &publicKey);
}

static void ui_screen_send_money(void)
{
    menuStateNext[0] = MENU_SEND_MONEY;
    menuStateNext[1] = MENU_SEND_MONEY_ENTRY;
    currentMenuMaxStep = 7;
    ui_render();
}

static void ui_screen_register_node(void)
{
    menuStateNext[0] = MENU_REGISTER_NODE;
    menuStateNext[1] = MENU_REGISTER_NODE_ENTRY;
    currentMenuMaxStep = 7;
    ui_render();
}

static void ui_screen_menu_entry(void)
{
    menuStateNext[0] = MENU_ENTRY;
    menuStateNext[1] = MENU_ENTRY_STEP_STATUS;
    currentMenuMaxStep = 3;
    ui_render();
}

void cancelAction(void)
{
    // G_io_apdu_buffer[0] = 0x90;
    // G_io_apdu_buffer[1] = 0x00;

    // io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 0);
    // ui_screen_menu_entry();
    io_seproxyhal_touch_exit(NULL);
}

static void render_menu_text_top(char text[TEXT_TOP_SIZE])
{
    for (int i = 0; i < TEXT_TOP_SIZE; i++)
    {
        textTop[i] = text[i];
    }
}

static void render_menu_text_bottom(char text[TEXT_BOTTOM_SIZE])
{
    for (int i = 0; i < TEXT_BOTTOM_SIZE; i++)
    {
        textBottom[i] = text[i];
    }
}

static const bagl_element_t *
io_seproxyhal_touch_ui_rerender(const bagl_element_t *e)
{
    ui_render();
    return 0;
}

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e)
{
    // Go back to the dashboard
    os_sched_exit(0);
    return NULL;
}

unsigned int ui_display_address_nanos_prepro(bagl_element_t *element)
{

    if (menuStateNext[1] == MENU_SEND_MONEY_SENDER && element->component.userid == 0x83)
    {
        element->component.x = element->component.x - 3;

        // element->text = "testsdfjhasdkfha";
        // unsigned int display = (ux_step == (0x7F & element->component.userid) - 1);
        UX_CALLBACK_SET_INTERVAL(
            MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));

        return 1;

        //         const bagl_element_t newComponent = {
        //             // text line 2
        //             {BAGL_LABELINE, 0x83, 15, 26, 97, 12, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000,
        //              BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 26},
        //             "dddd",
        // #ifdef TARGET_BLUE
        //             0,
        //             0,
        //             0,
        //             NULL,
        //             NULL,
        //             NULL,
        // #endif /* TARGET_BLUE */
        //         };
        // return &newComponent;
    }
    // switch (element->component.userid)
    // {
    // case 0x83:
    //     UX_CALLBACK_SET_INTERVAL(
    //         MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
    //     break;
    // }
    // if (element->component.userid > 0)
    // {
    //     switch (element->component.userid)
    //     {
    //     case 0x83:
    //         ux_loop_over_curr_element = 1;
    //         UX_CALLBACK_SET_INTERVAL(MAX(
    //             3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
    //         break;
    //     case 5:
    //         UX_CALLBACK_SET_INTERVAL(2000);
    //         ux_loop_over_curr_element = 0; // allow next timer to increment ux_step when triggered
    //         break;
    //     case 0x86:
    //         UX_CALLBACK_SET_INTERVAL(MAX(
    //             3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
    //         // ugly ux tricks, loops around last 2 screens
    //         ux_step -= 1;                  // loops back to previous element on next redraw
    //         ux_loop_over_curr_element = 1; // when the timer will trigger, ux_step won't be incremented, only redraw
    //         break;
    //     }
    // }
    return 1;
}

static void ui_render(void)
{
    switch (menuStateNext[0])
    {
    case MENU_ENTRY:
        switch (menuStateNext[1])
        {
        case MENU_ENTRY_STEP_STATUS:
            render_menu_text_top("Application");
            render_menu_text_bottom("is ready");
            break;
        case MENU_ENTRY_STEP_VERSION:
            render_menu_text_top("Version");
            render_menu_text_bottom(APPVERSION);
            break;
        case MENU_ENTRY_STEP_QUIT:
            render_menu_text_top("Quit");
            render_menu_text_bottom("Applicatiom");
            break;
        default:
            break;
        }
        break;

    case MENU_SEND_MONEY:
        switch (menuStateNext[1])
        {
        case MENU_SEND_MONEY_ENTRY:
            render_menu_text_top("Review");
            render_menu_text_bottom("Send Money");
            break;
        case MENU_SEND_MONEY_SENDER:
            render_menu_text_top("Sender");
            render_menu_text_bottom(txSenderAddress);
            break;
        case MENU_SEND_MONEY_RECIPIENT:
            render_menu_text_top("Recipient");
            render_menu_text_bottom(txRecipientAddress);
            break;
        case MENU_SEND_MONEY_AMOUNT:
            render_menu_text_top("Amount");
            render_menu_text_bottom(txAmount);
            break;
        case MENU_SEND_MONEY_FEE:
            render_menu_text_top("Fee");
            render_menu_text_bottom(txFee);
            break;
        case MENU_SEND_MONEY_ACCEPT:
            render_menu_text_top("Accept");
            render_menu_text_bottom("and send");
            break;
        case MENU_SEND_MONEY_REJECT:
            render_menu_text_top("Reject");
            render_menu_text_bottom("Send Money");
            break;

        default:
            break;
        }
        break;

    case MENU_REGISTER_NODE:
        switch (menuStateNext[1])
        {
        case MENU_REGISTER_NODE_ENTRY:
            render_menu_text_top("Review");
            render_menu_text_bottom("Register Node");
            break;
        case MENU_REGISTER_NODE_PUBLIC_KEY_NODE:
            render_menu_text_top("Node Public Key");
            render_menu_text_bottom(nodePublicKey);
            break;
        case MENU_REGISTER_NODE_ACCOUNT_ADDRESS_OWNER:
            render_menu_text_top("Owner Acc Address");
            render_menu_text_bottom(ownerAccountAddress);
            break;
        case MENU_REGISTER_NODE_LOCKED_BALANCE:
            render_menu_text_top("Locked Balance");
            render_menu_text_bottom(lockedBalance);
            break;
        case MENU_REGISTER_NODE_FEE:
            render_menu_text_top("Fee");
            render_menu_text_bottom(txFee);
            break;
        case MENU_REGISTER_NODE_ACCEPT:
            render_menu_text_top("Accept");
            render_menu_text_bottom("and send");
            break;
        case MENU_REGISTER_NODE_REJECT:
            render_menu_text_top("Reject");
            render_menu_text_bottom("Register Node");
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
    // UX_DISPLAY(bagl_ui_menu_template_nanos, ui_display_address_nanos_prepro);
    UX_DISPLAY(bagl_ui_menu_template_nanos, NULL);
}

static unsigned int
bagl_ui_menu_template_nanos_button(unsigned int button_mask,
                                   unsigned int button_mask_counter)
{
    uint32_t length = 0;
    for (int i = 0; i < MENU_STATE_SIZE; i++)
    {
        menuState[i] = menuStateNext[i];
    }

    switch (menuState[0])
    {
    case MENU_ENTRY:
        switch (menuState[1])
        {
        case MENU_ENTRY_STEP_QUIT:
            switch (button_mask)
            {
            case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // EXIT
                io_seproxyhal_touch_exit(NULL);
                return 0;
            }
            break;

        default:
            break;
        }
        break;

    case MENU_SEND_MONEY:
        switch (menuState[1])
        {
        case MENU_SEND_MONEY_REJECT:
            switch (button_mask)
            {
            case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // EXIT
                cancelAction();
                return 0;
            }
            break;

        case MENU_SEND_MONEY_ACCEPT:
            switch (button_mask)
            {
            case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // SIGN
                // Send the data in the APDU buffer, along with a special code that
                // indicates approval. 64 is the number of bytes in the response APDU
                length = get_tx_signature(G_io_apdu_buffer);

                G_io_apdu_buffer[length++] = 0x90;
                G_io_apdu_buffer[length++] = 0x00;
                io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, length);
                // Return to the main screen.
                ui_screen_menu_entry();
                return 0;
            }
            break;

        default:
            break;
        }
        break;
    case MENU_REGISTER_NODE:
        switch (menuState[1])
        {
        case MENU_REGISTER_NODE_REJECT:
            switch (button_mask)
            {
            case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // EXIT
                cancelAction();
                return 0;
            }
            break;

        case MENU_REGISTER_NODE_ACCEPT:
            switch (button_mask)
            {
            case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // SIGN
                // Send the data in the APDU buffer, along with a special code that
                // indicates approval. 64 is the number of bytes in the response APDU
                length = get_tx_signature(G_io_apdu_buffer);
                G_io_apdu_buffer[length++] = 0x90;
                G_io_apdu_buffer[length++] = 0x00;
                io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, length);
                // Return to the main screen.
                ui_screen_menu_entry();
                return 0;
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    // right and left default navigation
    switch (button_mask)
    {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        if (menuStateNext[1] >= currentMenuMaxStep - 1)
        {
            menuStateNext[1] = 0;
        }
        else
        {
            menuStateNext[1]++;
        }
        ui_render();
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        if (menuStateNext[1] <= 0)
        {
            menuStateNext[1] = currentMenuMaxStep - 1;
        }
        else
        {
            menuStateNext[1]--;
        }
        ui_render();
        break;
    }

    return 0;
}

static void process_sign_data(void)
{
    unsigned int txDataLength;
    uint32_t length = 0;
    unsigned char tempInt32Byte[INT32_LENGTH];
    int tempInt32;
    // Getting data length
    os_memmove(tempInt32Byte, G_io_apdu_buffer + APDU_OFFSET_DATA_LENGTH, INT32_LENGTH);
    tempInt32 = *(int *)tempInt32Byte;
    txDataLength = tempInt32 - 4; // minus the derivation path index info

    // getting derivation path index for the account used to sign
    os_memmove(tempInt32Byte, G_io_apdu_buffer + APDU_OFFSET_DATA_START, INT32_LENGTH);
    tempInt32 = *(int *)tempInt32Byte;
    derivationPathIndex = tempInt32;

    length = deriveAndSign(G_io_apdu_buffer, derivationPathIndex, G_io_apdu_buffer + APDU_OFFSET_TX_DATA_START, txDataLength);
    // Send the data in the APDU buffer, along with a special code that indicates approval
    G_io_apdu_buffer[length++] = 0x90;
    G_io_apdu_buffer[length++] = 0x00;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, length);
    // Return to the main screen.
    ui_screen_menu_entry();
}

static void process_sign_transaction(void)
{
    unsigned char tempInt32Byte[INT32_LENGTH];
    int tempInt32;

    switch (G_io_apdu_buffer[APDU_OFFSET_P1])
    {
    case P1_FIRST:
        // resetting tx data storage
        reset_tx_data();

        // getting derivation path index for the account used to sign
        os_memmove(tempInt32Byte, G_io_apdu_buffer + APDU_OFFSET_DATA_START, INT32_LENGTH);
        tempInt32 = *(int *)tempInt32Byte;
        set_derivation_path(tempInt32);

        // Getting data length
        os_memmove(tempInt32Byte, G_io_apdu_buffer + APDU_OFFSET_DATA_LENGTH, INT32_LENGTH);
        tempInt32 = *(int *)tempInt32Byte - 4; // minus the derivation path index info

        // appending tx data to the tx data storage
        append_tx_data(G_io_apdu_buffer + APDU_OFFSET_TX_DATA_START, tempInt32);
        break;

    case P1_MORE:
        if (G_io_apdu_buffer[APDU_OFFSET_P2] != P2_MORE)
        {
            process_more_tx();
        }
        break;

    default:
        break;
    }

    switch (G_io_apdu_buffer[APDU_OFFSET_P2])
    {
    case P2_LAST:
        finalize_transaction();
        parse_transaction_data(currentTx.txBytes);
        break;

    case P2_MORE:
        if (G_io_apdu_buffer[APDU_OFFSET_P1] != P1_FIRST)
        {
            process_more_tx();
        }
        THROW(0x9000);
        break;

    default:
        break;
    }
}

static void process_more_tx()
{
    unsigned char tempInt32Byte[INT32_LENGTH];
    int tempInt32;

    // Getting data length
    os_memmove(tempInt32Byte, G_io_apdu_buffer + APDU_OFFSET_DATA_LENGTH, INT32_LENGTH);
    tempInt32 = *(int *)tempInt32Byte;

    // appending tx data to the tx data storage
    append_tx_data(G_io_apdu_buffer + APDU_OFFSET_DATA_START, tempInt32);
}

static void process_hash_data(void)
{
    int length = HASH256_LENGTH;
    uint8_t hashResult[HASH256_LENGTH];

    unsigned char tempInt32Byte[INT32_LENGTH];
    int tempInt32;

    // Getting data length
    os_memmove(tempInt32Byte, G_io_apdu_buffer + APDU_OFFSET_DATA_LENGTH, INT32_LENGTH);
    tempInt32 = *(int *)tempInt32Byte;

    if (G_io_apdu_buffer[APDU_OFFSET_P1] == P1_FIRST)
    {
        cx_sha3_init(&hashContext, 256);
    }

    hash3_256(&hashContext, NULL, G_io_apdu_buffer + APDU_OFFSET_DATA_START, tempInt32);

    switch (G_io_apdu_buffer[APDU_OFFSET_P2])
    {
    case P2_LAST:
        // generate the result
        hash3_256(&hashContext, hashResult, NULL, 0);
        os_memmove(G_io_apdu_buffer, hashResult, length);
        G_io_apdu_buffer[length++] = 0x90;
        G_io_apdu_buffer[length++] = 0x00;
        io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, length);
        // Return to the main screen.
        ui_screen_menu_entry();
        break;

    default:
        break;
    }

    if (G_io_apdu_buffer[APDU_OFFSET_P1] == P1_MORE || G_io_apdu_buffer[APDU_OFFSET_P2] == P2_MORE)
    {
        // since it's not final yet, we don't expect result to be generated
        THROW(0x9000);
    }
}

static void parse_transaction_data(uint8_t *txBytes)
{
    unsigned char tempInt32Byte[INT32_LENGTH];
    unsigned char tempInt64Byte[INT64_LENGTH];
    int tempInt32, txType;
    int64_t tempInt64;
    int dataOffset = 0;

    // getting transaction type
    os_memmove(tempInt32Byte, txBytes + dataOffset, INT32_LENGTH);
    tempInt32 = *(int *)tempInt32Byte;
    dataOffset += 4;
    txType = tempInt32;

    // skipping version and timestamp information
    dataOffset += 1 + 8;

    switch (txType)
    {
    case TRANSACTION_TYPE_SEND_MONEY:
        // parsing type of sender address
        os_memmove(tempInt32Byte, txBytes + dataOffset, INT32_LENGTH);
        dataOffset += INT32_LENGTH;
        tempInt32 = *(int *)tempInt32Byte;
        bytesToZoobcAccountAddress(txSenderAddress, txBytes + dataOffset);
        dataOffset += getAddressLength(tempInt32);

        // parsing type of recipient address
        os_memmove(tempInt32Byte, txBytes + dataOffset, INT32_LENGTH);
        dataOffset += INT32_LENGTH;
        tempInt32 = *(int *)tempInt32Byte;
        bytesToZoobcAccountAddress(txRecipientAddress, txBytes + dataOffset);
        dataOffset += getAddressLength(tempInt32);

        // parsing the fee of a transaction
        os_memmove(tempInt64Byte, txBytes + dataOffset, INT64_LENGTH);
        tempInt64 = *(int64_t *)tempInt64Byte;
        tostring64(&tempInt64, 10, txFee, TEXT_BOTTOM_SIZE);
        convertDigits(txFee, TEXT_BOTTOM_SIZE);
        dataOffset += INT64_LENGTH;

        // parsing the amount of a transaction
        dataOffset += INT32_LENGTH; // skipping parse byte length
        os_memmove(tempInt64Byte, txBytes + dataOffset, INT64_LENGTH);
        tempInt64 = *(int64_t *)tempInt64Byte;
        tostring64(&tempInt64, 10, txAmount, TEXT_BOTTOM_SIZE);
        convertDigits(txAmount, TEXT_BOTTOM_SIZE);
        dataOffset += INT64_LENGTH;

        ui_screen_send_money();
        break;

    case TRANSACTION_TYPE_REGISTER_NODE:
        // skipping length and the bytes of sender address
        os_memmove(tempInt32Byte, txBytes + dataOffset, INT32_LENGTH);
        dataOffset += INT32_LENGTH;
        tempInt32 = *(int *)tempInt32Byte;
        dataOffset += getAddressLength(tempInt32);

        // skipping length and the bytes of recipient address
        os_memmove(tempInt32Byte, txBytes + dataOffset, INT32_LENGTH);
        dataOffset += INT32_LENGTH;
        tempInt32 = *(int *)tempInt32Byte;
        dataOffset += getAddressLength(tempInt32);

        // parsing the fee of a transaction
        os_memmove(tempInt64Byte, txBytes + dataOffset, INT64_LENGTH);
        tempInt64 = *(int64_t *)tempInt64Byte;
        tostring64(&tempInt64, 10, txFee, TEXT_BOTTOM_SIZE);
        convertDigits(txFee, TEXT_BOTTOM_SIZE);
        dataOffset += INT64_LENGTH;

        // skipping length of body byte
        dataOffset += INT32_LENGTH;

        // copying the node publicKey
        bytesToZoobcNodeAddress(nodePublicKey, txBytes + dataOffset);
        dataOffset += getAddressLength(0);

        // copying the owner account address to global variable
        os_memmove(tempInt32Byte, txBytes + dataOffset, INT32_LENGTH);
        dataOffset += INT32_LENGTH;
        tempInt32 = *(int *)tempInt32Byte;
        bytesToZoobcAccountAddress(ownerAccountAddress, txBytes + dataOffset);
        dataOffset += getAddressLength(tempInt32);

        // parsing the locked balance of node registration
        os_memmove(tempInt64Byte, txBytes + dataOffset, INT64_LENGTH);
        tempInt64 = *(int64_t *)tempInt64Byte;
        tostring64(&tempInt64, 10, lockedBalance, TEXT_BOTTOM_SIZE);
        convertDigits(lockedBalance, TEXT_BOTTOM_SIZE);
        dataOffset += INT64_LENGTH;

        ui_screen_register_node();
        break;

    default:
        break;
    }
}

static void sample_main(void)
{
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;
    char debugData = (CX_APILEVEL);

    // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
    // goal is to retrieve APDU.
    // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
    // sure the io_event is called with a
    // switch event, before the apdu is replied to the bootloader. This avoid
    // APDU injection faults.
    for (;;)
    {
        volatile unsigned short sw = 0;

        BEGIN_TRY
        {
            TRY
            {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws
                        // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0)
                {
                    THROW(0x6982);
                }

                if (G_io_apdu_buffer[APDU_OFFSET_CLA] != CLA)
                {
                    THROW(0x6E00);
                }

                // unauthenticated instruction
                switch (G_io_apdu_buffer[APDU_OFFSET_INS])
                {
                case INS_RESET: // reset
                    flags |= IO_RESET_AFTER_REPLIED;
                    THROW(0x9000);
                    break;

                case INS_DEBUG: // bounce back message
                    G_io_apdu_buffer[0] = CX_APILEVEL;
                    G_io_apdu_buffer[1] = 0X90;
                    G_io_apdu_buffer[2] = 0X00;
                    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 3);
                    break;

                case INS_GET_PUBLIC_KEY: // get public key
                    process_get_public_key();
                    tx = 32;
                    THROW(0x9000);
                    break;

                case INS_SIGN_TRANSACTION: // sign
                    process_sign_transaction();
                    flags |= IO_ASYNCH_REPLY;
                    break;

                case INS_SIGN_DATA:
                    process_sign_data();
                    flags |= IO_ASYNCH_REPLY;
                    break;

                case INS_HASH_DATA:
                    process_hash_data();
                    flags |= IO_ASYNCH_REPLY;
                    break;

                case 0xFF: // return to dashboard
                    goto return_to_dashboard;

                default:
                    THROW(0x6D00);
                    break;
                }
            }
            CATCH_OTHER(e)
            {
                switch (e & 0xF000)
                {
                case 0x6000:
                case 0x9000: // return okay
                    sw = e;
                    break;
                default:
                    sw = 0x6800 | (e & 0x7FF);
                    break;
                }
                // Unexpected exception => report
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY
            {
            }
        }
        END_TRY;
    }

return_to_dashboard:
    return;
}

void io_seproxyhal_display(const bagl_element_t *element)
{
    io_seproxyhal_display_default((bagl_element_t *)element);
}

unsigned char io_event(unsigned char channel)
{
    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0])
    {
    case SEPROXYHAL_TAG_FINGER_EVENT:
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
        if (UX_DISPLAYED())
        {
            // TODO perform actions after all screen elements have been
            // displayed
        }
        else
        {
            UX_DISPLAYED_EVENT();
        }
        break;

    // unknown events are acknowledged
    default:
        break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent())
    {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

__attribute__((section(".boot"))) int main(void)
{
    // exit critical section
    __asm volatile("cpsie i");

    UX_INIT();

    // ensure exception will work as planned
    os_boot();

    BEGIN_TRY
    {
        TRY
        {
            io_seproxyhal_init();

#ifdef LISTEN_BLE
            if (os_seph_features() &
                SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_BLE)
            {
                BLE_power(0, NULL);
                // restart IOs
                BLE_power(1, NULL);
            }
#endif

            USB_power(0);
            USB_power(1);

            ui_screen_menu_entry();

            sample_main();
        }
        CATCH_OTHER(e)
        {
        }
        FINALLY
        {
        }
    }
    END_TRY;
}