# Proxy inverso con balanceador de carga  
## Introducción  
Creamos un proxy inverso el cual recibe peticiones tipo GET, las entrega a 3 (o más) servidores  
Fue escrito en C, ya que estos temas manuales de sockets están más desarrollados y hay más ayudas en C, en Python hay muchos servicios que hacen lo mismo, pero ya desarrollados con varias librerías, y por las limitaciones de librerías nos pareció más correcto hacerlo en C. Aprendimos mucho de C y pudimos evidenciar sus fortalezas.  
## Desarrollo   
El PIBL lee el archivo c.config que está en el servidor, este archivo es modificable y contiene 1. Puerto, 2. El número de servidores, pueden haber de 1 a el máximo de clientes, 3. El número de clientes máximo que se espera en un solo momento, incluye los servidores y 4. El tamaño del cache, ósea, cuantas peticiones van a estar guardadas en cache en un solo momento.  
Se reciben las peticiones GET hasta el servidor, ya este lee la petición la cual es un solo carácter después del ‘/’, y enviar una respuesta HTTP correspondiente a esa petición, que en nuestro caso envía un archivo HTML con las opciones de los números 1 a 9.  
Hay una página HTML base la cual se le entrega al cliente si no entrega un número, por ejemplo, cuando usa solo la URL base, en esta pagina base se puede seleccionar que petición se desea hacer y envía la petición GET con la correcta petición.  
Para correr los servidores se necesitan 2 parámetros: ./server (IP elástica del PIBL) (Puerto: 8080). Los servidores deben establecer la conexión con el PIBL antes de que sea disponible a los clientes, esta sin seguridad, podría tener un protocolo de llaves o contraseñas, pero decidimos no incluirlas.  
Log: Por cada acción, el PIBL escribe en la consola y en el log que acción tomo, se llama conexiones.log.  
Cache: Por cada petición se busca si la respuesta adecuada se encuentra en el cache, si esta se le entrega al cliente sin consultar a los servidores, y si no esta se le hace la petición a los servidores. Cuando le llega la respuesta, lo guarda en cache, y si está lleno entonces sobrescribe al cache que lleva más tiempo sin ser usado.  
## Conclusiones  
Aprendimos a desarrollar y desplegar lenguaje C en maquinas Ubuntu remotas, evidenciamos el funcionamiento de sockets y la aplicación de los conocimientos adquiridos.  
Para que nuestro proyecto sea aplicado al mundo real, en los servidores debe haber un proceso pesado para que el balanceo de cargas y cache si tenga utilidad, que actualmente los servidores solo arman un texto HTML y el PIBL hace mucho más trabajo.  
## Referencias  
El código del proxy fue adaptado en base de www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/amp/ El código del servidor fue adaptado en base al código client.c de www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html y muchas dudas respecto a C fueron aclaradas gracias a stackoverflow.com   
