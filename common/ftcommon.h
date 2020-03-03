#ifndef FTSRV_SOCKET_H
#define FTSRV_SOCKET_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <netdb.h>

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 常数 */
#define DEBUG				1
#define MAXSIZE 			1024 	// 最大缓冲区大小
#define CLIENT_PORT_ID		16555

/* 存放命令代码和参数*/
struct command {
	char arg[255];
	char code[5];
};


/**
 * 在远程主机上创建监听套接字
 * 错误返回-1，成功返回套接字fd
 */
int socket_create(int port);


/**
 * 为传入的客户端连接请求创建新的套接字
 * 错误返回-1，成功返回新创建的套接字的fd
 */
int socket_accept(int sock_listen);


/**
 * 在sockfd上发送响应代码
 * 错误返回-1，成功返回0
 */
int send_response(int sockfd, int rc);


/**
 * 在给定端口连接到远程主机
 * 成功返回套接字fd，错误返回-1
 */
int socket_connect(int port, char *host);



/**
 * 在sockfd上接收数据
 * 错误时返回-1，成功返回接收到的字节数
 */
int recv_data(int sockfd, char* buf, int bufsize);


/**
 * 在sockfd上发送响应代码
 * 错误返回-1，成功返回0
 */
int send_response(int sockfd, int rc);


/**
 * 修剪字符串中的空格和行尾字符
 */
void trimstr(char *str, int n);



/** 
 * 从命令行读取输入
 */
void read_input(char* buffer, int size);

#endif