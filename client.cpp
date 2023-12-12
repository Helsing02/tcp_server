#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#define ADDRESS "mysocket" /* адрес для связи */

using namespace std;


int main (){	
	int s, len;
	struct sockaddr sa;
/* получаем свой сокет-дескриптор: */
	if ((s = socket (AF_UNIX, SOCK_STREAM, 0))<0){
		perror ("client: socket"); 
		exit(1);
	}
	printf("Socketfd: %d\n",s);
/* создаем адрес, по которому будем связываться с сервером: */
	sa.sa_family = AF_UNIX;
	strcpy (sa.sa_data, ADDRESS);
/* пытаемся связаться с сервером: */
	len = sizeof ( sa.sa_family) + strlen ( sa.sa_data);
	if (connect ( s, &sa, len) < 0 ){
		perror ("client: connect"); 
		exit (1);
	}

/* посылаем ответ серверу */
	char str[8]="client\n";
	send (s, str, 8, 0);

/* завершаем сеанс работы   */
	close (s);

	exit (0);
}