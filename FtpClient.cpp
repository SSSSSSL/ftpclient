#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include "FtpClient.h" 

#define BUF_SIZE	8192

/************************CONSTRUCTOR*************************/
FtpClient::FtpClient() {
	Socket = INVALID_SOCKET;
}
/************************DESTRUCTOR**************************/
FtpClient::~FtpClient() {
	Close();
}
/***********************CONNECT SERVER***********************/
bool FtpClient::Connect(const char * _ServerAddress, int _ServerPort) {
	struct sockaddr_in serv_addr;

	if (Socket != INVALID_SOCKET)
		return false;

	Socket = Create_Socket(_ServerAddress, _ServerPort);
	printf("Socket(%d)\n", Socket);
	if (Socket == INVALID_SOCKET) {
		printf("Error, %s Create_Socket(%s, %d) error\n", __func__, _ServerAddress, _ServerPort);
		return false;
	}

	if (Recv_Code("220")) 
		return true;
	
	Close();
        return false;
}

/***************************CLOSE****************************/
void FtpClient::Close() {
        close(Socket);
        Socket = INVALID_SOCKET;
}

/***************************LOGIN****************************/
bool FtpClient::Login(const char * ID, const char * PW) {
	char buf[BUF_SIZE];
	int n;

        if (Socket == INVALID_SOCKET) {
                printf("Error, %s not connected\n", __func__);
                return false;
        }

	if (Send("USER %s", ID) == false || Recv_Code("331") == false || Send("PASS %s", PW) == false || Recv_Code("230") == false)
		return false;

	return true;
}


/**************************UPLOAD****************************/
bool FtpClient::Upload(const char * FilePath) {
	char * FileName = NULL;

	if (!Exist_File(FilePath)) {
                printf("Error, %s Exist_File(%s) error\n", __func__, FilePath);
                return false;
	}
	printf("File %s Exist!\n", FilePath);

	if ((FileName = Get_Filename_By_Path(FilePath)) == NULL) {
		printf("Error, %s Get_Filename_By_Name(%s) error\n", __func__, FilePath);
		return false;
	}
	printf("Filename %s\n", FileName);

	if (Send_Binary() == false || Send_Passive() == false) {
		printf("Error, %s Send_Binary() & Send_Passive() error\n", __func__);
		free(FileName);
		return false;
	}
	printf("Send Binary & Passive\n");

	if (!Send("STOR %s", FileName)) {
		printf("Error, %s Send(\"STOR %s\") error\n", __func__, FileName);
		free(FileName);
		return false;
	}

	int socket = Create_Socket(DataIP, DataPort);
	if (socket == INVALID_SOCKET) {
                printf("Error, %s Create_Socket(%s, %d) error\n", __func__, DataIP, DataPort);
		free(FileName);
                return false;
        }

	int fd = open(FilePath, O_RDONLY);
	if (fd >= 0) {
		char send_buffer[BUF_SIZE];
		int read_len;

		while (1) {
			read_len = read(fd, send_buffer, sizeof(send_buffer));
			if (read_len <= 0) break;

			int n;
			int sending = 0;
		        while(1) {
        		        n = send(socket, send_buffer+sending, read_len - sending, 0);
		                if ( n == - 1) {
		                        printf("Error(%d), %s send(%d, %s, %d, 0)\n", errno, __func__, socket, send_buffer+sending, read_len - sending);
					free(FileName);
                		        return false;
		                }
                		sending += n;
		                if ( sending >= read_len)
		                        break;
		        }

			if ( sending != read_len ) {
				printf("Error, %s Not Equal error\n", __func__);
				free(FileName);
				return false;
			}
		}
		close(fd);
	}
	else {
                printf("Error, %s open(%s, O_RDONLY) error\n", __func__, FilePath);
		free(FileName);
                return false;
	}

	close(socket);

	if (Recv_Code("150") == false || Recv_Code("226") == false) {
		free(FileName);
		return false;
	}

	free(FileName);
	return true;
}
/*************************CURRENT DIR************************/
bool FtpClient::Current_Dir(char ** DirName) {
	char *Buffer, *start, *end;

	if (Send("PWD") == false || Recv_Code(&Buffer, "257") == false)
		return false;

	start = Buffer;

	while(*start != '"')
		start++;

	end = ++start;

	while(*end != '"')
		end++;
	*end = '\0';

	*DirName = (char*)malloc(sizeof(char)*(end-start)+1);
	strcpy(*DirName, start);

	free(Buffer);
	return true;

}
/**************************LIST DIR**************************/
bool FtpClient::List_Dir() {
	if (Send_Binary() == false || Send_Passive() == false) {
		printf("Error, %s Send_Binary() & Send_Passive() error\n", __func__);
		return false;
	}

	if (Send("List") == false) {
		printf("Error, %s Send(\"List\") error\n", __func__);
		return false;
	}

	int socket = Create_Socket(DataIP, DataPort);
        if (socket == INVALID_SOCKET) {
                printf("Error, %s Create_Socket(%s, %d) error\n", __func__, DataIP, DataPort);
                return false;
        }

	int read_len, n;
	char read_buffer[BUF_SIZE];
	struct pollfd _poll[1];
	
	printf("List :\n");
	while (1) {
		_poll[0].fd = socket;
		_poll[0].events = POLLIN;
		_poll[0].revents = 0;

		n = poll(_poll, 1, 1000);
		if ( n <= 0 ) break;

		read_len = recv(socket, read_buffer, sizeof(read_buffer), 0);
		if (read_len <= 0) break;

		printf("%s ", read_buffer);
	}
	printf("List End\n");

	close(socket);
	if (Recv_Code("150") == false)
		return false;

	printf("Here\n");

	if (Recv_Code("226") == false)
		return false;

	return true;
}
/*************************CHANGE DIR*************************/
bool FtpClient::Change_Dir(const char * Dir) {
	if (Send("CWD %s", Dir) == false || Recv_Code("250") == false)
		return false;

	return true;
}
/*************************CREATE DIR*************************/
bool FtpClient::Create_Dir(const char * Dir) {
	if (Send("MKD %s", Dir) == false || Recv_Code("257") == false)
		return false;

	return true;
}
/*************************REMOVE DIR*************************/
bool FtpClient::Remove_Dir(const char * Dir) {
	if (Send("RMD %s", Dir) == false || Recv_Code("250") == false)
		return false;

	return true;
}

/**********************CREATE SOCKET*************************/
int FtpClient::Create_Socket(const char * _ServerAddress, int _ServerPort) {
        struct sockaddr_in serv_addr;
        int fd;

        if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
                printf("Error, %s socket(PF_INET, SOCK_STREAM, 0) error\n", __func__);
                return INVALID_SOCKET;
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family            = AF_INET;
        serv_addr.sin_port              = htons(_ServerPort);

        if (isdigit(_ServerAddress[0]) == 0) {
                struct hostent * _host_ent;
                if ((_host_ent = gethostbyname(_ServerAddress)) == NULL) {
                        printf("Error, %s gethostbyname(%s) error\n", __func__, _ServerAddress);
                        return INVALID_SOCKET;
                }
                serv_addr.sin_addr = *(struct in_addr*)_host_ent->h_addr_list[0];
        }
        else
                serv_addr.sin_addr.s_addr = inet_addr(_ServerAddress);


        if (connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
                printf("Error, %s connect(%d, %p, %ld) error\n", __func__, fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
                close(fd);
                return INVALID_SOCKET;

        }

        return fd;
}



/*************************EXIST FILE*************************/
bool FtpClient::Exist_File(const char * Path) {
	return access(Path, F_OK)+1;
	//return access(Path, F_OK)==0 ? true : false;
}

/********************Get FILENAME BY PATH********************/
char * FtpClient::Get_Filename_By_Path(const char * Path) {
        char *ptr, *temp;
	int PathLen = strlen(Path);

	if (Path == NULL)
		return NULL;

	for (ptr = (char*)Path + PathLen; ptr >= Path; ptr--) {
		if (*ptr == '/') {
			ptr++;
			break;
		}
	}

	temp = (char*)malloc(sizeof(char)*strlen(ptr));
	strcpy(temp, ptr);
	return temp;
}

/************************SEND BINARY*************************/
bool FtpClient::Send_Binary() {
	if (Send("TYPE I") == false || Recv_Code("200") == false)
		return false;

	return true;
}

/************************SEND ASCII**************************/
bool FtpClient::Send_ASCII() {
        if (Send("TYPE A") == false || Recv_Code("200") == false)
                return false;

        return true;
}

/***********************SEND PASSIVE*************************/
bool FtpClient::Send_Passive() {
	char * Buffer, *token, *rest;
	int arr[6], i=0;

	if (Send("PASV") == false || Recv_Code(&Buffer, "227") == false)
		return false;

	rest = Buffer;

	while (*rest != '(')
		rest++;

	while ((token = strtok_r(rest, "(,", &rest)) != NULL) 
		arr[i++] = atoi(token);

	snprintf(DataIP, 20, "%d.%d.%d.%d", arr[0], arr[1], arr[2], arr[3]);
	DataPort = 256 * arr[4] + arr[5];

	free(Buffer);

	return true;
}

/****************************SEND****************************/
bool FtpClient::Send(const char * command, ... ) {
        int send_len, n, sending;
        char buffer[BUF_SIZE];

        if (Socket == INVALID_SOCKET) {
                printf("Error, %s not connected\n", __func__);
                return false;
        }

        va_list list;

        va_start(list, command);
        send_len = vsnprintf(buffer, sizeof(buffer)-3, command, list);
        va_end(list);

        snprintf(buffer+send_len, sizeof(buffer) - send_len, "\r\n");
        send_len += 2;

        sending = 0;
        while(1) {
                n = send(Socket, buffer+sending, send_len - sending, 0);
                if ( n == - 1) {
                        printf("Error(%d), %s send(%d, %s, %d, 0)\n", errno, __func__, Socket, buffer+sending, send_len - sending);
                        return false;
                }
                sending += n;
                if ( sending >= send_len)
                        break;
        }
        return true;
}

/*************************RECV CODE**************************/
bool FtpClient::Recv_Code(char ** Buffer, const char * Code) {
        int read_len;
        char buffer[BUF_SIZE];

        if (Socket == INVALID_SOCKET) {
                printf("Error, %s not connected\n", __func__);
                return false;
        }

        while ( (read_len = recv(Socket, buffer, sizeof(buffer), 0)) == -1 ) {  }
        buffer[read_len] = '\0';

	*Buffer = (char *)malloc(sizeof(char)*(read_len+1));
	strcpy(*Buffer, buffer);

        printf("recv : %s\n", buffer);

        return Code_Check(buffer, Code);
}

/*************************RECV CODE**************************/
bool FtpClient::Recv_Code(const char * Code) {
	int read_len;	
	char buffer[BUF_SIZE];
	
	if (Socket == INVALID_SOCKET) {
		printf("Error, %s not connected\n", __func__);
		return false;
	}
	
	while ( (read_len = recv(Socket, buffer, sizeof(buffer), 0)) == -1 ) {	}
        buffer[read_len] = '\0';

        printf("recv : %s\n", buffer);

	return Code_Check(buffer, Code);
}

/************************CHECK CODE**************************/
bool FtpClient::Code_Check(char * recv_line, const char * Code) {
	char *token, *rest = recv_line;

	while((token = strtok_r(rest, " ", &rest)) != NULL) 
		if (strcmp(token, Code) == 0)
			return true;

	return false;
	
}
