/*
 * ------------------------------------------------*
 * Typical pin or GPIO layout used:                |
 * ------------------------------------------------*
 *             MFRC522      Raspberry Pi B+ &      |
 *             Reader/PCD   2 B                    |
 * Signal      Pin          Pin           GPIO     |
 * ------------------------------------------------*
 * RST/Reset   RST          22            25       |
 * SPI SS      SDA(SS)      24            8        |
 * SPI MOSI    MOSI         19            10       |
 * SPI MISO    MISO         21            9        |
 * SPI SCK     SCK          23            11       |
 * ------------------------------------------------*
 */

#include <bcm2835.h>
#include <MFRC522.h>
#include <cstdio>
#include <signal.h>

// Volatile boolean variable to Terminate programm with SIGINT
volatile bool keepReading = true;

// Create MFRC522 instance.
MFRC522 mfrc522;


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
//void setup() {
////    Serial.begin(9600);         // Initialize serial communications with the PC
////    while (!Serial);            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
////    SPI.begin();                // Init SPI bus

//}

/*
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        printf(buffer[i] < 0x10 ? " 0" : " ");
        printf("%02x", buffer[i]);
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
        return false;
    }

    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
        // printf(("MIFARE_Read() failed: "));
        // printf("%s\n"mfrc522.GetStatusCodeName(status).c_str());
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
        printf("\n");
    }    
    printf("\n");

    mfrc522.PICC_HaltA();       // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
    return result;
}


void loop() {
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    printf("Card UID:");
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);   
    printf("\n");   
    printf("PICC type: ");
    byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);    
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

void end_Signal(int signal) {
     //printf("Signal : %u\n", signal);
     keepReading = false;
}

int main() {
    signal(SIGINT, end_Signal);
    //MFRC522 mfrc522;
    mfrc522.postConstruct();
    mfrc522.PCD_Init();         // Init MFRC522 card
    // Change the block number to get information from by default it's 0
    printf("Try the most used default keys to print block 0 of a MIFARE PICC.\n");
    printf("Press Ctrl + C to EXIT\n");
    while(keepReading) {
        loop();
    }
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}
