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
    char message[2048];
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    // Cabeza del HTML
    char file[1024] = "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n<html><head><meta charset=\"utf-8\"><title>Proxy Inverso</title><link rel=\"icon\" type=\"image/x-icon\" href=\"https://uploads-ssl.webflow.com/62cbec5fe81984e0aac7b23d/62dda486b127ce0a51debf91_6213a3f8e713f6292c8a9526_Prepera%20logo%202.png\"></head><body><h2>Hola desde el servidor 1</h2>";
    char url[128]; // URL construido con la ip elastica para tener botones funcionales
    sprintf(url, "http://%s:%s", argv[1], argv[2]);
    char noimg[2048]; // La pagina inicial tiene botones para ir a las otras paginas
    sprintf(noimg, "<a href=\"%s/1\" class=\"button\">1</a> <a href=\"%s/2\" class=\"button\">2</a> <a href=\"%s/3\" class=\"button\">3</a> <a href=\"%s/4\" class=\"button\">4</a> <a href=\"%s/5\" class=\"button\">5</a> <a href=\"%s/6\" class=\"button\">6</a> <a href=\"%s/7\" class=\"button\">7</a> <a href=\"%s/8\" class=\"button\">8</a> <a href=\"%s/9\" class=\"button\">9</a> </body></html>\r\n", url, url, url, url, url, url, url, url, url);
    char img1[1024]; // Cada pagina tiene una imagen distinta
    sprintf(img1, "<img src=\"https://c.tenor.com/GvP7L0FwFyMAAAAC/dancing-number-dancing-letter.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img2[1024]; //  y un boton para regresar a la principal
    sprintf(img2, "<img src=\"https://media.tenor.com/J-MYuLM6fXUAAAAi/dancing-number-dancing-letter.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img3[1024];
    sprintf(img3, "<img src=\"https://media.tenor.com/o96-hgLmCQ0AAAAC/dancing-number-dancing-letter.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img4[1024];
    sprintf(img4, "<img src=\"https://i.pinimg.com/originals/e5/36/86/e5368693123ea4615e05e11c93f87ce3.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img5[1024];
    sprintf(img5, "<img src=\"https://media.tenor.com/JD-y9a5fHBsAAAAi/dancing-number-dancing-letter.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img6[1024];
    sprintf(img6, "<img src=\"https://media.tenor.com/jaAZ3VgcobkAAAAC/dancing-six-dancing-number.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img7[1024];
    sprintf(img7, "<img src=\"https://media.tenor.com/6-3dxjy6qSMAAAAi/dancing-number-dancing-letter.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img8[1024];
    sprintf(img8, "<img src=\"https://media.tenor.com/d3HlvsnflX0AAAAC/dancing-number-dancing-letter.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);
    char img9[1024];
    sprintf(img9, "<img src=\"https://media.tenor.com/i0x1-y8QubAAAAAi/dancing-number-dancing-letter.gif\" width=\"250\" height=\"250\"> <a href=\"%s\" class=\"button\">Regresar</a></body></html>\r\n", url);

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

        if (buffer[4] == '/') // Si le llega una peticion valida
        {
            strcpy(message, file);
            switch (buffer[5]) // La petcion es solo el caracter que sigue del /
            {
            case '1':
                strcat(message, img1);
                break;
            case '2':
                strcat(message, img2);
                break;
            case '3':
                strcat(message, img3);
                break;
            case '4':
                strcat(message, img4);
                break;
            case '5':
                strcat(message, img5);
                break;
            case '6':
                strcat(message, img6);
                break;
            case '7':
                strcat(message, img7);
                break;
            case '8':
                strcat(message, img8);
                break;
            case '9':
                strcat(message, img9);
                break;
            default:
                strcat(message, noimg);
            }
            char tmpstr[2]; // Le envia al proxy cual es la peticion para su uso en cache
            tmpstr[0] = buffer[5];
            tmpstr[1] = 0;
            strcat(message, tmpstr);
            n = write(sockfd, message, strlen(message));
        }
    }
    return 0;
}
