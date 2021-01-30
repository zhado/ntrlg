#include <ctime>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <cstdlib>
#include <errno.h>
#include <pthread.h>
#include "trlg_common.h"

int int_char_size(int a){
	int count=0;
	while(a/10>0){
		a=a/10;
		count++;
	}
	return count;
}

bool match_end(char* start, char* match){
	int match_len=strlen(match);
	int start_len=strlen(start);
	char temp_ch[match_len];

	memset(temp_ch, 0, match_len-1);
	//memcpy(temp_ch, start-match_len+start_len,match_len-1);
	memcpy(temp_ch, start+start_len-match_len,match_len);
	int comp_res=strcmp(temp_ch, match);

	if(comp_res==0)
		return true;
	else 	
		return false;
}

void handle_con(int connfd){
	char rec_buff[MAX_NAME_SIZE];
	memset (rec_buff,0,MAX_NAME_SIZE);
	bool writing_dtbs=false;
	bool send_dtbs=false;
	int count=0;
	unsigned long l1,l2;
	int num=0;
	int buf_start=0;
	char save_msg[]="sandro FILE_START";
	char send_msg[]="sandro SEND";
	int n;
	while((n=read(connfd,rec_buff,MAX_NAME_SIZE-1))>0){
		if(!writing_dtbs && !send_dtbs){
			int write_msg_sizeo=sizeof(save_msg);
			int send_msg_size=sizeof(send_msg);
			char temp_ch[write_msg_sizeo];

			memset(temp_ch, 0, send_msg_size);
			memcpy(temp_ch, rec_buff,send_msg_size);
			int res=strcmp(temp_ch, send_msg);
			if(res==0){
				send_dtbs=true;
				break;
			}
		}
	}

	if(send_dtbs){
		char file_contents[MAX_NAME_SIZE];
		FILE* fp=fopen(database_file,"r");
		printf("sending dtbs\n");
		while((n = fread(file_contents,1,MAX_NAME_SIZE,fp ))>0){
			write(connfd,file_contents,n);
		}
		write(connfd,"MSG_END",7);
		send_dtbs=false;
		fclose(fp);
	}
	memset (rec_buff,0,MAX_NAME_SIZE);
}


void* listen_server(void* argv){
	int port=*(int*)argv;
	int listenfd,connfd,n;
	struct sockaddr_in address;
	struct sockaddr_in oth_serv_adr;
	int opt=1;
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
				&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

	if(listenfd<0){
		printf("socket error");
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr=INADDR_ANY;
	address.sin_port=htons(port);

	oth_serv_adr.sin_family = AF_INET;

	char ip[]="127.0.0.1";
	if(inet_pton(AF_INET, ip, &oth_serv_adr.sin_addr)<=0){
		printf("outer ip error\n");
	}

	if(bind(listenfd,(struct sockaddr *)&address,sizeof(address))<0){
		printf("bind error");
		printf("%s",strerror(errno));
		exit(1);
	}
	if(listen(listenfd, 10)<0){
		printf("listen error");
		exit(1);
	}

	while(1){
		if ((connfd = accept(listenfd, 0,0))<0) 
		{ 
			printf("accept error"); 
			exit(1); 
		} 

		handle_con(connfd);
		close(connfd);
		shutdown(connfd,2);
	}

	return NULL;
}

void start_net(int port, bool block){
	pthread_t listener;
	pthread_create(&listener, NULL, listen_server, &port);
	if(block){
		printf("starting server without tui\n");
		while(1){
			sleep(1000);
		}
	}
}

int get_from_server(int port, char* ip){
	//printf("starting client\n");
	int sockad;
	struct sockaddr_in serveraddr;
	int opt=1;
	sockad=socket(AF_INET,SOCK_STREAM,0);
	char rec_buff[MAX_NAME_SIZE];

	if (setsockopt(sockad, SOL_SOCKET, SO_REUSEADDR, 
				&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	if(sockad<0){
		printf("socket error");
		return 1;
	}
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port=htons(port);

	if(inet_pton(AF_INET, ip, &serveraddr.sin_addr)<=0){
		printf("outer ip error\n");
		return 1;
	}

	if(connect(sockad, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) <0 ){
		printf("connect error\n");
		return 1;
	}

	memset (rec_buff,0,MAX_NAME_SIZE);
	char send_msg[]="sandro SEND";
	write(sockad,send_msg,sizeof(send_msg));

	FILE* fp=fopen(net_recieved_database,"w");
	int n=0;
	while((n=read(sockad,rec_buff,MAX_NAME_SIZE-1))>0){

		bool end=match_end(rec_buff, "MSG_END");
		for(int i=0;i<n-7*end;i++){
			putc(rec_buff[i],fp);
		}
		if(end){
			//printf("\n");
			break;
		}
		memset (rec_buff,0,n);
	}
	fclose(fp);
	return 0;
}
