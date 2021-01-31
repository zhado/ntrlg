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
#include "draw.h"

int int_char_size(int a){
	int count=0;
	while(a/10>0){
		a=a/10;
		count++;
	}
	return count;
}

int match_end(char* start, char* match){
	int match_len=strlen(match);
	int start_len=strlen(start);
	char temp_ch[match_len];

	memset(temp_ch, 0, match_len-1);
	//memcpy(temp_ch, start-match_len+start_len,match_len-1);
	memcpy(temp_ch, start+start_len-match_len,match_len);
	int comp_res=strcmp(temp_ch, match);

	if(comp_res==0)
		return 1;
	else 	
		return false;
}

void handle_accept(int connfd){
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
		//char temp_chr[100];
		//memset(temp_chr, 0, 100);
		//fseek(fp, 0L, SEEK_END);
		//int f_size=ftell(fp);
		//rewind(fp);
		//sprintf(temp_chr, "%d",f_size);
		//write(connfd,temp_chr,7);

		FILE* fp=fopen(database_file,"r");
		//printf("sending dtbs\n");
		while((n = fread(file_contents,1,MAX_NAME_SIZE,fp ))>0){
			write(connfd,file_contents,n);
		}
		send_dtbs=false;
		fclose(fp);
	}
	memset (rec_buff,0,MAX_NAME_SIZE);
}

int setup_server(int port){
	int listenfd,connfd,n;
	struct sockaddr_in address;
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


	if(bind(listenfd,(struct sockaddr *)&address,sizeof(address))<0){
		printf("bind error");
		printf("%s",strerror(errno));
		exit(1);
	}
	if(listen(listenfd, 10)<0){
		printf("listen error");
		exit(1);
	}
	return listenfd;
}

int handle_connections(int server_fd){
	int connfd;

	fd_set current_sockets,ready_sockets;
	FD_ZERO(&current_sockets);
	FD_SET(server_fd, &current_sockets);
	timeval tm;
	tm.tv_sec=5;
	tm.tv_usec=0;

	ready_sockets=current_sockets;
	if(select(FD_SETSIZE, &ready_sockets, NULL, NULL, &tm)<0){
		printf("seleect error"); 
		return 1;
	}
	//printf("select initied\n");

	for(int i=0;i<FD_SETSIZE;i++){
		if(FD_ISSET(i, &ready_sockets)){
			if(i==server_fd){
				//printf("new conection\n");
				if ((connfd = accept(server_fd, 0,0))<0) 
				{ 
					printf("accept error"); 
					return 1;
				} 
				//FD_SET(connfd, &current_sockets);
				handle_accept(connfd);
				close(connfd);
				shutdown(connfd,2);
			}else{
				//printf("handling connection\n");
				//FD_CLR(i, &current_sockets);
			}
		}
	}
	return 0;
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
	uint32_t state=0;
	while((n=read(sockad,rec_buff,MAX_NAME_SIZE-1))>0){
		draw_status(&state);
		for(int i=0;i<n;i++){
			fputc(rec_buff[i],fp);
		}
		memset (rec_buff,0,n);
	}
	fclose(fp);
	return 0;
}
