//  - Write a C program for the server to listen on a specified port for incoming TCP connections.
//  - The server should be able to handle multiple client connections and relay messages received from one client to all other connected clients.

#include "lab.h"
#include <ctype.h>
#include <stdlib.h>

#define PORT 8080

int main()
{
    // Protocol to prepare to Launch
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, PORT, &hints, &bind_address);
    // End of the protocol


    // Creating a socket to accept connections
    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                           bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    
    //Bind socket to the specified port
    printf("Binding socket to local address...\n");
    if (bind(socket_listen,
             bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address); // free the linked list


    // Start listening for connections on the socket
    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0)
    {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    //List to store all active client sockets
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    // Crea

    printf("Waiting for connections...\n");

    while (1)
    {
        fd_set reads;
        reads = master;
        if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
        {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        SOCKET i;
        for (i = 1; i <= max_socket; ++i)
        {
            // If the server socket is ready for reading, then we have a new connection
            if (FD_ISSET(i, &reads))
            {

                if (i == socket_listen)// Validation of the socket listen
                {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(socket_listen,
                                                  (struct sockaddr *)&client_address,
                                                  &client_len);
                    if (!ISVALIDSOCKET(socket_client))
                    {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                GETSOCKETERRNO());
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)// Si el nuevo descriptor de archivo es mayor que max socket, se actualiza max_socket
                        max_socket = socket_client;
                    
                    // Se obtiene la dirección del cliente
                    char address_buffer[100];
                    getnameinfo((struct sockaddr *)&client_address,
                                client_len,
                                address_buffer, sizeof(address_buffer), 0, 0,
                                NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);
                }
                else
                {
                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);// Se leen los datos
                    if (bytes_received < 1)// Se comprueba si la recepción fue exitosa
                    {
                        FD_CLR(i, &master);// Si no se reciben datos, se elimina el descriptor 
                        CLOSESOCKET(i);// Y se cierra el socket
                        continue;
                    }

                    SOCKET j; // Por el contrario, si se recibieron datos, se envian a todos los otros clientes conectados. 
                    for (j = 1; j <= max_socket; ++j)
                    {
                        if (FD_ISSET(j, &master))
                        {
                            if (j == socket_listen || j == i)
                                continue;
                            else
                                send(j, read, bytes_received, 0);
                        }
                    }
                }
            } // if FD_ISSET
        }     // for i to max_socket
    }         // while(1)

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

    printf("Finished.\n");
    return 0;
}
