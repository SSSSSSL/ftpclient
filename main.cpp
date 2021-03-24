#include "FtpClient.h"
#include <iostream>


int main(){
	char * CurrDir;
	FtpClient client;

	std::cout << "Done" << std::endl;

	//client.Connect("222.104.199.50", 21);
	client.Connect("nasftp.arionflight.com", 21);

	std::cout << "Done2" << std::endl;

	client.Login("rgbftp", "anwlrodusrnth1!!");

	client.Current_Dir(&CurrDir);

	printf("Current Dir : %s\n", CurrDir);

	client.Create_Dir("/home/");

	client.Change_Dir("/home/");

//	client.List_Dir();

	client.Upload("/home/rgblab/out.txt");

	return 0;

}
