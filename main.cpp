/*
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 *
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 *
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * Example sketch/program which will try the most used default keys listed in
 * https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys to dump the
 * block 0 of a MIFARE RFID card using a RFID-RC522 reader.
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 */

#include <bcm2835.h>
#include <MFRC522.h>
#include <cstdio>

MFRC522 mfrc522;   // Create MFRC522 instance.

// Number of known default keys (hard-coded)
// NOTE: Synchronize the NR_KNOWN_KEYS define with the defaultKeys[] array
#define NR_KNOWN_KEYS   8
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
    {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
    {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
    {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

/*
 * Initialize.
 */
void setup() {
//    Serial.begin(9600);         // Initialize serial communications with the PC
//    while (!Serial);            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
//    SPI.begin();                // Init SPI bus
    mfrc522.postConstruct();
    mfrc522.PCD_Init();         // Init MFRC522 card
    printf("Try the most used default keys to print block 0 of a MIFARE PICC.\n");
}

/*
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        printf(buffer[i] < 0x10 ? " 0" : " ");
        printf("0x%02x", buffer[i]);
    }
}

/*
 * Try using the PICC (the tag/card) with the given key to access block 0.
 * On success, it will show the key details, and dump the block data on Serial.
 *
 * @return true when the given key worked, false otherwise.
 */
bool try_key(MFRC522::MIFARE_Key *key)
{
    bool result = false;
    byte buffer[18];
    byte block = 0;
    byte status;

    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        // Serial.print(F("PCD_Authenticate() failed: "));
        // Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
        // Serial.print(F("MIFARE_Read() failed: "));
        // Serial.println(mfrc522.GetStatusCodeName(status));
    }
    else {
        // Successful read
        result = true;
        printf("Success with key:");
        dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
        printf("\n");
        // Dump block data
        printf("Block "); printf("%u", block); printf(":");
        dump_byte_array(buffer, 16);
        //Serial.println();
        printf("\n");
    }
    //Serial.println();
    printf("\n");

    mfrc522.PICC_HaltA();       // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
    return result;
}

/*
 * Main loop.
 */
void loop() {
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    //Serial.print(F("Card UID:"));
    printf("Card UID:");
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    //Serial.println();
    printf("\n");
    //Serial.print(F("PICC type: "));
    printf("PICC type: ");
    byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    //Serial.println(mfrc522.PICC_GetTypeName(piccType));
    printf("%s\n", mfrc522.PICC_GetTypeName(piccType).c_str());
    // Try the known default keys
    MFRC522::MIFARE_Key key;
    for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
        // Copy the known key into the MIFARE_Key structure
        for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
            key.keyByte[i] = knownKeys[k][i];
        }
        // Try the key
        if (try_key(&key)) {
            // Found and reported on the key and block,
            // no need to try other keys for this PICC
            break;
        }
    }
}

int main() {
    setup();
    while(true) {
        loop();
    }
    bcm2835_spi_end();
    bcm2835_close();

    return 0;
}





















//#include <iostream>
//#include <unistd.h>
//#include "rfid.h"
//#include "rc522.h"
//#include "bcm2835.h"
//using namespace std;

//#define  DEFAULT_SPI_SPEED 5000L

//uint8_t  initRfidReader();

//char     statusRfidReader;
//uint16_t CType=0;
//uint8_t  serialNumber[10];
//uint8_t  serialNumberLength = 0;
//uint8_t  noTagFoundCount = 0;
//char     rfidChipSerialNumber[23];
//char     rfidChipSerialNumberRecentlyDetected[23];
//char     *p;
//int      loopCounter;

//int main()
//{
//    initRfidReader();
//    InitRc522();

//    for (;;) {
//        statusRfidReader = find_tag( &CType );
//        if (statusRfidReader == TAG_NOTAG) {
//             // The status that no tag is found is sometimes set even when a tag is within reach of the tag reader
//             // to prevent that the reset is performed the no tag event has to take place multiple times (ger: entrprellen)
//             if (noTagFoundCount > 2) {
//                 // Sets the content of the array 'rfidChipSerialNumberRecentlyDetected' back to zero
//                 memset(&rfidChipSerialNumberRecentlyDetected[0], 0, sizeof(rfidChipSerialNumberRecentlyDetected));
//                 noTagFoundCount = 0;
//             }
//             else {
//                 noTagFoundCount++;
//             }
//             usleep(200000);
//             continue;
//        } else if (statusRfidReader != TAG_OK && statusRfidReader != TAG_COLLISION) {
//            continue;
//        }
//        if (select_tag_sn(serialNumber, &serialNumberLength) != TAG_OK) {
//            continue;
//        }

//        // Is a successful detected, the counter will be set to zero
//        noTagFoundCount = 0;
//        p = rfidChipSerialNumber;
//        for (loopCounter = 0; loopCounter < serialNumberLength; loopCounter++) {
//            sprintf(p,"%02x", serialNumber[loopCounter]);
//            p += 2;
//        }

//        // Only when the serial number of the currently detected tag differs from the
//        // recently detected tag the callback will be executed with the serial number
//        if(strcmp(rfidChipSerialNumberRecentlyDetected, rfidChipSerialNumber) != 0)
//        {
//            //Local<Value> argv[argc] = { Local<Value>::New(String::New(&rfidChipSerialNumber[1])) };
//            //callback->Call(Context::GetCurrent()->Global(), argc, argv);
//        }

//        // Preserves the current detected serial number, so that it can be used
//        // for future evaluations
//        strcpy(rfidChipSerialNumberRecentlyDetected, rfidChipSerialNumber);

//        *(p++) = 0;
//    }

//    bcm2835_spi_end();
//    bcm2835_close();

//    return 0;
//}

//uint8_t initRfidReader() {
//    uint16_t SPI_SPEED;

//    SPI_SPEED = (uint16_t)(250000L / DEFAULT_SPI_SPEED);
//    if (!bcm2835_init()) {
//        return 1;
//    }

//    bcm2835_spi_begin();
//    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
//    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
//    bcm2835_spi_setClockDivider(SPI_SPEED); 							  // The default
//    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
//    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
//    return 0;
//}
