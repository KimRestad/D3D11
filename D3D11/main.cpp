#include <Windows.h>
#include "BasicWindow.hpp"

int WINAPI WinMain(HINSTANCE appHandle, HINSTANCE prevHandle, PSTR cmdLine, int showStyle)
{
	BasicWindow myWindow(appHandle, showStyle);
	//BasicWindow myWindow2(appHandle, showStyle);
	//BasicWindow myWindow3(appHandle, showStyle);

	myWindow.Run();
}