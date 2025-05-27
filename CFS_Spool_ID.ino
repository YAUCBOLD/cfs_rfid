#include <FS.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <Update.h>
#include <LittleFS.h>
#include "src/includes.h"
// #include <algorithm> // For std::min, but Arduino min() macro is usually fine.

#define SS_PIN 5
#define RST_PIN 21
#define SPK_PIN 27

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::MIFARE_Key ekey;
WebServer webServer(80);
AES aes;
File upFile;
String upMsg;
MD5Builder md5;

IPAddress Server_IP(10, 1, 0, 1);
IPAddress Subnet_Mask(255, 255, 255, 0);
String spoolData = "AB1240276A210100100000FF016500000100000000000000"; // Default spool data
String AP_SSID = "K2_RFID";
String AP_PASS = "password";
String WIFI_SSID = "";
String WIFI_PASS = "";
String WIFI_HOSTNAME = "k2.local";
String PRINTER_HOSTNAME = "";
bool encrypted = false; // Flag to indicate if card was authenticated with ekey

String currentMode = "write"; // Default to write mode

// Global variables to store data read from card
String readMaterialBrand = ""; // Will be determined by client based on ID
String readMaterialType = "";  // Stores Filament ID read from card
String readMaterialWeight = "";
String readMaterialColor = "";
String readRawSpoolData = "";   // Stores concatenated, (eventually) decrypted data
bool cardReadSuccess = false;   // Flag for UI to know if read was successful


// Web server handler to set the current operating mode
void handleSetMode() {
  if (webServer.hasArg("mode")) {
    currentMode = webServer.arg("mode");
    webServer.send(200, "text/plain", "Mode set to " + currentMode);
    Serial.println("Mode set to: " + currentMode);
  } else {
    webServer.send(400, "text/plain", "Mode argument missing");
  }
}

// Function to handle reading data from the Mifare card
void handleReadMode() {
  Serial.println(F("Attempting to read spool data from card..."));
  cardReadSuccess = false;
  readRawSpoolData = "";
  readMaterialType = "";
  readMaterialColor = "";
  readMaterialWeight = "";

  byte blockBuffer[18];
  byte encryptedBlock[16];
  byte decryptedBlock[16];

  MFRC522::StatusCode status_read;
  String tempSpoolData = "";

  for (int blockNum = 4; blockNum <= 6; blockNum++) {
    byte bufferSize = sizeof(blockBuffer);
    status_read = mfrc522.MIFARE_Read(blockNum, blockBuffer, &bufferSize);

    if (status_read == MFRC522::STATUS_OK) {
      if (bufferSize < 16) {
        Serial.print(F("MIFARE_Read for block ")); Serial.print(blockNum);
        Serial.print(F(" returned less than 16 bytes: ")); Serial.println(bufferSize);
        memset(encryptedBlock, 'X', sizeof(encryptedBlock)); // Fill with error marker
      } else {
        memcpy(encryptedBlock, blockBuffer, 16);
      }

      // --- AES DECRYPTION STUB ---
      // actual aes.decrypt(1, encryptedBlock, decryptedBlock) call will replace this.
      bool decryption_ok = false; // Placeholder
      // When AES decrypt is added:
      // if (aes.decrypt(1, encryptedBlock, decryptedBlock) == 0) { // Assuming 0 is success
      //   decryption_ok = true;
      //   for (int i = 0; i < 16; i++) { tempSpoolData += (char)decryptedBlock[i]; }
      // } else {
      //   Serial.print(F("Decryption failed for block ")); Serial.println(blockNum);
      //   for (int i = 0; i < 16; i++) { tempSpoolData += (char)encryptedBlock[i]; } // Append raw for now
      // }
      // For now, without decrypt, just use the (likely encrypted) data:
      for (int i = 0; i < 16; i++) {
        tempSpoolData += (char)encryptedBlock[i];
      }
      // --- END AES DECRYPTION STUB ---

    } else {
      Serial.print(F("MIFARE_Read failed for block ")); Serial.print(blockNum);
      Serial.print(F(": ")); Serial.println(mfrc522.GetStatusCodeName(status_read));
      tone(SPK_PIN, 200, 200); delay(100); tone(SPK_PIN, 200, 200);
      return;
    }
  }

  int firstUnprintable = -1;
  for(int i=0; i < tempSpoolData.length(); ++i) {
    if(tempSpoolData.charAt(i) < 32 || tempSpoolData.charAt(i) > 126) {
      firstUnprintable = i;
      break;
    }
  }
  if(firstUnprintable != -1) {
    readRawSpoolData = tempSpoolData.substring(0, firstUnprintable);
  } else {
    readRawSpoolData = tempSpoolData;
  }
  if (readRawSpoolData.length() > 48) {
    readRawSpoolData = readRawSpoolData.substring(0, 48);
  }

  Serial.print(F("Concatenated raw data from card (cleaned, up to 48 chars): "));
  Serial.println(readRawSpoolData);

  if (readRawSpoolData.startsWith("AB124") && readRawSpoolData.length() >= 27) {
    readMaterialType = readRawSpoolData.substring(11, 16);
    String colorHexWithPrefix = readRawSpoolData.substring(16, 23);
    String filamentLenHex = readRawSpoolData.substring(23, 27);

    if (filamentLenHex == "0330") readMaterialWeight = "1 KG";
    else if (filamentLenHex == "0247") readMaterialWeight = "750 G";
    else if (filamentLenHex == "0198") readMaterialWeight = "600 G";
    else if (filamentLenHex == "0165") readMaterialWeight = "500 G";
    else if (filamentLenHex == "0082") readMaterialWeight = "250 G";
    else readMaterialWeight = "";

    if (colorHexWithPrefix.length() == 7 && colorHexWithPrefix.startsWith("0")) {
      readMaterialColor = "#" + colorHexWithPrefix.substring(1);
    } else {
      readMaterialColor = "#000000";
      Serial.println(F("Warning: Parsed color hex format was not as expected."));
    }

    Serial.println(F("Card data parsed successfully:"));
    Serial.print(F("  Filament ID (readMaterialType): ")); Serial.println(readMaterialType);
    Serial.print(F("  Color (readMaterialColor): ")); Serial.println(readMaterialColor);
    Serial.print(F("  Weight (readMaterialWeight): ")); Serial.println(readMaterialWeight);

    cardReadSuccess = true;
    tone(SPK_PIN, 1200, 150); delay(150); tone(SPK_PIN, 1200, 150);
  } else {
    Serial.println(F("Failed to parse spool data: Invalid format, prefix, or insufficient length."));
    Serial.print(F("  Read data prefix: ")); Serial.println(readRawSpoolData.substring(0,min((unsigned int)20, (unsigned int)readRawSpoolData.length())));
    Serial.print(F("  Read data length: ")); Serial.println(readRawSpoolData.length());
    readMaterialType = ""; readMaterialColor = ""; readMaterialWeight = "";
    cardReadSuccess = false;
    tone(SPK_PIN, 200, 500);
  }
}


void setup()
{
  Serial.begin(115200);
  LittleFS.begin(true);
  loadConfig();
  SPI.begin();
  mfrc522.PCD_Init();
  key = {255, 255, 255, 255, 255, 255};
  pinMode(SPK_PIN, OUTPUT);
  if (AP_SSID == "" || AP_PASS == "")
  {
    AP_SSID = "K2_RFID";
    AP_PASS = "password";
  }
  WiFi.softAPConfig(Server_IP, Server_IP, Subnet_Mask);
  WiFi.softAP(AP_SSID.c_str(), AP_PASS.c_str());

  if (WIFI_SSID != "" && WIFI_PASS != "")
  {
    WiFi.setAutoReconnect(true);
    WiFi.hostname(WIFI_HOSTNAME);
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    if (WiFi.waitForConnectResult() == WL_CONNECTED)
    {
      IPAddress LAN_IP = WiFi.localIP();
      Serial.print("WiFi Connected. IP Address: ");
      Serial.println(LAN_IP);
    } else {
      Serial.println("WiFi connection failed.");
    }
  }
  if (WIFI_HOSTNAME != "")
  {
    String mdnsHost = WIFI_HOSTNAME;
    mdnsHost.replace(".local", "");
    if (MDNS.begin(mdnsHost.c_str())) {
        Serial.println("MDNS responder started");
    } else {
        Serial.println("Error setting up MDNS responder!");
    }
  }

  webServer.on("/config", HTTP_GET, handleConfig);
  webServer.on("/index.html", HTTP_GET, handleIndex);
  webServer.on("/", HTTP_GET, handleIndex);
  webServer.on("/material_database.json", HTTP_GET, handleDb);
  webServer.on("/config", HTTP_POST, handleConfigP);
  webServer.on("/spooldata", HTTP_POST, handleSpoolData);
  webServer.on("/setmode", HTTP_POST, handleSetMode); // Handler for mode setting
  webServer.on("/update.html", HTTP_POST, []() {
    webServer.send(200, "text/plain", upMsg);
    delay(1000);
    ESP.restart();
  }, []() {
    handleFwUpdate();
  });
  webServer.on("/updatedb.html", HTTP_POST, []() {
    webServer.send(200, "text/plain", upMsg);
    delay(1000);
    ESP.restart();
  }, []() {
    handleDbUpdate();
  });
  webServer.onNotFound(handle404);
  webServer.begin();
  Serial.println("HTTP server started");
}


void loop()
{
  webServer.handleClient();
  if (!mfrc522.PICC_IsNewCardPresent()){
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()){
    return;
  }

  encrypted = false;

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Unsupported card type."));
    tone(SPK_PIN, 400, 400);
    delay(2000);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  createKey();

  MFRC522::StatusCode status;
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &ekey, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
      Serial.print(F("Authentication failed (both keys): "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      tone(SPK_PIN, 400, 150); delay(300); tone(SPK_PIN, 400, 150);
      delay(2000);
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      return;
    }
    encrypted = true;
    Serial.println(F("Authenticated with ekey."));
  } else {
    Serial.println(F("Authenticated with default key."));
  }

  if (currentMode == "write") {
    Serial.println(F("Write Mode Active"));

    MFRC522::StatusCode status_write = MFRC522::STATUS_OK;

    byte blockData[17];
    byte encData[16];
    int writeBlockIDStart = 4;

    int numSpoolDataBlocksToWrite = (spoolData.length() + 15) / 16;
    if (numSpoolDataBlocksToWrite > 3) {
        numSpoolDataBlocksToWrite = 3;
    }
    if (numSpoolDataBlocksToWrite <= 0 && spoolData.length() > 0) {
        numSpoolDataBlocksToWrite = 1;
    }

    for (int i = 0; i < numSpoolDataBlocksToWrite; ++i) {
      int currentBlockAbsolute = writeBlockIDStart + i;
      unsigned int spool_len = spoolData.length();
      unsigned int segment_end = (unsigned int)((i + 1) * 16);
      String segment = spoolData.substring(i * 16, min(segment_end, spool_len));

      memset(blockData, 0, sizeof(blockData));
      segment.getBytes(blockData, 17);

      aes.encrypt(1, blockData, encData);

      status_write = (MFRC522::StatusCode)mfrc522.MIFARE_Write(currentBlockAbsolute, encData, 16);
      if (status_write != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write failed for spool data at block "));
        Serial.print(currentBlockAbsolute); Serial.print(F(": "));
        Serial.println(mfrc522.GetStatusCodeName(status_write));
        break;
      }
    }

    if (status_write == MFRC522::STATUS_OK && !encrypted) {
      byte trailer_buffer[18];
      // Prepare trailer with ekey and standard access bits FF078069
      for (int k = 0; k < 6; k++) trailer_buffer[k] = ekey.keyByte[k]; // Key A
      trailer_buffer[6] = 0xFF; trailer_buffer[7] = 0x07; trailer_buffer[8] = 0x80; trailer_buffer[9] = 0x69; // Access Bits
      for (int k = 0; k < 6; k++) trailer_buffer[10 + k] = ekey.keyByte[k]; // Key B

      status_write = (MFRC522::StatusCode)mfrc522.MIFARE_Write(7, trailer_buffer, 16);
      if (status_write != MFRC522::STATUS_OK) {
          Serial.print(F("MIFARE_Write to trailer block 7 failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status_write));
      } else {
          Serial.println(F("Trailer block 7 updated with ekey."));
      }
    }

    if (status_write == MFRC522::STATUS_OK) {
      tone(SPK_PIN, 1000, 200);
    } else {
      tone(SPK_PIN, 200, 500);
    }
    delay(2000);

  } else if (currentMode == "read") {
    Serial.println(F("Read Mode Active"));
    handleReadMode();
  } else {
    Serial.print(F("Unknown mode: ")); Serial.println(currentMode);
    tone(SPK_PIN, 100, 500);
    delay(2000);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void createKey()
{
  int x = 0;
  byte uid_bytes_for_key[16]; // Changed name for clarity
  byte bufOut[16];
  for (int i = 0; i < 16; i++)
  {
    if (x >= mfrc522.uid.size) // Use actual UID size if available, typically 4 for Mifare Classic
      x = 0;
    uid_bytes_for_key[i] = mfrc522.uid.uidByte[x];
    x++;
  }
  aes.encrypt(0, uid_bytes_for_key, bufOut); // aes.encrypt with type 0 for key generation
  for (int i = 0; i < 6; i++)
  {
    ekey.keyByte[i] = bufOut[i];
  }
}

void handleIndex()
{
  webServer.send_P(200, "text/html", indexData);
}

void handle404()
{
  webServer.send(404, "text/plain", "Not Found");
}

void handleConfig()
{
  String htmStr = AP_SSID + "|-|" + WIFI_SSID + "|-|" + WIFI_HOSTNAME + "|-|" + PRINTER_HOSTNAME;
  webServer.setContentLength(htmStr.length());
  webServer.send(200, "text/plain", htmStr);
}

void handleConfigP()
{
  if (webServer.hasArg("ap_ssid") && webServer.hasArg("ap_pass") && webServer.hasArg("wifi_ssid") && webServer.hasArg("wifi_pass") && webServer.hasArg("wifi_host") && webServer.hasArg("printer_host"))
  {
    AP_SSID = webServer.arg("ap_ssid");
    if (!webServer.arg("ap_pass").equals("********"))
    {
      AP_PASS = webServer.arg("ap_pass");
    }
    WIFI_SSID = webServer.arg("wifi_ssid");
    if (!webServer.arg("wifi_pass").equals("********"))
    {
      WIFI_PASS = webServer.arg("wifi_pass");
    }
    WIFI_HOSTNAME = webServer.arg("wifi_host");
    PRINTER_HOSTNAME = webServer.arg("printer_host");
    File file = LittleFS.open("/config.ini", "w");
    if (file)
    {
      file.print("\r\nAP_SSID=" + AP_SSID + "\r\nAP_PASS=" + AP_PASS + "\r\nWIFI_SSID=" + WIFI_SSID + "\r\nWIFI_PASS=" + WIFI_PASS + "\r\nWIFI_HOST=" + WIFI_HOSTNAME + "\r\nPRINTER_HOST=" + PRINTER_HOSTNAME + "\r\n");
      file.close();
    }
    String htmStr = "OK";
    webServer.setContentLength(htmStr.length());
    webServer.send(200, "text/plain", htmStr);
    delay(1000);
    ESP.restart();
  }
  else
  {
    webServer.send(417, "text/plain", "Expectation Failed");
  }
}

void handleDb()
{
  File dataFile = LittleFS.open("/matdb.gz", "r");
  if (!dataFile) {
    webServer.sendHeader("Content-Encoding", "gzip");
    webServer.send_P(200, "application/json", material_database, sizeof(material_database));
  }
  else
  {
    webServer.streamFile(dataFile, "application/json");
    dataFile.close();
  }
}

void handleDbUpdate()
{
  upMsg = "";
  if (webServer.uri() != "/updatedb.html") {
    upMsg = "Error";
    return;
  }
  HTTPUpload &upload = webServer.upload();
  if (upload.filename != "material_database.json") {
    upMsg = "Invalid database file<br><br>" + upload.filename;
    return;
  }
  if (upload.status == UPLOAD_FILE_START) {
    if (LittleFS.exists("/matdb.gz")) {
      LittleFS.remove("/matdb.gz");
    }
    upFile = LittleFS.open("/matdb.gz", "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (upFile) {
      upFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (upFile) {
      upFile.close();
      upMsg = "Database update complete, Rebooting";
    }
  }
}

void handleFwUpdate()
{
  upMsg = "";
  if (webServer.uri() != "/update.html") {
    upMsg = "Error";
    return;
  }
  HTTPUpload &upload = webServer.upload();
  if (!upload.filename.endsWith(".bin")) {
    upMsg = "Invalid update file<br><br>" + upload.filename;
    return;
  }
  if (upload.status == UPLOAD_FILE_START) {
    if (LittleFS.exists("/update.bin")) {
      LittleFS.remove("/update.bin");
    }
    upFile = LittleFS.open("/update.bin", "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (upFile) {
      upFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (upFile) {
      upFile.close();
    }
    updateFw();
  }
}

void updateFw()
{
  if (LittleFS.exists("/update.bin")) {
    File updateFile;
    updateFile = LittleFS.open("/update.bin", "r");
    if (updateFile) {
      size_t updateSize = updateFile.size();
      if (updateSize > 0) {
        md5.begin();
        md5.addStream(updateFile, updateSize);
        md5.calculate();
        String md5Hash = md5.toString();
        updateFile.close();
        updateFile = LittleFS.open("/update.bin", "r");
        if (updateFile) {
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if (!Update.begin(maxSketchSpace, U_FLASH)) {
            updateFile.close();
            upMsg = "Update failed<br><br>" + errorMsg(Update.getError());
            return;
          }
          int md5BufSize = md5Hash.length() + 1;
          char md5Buf[md5BufSize];
          md5Hash.toCharArray(md5Buf, md5BufSize) ;
          Update.setMD5(md5Buf);
          long bsent = 0;
          int cprog = 0;
          while (updateFile.available()) {
            uint8_t ibuffer[1];
            updateFile.read((uint8_t *)ibuffer, 1);
            Update.write(ibuffer, sizeof(ibuffer));
            bsent++;
            int progr = ((double)bsent /  updateSize) * 100;
            if (progr >= cprog) {
              cprog = progr + 10;
            }
          }
          updateFile.close();
          LittleFS.remove("/update.bin");
          if (Update.end(true))
          {
            String uHash = md5Hash.substring(0, 10);
            String iHash = Update.md5String().substring(0, 10);
            iHash.toUpperCase();
            uHash.toUpperCase();
            upMsg = "Uploaded:&nbsp; " + uHash + "<br>Installed: " + iHash + "<br><br>Update complete, Rebooting";
          }
          else
          {
            upMsg = "Update failed";
          }
        }
      }
      else {
        updateFile.close();
        LittleFS.remove("/update.bin");
        upMsg = "Error, file is invalid";
        return;
      }
    }
  }
  else
  {
    upMsg = "No update file found";
  }
}

void handleSpoolData()
{
  if (webServer.hasArg("materialColor") && webServer.hasArg("materialType") && webServer.hasArg("materialWeight"))
  {
    String materialColor = webServer.arg("materialColor");
    materialColor.replace("#", "");
    String filamentId = "1" + webServer.arg("materialType");
    String vendorId = "0276";
    String color = "0" + materialColor;
    String filamentLen = GetMaterialLength(webServer.arg("materialWeight"));
    String serialNum = String(random(100000, 999999));
    String reserve = "000000";
    spoolData = "AB124" + vendorId + "A2" + filamentId + color + filamentLen + serialNum + reserve + "00000000";

    File file = LittleFS.open("/spool.ini", "w");
    if (file)
    {
      file.print(spoolData);
      file.close();
    }
    String htmStr = "OK";
    webServer.setContentLength(htmStr.length());
    webServer.send(200, "text/plain", htmStr);
  }
  else
  {
    webServer.send(417, "text/plain", "Expectation Failed");
  }
}

String GetMaterialLength(String materialWeight)
{
  if (materialWeight == "1 KG") return "0330";
  else if (materialWeight == "750 G") return "0247";
  else if (materialWeight == "600 G") return "0198";
  else if (materialWeight == "500 G") return "0165";
  else if (materialWeight == "250 G") return "0082";
  return "0330"; // Default
}

String errorMsg(int errnum)
{
  if (errnum == UPDATE_ERROR_OK) return "No Error";
  else if (errnum == UPDATE_ERROR_WRITE) return "Flash Write Failed";
  else if (errnum == UPDATE_ERROR_ERASE) return "Flash Erase Failed";
  else if (errnum == UPDATE_ERROR_READ) return "Flash Read Failed";
  else if (errnum == UPDATE_ERROR_SPACE) return "Not Enough Space";
  else if (errnum == UPDATE_ERROR_SIZE) return "Bad Size Given";
  else if (errnum == UPDATE_ERROR_STREAM) return "Stream Read Timeout";
  else if (errnum == UPDATE_ERROR_MD5) return "MD5 Check Failed";
  else if (errnum == UPDATE_ERROR_MAGIC_BYTE) return "Magic byte is wrong, not 0xE9";
  return "UNKNOWN";
}

void loadConfig()
{
  if (LittleFS.exists("/config.ini"))
  {
    File file = LittleFS.open("/config.ini", "r");
    if (file)
    {
      String iniData;
      while (file.available())
      {
        char chnk = file.read();
        iniData += chnk;
      }
      file.close();
      if (instr(iniData, "AP_SSID=")) { AP_SSID = split(iniData, "AP_SSID=", "\r\n"); AP_SSID.trim(); }
      if (instr(iniData, "AP_PASS=")) { AP_PASS = split(iniData, "AP_PASS=", "\r\n"); AP_PASS.trim(); }
      if (instr(iniData, "WIFI_SSID=")) { WIFI_SSID = split(iniData, "WIFI_SSID=", "\r\n"); WIFI_SSID.trim(); }
      if (instr(iniData, "WIFI_PASS=")) { WIFI_PASS = split(iniData, "WIFI_PASS=", "\r\n"); WIFI_PASS.trim(); }
      if (instr(iniData, "WIFI_HOST=")) { WIFI_HOSTNAME = split(iniData, "WIFI_HOST=", "\r\n"); WIFI_HOSTNAME.trim(); }
      if (instr(iniData, "PRINTER_HOST=")) { PRINTER_HOSTNAME = split(iniData, "PRINTER_HOST=", "\r\n"); PRINTER_HOSTNAME.trim(); }
    }
  }
  else
  {
    File file = LittleFS.open("/config.ini", "w");
    if (file) {
      file.print("\r\nAP_SSID=" + AP_SSID + "\r\nAP_PASS=" + AP_PASS + "\r\nWIFI_SSID=" + WIFI_SSID + "\r\nWIFI_PASS=" + WIFI_PASS + "\r\nWIFI_HOST=" + WIFI_HOSTNAME + "\r\nPRINTER_HOST=" + PRINTER_HOSTNAME + "\r\n");
      file.close();
    }
  }

  if (LittleFS.exists("/spool.ini"))
  {
    File file = LittleFS.open("/spool.ini", "r");
    if (file) {
      String iniData;
      while (file.available()) {
        char chnk = file.read();
        iniData += chnk;
      }
      file.close();
      spoolData = iniData;
    }
  }
  else
  {
    File file = LittleFS.open("/spool.ini", "w");
    if (file) {
      file.print(spoolData);
      file.close();
    }
  }
}

String split(String str, String from, String to)
{
  String tmpstr = str;
  tmpstr.toLowerCase();
  from.toLowerCase();
  to.toLowerCase();
  int pos1 = tmpstr.indexOf(from);
  if (pos1 == -1) return "";
  int pos2 = tmpstr.indexOf(to, pos1 + from.length());
  if (pos2 == -1) return "";
  String retval = str.substring(pos1 + from.length(), pos2);
  return retval;
}

bool instr(String str, String search)
{
  return str.indexOf(search) != -1;
}
