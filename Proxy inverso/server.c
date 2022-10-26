// Codigo adaptado de https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
// Por Pablo Maya Villegas
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Metodo que justifica la causa del error.
void error(char *msg)
{
    perror(msg);
    exit(0);
}

// Metodo main recibe dos argumentos,
// la ip publica o privada del servidor proxy y el puerto 8080
int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);                   // El puerto
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Se abre la socket con el servidor proxy
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]); // Se establece la conexion con la ip
    if (server == NULL)              // No se encontro el host con esa ip
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    while (1)
    {
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255); // Lee la peticion que le envie el servidor proxy
        if (n < 0)
            error("ERROR reading from socket");
        printf("%s", buffer);

        if (buffer[0] != 0) // Si le llega una peticion valida
        {
            // Envia una respuesta HTTP la cual le entrega al cliente un html, estos deben tener menos de 2048 caracteres
            char message[2048] = "HTTP/1.0 200 OK\r\nHost: http://54.147.55.156\r\nContent-type: text/html\r\n\r\n<html><head><meta charset=\"utf-8\"><title>Proxy Inverso</title></head><body><h2>Hola desde el servidor 1</h2><img src=\"https://pioneeroptimist.com/wp-content/uploads/2021/03/among-us-6008615_1920-838x900.png\"></body></html>\r\n";
            n = write(sockfd, message, strlen(message));
        }
    }
    return 0;
}
