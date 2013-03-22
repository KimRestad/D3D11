#include "GameWindow.hpp"
#include <cassert> // DEBUG: D3D11 code

GameWindow::GameWindow()
	: BasicWindow()
{
	D3D_FEATURE_LEVEL featLvl;
	UINT deviceCreateFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	deviceCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result = D3D11CreateDevice(NULL,						// Use default adapter.
									   D3D_DRIVER_TYPE_HARDWARE,	// Use hardware for rendering.
									   NULL,						// No software device (as hardware is used).
									   deviceCreateFlags,			// Creation flags as specified above.
									   0,							// No array of desired feature levels to check.
									   0,							// Zero elements in array of feature levels.
									   D3D11_SDK_VERSION,			// TODO: Why?
									   &mDevice,					// Out: The created device.
									   &featLvl,					// Out: The highest supported feature level.
									   &mContext);					// Out: The created context.

	if(FAILED(result))
		ShowMessage(L"CreateDevice failed");
	if(featLvl != D3D_FEATURE_LEVEL_11_0) // TODO: Support for D3D10?
		ShowMessage(L"Direct3D 11 is not supported");

	// Make sure there is 4X MSAA Quality support, should always be yes.
	UINT msaaQuality;
	mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaaQuality);
	assert(msaaQuality > 0);

	// Describe the swap chain.
	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Width = mWidth;							// Same width as the window.
	scDesc.BufferDesc.Height = mHeight;							// Same height as the window.
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// Back buffer pixel format, more than 24 bit colour
																// is wasted: monitors don't support it.
	scDesc.BufferDesc.RefreshRate.Numerator = 60;				// TODO: why?
	scDesc.BufferDesc.RefreshRate.Denominator = 1;				// TODO: why?
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// TODO: why?
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// TODO: why?

	scDesc.SampleDesc.Count = 1;							// If 4X MSAA then count = 4
	scDesc.SampleDesc.Quality = 0;							// If 4X MSAA then quality = msaaQuality - 1
	scDesc.BufferCount = 1;									// Only one back buffer in swap chain.
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// Use buffer as a render target.
	scDesc.Flags = 0;										// No additional flags.
	scDesc.OutputWindow = mHandle;							// Handle to the window to render to.
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// Let the display driver chooce presentation mode.
	scDesc.Windowed = true;									// Run windowed.

	// The factory used to create the device is accessed through dxgiDevice's parent dxgiAdapter's parent 
	// dxgiFactory. If they are not obtained an error message is showed.
	IDXGIDevice* dxgiDevice = 0;
	IDXGIAdapter* dxgiAdapter = 0;
	IDXGIFactory* dxgiFactory = 0;
	if(FAILED(mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice)))
		ShowMessage(L"Could not find IDXGIDevice");
	if(FAILED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter)))
		ShowMessage(L"Could not find the dxgiDevice parent (DXGIAdapter)");
	if(FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
		ShowMessage(L"Could not find the dxgiAdapter parent (DXGIFactory)");

	// Create the swap chain and show error message on failure.
	if(FAILED(dxgiFactory->CreateSwapChain(mDevice, &scDesc, &mSwapChain)))
		ShowMessage(L"Swap chain creation failed");
	
	// Release the temporary resources.
	ReleaseCOM(dxgiDevice)
	ReleaseCOM(dxgiAdapter)
	ReleaseCOM(dxgiFactory)

	// Get a pointer to the back buffer in order to create a render target view to it, then release it.
	ID3D11Texture2D* backBuffer;
	mSwapChain->GetBuffer(0,		// Which back buffer to use (if there are several, 0 is the first).
						  __uuidof(ID3D11Texture2D),				// Interface type of the buffer.
						  reinterpret_cast<void**>(&backBuffer));	// Out: The pointer to the back buffer.
	mDevice->CreateRenderTargetView(backBuffer,			// The resource to use as render target.
									0,					// Possible since the render target was created with a typed
														// format, this is default: create a view to the first  
														// mipmap level (back buffer only has one level).
									&mBackBufferRTV);	// Out: pointer to the created render target view.
	ReleaseCOM(backBuffer);

	// Describe the depth/stencil texture.
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = mWidth;							// Width of the texture - same as the window width;
	dsDesc.Height = mHeight;						// Height of the texture - same as the window height;
	dsDesc.ArraySize = 1;							// Number of textures in an array - only create one texture.
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// Use this texture as a depth stencil.
	dsDesc.CPUAccessFlags = 0;						// The CPU will not use this resource.
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24-bit [1,0] depth buffer with 8-bit stencil buffer [0, 255].
	dsDesc.MipLevels = 1;							// Only one is needed for a depth buffer.
	dsDesc.MiscFlags = 0;							// No other flags.
	dsDesc.Usage = D3D11_USAGE_DEFAULT;				// The GPU does all the reading and writing to the texture.
	dsDesc.SampleDesc.Count = 1;					// Must match the settings for teh render target (swap chain).
	dsDesc.SampleDesc.Quality = 0;					// Must match the settings for teh render target (swap chain).

	// Create the depth stencil texture and view, showing an error message if either fails.
	if(FAILED(mDevice->CreateTexture2D(&dsDesc,				// Description of the texture.
									   0,					// Initial data to fill the texture with, not needed.
									   &mDSBuffer)))		// Out: the created texture.
		ShowMessage(L"Depth stencil texture creation failed!");

	if(FAILED(mDevice->CreateDepthStencilView(mDSBuffer,	// The resource to create a view to.
											  0,			// DSV description can be 0 as (dsDesc) format is typed.
											  &mDSV)))		// Out: the created depth/stencil view.
		ShowMessage(L"Depth stencil view creation failed!");

	// Bind the depth/stencil and back buffer render target views to the output merger stage.
	mContext->OMSetRenderTargets(1,							// The number of render targets to bind.
								 &mBackBufferRTV,			// The first element to bind in a render target array.
								 mDSV);						// The pointer to the depth/stencil view.


}

int GameWindow::Run()
{
	MSG message = {0};

	// Run loop until a quit message is posted, then return the exit code 
	// (stored in wParam of a quit message).
	while(message.message != WM_QUIT)
	{
		// If there are messages from Windows, process and remove them,
		// else run game loop.
		if(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			Update();
			Render();
		}
	}

	return (int)message.wParam;
}

LRESULT GameWindow::MessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_LBUTTONDOWN:
#if defined(DEBUG)
			PrintDebugString(L"Hej världen!");
			ShowMessage(L"Vänsterklick");
#endif
			return 0;
		case WM_RBUTTONDOWN:
#if defined(DEBUG)
			ShowMessage(GetDebugWString());
#endif
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

void GameWindow::Update()
{
	
}

void GameWindow::Render()
{
}