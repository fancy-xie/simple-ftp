#include "ftcommon.h"



/**
 * 在远程主机上创建监听套接字
 * 错误返回-1，成功返回套接字fd
 */
int socket_create(int port)
{
	int sockfd;
	int yes = 1;
	struct sockaddr_in sock_addr;

	// 创建套接字
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error"); 
		return -1; 
	}

    // 设置地址信息
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);		

    // 设置允许重用地址和端口
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		close(sockfd);
		perror("setsockopt() error");
		return -1; 
	}

	// bind
	if (bind(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		close(sockfd);
		perror("bind() error"); 
		return -1; 
	}
   
	// 开始监听传入的TCP请求
	if (listen(sockfd, 5) < 0) {
		close(sockfd);
		perror("listen() error");
		return -1;
	}              
	return sockfd;
}



/**
 * 为传入的客户端连接请求创建新的套接字
 * 错误返回-1，成功返回新创建的套接字的fd
 */
int socket_accept(int sock_listen)
{
	int sockfd;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);


	// 等待传入请求，将客户端信息存储在client_addr中
	sockfd = accept(sock_listen, (struct sockaddr *) &client_addr, &len);
	
	if (sockfd < 0) {
		perror("accept() error"); 
		return -1; 
	}
	return sockfd;
}



/**
 * 在sockfd上发送响应代码
 * 错误返回-1，成功返回0
 */
int send_response(int sockfd, int rc)
{
	int conv = htonl(rc);
	if (send(sockfd, &conv, sizeof conv, 0) < 0 ) {
		perror("error sending...\n");
		return -1;
	}
	return 0;
}



/**
 * 在给定端口连接到远程主机
 * 成功返回套接字fd，错误返回-1
 */
int socket_connect(int port, char*host)
{
	int sockfd;  					
	struct sockaddr_in dest_addr;

	// 创建套接字
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        	perror("error creating socket");
        	return -1;
    }

	// 地址
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(host);

	if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0 ) {
        	perror("error connecting to server");
		return -1;
    	}    
	return sockfd;
}



/**
 * 在sockfd上接收数据
 * 错误时返回-1，成功返回接收到的字节数
 */
int recv_data(int sockfd, char* buf, int bufsize){
	size_t num_bytes;
	memset(buf, 0, bufsize);
	num_bytes = recv(sockfd, buf, bufsize, 0);
	if (num_bytes < 0) {
		return -1;
	}
	return num_bytes;
}




/**
 * 修剪字符串中的空格和行尾字符
 */
void trimstr(char *str, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		if (isspace(str[i])) str[i] = 0;
		if (str[i] == '\n') str[i] = 0;
	}
}




/** 
 * 从命令行读取输入
 */
void read_input(char* buffer, int size)
{
	char *nl = NULL;
	memset(buffer, 0, size);

	if ( fgets(buffer, size, stdin) != NULL ) {
		nl = strchr(buffer, '\n');
		if (nl) 
			*nl = '\0'; // 截断，替换换行符
	}
}
