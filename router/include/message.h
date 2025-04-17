#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <string>
#define ID_MESSAGE_SIZE 3
#define DATA_MESSAGE_SIZE 32

/**
 * @class Message
 * @brief Provides static utility functions to extract source and destination IDs from messages.
 *
 * The Message class offers static methods to parse message strings and retrieve source and destination
 * identifiers based on fixed positions within the message content. It assumes that the message format
 * contains numeric IDs at specific offsets.
 */
class Message
{
public:
    /**
     * @brief Extracts the source ID from a message string.
     * @param msg Pointer to the message string.
     * @param msg_len Length of the message string.
     * @return The source ID as an integer if successful; -1 otherwise.
     */
    static int extract_src_id(const char* msg, int msg_len) {
        if (msg_len < ID_MESSAGE_SIZE) {
            return -1; // Message too short to contain source ID
        }
        try {
            // Convert the initial segment of the message to an integer
            int id = std::stoi(msg);
            return id;
        }
        catch (const std::invalid_argument&) {
            // Error: The message does not start with a valid number
        }
        return -1; // Failed to parse source ID
    }

    /**
     * @brief Extracts the destination ID from a message string.
     * @param msg Pointer to the message string.
     * @param msg_len Length of the message string.
     * @return The destination ID as an integer if successful; -1 otherwise.
     */
    static int extract_dst_id(const char* msg, int msg_len) {
        if (msg_len < DATA_MESSAGE_SIZE) {
            return -1; // Message too short to contain destination ID
        }
        try {
            // Convert substring starting at offset 29 to an integer
            int id = std::stoi(msg + 29);
            return id;
        }
        catch (const std::invalid_argument&) {
            // Error: The substring does not contain a valid number
        }
        return -1; // Failed to parse destination ID
    }
};

#endif