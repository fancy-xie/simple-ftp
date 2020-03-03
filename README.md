FTP Client-Server Implementation
===========
Simple implementation of a file transfer program. It includes custom client and server programs that provide functionality to authenticate a user, list remote files, and retrieve remote files.

文件传输程序的简单实现。包括自定义的客户端和服务器程序，这些程序提供了对用户进行身份验证，列出远程文件以及获取远程文件的功能。

### Directory layout:
	simple-ftp/
		client/
			ftclient.c
			ftclient.h
			makefile
		common/
			ftcommon.c
			ftcommon.h
		server/
			ftserver.c
			ftserver.h
			makefile
			.auth

###Usage

To compile and link ftserver:
```
	$ cd server/
	$ make
```

To compile and link ftclient:
```
	$ cd client/
	$ make
```

To run ftserve:
```
	$ server/ftserve PORTNO
```

To run ftclient:
```
	$ client/ftclient HOSTNAME PORTNO

	Commands:
		list
		get <filename>
		quit
```

Available commands:
```
list            - retrieve list of files in the current remote directory / 检索当前远程目录中的文件列表
get <filename>  - get the specified file / 获取指定的文件
quit            - end the ftp session / 结束ftp会话
```

Logging In:
```
	Name: anonymous
	Password: [empty]
```
