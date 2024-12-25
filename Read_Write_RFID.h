#include <SPI.h>
#include <MFRC522.h>

/* These are the pins that are needed for the,
   SPI communication with the RFID module
*/
#define RST_PIN 5
#define SS_PIN 9
#define DEBUG_LEVEL_01
// #define DEBUG_LEVEL_02
#define PRINT_READ_DATA
#define NEEDED1
/* Using the constructor of the MFRC522 class create the class variable,
 with the ss_pin and the reset pin specified.
*/
MFRC522 mfrc522(SS_PIN,RST_PIN); 
/*  Create the key instancce for the MIFARE CARD
  MIFARE cards generally have two kinds of Authentication keys
  1. Key A
  2. Key B

  each key of the size 6-Bytes.
  Generally the keys of the mifare cards are 0xFFFFFFFFFFFF :: max value of the 6 Bytes
  The cards we are dealing with are 1K in size.

*/
MFRC522::MIFARE_Key key;
static void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
    Serial.println("");
}
void run_setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  SPI.begin();
  /*
    PCD init is the Proximity Coupling Device. That is the RC522 module is initialized with the SPI bus activated.

  */
  mfrc522.PCD_Init();
  /*
    Preparing the general key for all kinds of MIFARE Cards. here we are using 0xFFFFFFFFFFFF as both key A and key B
  */
  key.keyByte[0] = 0xFF;
  key.keyByte[1] = 0xFF;
  key.keyByte[2] = 0xFF;
  key.keyByte[3] = 0xFF;
  key.keyByte[4] = 0xFF;
  key.keyByte[5] = 0xFF;
#ifdef DEBUG_LEVEL_02
  Serial.println("Key A and Key B Set : ");
  dump_byte_array(key.keyByte,MFRC522::MF_KEY_SIZE);
#endif
}

void write_to_card(byte data_block[],byte *sector,byte *blockAddr,byte *trailer) {
  
  
  /*
    We generally have a procedure to reset the loop is we do not find a new card
    There are two methods in the MRFC522 class for doing the job:

    mfrc522.PICC_IsNewCardPresent() : We use it for detecting if there is a new card detected or not?
      Output: 
        1. True: If a new card is detected
        2. False: No new card is detected

    mfrc522.PICC_ReadCardSerial() : We use this function for detecting some card that is placed on the reader.
      Output: 
        1. True: If a new card is detected
        2. False: No new card is detected
  */
  // Serial.println("AA");
  if(!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  Serial.println("Start Here");
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
#ifdef DEBUG_LEVEL_02
  // dump_byte_array is the function used to print the array based on pointer to the head of the array and the size of the array
  Serial.println("UID OF THE NEW CARD IS: ");
  dump_byte_array(mfrc522.uid.uidByte,mfrc522.uid.size); // print the UID of the card that is read.
  Serial.println("PICC Type of the card placed is : ");
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
#endif
    /*
      Checking the compatibility of the card that is being scanned
    */
  if ((piccType != MFRC522::PICC_TYPE_MIFARE_MINI)&&(piccType != MFRC522::PICC_TYPE_MIFARE_1K)&&(piccType != MFRC522::PICC_TYPE_MIFARE_4K)) 
  {
      Serial.println(F("This sample only works with MIFARE Classic cards."));
      return;
  }
    // byte sector = 1;
    // byte blockAddr = 5
  // byte sector         = 1;
  // byte blockAddr      = 5;
  // byte trailer        = 7;
  byte buffer[18];
  
  byte size = sizeof(buffer);
  MFRC522::StatusCode status;
  MFRC522::StatusCode status1;
  MFRC522::StatusCode status2;
  
  /* Authenticate with key A first and get the status code back onto the status variable.*/
  
  status1 = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailer, &key, &(mfrc522.uid));
  status2 = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailer, &key, &(mfrc522.uid));
  
  {
#ifdef NEEDED
      if(status1 != MFRC522::STATUS_OK){
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status1));
        /*
          Ending or terminating the entire operation that was stored in the r/w class variable
        */
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
      }
      else if(status2 != MFRC522::STATUS_OK){
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status2));
        /*
          Ending or terminating the entire operation that was stored in the r/w class variable
        */
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
      }
      else{
        // Eat 5 Star
      }
#endif
  }
  // Serial.println("Problem here");
#ifdef DEBUG_LEVEL_02
  // print the data in the sector after authentication
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();
#endif
  // read to the blocks after authenticating either using key A or key B
  if((status1 == MFRC522::STATUS_OK)||(status2 == MFRC522::STATUS_OK)){
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if(status != MFRC522::STATUS_OK){
#ifdef DEBUG_LEVEL_02      
      Serial.println("Some thing wrong while reading the block.");
#endif
      /*
          Ending or terminating the entire operation that was stored in the r/w class variable
      */
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
    else{
#ifdef DEBUG_LEVEL_02
      Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
      dump_byte_array(buffer, 16); Serial.println();
      Serial.println();
#endif
    }
  }

  /*
    Next step is to Write our block to the Card.
  */
#ifdef DEBUG_LEVEL_02
  Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
  Serial.println(F(" ..."));
  dump_byte_array(data_block, 16); Serial.println();
#endif
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, data_block, 16);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
  }
  else{
    Serial.println("Written!");
  }
  /*
    Ending or terminating the entire operation that was stored in the r/w class variable
  */
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void read_from_card(byte sector,byte blockAddr,byte trailer) {
  
  
  /*
    We generally have a procedure to reset the loop is we do not find a new card
    There are two methods in the MRFC522 class for doing the job:

    mfrc522.PICC_IsNewCardPresent() : We use it for detecting if there is a new card detected or not?
      Output: 
        1. True: If a new card is detected
        2. False: No new card is detected

    mfrc522.PICC_ReadCardSerial() : We use this function for detecting some card that is placed on the reader.
      Output: 
        1. True: If a new card is detected
        2. False: No new card is detected
  */
  // Serial.println("AA");
  if(!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  Serial.println("Start Here");
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
#ifdef DEBUG_LEVEL_02
  // dump_byte_array is the function used to print the array based on pointer to the head of the array and the size of the array
  Serial.println("UID OF THE NEW CARD IS: ");
  dump_byte_array(mfrc522.uid.uidByte,mfrc522.uid.size); // print the UID of the card that is read.
  Serial.println("PICC Type of the card placed is : ");
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
#endif
    /*
      Checking the compatibility of the card that is being scanned
    */
  if ((piccType != MFRC522::PICC_TYPE_MIFARE_MINI)&&(piccType != MFRC522::PICC_TYPE_MIFARE_1K)&&(piccType != MFRC522::PICC_TYPE_MIFARE_4K)) 
  {
      Serial.println(F("This sample only works with MIFARE Classic cards."));
      return;
  }
    // byte sector = 1;
    // byte blockAddr = 5
  // byte sector         = 1;
  // byte blockAddr      = 5;
  // byte trailer        = 7;
  byte buffer[18];
  
  byte size = sizeof(buffer);
  MFRC522::StatusCode status;
  MFRC522::StatusCode status1;
  MFRC522::StatusCode status2;
  
  /* Authenticate with key A first and get the status code back onto the status variable.*/
  
  status1 = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailer, &key, &(mfrc522.uid));
  status2 = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailer, &key, &(mfrc522.uid));
  
  {
#ifdef NEEDED
      if(status1 != MFRC522::STATUS_OK){
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status1));
        /*
          Ending or terminating the entire operation that was stored in the r/w class variable
        */
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
      }
      else if(status2 != MFRC522::STATUS_OK){
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status2));
        /*
          Ending or terminating the entire operation that was stored in the r/w class variable
        */
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
      }
      else{
        // Eat 5 Star
      }
#endif
  }
  // Serial.println("Problem here");
#ifdef PRINT_READ_DATA
  // print the data in the sector after authentication
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();
#endif
  // read to the blocks after authenticating either using key A or key B
  if((status1 == MFRC522::STATUS_OK)||(status2 == MFRC522::STATUS_OK)){
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if(status != MFRC522::STATUS_OK){
#ifdef DEBUG_LEVEL_02      
      Serial.println("Some thing wrong while reading the block.");
#endif
      /*
          Ending or terminating the entire operation that was stored in the r/w class variable
      */
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
    else{
#ifdef PRINT_READ_DATA
      Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
      dump_byte_array(buffer, 16); Serial.println();
      Serial.println();
#endif
    }
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}