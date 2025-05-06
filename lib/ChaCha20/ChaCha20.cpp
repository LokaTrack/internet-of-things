#include <Crypto.h>
#include <ChaCha.h>
#include <string.h>
#include <Arduino.h>
#include <ArduinoJson.h>

// Default encryption settings
#define DEFAULT_CHACHA_ROUNDS 20
#define DEFAULT_KEY_SIZE 32    // 256-bit key
#define DEFAULT_IV_SIZE 8      // 64-bit IV/nonce
#define DEFAULT_COUNTER_SIZE 8 // 64-bit counter

// Global instance of the ChaCha cipher
static ChaCha chaCha;

// Default encryption key - IMPORTANT: Replace with your own secure key in production
static byte defaultKey[DEFAULT_KEY_SIZE] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
    0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8};

// Default initialization vector - IMPORTANT: Use a unique IV for each encryption operation
static byte defaultIV[DEFAULT_IV_SIZE] = {
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C};

// Default counter - IMPORTANT: This should be unique for each message with the same key/IV
static byte defaultCounter[DEFAULT_COUNTER_SIZE] = {
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74};

// Current IV and counter in use - updated for each encryption operation
static byte currentIV[DEFAULT_IV_SIZE];
static byte currentCounter[DEFAULT_COUNTER_SIZE];

// Flag to indicate if we need to generate a new IV/counter for the next encryption
static bool useNewIvCounter = true;

/**
 * Initialize the ChaCha cipher with default parameters
 */
void initChaCha()
{
    chaCha.setNumRounds(DEFAULT_CHACHA_ROUNDS);
    chaCha.setKey(defaultKey, DEFAULT_KEY_SIZE);

    // Initialize the current IV and counter with the default values
    memcpy(currentIV, defaultIV, DEFAULT_IV_SIZE);
    memcpy(currentCounter, defaultCounter, DEFAULT_COUNTER_SIZE);

    // Set these in the cipher
    chaCha.setIV(currentIV, DEFAULT_IV_SIZE);
    chaCha.setCounter(currentCounter, DEFAULT_COUNTER_SIZE);
}

/**
 * Initialize the ChaCha cipher with custom parameters
 *
 * @param key Pointer to the key bytes
 * @param keySize Size of the key in bytes (16 or 32)
 * @param iv Pointer to the initialization vector bytes
 * @param counter Pointer to the counter bytes
 * @param rounds Number of ChaCha rounds (8, 12, or 20)
 * @return true if initialization was successful, false otherwise
 */
bool initChaChaCustom(const byte *key, size_t keySize, const byte *iv, const byte *counter, uint8_t rounds)
{
    if (keySize != 16 && keySize != 32)
    {
        return false;
    }

    if (rounds != 8 && rounds != 12 && rounds != 20)
    {
        return false;
    }

    chaCha.setNumRounds(rounds);
    if (!chaCha.setKey(key, keySize))
    {
        return false;
    }

    // Update the current IV and counter
    if (iv != nullptr)
    {
        memcpy(currentIV, iv, DEFAULT_IV_SIZE);
    }

    if (counter != nullptr)
    {
        memcpy(currentCounter, counter, DEFAULT_COUNTER_SIZE);
    }

    // Set these in the cipher
    if (!chaCha.setIV(currentIV, DEFAULT_IV_SIZE))
    {
        return false;
    }

    if (!chaCha.setCounter(currentCounter, DEFAULT_COUNTER_SIZE))
    {
        return false;
    }

    return true;
}

/**
 * Generate a random initialization vector
 *
 * @param iv Buffer to store the generated IV (must be at least DEFAULT_IV_SIZE bytes)
 */
void generateRandomIV(byte *iv)
{
    for (size_t i = 0; i < DEFAULT_IV_SIZE; i++)
    {
        iv[i] = random(256);
    }
}

/**
 * Reset the counter to initial value
 * Used when generating a new IV
 */
void resetCounter()
{
    memcpy(currentCounter, defaultCounter, DEFAULT_COUNTER_SIZE);
}

/**
 * Increment the counter value
 *
 * @param counter Buffer containing the counter value to increment
 */
void incrementCounter(byte *counter)
{
    for (int i = 0; i < DEFAULT_COUNTER_SIZE; i++)
    {
        counter[i]++;
        if (counter[i] != 0)
        {
            break;
        }
    }
}

/**
 * Generate new IV and reset counter for a new encryption session
 */
void newEncryptionSession()
{
    generateRandomIV(currentIV);
    resetCounter();

    // Update the cipher with new values
    chaCha.setIV(currentIV, DEFAULT_IV_SIZE);
    chaCha.setCounter(currentCounter, DEFAULT_COUNTER_SIZE);

    useNewIvCounter = false;
}

/**
 * Prepare for next message in the current encryption session
 * by incrementing the counter
 */
void prepareNextMessage()
{
    incrementCounter(currentCounter);
    chaCha.setCounter(currentCounter, DEFAULT_COUNTER_SIZE);
}

/**
 * Encrypt data using ChaCha
 *
 * @param output Buffer to store the encrypted data
 * @param input Data to encrypt
 * @param len Length of the data to encrypt
 * @return true if encryption was successful, false otherwise
 */
bool encryptData(byte *output, const byte *input, size_t len)
{
    if (useNewIvCounter)
    {
        newEncryptionSession();
    }
    else
    {
        prepareNextMessage();
    }

    chaCha.encrypt(output, input, len);
    return true;
}

/**
 * Decrypt data using ChaCha with provided IV and counter
 *
 * @param output Buffer to store the decrypted data
 * @param input Data to decrypt
 * @param len Length of the data to decrypt
 * @param iv The initialization vector to use
 * @param counter The counter value to use
 * @return true if decryption was successful, false otherwise
 */
bool decryptDataWithIV(byte *output, const byte *input, size_t len, const byte *iv, const byte *counter)
{
    // Save current state
    byte savedIV[DEFAULT_IV_SIZE];
    byte savedCounter[DEFAULT_COUNTER_SIZE];
    memcpy(savedIV, currentIV, DEFAULT_IV_SIZE);
    memcpy(savedCounter, currentCounter, DEFAULT_COUNTER_SIZE);

    // Set new IV and counter for decryption
    chaCha.setIV(iv, DEFAULT_IV_SIZE);
    chaCha.setCounter(counter, DEFAULT_COUNTER_SIZE);

    // Decrypt
    chaCha.decrypt(output, input, len);

    // Restore previous state
    chaCha.setIV(savedIV, DEFAULT_IV_SIZE);
    chaCha.setCounter(savedCounter, DEFAULT_COUNTER_SIZE);

    return true;
}

/**
 * Decrypt data using ChaCha with the current IV and counter
 *
 * @param output Buffer to store the decrypted data
 * @param input Data to decrypt
 * @param len Length of the data to decrypt
 * @return true if decryption was successful, false otherwise
 */
bool decryptData(byte *output, const byte *input, size_t len)
{
    chaCha.decrypt(output, input, len);
    return true;
}

/**
 * Get the IV format header size in bytes
 * (IV + Counter)
 *
 * @return Size of the header in bytes
 */
size_t getIvHeaderSize()
{
    return DEFAULT_IV_SIZE + DEFAULT_COUNTER_SIZE;
}

/**
 * Create IV header for the encrypted data
 *
 * @param header Buffer to store the header (must be at least getIvHeaderSize() bytes)
 * @return Size of the header in bytes
 */
size_t createIvHeader(byte *header)
{
    // Copy the current IV and counter to the header
    memcpy(header, currentIV, DEFAULT_IV_SIZE);
    memcpy(header + DEFAULT_IV_SIZE, currentCounter, DEFAULT_COUNTER_SIZE);
    return getIvHeaderSize();
}

/**
 * Extract IV and counter from header
 *
 * @param header The header bytes containing the IV and counter
 * @param iv Buffer to store the IV (must be at least DEFAULT_IV_SIZE bytes)
 * @param counter Buffer to store the counter (must be at least DEFAULT_COUNTER_SIZE bytes)
 */
void extractIvHeader(const byte *header, byte *iv, byte *counter)
{
    memcpy(iv, header, DEFAULT_IV_SIZE);
    memcpy(counter, header + DEFAULT_IV_SIZE, DEFAULT_COUNTER_SIZE);
}

/**
 * Encrypt a string using ChaCha with automatic IV handling
 * The encrypted result includes the IV and counter in the format:
 * [IV(8 bytes)][Counter(8 bytes)][Encrypted Data(variable)]
 *
 * @param input String to encrypt
 * @param useNewSession Whether to generate a new IV for this encryption (true) or increment the counter (false)
 * @return String with the encrypted data in hexadecimal format, including the IV and counter
 */
String encryptString(const String &input, bool useNewSession)
{
    useNewIvCounter = useNewSession;

    size_t inputLen = input.length();
    size_t headerSize = getIvHeaderSize();
    byte *inputBytes = new byte[inputLen];
    byte *headerBytes = new byte[headerSize];
    byte *outputBytes = new byte[inputLen];

    // Copy input string to byte array
    memcpy(inputBytes, input.c_str(), inputLen);

    // Encrypt the data
    encryptData(outputBytes, inputBytes, inputLen);

    // Create the IV header
    createIvHeader(headerBytes);

    // Convert header and encrypted bytes to hexadecimal string
    String encryptedHex = "";

    // First add the header (IV + counter)
    for (size_t i = 0; i < headerSize; i++)
    {
        char hex[3];
        sprintf(hex, "%02X", headerBytes[i]);
        encryptedHex += hex;
    }

    // Then add the encrypted data
    for (size_t i = 0; i < inputLen; i++)
    {
        char hex[3];
        sprintf(hex, "%02X", outputBytes[i]);
        encryptedHex += hex;
    }

    // Clean up
    delete[] inputBytes;
    delete[] headerBytes;
    delete[] outputBytes;

    return encryptedHex;
}

/**
 * Encrypt a string using ChaCha with automatic IV handling
 * Uses the default new session behavior (generating new IV)
 *
 * @param input String to encrypt
 * @return String with the encrypted data in hexadecimal format, including the IV and counter
 */
String encryptString(const String &input)
{
    return encryptString(input, true);
}

/**
 * Decrypt a hexadecimal string that was encrypted using ChaCha
 * The input string should include the IV and counter in the format:
 * [IV(8 bytes)][Counter(8 bytes)][Encrypted Data(variable)]
 *
 * @param hexInput Encrypted data in hexadecimal format, including IV and counter
 * @return Decrypted string
 */
String decryptString(const String &hexInput)
{
    size_t hexLen = hexInput.length();
    size_t headerSize = getIvHeaderSize() * 2; // *2 because each byte is 2 hex chars
    size_t dataHexLen = hexLen - headerSize;
    size_t dataByteLen = dataHexLen / 2;

    // Check if the input has enough data for a header
    if (hexLen < headerSize)
    {
        Serial.println("Error: Input too short to contain IV header");
        return "";
    }

    // Allocate memory for IV, counter, and data
    byte *iv = new byte[DEFAULT_IV_SIZE];
    byte *counter = new byte[DEFAULT_COUNTER_SIZE];
    byte *encryptedBytes = new byte[dataByteLen];
    byte *decryptedBytes = new byte[dataByteLen + 1]; // +1 for null terminator

    // Extract the IV and counter from the hex string
    for (size_t i = 0; i < DEFAULT_IV_SIZE; i++)
    {
        String byteHex = hexInput.substring(i * 2, i * 2 + 2);
        iv[i] = (byte)strtol(byteHex.c_str(), NULL, 16);
    }

    for (size_t i = 0; i < DEFAULT_COUNTER_SIZE; i++)
    {
        String byteHex = hexInput.substring((DEFAULT_IV_SIZE + i) * 2, (DEFAULT_IV_SIZE + i) * 2 + 2);
        counter[i] = (byte)strtol(byteHex.c_str(), NULL, 16);
    }

    // Convert hex data to bytes
    for (size_t i = 0; i < dataByteLen; i++)
    {
        String byteHex = hexInput.substring(headerSize + i * 2, headerSize + i * 2 + 2);
        encryptedBytes[i] = (byte)strtol(byteHex.c_str(), NULL, 16);
    }

    // Decrypt the data using the extracted IV and counter
    decryptDataWithIV(decryptedBytes, encryptedBytes, dataByteLen, iv, counter);

    // Add null terminator
    decryptedBytes[dataByteLen] = 0;

    // Convert decrypted bytes to string
    String decryptedStr = String((char *)decryptedBytes);

    // Clean up
    delete[] iv;
    delete[] counter;
    delete[] encryptedBytes;
    delete[] decryptedBytes;

    return decryptedStr;
}

/**
 * Encrypt a JSON document
 *
 * @param doc JsonDocument to encrypt
 * @param useNewSession Whether to generate a new IV for this encryption (true) or increment the counter (false)
 * @return String with the encrypted JSON in hexadecimal format
 */
String encryptJson(const JsonDocument &doc, bool useNewSession)
{
    String jsonStr;
    serializeJson(doc, jsonStr);
    return encryptString(jsonStr, useNewSession);
}

/**
 * Encrypt a JSON document with default new session behavior
 *
 * @param doc JsonDocument to encrypt
 * @return String with the encrypted JSON in hexadecimal format
 */
String encryptJson(const JsonDocument &doc)
{
    return encryptJson(doc, true);
}

/**
 * Decrypt a JSON document that was encrypted using ChaCha
 *
 * @param hexInput Encrypted JSON in hexadecimal format
 * @param doc JsonDocument to store the decrypted JSON
 * @return true if decryption and deserialization was successful, false otherwise
 */
bool decryptJson(const String &hexInput, JsonDocument &doc)
{
    String jsonStr = decryptString(hexInput);
    DeserializationError error = deserializeJson(doc, jsonStr);
    return (error == DeserializationError::Ok);
}

/**
 * Get the current IV being used
 *
 * @param iv Buffer to store the IV (must be at least DEFAULT_IV_SIZE bytes)
 */
void getCurrentIV(byte *iv)
{
    memcpy(iv, currentIV, DEFAULT_IV_SIZE);
}

/**
 * Get the current counter being used
 *
 * @param counter Buffer to store the counter (must be at least DEFAULT_COUNTER_SIZE bytes)
 */
void getCurrentCounter(byte *counter)
{
    memcpy(counter, currentCounter, DEFAULT_COUNTER_SIZE);
}