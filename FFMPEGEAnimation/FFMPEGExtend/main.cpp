// FFMPEGExtend.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string.h>
#include <iostream>

extern int decode_video(const char *filename, const char *outfilename);


int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout << "[ERROR] FFMPEGExtend argc < 2\n";
		return 0;
	}

	//解析命令行
	const char* strCmdName = argv[1];
	if (strcmp("mp42png", strCmdName) == 0) {
		if (argc != 4) {
			std::cout << "[ERROR] FFMPEGExtend mp42png argc != 4\n";
			return 0;
		}

		const char* strMp4FilePath = argv[2];
		const char* strSaveFolder = argv[3];

		std::cout << "strMp4FilePath:" << strMp4FilePath;
		std::cout << "strSaveFolder:" << strSaveFolder;

		decode_video(strMp4FilePath, strSaveFolder);
		return 0;
	}


	std::cout << "[ERROR] FFMPEGExtend unknown cmd name" << strCmdName << '\n';

    return 0;
}

