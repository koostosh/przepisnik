
#include "app.hpp"

#include "textConstants.hpp"

#include <d3d11.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_sdl2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <cstdio>

class App : public AppI
{
public:
    ~App() override
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        SDL_DestroyWindow( m_sdlWindow );
        SDL_Quit();
    }

    bool EventLoop() override
    {
        SDL_Event event;
        while ( SDL_PollEvent( &event ) )
        {
            ImGui_ImplSDL2_ProcessEvent( &event );
            if ( event.type == SDL_QUIT )
                return false;
            if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID( m_sdlWindow ) )
                return false;
            if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID( m_sdlWindow ) )
            {
                CleanupRenderTarget();
                m_swapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 );
                CreateRenderTarget();
            }
        }
        return true;
    }

    bool BeginFrame() override
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        return true;
    }

    bool EndFrame() override
    {
        ImGui::Render();

        ImVec4 clear_color = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );
        const float clear_color_with_alpha[ 4 ] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        ID3D11RenderTargetView * mrtv_ptr = m_mainRenderTargetView.get();
        m_d3dDeviceContext->OMSetRenderTargets( 1, &mrtv_ptr, nullptr );
        m_d3dDeviceContext->ClearRenderTargetView( m_mainRenderTargetView.get(), clear_color_with_alpha );
        ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

        m_swapChain->Present( 1, 0 ); // Present with vsync
        //m_swapChain->Present(0, 0); // Present without vsync

        return true;
    }
private:
    friend std::unique_ptr<App> std::make_unique();
    App()
    {
        if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER ) != 0 )
        {
            std::exit( 1 );
        }
        SDL_SetHint( SDL_HINT_IME_SHOW_UI, "1" );

        m_sdlWindow = SDL_CreateWindow(
            language::bigWindowName.data(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1280, 720,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI );
        if ( m_sdlWindow == nullptr )
        {
            std::exit( 1 );
        }

        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );
        SDL_GetWindowWMInfo( m_sdlWindow, &wmInfo );
        HWND hwnd = ( HWND )wmInfo.info.win.window;

        // Initialize Direct3D
        if ( !CreateDeviceD3D( hwnd ) )
        {
            CleanupDeviceD3D();
            std::exit( 1 );
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO & io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForD3D( m_sdlWindow );
        ImGui_ImplDX11_Init( m_d3dDevice.get(), m_d3dDeviceContext.get() );
        static const ImWchar fontRanges[] =
        {
            0x0020, 0x017F, // Basic Latin + Latin-1 Supplement + Latin Extended-A
            0,
        };
        io.Fonts->AddFontFromFileTTF( "data/OpenSans-Medium.ttf", 15.0f, nullptr, fontRanges );
    }

    template <typename T>
    struct d3dDeleter
    {
        void operator()( T * item )
        {
            item->Release();
        }
    };
    template <typename T>
    using d3dPtr = std::unique_ptr<T, d3dDeleter<T>>;

    d3dPtr<ID3D11Device> m_d3dDevice;
    d3dPtr<ID3D11DeviceContext> m_d3dDeviceContext;
    d3dPtr<IDXGISwapChain> m_swapChain;
    d3dPtr<ID3D11RenderTargetView> m_mainRenderTargetView;
    SDL_Window * m_sdlWindow;

    bool CreateDeviceD3D( HWND hWnd )
    {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory( &sd, sizeof( sd ) );
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[ 2 ] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

        ID3D11Device * device;
        ID3D11DeviceContext * deviceContext;
        IDXGISwapChain * swapChain;

        if ( D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &deviceContext ) != S_OK )
            return false;

        m_d3dDevice.reset( device );
        m_d3dDeviceContext.reset( deviceContext );
        m_swapChain.reset( swapChain );

        CreateRenderTarget();
        return true;
    }
    void CleanupDeviceD3D()
    {
        m_mainRenderTargetView.reset();
        m_swapChain.reset();
        m_d3dDeviceContext.reset();
        m_d3dDevice.reset();
    }
    void CreateRenderTarget()
    {
        ID3D11Texture2D * pBackBuffer;
        m_swapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) );
        ID3D11RenderTargetView * mrtv = m_mainRenderTargetView.release();
        m_d3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &mrtv );
        m_mainRenderTargetView.reset( mrtv );
        pBackBuffer->Release();
    }
    void CleanupRenderTarget()
    {
        m_mainRenderTargetView.reset();
    }
};

std::unique_ptr<AppI> AppI::Create()
{
    return std::make_unique<App>();
}
