/* ftserve.h
 * 
 * TCP文件传输实现的服务器端，与客户端ftclient.c一起运行。 将当前目录中的文件列表和
 * 文件发送到ftclient。 需要用户登录。
 * 
 * 使用方法: 
 *    ./ftserve PORT#
 */

#ifndef FTSERVER_H
#define FTSERVER_H

#include "../common/ftcommon.h"


/**
 * 通过数据连接发送以文件名指定的文件,
 * 通过控制连接发送控制消息处理文件名为空或无效的情况
 */
void ftserve_retr(int sock_control, int sock_data, char* filename);



/**
 * 通过数据连接发送当前目录中的文件列表
 * 错误返回-1，成功返回0
 */
int ftserve_list(int sock_data, int sock_control);




/**
 * 打开对客户端的数据连接
 * 错误时返回-1，成功返回用于数据连接的套接字
 */
int ftserve_start_data_conn(int sock_control);



/**
 * 验证用户
 * 如果通过验证，则返回1；否则，返回0
 */
int ftserve_check_user(char*user, char*pass);



/**  
 * 登录连接的客户端
 */
int ftserve_login(int sock_control);


/**
 * 等待客户端的命令并发送响应
 * 返回响应码
 */
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg);



/** 
 * 子进程处理与客户端的连接
 */
void ftserve_process(int sock_control);


#endif