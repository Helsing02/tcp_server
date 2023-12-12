
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <list>
#include <signal.h>
#define ADDRESS "mysocket" /* адрес для связи */
#define MAXLINE 1024

using namespace std;


void accept_new_con(int main_socket, list <int>& descriptors){
	struct sockaddr_un ca;
	socklen_t ca_len;
	descriptors.push_back(0);
	if ((descriptors.back() = accept (main_socket, (struct sockaddr *)&ca, &ca_len)) < 0 ) {
		cout<<"accept didn't work";
		exit (1);
	} else{
		cout<<"descriptor # "<<descriptors.back()<<" was connected\n";
	}
}

void disconnect(int descr){
	cout<<"disconnected "<<descr<<" descriptor\n";
	close(descr);
}

ssize_t read_data(int client, char* buffer){
	return recv(client, buffer, MAXLINE, 0);
}



volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r)
{
	wasSigHup = 1;
}

int main(){
	cout<<"my pid is "<<getpid()<<endl;


	sigset_t origMask, blockedMask;
	sigemptyset(&origMask);
	sigemptyset(&blockedMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, NULL);


	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);
	sa.sa_handler = sigHupHandler;
	sa.sa_flags |= SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, NULL);

	

	int main_socket, max_socket;
	list <int> descriptors;

	if((main_socket = socket (AF_UNIX, SOCK_STREAM, 0)) < 0){
		cout<<"Socket wasn't made\n";
		exit (1);
	}

	struct sockaddr_un saddr;
	saddr.sun_family = AF_UNIX;
	strcpy (saddr.sun_path, ADDRESS);

	unlink (ADDRESS);
	socklen_t len = sizeof ( saddr.sun_family) + strlen (saddr.sun_path);
	if (bind ( main_socket, (struct sockaddr *)&saddr, len) < 0 ) {
		cout<<"bind didn't work\n";
		exit (1);
	}

	if (listen (main_socket, 5) < 0 ) {
		cout<<"listen didn't work";
		exit (1);
	}
	list <int>::iterator iter;

	char str[MAXLINE];

	fd_set rfd;
	for (int i=0; i<3; i++){
		cout<<endl;
		FD_ZERO(&rfd);
		FD_SET(main_socket, &rfd);
		max_socket=main_socket;
		for(iter=descriptors.begin(); iter!=descriptors.end(); iter++){
			FD_SET(*iter, &rfd);
			if(*iter>max_socket)
				max_socket=*iter;
		}
		if(pselect(max_socket+1, &rfd, NULL, NULL, NULL, &origMask)==-1){
			if (errno == EINTR && wasSigHup){
				cout<<"sigHup was caught\n";
				wasSigHup=0;
			}
			continue;
		}
		if(FD_ISSET(main_socket, &rfd)){
			cout<<"somebody tries to connect\n";
			int a = read_data(main_socket, str);
			accept_new_con(main_socket, descriptors);
			if(descriptors.size()>1){
				disconnect(descriptors.back());
				descriptors.pop_back();
			}
		} else{
			cout<<"nobody tries to connect\n";
		}
		for(iter=descriptors.begin(); iter!=descriptors.end(); iter++){
			if(FD_ISSET(*iter, &rfd)){
				cout<<"somebody sent message\n";
				ssize_t len=read_data(*iter, str);
				if(len==0){
					disconnect(*iter);
					descriptors.erase(iter);
					break;
				} 
				else{
					cout<<len<<" bytes received\n";
				}

			}
		}
	}
	cout<<"main loop ended\n";
	

	for(iter=descriptors.begin(); iter!=descriptors.end(); iter++){
		disconnect(*iter);		
	}
	exit (0);
}