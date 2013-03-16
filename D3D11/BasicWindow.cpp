#include "BasicWindow.hpp"
#include <sstream>

LRESULT CALLBACK BasicWindow::WindowProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BasicWindow* wnd = NULL;

	if(message == WM_CREATE)
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		wnd = (BasicWindow*)cs->lpCreateParams;
	}

	if(wnd)
		return wnd->MessageHandler(message, wParam, lParam);
	else
		return DefWindowProc(hWindow, message, wParam, lParam);
}

BasicWindow::BasicWindow()
{
	mHandle = 0;
}

BasicWindow::BasicWindow(HINSTANCE appHandle, int showStyle)
	: mCaption("")
{
	ReadIniFile();

	// If window creation failed, show a message.
	if(!InitWindowsApp(appHandle, showStyle))
		ShowMessage("Window was not created!");
}

bool BasicWindow::InitWindowsApp(HINSTANCE appHandle, int showStyle)
{
	int classStyle, bgColor, wndStyle, width, height, x, y;
	char caption[255];

	// Read integers from ini file: class style, bgcolor, window style, width, height and starting x and y.
	classStyle = GetPrivateProfileInt("Window", "class_style", 0, "./D3D11.ini");
	bgColor = GetPrivateProfileInt("Window", "bgcolor", 0, "./D3D11.ini");
	wndStyle = GetPrivateProfileInt("Window", "window_style", 0, "./D3D11.ini");
	width = GetPrivateProfileInt("Window", "width", 0, "./D3D11.ini");
	height = GetPrivateProfileInt("Window", "height", 0, "./D3D11.ini");
	x = GetPrivateProfileInt("Window", "x", 0, "./D3D11.ini");
	y = GetPrivateProfileInt("Window", "y", 0, "./D3D11.ini");

	// Make sure the width, height and starting x, y are valid numbers.
	width = width <=0 ? CW_USEDEFAULT : width;
	height = height <=0 ? CW_USEDEFAULT : height;
	x = x <=0 ? CW_USEDEFAULT : x;
	y = y <=0 ? CW_USEDEFAULT : y;

	// Read the window caption from file and store.
	GetPrivateProfileString("Window", "caption", "Ny Window", caption, 255, "./D3D11.ini");
	mCaption = caption;

	WNDCLASS wc;

	// If the window class is not already registered, register it.
	if(!GetClassInfo(appHandle, "BasicWindowClass", &wc))
	{
		wc.style = classStyle;						// Window class style: Redraw when movement or size changes.
		wc.lpfnWndProc = WindowProc;				// Pointer to the window procedure.
		wc.cbClsExtra = 0;							// No extra bytes allocated following the class structure.
		wc.cbWndExtra = 0;							// No extra bytes allocated followint the window instance.
		wc.hInstance = appHandle;					// Handle to the instance where the window procedure is contained.
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);	// Handle to the class icon.
		wc.hCursor = LoadCursor(0, IDC_ARROW);		// Handle to the class cursor.
		wc.hbrBackground = (HBRUSH)bgColor;			// Handle to the class background brush.
		wc.lpszMenuName = 0;						// Resource name of the class menu.
		wc.lpszClassName = "BasicWindowClass";		// Name of the class for use when the window is created.

		// If the class registration failed, show a message and return failure.
		if(!RegisterClass(&wc))
		{
			std::stringstream ss;
			ss << "RegisterClass failed, error code: ";
			ss << GetLastError();

			ShowMessage(ss.str());
			return false;
		}
	}

	// Create the window and save a handle to it.
	mHandle = CreateWindow("BasicWindowClass",	// Name of the registered window class to use at creation.
						   mCaption.c_str(),	// The caption of the window.
						   wndStyle,			// Window style.
						   x,					// Initial x-position.
						   y,					// Initial y-position.
						   width,				// Width of the window in device units.
						   height,				// Height of the window in device units.
						   0,					// Handle to parent (there is no parent).
						   0,					// Handle to a menu (there is no menu).
						   appHandle,			// Application instance handle.
						   this);				// The value passed to the window through CREATESTRUCT's 
												// lpCreateParams member (used in WindowProc).

	// If the window creation failed, show a message and return failure.
	if(mHandle == 0)
	{
		std::stringstream ss;
		ss << "CreateWindow failed, error code: ";
		ss << GetLastError();

		ShowMessage(ss.str());
		return false;
	}

	// Show and update the window and return success.
	ShowWindow(mHandle, showStyle);
	UpdateWindow(mHandle);

	return true;
}

bool BasicWindow::ReadIniFile()
{
	int classStyle, bgColor, wndStyle, width, height, x, y;
	std::string caption;

	classStyle = GetPrivateProfileInt("Window", "class_style", 0,  "./D3D11.ini");
	bgColor = GetPrivateProfileInt("Window", "bgcolor", 0,  "./D3D11.ini");
	wndStyle = GetPrivateProfileInt("Window", "window_style", 0,  "./D3D11.ini");
	width = GetPrivateProfileInt("Window", "width", 0,  "./D3D11.ini");
	height = GetPrivateProfileInt("Window", "height", 0,  "./D3D11.ini");
	x = GetPrivateProfileInt("Window", "x", 0,  "./D3D11.ini");
	y = GetPrivateProfileInt("Window", "y", 0,  "./D3D11.ini");

	width = width <=0 ? CW_USEDEFAULT : width;
	height = height <=0 ? CW_USEDEFAULT : height;
	x = x <=0 ? CW_USEDEFAULT : x;
	y = y <=0 ? CW_USEDEFAULT : y;

	//GetPrivateProfileString("Window", "caption", "myWindow", caption, 48, "./D3D11.ini");

	return true;
}

LRESULT BasicWindow::MessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
{
	std::stringstream ss;
	switch(message)
	{
		case WM_LBUTTONDOWN:
			ss << "You picked: " << ShowMessage("Hello World", MsgSetup(MsgButtonSetup::YesNo));
			ShowMessage(ss.str());
			return 0;
		case WM_KEYDOWN:
			if(wParam == VK_ESCAPE)
				DestroyWindow(mHandle);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(mHandle, message, wParam, lParam);
}

MsgButton::Button BasicWindow::ShowMessage(std::string message, MsgSetup setup, std::string caption)
{
	// If the caption is unspecified, make it the same as the window's caption.
	if(caption == "")
		caption = mCaption;

	// Return the result of the message box, i.e. what button was pressed.
	return (MsgButton::Button)(MessageBox(0, message.c_str(), caption.c_str(), (UINT)setup.Buttons | (UINT)setup.Icon));
}

int BasicWindow::Run()
{
	MSG message = {0};

	BOOL msgReturn = 1;
	while((msgReturn = GetMessage(&message, 0, 0, 0)) != 0)
	{
		if(msgReturn == -1)
		{
			MessageBox(0, "GetMessage failed", 0, 0);
			break;
		}
		else
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	return (int)message.wParam;
}