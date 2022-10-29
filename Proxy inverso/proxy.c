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
#include <time.h>

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
    FILE *log;

    int opt = TRUE;
    // num_servers es la cantidad de servidores que responden las peticiones
    int num_servers = 3;
    // max_clients es la cantidad maxima de conecciones distintas que se permiten
    int max_clients = 30;
    // cache_size es el numero maximo de archivos que se guardan en cache
    int cache_size = 5;

    int master_socket, addrlen, new_socket, activity;
    int server = 0, i, j, valread, cache_current = 0;
    int client_socket[max_clients], sd, max_sd;
    // Cada elemento del cache esta compuesto de 3 cosas: cual es, que es, y la ultima vez que fue usado
    char cache[cache_size][2048], cachename[cache_size];
    double TTL, cacheTTL[cache_size];
    // Usados para el log con tiempo y para el TTL de cada elemento en cache
    struct timeval tv;
    time_t rawtime;
    struct tm *timeinfo;
    struct sockaddr_in address;

    char buffer[2048];            // data buffer de 2K
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
    puts("Esperando a los servidores...\n");
    // Loop infinito para mantener el servidor funcionando indefinidamente
    while (TRUE)
    {
        // Reiniciar el set de sockets
        FD_ZERO(&readfds);

        // Incluir al master socket en el set de sockets
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        log = fopen("conexiones.log", "a+");

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
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            printf("%sNueva conexion %s\n", asctime(timeinfo), inet_ntoa(address.sin_addr));
            fprintf(log, "%sNueva conexion %s\n", asctime(timeinfo), inet_ntoa(address.sin_addr));

            // Añade la nueva socket al array de sockets disponibles
            for (i = 0; i < max_clients; i++)
            {
                // Si no hay una socket en la posicion
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Guardando la socket como numero %d\n\n", i);
                    fprintf(log, "Guardando la socket como numero %d\n\n", i);
                    // Las n primeras conexiones, son los servidores
                    if (i < num_servers)
                    {
                        // Se le envia un mensaje al servidor para confirmarle que es un servidor
                        char num[35];
                        snprintf(num, 35, "# Asignado como server %d\n", i);
                        send(new_socket, num, strlen(num), 0);
                    }
                    break;
                }
            }
        }
        fclose(log); // El archivo log debe cerrarse y abrirse para que se mantenga actualizando
        // Si no fue con la master socket, es comunicacion con una socket existente
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds))
            {
                log = fopen("conexiones.log", "a+");
                // Mira si fue el cierre de conexion
                // read lee el mensaje que llego y lo guarda en buffer
                if ((valread = read(sd, buffer, 2048)) == 0)
                {
                    // Se desconecto, se imprime los detalles de quien fue
                    getpeername(sd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);
                    time(&rawtime);
                    timeinfo = localtime(&rawtime);
                    printf("%sHost desconectado: %s\n\n", asctime(timeinfo), inet_ntoa(address.sin_addr));
                    fprintf(log, "%sHost desconectado: %s\n\n", asctime(timeinfo), inet_ntoa(address.sin_addr));
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
                        getpeername(sd, (struct sockaddr *)&address,
                                    (socklen_t *)&addrlen);
                        time(&rawtime);
                        timeinfo = localtime(&rawtime);
                        printf("%sPeticion de %s:\n%s\n\n", asctime(timeinfo), inet_ntoa(address.sin_addr), buffer);
                        fprintf(log, "%sPeticion de %s:\n%s\n\n", asctime(timeinfo), inet_ntoa(address.sin_addr), buffer);

                        int nocache = TRUE;
                        for (j = 0; j < cache_size; j++) // Se busca en el cache por la peticion /p
                        {
                            if (cachename[j] == buffer[5]) // Si encuentra que en el cahce esta esa peticion
                            {
                                // Se le envia la peticion desde cache al cliente y se cierra la conexion
                                send(sd, cache[j], strlen(cache[j]), 0);
                                time(&rawtime);
                                timeinfo = localtime(&rawtime);
                                printf("%sRespuesta desde cache a %s:\n%s\n", asctime(timeinfo), inet_ntoa(address.sin_addr), cache[j]);
                                fprintf(log, "%sRespuesta desde cache a %s:\n%s\n", asctime(timeinfo), inet_ntoa(address.sin_addr), cache[j]);
                                gettimeofday(&tv, NULL);
                                TTL = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
                                cacheTTL[j] = TTL; // El TTL de esa peticion se refresca
                                printf("Host desconectado: %s\n\n", inet_ntoa(address.sin_addr));
                                fprintf(log, "Host desconectado: %s\n\n", inet_ntoa(address.sin_addr));
                                // Se cierra la conexion con el cliente quien ya recibio su respuesta
                                close(sd);
                                client_socket[i] = 0;
                                nocache = FALSE;
                                break;
                            }
                        }

                        if (nocache) // Solo hace la peticion a los servidores si no se encontro en el cache
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
                    }
                    else if (peticiones[i] != 0) // Es una respuesta de un servidor
                    {
                        if (cache_current < cache_size) // El cache esta vacio, sucede al inicio del servidor
                        {
                            // Guarda el nombre de la peticion
                            cachename[cache_current] = buffer[strlen(buffer) - 1];
                            buffer[strlen(buffer) - 1] = '\0';
                            // Guarda el cache
                            strcpy(cache[cache_current], buffer);
                            gettimeofday(&tv, NULL);
                            TTL = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
                            // Guarda el TTL del cache
                            cacheTTL[cache_current] = TTL;
                            printf("Guardado en el cache %c\n", cachename[cache_current]);
                            fprintf(log, "Guardado en el cache %c\n", cachename[cache_current]);
                            cache_current = cache_current + 1;
                        }
                        else // El cache esta lleno
                        {
                            double temp = cacheTTL[0];
                            for (j = 1; j < cache_size; j++)
                            {
                                // Busca el cache que ha pasado mas tiempo sin ser usado
                                if (cacheTTL[j] < temp)
                                    temp = cacheTTL[j];
                            }
                            for (j = 0; j < cache_size; j++)
                            {
                                if (cacheTTL[j] == temp)
                                {
                                    // Sobre escribe el cahce que paso mas tiempo sin ser usado
                                    // Guarda el nombre de la peticion
                                    cachename[j] = buffer[strlen(buffer) - 1];
                                    buffer[strlen(buffer) - 1] = '\0';
                                    // Guarda el cache
                                    strcpy(cache[j], buffer);
                                    gettimeofday(&tv, NULL);
                                    TTL = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
                                    // Guarda el TTL del cache
                                    cacheTTL[j] = TTL;
                                    printf("Sobreguardado en el cache %c\n", cachename[j]);
                                    fprintf(log, "sobreguardado en el cache %c\n", cachename[j]);
                                }
                            }
                        }
                        sd = client_socket[peticiones[i]]; // Se le entrega al socket que le hizo la peticion

                        getpeername(sd, (struct sockaddr *)&address,
                                    (socklen_t *)&addrlen);
                        time(&rawtime);
                        timeinfo = localtime(&rawtime);
                        printf("%sRespuesta a %s:\n%s\n", asctime(timeinfo), inet_ntoa(address.sin_addr), buffer);
                        fprintf(log, "%sRespuesta a %s:\n%s\n", asctime(timeinfo), inet_ntoa(address.sin_addr), buffer);

                        send(sd, buffer, strlen(buffer), 0);
                        printf("Host desconectado: %s\n\n", inet_ntoa(address.sin_addr));
                        fprintf(log, "Host desconectado: %s\n\n", inet_ntoa(address.sin_addr));
                        // Se cierra la conexion con el cliente quien ya recibio su respuesta
                        close(sd);
                        client_socket[peticiones[i]] = 0;
                    }
                }
                fclose(log);
            }
        }
    }
    return 0;
}
