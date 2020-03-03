#include "ftserver.h"

int main(int argc, char *argv[])
{	
	int sock_listen, sock_control, port, pid;

	if (argc != 2) {
		printf("usage: ./ftserve port\n");
		exit(0);
	}

	port = atoi(argv[1]);

	if ((sock_listen = socket_create(port)) < 0 ) {
		perror("Error creating socket");
		exit(1);
	}		
	
	while(1) {	// 等待客户端请求

		// 创建用于控制连接的新套接字
		if ((sock_control = socket_accept(sock_listen))	< 0 )
			break;			
		
		// 创建子进程以进行实际的文件传输
		if ((pid = fork()) < 0) { 
			perror("Error forking child process");
		} else if (pid == 0) { 
			close(sock_listen);
			ftserve_process(sock_control);		
			close(sock_control);
			exit(0);
		}
			
		close(sock_control);
	}

	close(sock_listen);	

	return 0;
}



/**
 * 通过数据连接发送以文件名指定的文件,
 * 通过控制连接发送控制消息处理文件名为空或无效的情况
 */
void ftserve_retr(int sock_control, int sock_data, char* filename)
{	
	FILE* fd = NULL;
	char data[MAXSIZE];
	size_t num_read;							
		
	fd = fopen(filename, "r");
	
	if (!fd) {	
		// 发送错误代码（550 未执行请求的操作）
		send_response(sock_control, 550);
		
	} else {	
		// 发送成功 (150 文件状态正常)
		send_response(sock_control, 150);
	
		do {
			num_read = fread(data, 1, MAXSIZE, fd);

			if (num_read < 0) {
				printf("error in fread()\n");
			}

			// 发送文件块
			if (send(sock_data, data, num_read, 0) < 0)
				perror("error sending file\n");

		} while (num_read > 0);													
			
		// 发送消息: 226: 关闭连接, 文件传输成功
		send_response(sock_control, 226);

		fclose(fd);
	}
}





/**
 * 通过数据连接发送当前目录中的文件列表
 * 错误返回-1，成功返回0
 */
int ftserve_list(int sock_data, int sock_control)
{
	char data[MAXSIZE];
	size_t num_read;									
	FILE* fd;

	int rs = system("ls -l > tmp.txt");
	if ( rs < 0) {
		exit(1);
	}
	
	fd = fopen("tmp.txt", "r");	
	if (!fd) {
		exit(1);
	}

	/* 转到文件的开头 */
	fseek(fd, SEEK_SET, 0);

	send_response(sock_control, 1); //开始

	memset(data, 0, MAXSIZE);
	while ((num_read = fread(data, 1, MAXSIZE, fd)) > 0) {
		if (send(sock_data, data, num_read, 0) < 0) {
			perror("err");
		}
		memset(data, 0, MAXSIZE);
	}

	fclose(fd);

	send_response(sock_control, 226);

	return 0;	
}






/**
 * 打开对客户端的数据连接
 * 错误时返回-1，成功返回用于数据连接的套接字
 */
int ftserve_start_data_conn(int sock_control)
{
	char buf[1024];	
	int wait, sock_data;

	// 等待控制连接通过
	if (recv(sock_control, &wait, sizeof wait, 0) < 0 ) {
		perror("Error while waiting");
		return -1;
	}

	// 获取客户地址
	struct sockaddr_in client_addr;
	socklen_t len = sizeof client_addr;
	getpeername(sock_control, (struct sockaddr*)&client_addr, &len);
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

	// 启动与客户端的数据连接
	if ((sock_data = socket_connect(CLIENT_PORT_ID, buf)) < 0)
		return -1;

	return sock_data;		
}





/**
 * 验证用户
 * 如果通过验证，则返回1；否则，返回0
 */
int ftserve_check_user(char*user, char*pass)
{
	char username[MAXSIZE];
	char password[MAXSIZE];
	char *pch;
	char buf[MAXSIZE];
	char *line = NULL;
	size_t num_read;									
	size_t len = 0;
	FILE* fd;
	int auth = 0;
	
	fd = fopen(".auth", "r");
	if (fd == NULL) {
		perror("file not found");
		exit(1);
	}	

	while ((num_read = getline(&line, &len, fd)) != -1) {
		memset(buf, 0, MAXSIZE);
		strcpy(buf, line);
		
		pch = strtok (buf," ");
		strcpy(username, pch);

		if (pch != NULL) {
			pch = strtok (NULL, " ");
			strcpy(password, pch);
		}

		// 删除行尾和空格
		trimstr(password, (int)strlen(password));

		if ((strcmp(user,username)==0) && (strcmp(pass,password)==0)) {
			auth = 1;
			break;
		}		
	}
	free(line);	
	fclose(fd);	
	return auth;
}





/** 
 * 登录连接的客户端
 */
int ftserve_login(int sock_control)
{	
	char buf[MAXSIZE];
	char user[MAXSIZE];
	char pass[MAXSIZE];	
	memset(user, 0, MAXSIZE);
	memset(pass, 0, MAXSIZE);
	memset(buf, 0, MAXSIZE);
	
	// 等待接收用户名
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}	

	int i = 5;
	int n = 0;
	while (buf[i] != 0)
		user[n++] = buf[i++];
	
	// 通知客户端已经准备好输入密码
	send_response(sock_control, 331);					
	
	// 等待接收密码
	memset(buf, 0, MAXSIZE);
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) {
		perror("recv error\n"); 
		exit(1);
	}
	
	i = 5;
	n = 0;
	while (buf[i] != 0) {
		pass[n++] = buf[i++];
	}
	
	return (ftserve_check_user(user, pass));
}





/**
 * 等待客户端的命令并发送响应
 * 返回响应码
 */
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg)
{	
	int rc = 200;
	char buffer[MAXSIZE];
	
	memset(buffer, 0, MAXSIZE);
	memset(cmd, 0, 5);
	memset(arg, 0, MAXSIZE);
		
	// 等待接收命令
	if ((recv_data(sock_control, buffer, sizeof(buffer)) ) == -1) {
		perror("recv error\n"); 
		return -1;
	}
	
	strncpy(cmd, buffer, 4);
	char *tmp = buffer + 5;
	strcpy(arg, tmp);
	
	if (strcmp(cmd, "QUIT")==0) {
		rc = 221;
	} else if((strcmp(cmd, "USER")==0) || (strcmp(cmd, "PASS")==0) ||
			(strcmp(cmd, "LIST")==0) || (strcmp(cmd, "RETR")==0)) {
		rc = 200;
	} else { // 无效的命令
		rc = 502;
	}

	send_response(sock_control, rc);	
	return rc;
}






/** 
 * 子进程处理与客户端的连接
 */
void ftserve_process(int sock_control)
{
	int sock_data;
	char cmd[5];
	char arg[MAXSIZE];

	// 发送欢迎消息
	send_response(sock_control, 220);

	// 验证用户
	if (ftserve_login(sock_control) == 1) {
		send_response(sock_control, 230);
	} else {
		send_response(sock_control, 430);	
		exit(0);
	}	
	
	while (1) {
		// 等待命令
		int rc = ftserve_recv_cmd(sock_control, cmd, arg);
		
		if ((rc < 0) || (rc == 221)) {
			break;
		}
		
		if (rc == 200 ) {
			// 打开与客户端的数据连接
			if ((sock_data = ftserve_start_data_conn(sock_control)) < 0) {
				close(sock_control);
				exit(1); 
			}

			// 执行命令
			if (strcmp(cmd, "LIST")==0) { // list命令
				ftserve_list(sock_data, sock_control);
			} else if (strcmp(cmd, "RETR")==0) { // get <filename> 命令
				ftserve_retr(sock_control, sock_data, arg);
			}
		
			// 关闭数据连接
			close(sock_data);
		} 
	}
}