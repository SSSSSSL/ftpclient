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

	bool Connect(const char *, int);
	void Close();

	bool Login(const char *, const char *);
	bool Upload(char *);
	
	bool Current_Dir(char **);
	bool List_Dir();
	bool Change_Dir(char *);
	bool Create_Dir(char *);
	bool Remove_Dir(char *);

        int Create_Socket(const char *, int);
	bool Exist_File(char *);
	char* Get_Filename_By_Path(char *);

	bool Send_Binary();
	bool Send_ASCII();
	bool Send_Passive();

	bool Send(const char *, ...);
	char* Recv_Code_Buffer(const char *);
	bool Recv_Code(const char *);
	bool Code_Check(char *, const char *);
};

#endif
