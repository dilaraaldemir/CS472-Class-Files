/**
 * =============================================================================
 * STUDENT ASSIGNMENT: CRYPTO-SERVER.C
 * =============================================================================
 * 
 * ASSIGNMENT OBJECTIVE:
 * Implement a TCP server that accepts client connections and processes
 * encrypted/plaintext messages. Your focus is on socket programming, connection
 * handling, and the server-side protocol implementation.
 * 
 * =============================================================================
 * WHAT YOU NEED TO IMPLEMENT:
 * =============================================================================
 * 
 * 1. SERVER SOCKET SETUP (start_server function):
 *    - Create a TCP socket using socket()
 *    - Set SO_REUSEADDR socket option (helpful during development)
 *    - Configure server address structure (struct sockaddr_in)
 *    - Bind the socket to the address using bind()
 *    - Start listening with listen()
 *    - Call your server loop function
 *    - Close socket on shutdown
 * 
 * 2. SERVER MAIN LOOP:
 *    - Create a function that handles multiple clients sequentially
 *    - Loop to:
 *      a) Accept incoming connections using accept()
 *      b) Get client's IP address for logging (inet_ntop)
 *      c) Call your client service function
 *      d) Close the client socket when done
 *      e) Return to accept next client (or exit if shutdown requested)
 * 
 * 3. CLIENT SERVICE LOOP:
 *    - Create a function that handles communication with ONE client
 *    - Allocate buffers for sending and receiving
 *    - Maintain session keys (client_key and server_key)
 *    - Loop to:
 *      a) Receive a PDU from the client using recv()
 *      b) Handle recv() return values (0 = closed, <0 = error)
 *      c) Parse the received PDU
 *      d) Check for special commands (exit, server shutdown)
 *      e) Build response PDU based on message type
 *      f) Send response using send()
 *      g) Return appropriate status code when client exits
 *    - Free buffers before returning
 * 
 * 4. RESPONSE BUILDING:
 *    - Consider creating a helper function to build response PDUs
 *    - Handle different message types:
 *      * MSG_KEY_EXCHANGE: Call gen_key_pair(), send client_key to client
 *      * MSG_DATA: Echo back with "echo " prefix
 *      * MSG_ENCRYPTED_DATA: Decrypt, add "echo " prefix, re-encrypt
 *      * MSG_CMD_CLIENT_STOP: No response needed (client will exit)
 *      * MSG_CMD_SERVER_STOP: No response needed (server will exit)
 *    - Set proper direction (DIR_RESPONSE)
 *    - Return total PDU size
 * 
 * =============================================================================
 * ONE APPROACH TO SOLVE THIS PROBLEM:
 * =============================================================================
 * 
 * FUNCTION STRUCTURE:
 * 
 * void start_server(const char* addr, int port) {
 *     // 1. Create TCP socket
 *     // 2. Set SO_REUSEADDR option (for development)
 *     // 3. Configure server address (sockaddr_in)
 *     //    - Handle "0.0.0.0" specially (use INADDR_ANY)
 *     // 4. Bind socket to address
 *     // 5. Start listening (use BACKLOG constant)
 *     // 6. Call your server loop function
 *     // 7. Close socket
 * }
 * 
 * int server_loop(int server_socket, const char* addr, int port) {
 *     // 1. Print "Server listening..." message
 *     // 2. Infinite loop:
 *     //    a) Accept connection (creates new client socket)
 *     //    b) Get client IP using inet_ntop()
 *     //    c) Print "Client connected..." message
 *     //    d) Call service_client_loop(client_socket)
 *     //    e) Check return code:
 *     //       - RC_CLIENT_EXITED: close socket, accept next client
 *     //       - RC_CLIENT_REQ_SERVER_EXIT: close sockets, return
 *     //       - Error: close socket, continue
 *     //    f) Close client socket
 *     // 3. Return when server shutdown requested
 * }
 * 
 * int service_client_loop(int client_socket) {
 *     // 1. Allocate send/receive buffers
 *     // 2. Initialize keys to NULL_CRYPTO_KEY
 *     // 3. Loop:
 *     //    a) Receive PDU from client
 *     //    b) Check recv() return:
 *     //       - 0: client closed, return RC_CLIENT_EXITED
 *     //       - <0: error, return RC_CLIENT_EXITED
 *     //    c) Cast buffer to crypto_msg_t*
 *     //    d) Check for MSG_CMD_SERVER_STOP -> return RC_CLIENT_REQ_SERVER_EXIT
 *     //    e) Build response PDU (use helper function)
 *     //    f) Send response
 *     //    g) Loop back
 *     // 4. Free buffers before returning
 * }
 * 
 * int build_response(crypto_msg_t *request, crypto_msg_t *response, 
 *                    crypto_key_t *client_key, crypto_key_t *server_key) {
 *     // 1. Set response->header.direction = DIR_RESPONSE
 *     // 2. Set response->header.msg_type = request->header.msg_type
 *     // 3. Switch on request type:
 *     //    MSG_KEY_EXCHANGE:
 *     //      - Call gen_key_pair(server_key, client_key)
 *     //      - Copy client_key to response->payload
 *     //      - Set payload_len = sizeof(crypto_key_t)
 *     //    MSG_DATA:
 *     //      - Format: "echo <original message>"
 *     //      - Copy to response->payload
 *     //      - Set payload_len
 *     //    MSG_ENCRYPTED_DATA:
 *     //      - Decrypt request->payload using decrypt_string()
 *     //      - Format: "echo <decrypted message>"
 *     //      - Encrypt result using encrypt_string()
 *     //      - Copy encrypted data to response->payload
 *     //      - Set payload_len
 *     //    MSG_CMD_*:
 *     //      - Set payload_len = 0
 *     // 4. Return sizeof(crypto_pdu_t) + payload_len
 * }
 * 
 * =============================================================================
 * IMPORTANT PROTOCOL DETAILS:
 * =============================================================================
 * 
 * SERVER RESPONSIBILITIES:
 * 1. Generate encryption keys when client requests (MSG_KEY_EXCHANGE)
 * 2. Send the CLIENT'S key to the client (not the server's key!)
 * 3. Keep both keys: server_key (for decrypting client messages)
 *                    client_key (to send to client)
 * 4. Echo messages back with "echo " prefix
 * 5. Handle encrypted data: decrypt -> process -> encrypt -> send
 * 
 * KEY GENERATION:
 *   crypto_key_t server_key, client_key;
 *   gen_key_pair(&server_key, &client_key);
 *   // Send client_key to the client in MSG_KEY_EXCHANGE response
 *   memcpy(response->payload, &client_key, sizeof(crypto_key_t));
 * 
 * DECRYPTING CLIENT DATA:
 *   // Client encrypted with their key, we decrypt with server_key
 *   uint8_t decrypted[MAX_SIZE];
 *   decrypt_string(server_key, decrypted, request->payload, request->header.payload_len);
 *   decrypted[request->header.payload_len] = '\0'; // Null-terminate
 * 
 * ENCRYPTING RESPONSE:
 *   // We encrypt with server_key for client to decrypt with their key
 *   uint8_t encrypted[MAX_SIZE];
 *   int encrypted_len = encrypt_string(server_key, encrypted, plaintext, plaintext_len);
 *   memcpy(response->payload, encrypted, encrypted_len);
 *   response->header.payload_len = encrypted_len;
 * 
 * RETURN CODES:
 *   RC_CLIENT_EXITED          - Client disconnected normally
 *   RC_CLIENT_REQ_SERVER_EXIT - Client requested server shutdown
 *   RC_OK                     - Success
 *   Negative values           - Errors
 * 
 * =============================================================================
 * SOCKET PROGRAMMING REMINDERS:
 * =============================================================================
 * 
 * CREATING AND BINDING:
 *   int sockfd = socket(AF_INET, SOCK_STREAM, 0);
 *   
 *   struct sockaddr_in addr;
 *   memset(&addr, 0, sizeof(addr));
 *   addr.sin_family = AF_INET;
 *   addr.sin_port = htons(port);
 *   addr.sin_addr.s_addr = INADDR_ANY;  // or use inet_pton()
 *   
 *   bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
 *   listen(sockfd, BACKLOG);
 * 
 * ACCEPTING CONNECTIONS:
 *   struct sockaddr_in client_addr;
 *   socklen_t addr_len = sizeof(client_addr);
 *   int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
 * 
 * GETTING CLIENT IP:
 *   char client_ip[INET_ADDRSTRLEN];
 *   inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
 * 
 * =============================================================================
 * DEBUGGING TIPS:
 * =============================================================================
 * 
 * 1. Use print_msg_info() to display received and sent PDUs
 * 2. Print client IP when connections are accepted
 * 3. Check all socket operation return values
 * 4. Test with plaintext (MSG_DATA) before trying encryption
 * 5. Verify keys are generated correctly (print key values)
 * 6. Use telnet or netcat to test basic connectivity first
 * 7. Handle partial recv() - though for this assignment, assume full PDU arrives
 * 
 * =============================================================================
 * TESTING RECOMMENDATIONS:
 * =============================================================================
 * 
 * 1. Start simple: Accept connection and echo plain text
 * 2. Test key exchange: Client sends '#', server generates and returns key
 * 3. Test encryption: Client sends '!message', server decrypts, echoes, encrypts
 * 4. Test multiple clients: Connect, disconnect, connect again
 * 5. Test shutdown: Client sends '=', server exits gracefully
 * 6. Test error cases: Client disconnects unexpectedly
 * 
 * Good luck! Server programming requires careful state management!
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>
#include "crypto-server.h"
#include "crypto-lib.h"
#include "protocol.h"
int server_loop(int server_socket, const char* addr, int port);
int service_client_loop(int client_socket);
int build_response(crypto_msg_t *request, crypto_msg_t *response, crypto_key_t *client_key, crypto_key_t *server_key);
/* =============================================================================
 * STUDENT TODO: IMPLEMENT THIS FUNCTION
 * =============================================================================
 * This is the main server initialization function. You need to:
 * 1. Create a TCP socket
 * 2. Set socket options (SO_REUSEADDR)
 * 3. Bind to the specified address and port
 * 4. Start listening for connections
 * 5. Call your server loop function
 * 6. Clean up when done
 * 
 * Parameters:
 *   addr - Server bind address (e.g., "0.0.0.0" for all interfaces)
 *   port - Server port number (e.g., 1234)
 * 
 * NOTE: If addr is "0.0.0.0", use INADDR_ANY instead of inet_pton()
 */
void start_server(const char* addr, int port) {
    int server_socket;
    int opt = 1;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (strcmp(addr, "0.0.0.0") == 0) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, addr, &server_addr.sin_addr) <= 0) {
            perror("inet_pton");
            close(server_socket);
            return;
        }
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return;
    }

    if (listen(server_socket, BACKLOG) < 0) {
        perror("listen");
        close(server_socket);
        return;
    }

    printf("Server listening on %s:%d\n", addr, port);
    server_loop(server_socket, addr, port);
    close(server_socket);
}
int server_loop(int server_socket, const char* addr, int port)
{
    (void)addr;
    (void)port;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        char client_ip[INET_ADDRSTRLEN];

        int client_socket = accept(
            server_socket,
            (struct sockaddr *)&client_addr,
            &addr_len
        );

        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        inet_ntop(AF_INET, &client_addr.sin_addr,
                  client_ip, INET_ADDRSTRLEN);

        printf("Client connected from %s\n", client_ip);

        int result = service_client_loop(client_socket);

        close(client_socket);

        if (result == RC_CLIENT_REQ_SERVER_EXIT) {
            printf("Server shutdown requested.\n");
            return 0;
        }

        if (result == RC_CLIENT_EXITED) {
            printf("Client disconnected.\n");
        }
    }

    return 0;
}
int build_response(crypto_msg_t *request, crypto_msg_t *response,
                   crypto_key_t *client_key, crypto_key_t *server_key)
{
    response->header.msg_type = request->header.msg_type;
    response->header.direction = DIR_RESPONSE;
    response->header.payload_len = 0;

    switch (request->header.msg_type) {

        case MSG_KEY_EXCHANGE:
            if (gen_key_pair(server_key, client_key) != RC_OK) {
                return RC_ENCRYPTION_ERR;
            }

            memcpy(response->payload, client_key, sizeof(crypto_key_t));
            response->header.payload_len = sizeof(crypto_key_t);
            break;

        case MSG_DATA: {
            const char prefix[] = "echo ";
            size_t prefix_len = strlen(prefix);
            size_t data_len = request->header.payload_len;

            if (prefix_len + data_len > MAX_MSG_DATA_SIZE) {
                return RC_MEMORY_ERR;
            }

            memcpy(response->payload, prefix, prefix_len);
            memcpy(response->payload + prefix_len,
                   request->payload, data_len);

            response->header.payload_len = prefix_len + data_len;
            break;
        }

        case MSG_ENCRYPTED_DATA: {
            const char prefix[] = "echo ";
            size_t prefix_len = strlen(prefix);
            size_t data_len = request->header.payload_len;
            uint8_t plaintext[MAX_MSG_DATA_SIZE];
            uint8_t echoed[MAX_MSG_DATA_SIZE];

            if (*server_key == NULL_CRYPTO_KEY) {
                return RC_DECRYPTION_ERR;
            }

            int decrypted_len = decrypt_string(
                *server_key, plaintext, request->payload, data_len
            );

            if (decrypted_len < 0 ||
                prefix_len + decrypted_len > MAX_MSG_DATA_SIZE) {
                return RC_DECRYPTION_ERR;
            }

            memcpy(echoed, prefix, prefix_len);
            memcpy(echoed + prefix_len, plaintext, decrypted_len);

            int encrypted_len = encrypt_string(
                *server_key, response->payload,
                echoed, prefix_len + decrypted_len
            );

            if (encrypted_len < 0 ||
               (size_t)encrypted_len > MAX_MSG_DATA_SIZE) {
                return RC_ENCRYPTION_ERR;
            }

            response->header.payload_len = encrypted_len;
            break;
        }

        case MSG_CMD_CLIENT_STOP:
        case MSG_CMD_SERVER_STOP:
            break;

        default:
            response->header.msg_type = MSG_ERROR;
            break;
    }

    return sizeof(crypto_pdu_t) + response->header.payload_len;
}
int service_client_loop(int client_socket)
{
    crypto_msg_t *request = malloc(BUFFER_SIZE);
    crypto_msg_t *response = malloc(BUFFER_SIZE);

    crypto_key_t client_key = NULL_CRYPTO_KEY;
    crypto_key_t server_key = NULL_CRYPTO_KEY;

    if (request == NULL || response == NULL) {
        perror("malloc");
        free(request);
        free(response);
        return RC_MEMORY_ERR;
    }

    while (1) {
        size_t received = 0;
        size_t header_size = sizeof(crypto_pdu_t);

        while (received < header_size) {
            ssize_t bytes = recv(
                client_socket,
                (char *)&request->header + received,
                header_size - received,
                0
            );

            if (bytes <= 0) {
                free(request);
                free(response);
                return RC_CLIENT_EXITED;
            }

            received += bytes;
        }
	request->header.payload_len = ntohs(request->header.payload_len);
        if (request->header.payload_len > MAX_MSG_DATA_SIZE) {
            free(request);
            free(response);
            return RC_MEMORY_ERR;
        }

        received = 0;

        while (received < request->header.payload_len) {
            ssize_t bytes = recv(
                client_socket,
                request->payload + received,
                request->header.payload_len - received,
                0
            );

            if (bytes <= 0) {
                free(request);
                free(response);
                return RC_CLIENT_EXITED;
            }

            received += bytes;
        }

        if (request->header.msg_type == MSG_CMD_CLIENT_STOP) {
            free(request);
            free(response);
            return RC_CLIENT_EXITED;
        }

        if (request->header.msg_type == MSG_CMD_SERVER_STOP) {
            free(request);
            free(response);
            return RC_CLIENT_REQ_SERVER_EXIT;
        }

        int response_size = build_response(
            request, response, &client_key, &server_key
        );

        if (response_size < 0) {
            free(request);
            free(response);
            return response_size;
        }
	response->header.payload_len = htons(response->header.payload_len);
        size_t sent = 0;

        while (sent < (size_t)response_size) {
            ssize_t bytes = send(
                client_socket,
                (char *)response + sent,
                response_size - sent,
                0
            );

            if (bytes <= 0) {
                free(request);
                free(response);
                return RC_CLIENT_EXITED;
            }

            sent += bytes;
        }
    }

    free(request);
    free(response);
    return RC_CLIENT_EXITED;
}
