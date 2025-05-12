#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * Initialize the ChaCha cipher with default parameters
 */
void initChaCha();

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
bool initChaChaCustom(const byte *key, size_t keySize, const byte *iv, const byte *counter, uint8_t rounds);

/**
 * Generate a random initialization vector
 *
 * @param iv Buffer to store the generated IV (must be at least 8 bytes)
 */
void generateRandomIV(byte *iv);

/**
 * Reset the counter to initial value
 * Used when generating a new IV
 */
void resetCounter();

/**
 * Increment the counter value
 *
 * @param counter Buffer containing the counter value to increment
 */
void incrementCounter(byte *counter);

/**
 * Generate new IV and reset counter for a new encryption session
 */
void newEncryptionSession();

/**
 * Prepare for next message in the current encryption session
 * by incrementing the counter
 */
void prepareNextMessage();

/**
 * Encrypt data using ChaCha
 *
 * @param output Buffer to store the encrypted data
 * @param input Data to encrypt
 * @param len Length of the data to encrypt
 * @return true if encryption was successful, false otherwise
 */
bool encryptData(byte *output, const byte *input, size_t len);

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
bool decryptDataWithIV(byte *output, const byte *input, size_t len, const byte *iv, const byte *counter);

/**
 * Decrypt data using ChaCha with the current IV and counter
 *
 * @param output Buffer to store the decrypted data
 * @param input Data to decrypt
 * @param len Length of the data to decrypt
 * @return true if decryption was successful, false otherwise
 */
bool decryptData(byte *output, const byte *input, size_t len);

/**
 * Get the IV format header size in bytes
 * (IV + Counter)
 *
 * @return Size of the header in bytes
 */
size_t getIvHeaderSize();

/**
 * Create IV header for the encrypted data
 *
 * @param header Buffer to store the header (must be at least getIvHeaderSize() bytes)
 * @return Size of the header in bytes
 */
size_t createIvHeader(byte *header);

/**
 * Extract IV and counter from header
 *
 * @param header The header bytes containing the IV and counter
 * @param iv Buffer to store the IV (must be at least DEFAULT_IV_SIZE bytes)
 * @param counter Buffer to store the counter (must be at least DEFAULT_COUNTER_SIZE bytes)
 */
void extractIvHeader(const byte *header, byte *iv, byte *counter);

/**
 * Encrypt a string using ChaCha with automatic IV handling
 * The encrypted result includes the IV and counter in the format:
 * [IV(8 bytes)][Counter(8 bytes)][Encrypted Data(variable)]
 *
 * @param input String to encrypt
 * @param useNewSession Whether to generate a new IV for this encryption (true) or increment the counter (false)
 * @return String with the encrypted data in hexadecimal format, including the IV and counter
 */
String encryptString(const String &input, bool useNewSession);

/**
 * Encrypt a string using ChaCha with automatic IV handling
 * Uses the default new session behavior (generating new IV)
 *
 * @param input String to encrypt
 * @return String with the encrypted data in hexadecimal format, including the IV and counter
 */
String encryptString(const String &input);

/**
 * Decrypt a hexadecimal string that was encrypted using ChaCha
 * The input string should include the IV and counter in the format:
 * [IV(8 bytes)][Counter(8 bytes)][Encrypted Data(variable)]
 *
 * @param hexInput Encrypted data in hexadecimal format, including IV and counter
 * @return Decrypted string
 */
String decryptString(const String &hexInput);

/**
 * Encrypt a JSON document
 *
 * @param doc JsonDocument to encrypt
 * @param useNewSession Whether to generate a new IV for this encryption (true) or increment the counter (false)
 * @return String with the encrypted JSON in hexadecimal format
 */
String encryptJson(const JsonDocument &doc, bool useNewSession);

/**
 * Encrypt a JSON document with default new session behavior
 *
 * @param doc JsonDocument to encrypt
 * @return String with the encrypted JSON in hexadecimal format
 */
String encryptJson(const JsonDocument &doc);

/**
 * Decrypt a JSON document that was encrypted using ChaCha
 *
 * @param hexInput Encrypted JSON in hexadecimal format
 * @param doc JsonDocument to store the decrypted JSON
 * @return true if decryption and deserialization was successful, false otherwise
 */
bool decryptJson(const String &hexInput, JsonDocument &doc);

/**
 * Get the current IV being used
 *
 * @param iv Buffer to store the IV (must be at least 8 bytes)
 */
void getCurrentIV(byte *iv);

/**
 * Get the current counter being used
 *
 * @param counter Buffer to store the counter (must be at least 8 bytes)
 */
void getCurrentCounter(byte *counter);

#endif // ENCRYPT_H