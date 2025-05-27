# ESP32 RFID Filament Spool ID System

This project is an ESP32-based system for managing 3D printer filament spool information using MIFARE RFID tags. It features a web interface for configuration, manual data entry, and updates. The system can read, write, and encrypt data on RFID tags, making it easier to track filament properties like type, weight, color, and usage.

## Features

*   **RFID Tag Management:**
    *   Reads and writes data to MIFARE Classic RFID tags.
    *   Supports 4-byte and 7-byte UIDs.
    *   Automatic personalization of new tags with a unique derived encryption key.
*   **Data Encryption:**
    *   Utilizes AES encryption to secure filament data stored on RFID tags.
    *   Encryption key is derived from the tag's UID for tag-specific security.
*   **Web Interface:**
    *   Intuitive web UI for system configuration and interaction.
    *   Set up Access Point (AP) mode or connect to an existing WiFi network.
    *   Configure WiFi credentials, system hostname, and printer hostname.
    *   Manually input filament details (brand, type, weight, color) to be written to tags.
    *   View detailed parameters for selected filament types.
*   **Firmware and Database Updates:**
    *   Over-the-air (OTA) firmware updates via the web interface (`.bin` files).
    *   Update the material database by uploading a `material_database.json` file.
    *   Option to fetch and update the material database from a Klipper-enabled printer's web interface.
*   **Local Data Storage:**
    *   Uses LittleFS to store:
        *   Network configuration (`config.ini`).
        *   Default/last used spool data (`spool.ini`).
        *   The material database (`matdb.gz`).
*   **User Feedback:**
    *   Provides audio feedback (tones) for RFID operations (success, failure, new tag).
    *   Status messages on the web interface.
*   **Material Database:**
    *   Includes a pre-compiled material database (`matdb.h`).
    *   Dynamically loads filament brands and types in the web UI based on the database.

## Hardware Requirements

*   **ESP32 Microcontroller:** Any ESP32 development board should work.
*   **MFRC522 RFID Reader/Writer Module:** For interacting with MIFARE RFID tags.
    *   Connections are typically via SPI. Default pins used in `CFS_Spool_ID.ino`:
        *   `SS_PIN` (SDA/CS): 5
        *   `RST_PIN` (Reset): 21
        *   SCK, MISO, MOSI pins should be connected to the ESP32's default SPI pins.
*   **Speaker/Buzzer:** For audio feedback.
    *   Connected to `SPK_PIN`: 27
*   **MIFARE Classic RFID Tags:** For storing filament data.

## Software Setup and Dependencies

This project is written using the Arduino framework for the ESP32.

1.  **Arduino IDE:**
    *   Ensure you have the latest Arduino IDE installed.
    *   Install the ESP32 board support package. You can find instructions on the official Espressif GitHub repository or various online tutorials.
2.  **Libraries:**
    *   The project relies on several standard ESP32 libraries that are usually included with the ESP32 core installation:
        *   `FS.h`: For file system operations (LittleFS).
        *   `SPI.h`: For SPI communication with the MFRC522 module.
        *   `WiFi.h`: For WiFi connectivity (AP and STA modes).
        *   `ESPmDNS.h`: For mDNS (e.g., `k2.local` hostname resolution).
        *   `WebServer.h`: For the embedded web server.
        *   `Update.h`: For OTA firmware updates.
        *   `LittleFS.h`: The specific file system implementation.
    *   The following libraries are included directly in the `src/` directory:
        *   `src/AES/`: AES encryption library.
        *   `src/MFRC522/`: Library for the MFRC522 RFID module.
        *   `src/DB/matdb.h`: Contains the gzipped material database.
        *   `src/WWW/html.h`: Contains the HTML, CSS, and JavaScript for the web interface.
3.  **Compilation and Upload:**
    *   Open the `CFS_Spool_ID.ino` file in the Arduino IDE.
    *   Select your ESP32 board from the Tools > Board menu.
    *   Select the correct COM port.
    *   If using LittleFS for the first time with your ESP32 board, you might need to upload the LittleFS filesystem image. This can typically be done using an Arduino IDE plugin like "ESP32 Sketch Data Upload".
    *   Click the "Upload" button to compile and flash the firmware to the ESP32.

## Configuration

The system uses two main configuration files stored on the ESP32's LittleFS filesystem:

*   **`config.ini`**: This file stores network and device settings. It is created with default values if not present. You can modify these settings via the web interface's configuration page (accessible via the gear icon).
    *   `AP_SSID`: SSID for the Access Point mode (default: "K2_RFID").
    *   `AP_PASS`: Password for the Access Point mode (default: "password").
    *   `WIFI_SSID`: SSID of your local WiFi network for client mode.
    *   `WIFI_PASS`: Password for your local WiFi network.
    *   `WIFI_HOST`: Hostname for the device on your local network (default: "k2.local").
    *   `PRINTER_HOST`: Hostname or IP address of your Klipper-enabled 3D printer (used for fetching the material database).

*   **`spool.ini`**: This file stores the last entered spool data from the web interface. This allows the system to remember the last settings used for writing to a tag.
    *   It stores a single line string representing the `spoolData` that is written to the RFID tags.

Both files are loaded during setup. If `config.ini` is missing, it's created with default values. If `spool.ini` is missing, it's created with a default filament data string.

## Web Interface Usage

The ESP32 hosts a web server that provides an interface for interacting with the RFID system.

*   **Accessing the Interface:**
    *   If the ESP32 is in Access Point (AP) mode, connect your computer or phone to the WiFi network (default SSID: "K2_RFID", password: "password"). Then, open a web browser and navigate to `http://10.1.0.1`.
    *   If the ESP32 is connected to your local WiFi network, you can access it via its assigned IP address or its mDNS hostname (default: `http://k2.local`).

*   **Main Page (`/` or `/index.html`):**
    *   This is the primary page for setting filament data to be written to an RFID tag.
    *   **Filament Brand:** Select the brand of the filament. This list is populated from the material database.
    *   **Filament Type:** Select the specific type of filament for the chosen brand. This list is also dynamic.
    *   **Spool Size:** Choose the weight of the spool (e.g., 1KG, 750G).
    *   **Filament Color:** Select the color of the filament using a color picker.
    *   **Buttons:**
        *   `Reset`: Clears the current selections and local storage for these settings.
        *   `Apply`: Saves the selected filament data. This data will be written to the next RFID tag presented to the reader.
    *   The selections are saved in your browser's local storage, so they persist across sessions.
    *   **Information Icon (Top-Left):** Clicking this icon opens a dialog showing detailed parameters for the currently selected filament type (e.g., temperatures, speeds) based on the material database.
    *   **Settings Icon (Top-Right):** Clicking this icon opens the configuration dialog.

*   **Configuration Dialog (Accessed via Settings Icon):**
    *   **Access Point Settings:**
        *   `AP SSID`: Set the SSID for the device's own Access Point.
        *   `AP PASSWORD`: Set the password for the AP (minimum 8 characters).
    *   **WiFi Connection Settings:**
        *   `WIFI SSID`: Enter the SSID of your existing WiFi network to connect the device as a client.
        *   `WIFI PASSWORD`: Enter the password for your WiFi network.
        *   `WIFI HOSTNAME`: Set the mDNS hostname for the device on your network.
    *   **Printer Settings:**
        *   `PRINTER HOST/IP`: Enter the hostname or IP address of your Klipper-enabled 3D printer. This is used for the "Update From Printer" feature in the Database Updater.
    *   **Save Button:** Saves the configuration to `config.ini` and reboots the ESP32.
    *   **Icons within Configuration Dialog:**
        *   **Database Icon:** Opens the "Database Updater" dialog.
        *   **Firmware Icon:** Opens the "Firmware Updater" dialog.
        *   **Home Icon:** Closes the configuration dialog and returns to the main page.

*   **Firmware Updater Dialog:**
    *   Allows you to upload a new firmware file (`.bin`) to the ESP32.
    *   Select the file, and the system will check for a valid magic byte (0xE9).
    *   Click "Update Firmware" to start the OTA update process. The device will reboot upon completion.

*   **Database Updater Dialog:**
    *   Allows you to update the material database.
    *   **Select file:** Choose a `material_database.json` file from your computer.
    *   **Update Database:** Uploads the selected JSON file. The ESP32 will then gzip it and save it as `matdb.gz`, overwriting the old database. The device reboots after the update.
    *   **Update From Printer:** If a "Printer Host/IP" is configured, this button attempts to download `material_database.json` from `http://[PRINTER_HOST]/downloads/defData/material_database.json` (a common location for Klipper filament databases, e.g., from Mainsail/Fluidd). If successful, it updates the local database on the ESP32.

## RFID Tag Operations

The core of the system involves interacting with MIFARE Classic RFID tags:

1.  **Tag Detection:** The MFRC522 module continuously scans for new RFID tags.
2.  **Authentication:**
    *   When a tag is detected, the system first attempts to authenticate to sector 1 (blocks 4-7) using a default MIFARE key (`FF FF FF FF FF FF`).
    *   If this fails, it attempts to authenticate using a derived key (`ekey`). This `ekey` is generated by AES-encrypting the tag's UID with a pre-set AES key. This means each tag, once personalized, will have its own unique key known to the ESP32.
3.  **Data Writing:**
    *   Once authenticated (either with the default key for a new tag or `ekey` for a personalized tag), the system writes the current `spoolData` (set via the web interface) to blocks 4, 5, and 6 of the tag.
    *   The data written to these blocks is first encrypted using AES with `keytype = 1`.
4.  **Tag Personalization (Key Update):**
    *   If the tag was authenticated using the default key (meaning it's likely a new or unpersonalized tag), the system will then update the access keys for sector 1.
    *   It reads block 7 (the sector trailer), modifies the Key A and Key B fields with the derived `ekey`, and writes the modified sector trailer back to the tag.
    *   This effectively "personalizes" the tag, so subsequent interactions will use the `ekey` for authentication.
5.  **Feedback:**
    *   A short, high-pitched tone indicates a successful read/write operation.
    *   Two short, lower-pitched tones indicate a failure (e.g., authentication failed, write failed).
    *   A different tone is used if the tag type is not supported (not MIFARE Mini, 1K, or 4K).

The system uses `PCD_StopCrypto1()` after operations to ensure the crypto unit on the MFRC522 is reset.

## Code Structure

*   **`CFS_Spool_ID.ino`**: The main Arduino sketch file. It contains:
    *   Global variables and pin definitions.
    *   `setup()`: Initializes hardware (SPI, MFRC522, LittleFS), WiFi (AP or STA), mDNS, and the web server with all its routes.
    *   `loop()`: Handles web server clients and the main RFID tag interaction logic (detection, authentication, reading/writing).
    *   Web server request handler functions (e.g., `handleIndex`, `handleConfig`, `handleSpoolData`, `handleFwUpdate`, `handleDbUpdate`).
    *   RFID helper functions (e.g., `createKey`).
    *   Configuration loading/saving functions (`loadConfig`, `split`, `instr`).
    *   Utility functions (e.g., `GetMaterialLength`, `errorMsg`).

*   **`src/` Directory**: Contains the libraries used by the project.
    *   `src/AES/`: AES encryption library (AES.cpp, AES.h).
    *   `src/DB/matdb.h`: Contains the default gzipped material database embedded as a C array.
    *   `src/MFRC522/`: Library for the MFRC522 RFID module (MFRC522.cpp, MFRC522.h, and supporting files).
    *   `src/WWW/html.h`: Contains the HTML, CSS, and JavaScript for the web interface, embedded as a C string literal.
    *   `src/includes.h`: A helper header to include all necessary library headers in the main `.ino` file.
