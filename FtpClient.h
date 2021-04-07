#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#define FTP_PORT		21
#define INVALID_SOCKET		-1

class FtpClient
{
private:
	int		Socket;

	char		DataIP[20];
	int 		DataPort;
public:
	FtpClient(const char *, int);				//Constructor
	FtpClient();
	~FtpClient();

	bool Connect(const char *, int, int);
	void Close();

	bool Login(const char *, const char *, int);
	bool Upload(char *, int);
	
	bool Current_Dir(char **, int);
	bool List_Dir(int);
	bool Change_Dir(char *, int);
	bool Create_Dir(char *, int);
	bool Remove_Dir(char *, int);

        int Create_Socket(const char *, int);
	bool Exist_File(char *);
	char* Get_Filename_By_Path(char *);

	bool Send_Binary(int);
	bool Send_ASCII(int);
	bool Send_Passive(int);

	bool Send(const char *, ...);

	char* Recv_Code_Buffer(const char *);
	char* Recv_Code_Buffer(const char *, int);

	bool Recv_Code(const char *);
	bool Recv_Code(const char *, int);

	bool Code_Check(char *, const char *);
};

#endif
