#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <Windows.h>
#include <string>

namespace MsgButtonSetup
{
	enum Type
	{
		AbortRetryIgnore = MB_ABORTRETRYIGNORE,
		CancelTryCont = MB_CANCELTRYCONTINUE,
		Help = MB_HELP,
		Ok = MB_OK,
		OkCancel = MB_OKCANCEL,
		RetryCancel = MB_RETRYCANCEL,
		YesNo = MB_YESNO,
		YesNoCancel = MB_YESNOCANCEL
	};
}

namespace MsgIcons
{
	enum Icons
	{
		NoIcon = 0,
		Warning = MB_ICONWARNING,
		Information = MB_ICONINFORMATION,
		Question = MB_ICONQUESTION,
		Stop = MB_ICONSTOP
	};
}

namespace MsgButton
{
	enum Button
	{
		Ok = IDOK,
		Cancel = IDCANCEL,
		Abort = IDABORT,
		Retry = IDRETRY,
		Ignore = IDIGNORE,
		Yes = IDYES,
		No = IDNO,
		TryAgain = IDTRYAGAIN,
		Continue = IDCONTINUE
	};
}

struct MsgSetup
{
public:
	MsgButtonSetup::Type Buttons;
	MsgIcons::Icons Icon;

	MsgSetup(MsgButtonSetup::Type buttons = MsgButtonSetup::Ok, MsgIcons::Icons icon = MsgIcons::NoIcon) : Buttons(buttons), Icon(icon) {}
	MsgSetup(MsgIcons::Icons icon) : Buttons(MsgButtonSetup::Ok), Icon(icon) {}
};

class BasicWindow
{
public:
	BasicWindow();
	BasicWindow(HINSTANCE appHandle, int showStyle);

	virtual int Run();

protected:
	MsgButton::Button ShowMessage(std::string message, MsgSetup setup = MsgSetup(), std::string caption = "");
	virtual LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND mHandle;
	std::string mCaption;
	
	bool InitWindowsApp(HINSTANCE handle, int show);
	bool ReadIniFile();

	static LRESULT CALLBACK WindowProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif