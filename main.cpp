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
	client.Current_Dir(&CurrDir);

	client.Create_Dir("./test/");

	client.Change_Dir("./test/");
	client.Current_Dir(&CurrDir);

	client.Upload("/home/rgblab/out.txt");

	client.Upload("/home/rgblab/temp/123456789012345.txt");
	client.Upload("/home/rgblab/temp/234567890123456.txt");
	client.Upload("/home/rgblab/temp/345678901234567.txt");

	return 0;

}
