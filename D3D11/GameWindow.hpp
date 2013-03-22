#ifndef GAME_WINDOW_HPP
#define GAME_WINDOW_HPP

#include <D3D11.h>
#include "BasicWindow.hpp"
#include "Utilities.hpp"

class GameWindow : public BasicWindow
{
public:
	GameWindow();

	virtual int Run();

protected:
	virtual LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void Update();
	virtual void Render();

private:
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mContext;
	IDXGISwapChain* mSwapChain;
	ID3D11RenderTargetView* mBackBufferRTV;
	ID3D11Texture2D* mDSBuffer;
	ID3D11DepthStencilView* mDSV;
};
#endif