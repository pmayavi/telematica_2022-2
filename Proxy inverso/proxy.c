// Codigo adaptado de https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/amp/
// Por Pablo Maya Villegas
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <strings.h>
#include <netdb.h>

// Definicion de constantes
#define TRUE 1
#define FALSE 0

// Metodo que justifica la causa del error.
void error(char *msg)
{
    perror(msg);
    exit(0);
}

// Metodo main recibe un argumento, el puerto 8080
int main(int argc, char *argv[])
{
    int PORT = atoi(argv[1]);
    int opt = TRUE;
    // num_servers es la cantidad de servidores que responden las peticiones
    int master_socket, addrlen, new_socket, num_servers = 3;
    // max_clients es la cantidad maxima de conecciones distintas que se permiten
    int server = 0, max_clients = 30, activity, i, valread, sd;
    int max_sd, client_socket[max_clients];
    struct sockaddr_in address;

    char buffer[2049];            // data buffer de 2K
    char peticiones[num_servers]; // Guarda cual cliente le hizo peticion a un servidor

    // set of socket descriptors
    fd_set readfds;

    // inicializar el array de los sockets guadados como vacios
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    // se crea el master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Permitirle al master socket multiples conexiones
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // tipo de socket creada
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // hacer el bind con el puerto
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Escuchando el puerto %d \n", PORT);

    // Especifica que el master socket puede escuchar a 3 clientes en un solo momento
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Aceptar las conexiones entrantes
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (TRUE)
    {
        // Reiniciar el set de sockets
        FD_ZERO(&readfds);

        // Incluir al master socket en el set de sockets
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // Incluir las sockets hijas al set de sockets
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            // Si es una socket valida
            if (sd > 0)
                FD_SET(sd, &readfds);

            // Se guarda el file descriptor mayor para la funcion select mas adelante
            if (sd > max_sd)
                max_sd = sd;
        }

        // Espera a actividad de alguna socket
        // Espera indefinidamente/para siempre
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // Se detecta un tipo de conexion con la master socket/la ip publica
        // asi que alguien quiere establecer conexion
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Se guarda con quien se establecio la conexion y mas datos
            printf("Nueva conexion, socketfd: %d , ip: %s , puerto: %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Añade la nueva socket al array de sockets disponibles
            for (i = 0; i < max_clients; i++)
            {
                // Si no hay una socket en la posicion
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Guardando la socket como numero %d\n", i);

                    // Las n primeras conexiones, son los servidores
                    if (i < num_servers)
                    {
                        char num[35];
                        snprintf(num, 35, "Asignado como server %d\n", i);
                        send(new_socket, num, strlen(num), 0);
                    }
                    break;
                }
            }
        }

        // Si no fue con la master socket, es comunicacion con una socket existente
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds))
            {
                // Mira si fue el cierre de conexion
                // read lee el mensaje que llego y lo guarda en buffer
                if ((valread = read(sd, buffer, 2048)) == 0)
                {
                    // Se desconecto, se imprime los detalles de quien fue
                    getpeername(sd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);
                    printf("Host desconectado, ip: %s , puerto: %d \n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    // Cerrar la socket y dejar disponible el espacio en el array de sockets
                    close(sd);
                    client_socket[i] = 0;
                }

                else // LLego un mensaje o peticion
                {
                    // Poner el fin de la string del mensaje
                    buffer[valread] = '\0';
                    // Si su numero de socket es mayor al numero de servidores, es un cliente
                    if (i > num_servers - 1)
                    {
                        // Se envia la peticion a un servidor
                        send(client_socket[server], buffer, strlen(buffer), 0);
                        peticiones[server] = i; // Se guarda cual cliente hizo la solicitud a ese servidor
                        // Cicla al siguiente servidor de forma Round Robin
                        if (server < num_servers - 1)
                            server = server + 1;
                        else
                            server = 0;
                    }
                    else if (client_socket[peticiones[i]] != 0) // Es una respuesta de un servidor
                    {
                        sd = client_socket[peticiones[i]]; // Se le entrega al socket que le hizo la peticion
                        send(sd, buffer, strlen(buffer), 0);
                        getpeername(sd, (struct sockaddr *)&address,
                                    (socklen_t *)&addrlen);
                        printf("Host desconectado, ip: %s , puerto: %d \n",
                               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        // Se cierra la conexion con el cliente quien ya recibio su respuesta
                        close(sd);
                        client_socket[peticiones[i]] = 0;
                    }
                }
            }
        }
    }
    return 0;
}
