/* ftclient.h
 * 
 * TCP文件传输实现的客户端与自定义服务器ftserve.c一起运行。 接收来自输入的命令，
 * 并检索当前和文件中的文件列表。
 * 
 * 可用的命令: 
 *    get <filename>
 *    list
 *    quit
 *    
 * 使用方法: 
 *    ./ftclient SERVER_HOSTNAME PORT#
 */

#ifndef FTCLIENT_H
#define FTCLIENT_H

#include "../common/ftcommon.h"


/**
 * 从服务器接收响应
 * 错误返回-1，成功返回代码
 */
int read_reply();


/**
 * 打印响应消息
 */
void print_reply(int rc);


/**
 * 在cstruct中解析命令
 */ 
int ftclient_read_command(char* buf, int size, struct command *cstruct);


/**
 * 执行get <filename>命令
 */
int ftclient_get(int data_sock, int sock_control, char* arg);


/**
 * 打开数据连接
 */
int ftclient_open_conn(int sock_con);


/** 
 * 执行list命令
 */
int ftclient_list(int sock_data, int sock_con);


/**
 * 输入：带有code和arg的command结构体
 * 将code和arg连接到字符串中并发送到服务器
 */
int ftclient_send_cmd(struct command *cmd);


/**
 * 从用户获取登录详细信息并发送到服务器进行身份验证
 */
void ftclient_login();


#endif
