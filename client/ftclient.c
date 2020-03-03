#include "ftclient.h"
	

int sock_control; 


/**
 * 从服务器接收响应
 * 错误返回-1，成功返回代码
 */
int read_reply(){
	int retcode = 0;
	if (recv(sock_control, &retcode, sizeof retcode, 0) < 0) {
		perror("client: error reading message from server\n");
		return -1;
	}	
	return ntohl(retcode);
}



/**
 * 打印响应消息
 */
void print_reply(int rc) 
{
	switch (rc) {
		case 220:
			printf("220 Welcome, server ready.\n");
			break;
		case 221:
			printf("221 Goodbye!\n");
			break;
		case 226:
			printf("226 Closing data connection. Requested file action successful.\n");
			break;
		case 550:
			printf("550 Requested action not taken. File unavailable.\n");
			break;
	}
	
}


/**
 * 在cstruct中解析命令
 */ 
int ftclient_read_command(char* buf, int size, struct command *cstruct)
{
	memset(cstruct->code, 0, sizeof(cstruct->code));
	memset(cstruct->arg, 0, sizeof(cstruct->arg));
	
	printf("ftclient> ");	// 提示输入
	fflush(stdout); 	

	// 等待用户输入命令
	read_input(buf, size);
		
	char *arg = NULL;
	arg = strtok (buf," ");
	arg = strtok (NULL, " ");

	if (arg != NULL){
		// 如果有参数则存储参数
		strncpy(cstruct->arg, arg, strlen(arg));
	}

	// buf = command
	if (strcmp(buf, "list") == 0) {
		strcpy(cstruct->code, "LIST");		
	}
	else if (strcmp(buf, "get") == 0) {
		strcpy(cstruct->code, "RETR");		
	}
	else if (strcmp(buf, "quit") == 0) {
		strcpy(cstruct->code, "QUIT");		
	}
	else {//无效
		return -1;
	}

	// 将code存储在缓冲区的开头
	memset(buf, 0, 400);
	strcpy(buf, cstruct->code);

	// 如果有arg，将其附加到缓冲区
	if (arg != NULL) {
		strcat(buf, " ");
		strncat(buf, cstruct->arg, strlen(cstruct->arg));
	}
	
	return 0;
}



/**
 * 执行get <filename>命令
 */
int ftclient_get(int data_sock, int sock_control, char* arg)
{
    char data[MAXSIZE];
    int size;
    FILE* fd = fopen(arg, "w");
    
    while ((size = recv(data_sock, data, MAXSIZE, 0)) > 0) {
        fwrite(data, 1, size, fd);
    }

    if (size < 0) {
        perror("error\n");
    }

    fclose(fd);
    return 0;
}


/**
 * 打开数据连接
 */
int ftclient_open_conn(int sock_con)
{
	int sock_listen = socket_create(CLIENT_PORT_ID);

	// 在控制连接上发送ACK
	int ack = 1;
	if ((send(sock_con, (char*) &ack, sizeof(ack), 0)) < 0) {
		printf("client: ack write error :%d\n", errno);
		exit(1);
	}		

	int sock_conn = socket_accept(sock_listen);
	close(sock_listen);
	return sock_conn;
}




/** 
 * 执行list命令
 */
int ftclient_list(int sock_data, int sock_con)
{
	size_t num_recvd;			// recv()接收的字节数
	char buf[MAXSIZE];			// h保存从服务器接收的文件名
	int tmp = 0;

	// 等待服务器启动消息
	if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) {
		perror("client: error reading message from server\n");
		return -1;
	}
	
	memset(buf, 0, sizeof(buf));
	while ((num_recvd = recv(sock_data, buf, MAXSIZE, 0)) > 0) {
        	printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}
	
	if (num_recvd < 0) {
	        perror("error");
	}

	// 等待服务器完成消息
	if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) {
		perror("client: error reading message from server\n");
		return -1;
	}
	return 0;
}



/**
 * 输入：带有code和arg的command结构体
 * 将code和arg连接到字符串中并发送到服务器
 */
int ftclient_send_cmd(struct command *cmd)
{
	char buffer[MAXSIZE];
	int rc;

	sprintf(buffer, "%s %s", cmd->code, cmd->arg);
	
	// 发送命令字符串到服务器
	rc = send(sock_control, buffer, (int)strlen(buffer), 0);	
	if (rc < 0) {
		perror("Error sending command to server");
		return -1;
	}
	
	return 0;
}



/**
 * 从用户获取登录详细信息并发送到服务器进行身份验证
 */
void ftclient_login()
{
	struct command cmd;
	char user[256];
	memset(user, 0, 256);

	// 从用户获取用户名
	printf("Name: ");	
	fflush(stdout); 		
	read_input(user, 256);

	// 将USER命令发送到服务器
	strcpy(cmd.code, "USER");
	strcpy(cmd.arg, user);
	ftclient_send_cmd(&cmd);
	
	// 等待批准发送密码
	int wait;
	recv(sock_control, &wait, sizeof wait, 0);

	// 获取用户密码
	fflush(stdout);	
	char *pass = getpass("Password: ");	

	// S发送PASS命令到服务器
	strcpy(cmd.code, "PASS");
	strcpy(cmd.arg, pass);
	ftclient_send_cmd(&cmd);
	
	// 等待回应
	int retcode = read_reply();
	switch (retcode) {
		case 430:
			printf("Invalid username/password.\n");
			exit(0);
		case 230:
			printf("Successful login.\n");
			break;
		default:
			perror("error reading message from server");
			exit(1);		
			break;
	}
}





int main(int argc, char* argv[]) 
{		
	int data_sock, retcode, s;
	char buffer[MAXSIZE];
	struct command cmd;	
	struct addrinfo hints, *res, *rp;

	if (argc != 3) {
		printf("usage: ./ftclient hostname port\n");
		exit(0);
	}

	char *host = argv[1];
	char *port = argv[2];

	// 获取匹配的地址
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	s = getaddrinfo(host, port, &hints, &res);
	if (s != 0) {
		printf("getaddrinfo() error %s", gai_strerror(s));
		exit(1);
	}
	
	// 查找地址以连接并连接
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (sock_control < 0)
			continue;

		if(connect(sock_control, res->ai_addr, res->ai_addrlen)==0) {
			break;
		} else {
			perror("connecting stream socket");
			exit(1);
		}
		close(sock_control);
	}
	freeaddrinfo(rp);


	// 获得连接，迎接消息
	printf("Connected to %s.\n", host);
	print_reply(read_reply()); 
	

	/* 获取名称和密码并发送到服务器 */
	ftclient_login();

	while (1) { // 循环直到用户输入quit

		// 从用户获取命令
		if ( ftclient_read_command(buffer, sizeof buffer, &cmd) < 0) {
			printf("Invalid command\n");
			continue;	// 循环返回另一个命令
		}

		// 发送命令到服务器
		if (send(sock_control, buffer, (int)strlen(buffer), 0) < 0 ) {
			close(sock_control);
			exit(1);
		}

		retcode = read_reply();		
		if (retcode == 221) {
			/* 如果命令为quit，则退出 */
			print_reply(221);		
			break;
		}
		
		if (retcode == 502) {
			// 如果命令无效，则显示错误消息
			printf("%d Invalid command.\n", retcode);
		} else {			
			// 命令有效（RC = 200），处理命令
		
			// 打开数据连接
			if ((data_sock = ftclient_open_conn(sock_control)) < 0) {
				perror("Error opening socket for data connection");
				exit(1);
			}			
			
			// 执行命令
			if (strcmp(cmd.code, "LIST") == 0) {
				ftclient_list(data_sock, sock_control);
			} 
			else if (strcmp(cmd.code, "RETR") == 0) {
				// 等待回复（文件有效）
				if (read_reply() == 550) {
					print_reply(550);		
					close(data_sock);
					continue; 
				}
				ftclient_get(data_sock, sock_control, cmd.arg);
				print_reply(read_reply()); 
			}
			close(data_sock);
		}

	} // 循环返回以获得更多用户输入

	// 关闭套接字（控制连接）
	close(sock_control);
    return 0;  
}
