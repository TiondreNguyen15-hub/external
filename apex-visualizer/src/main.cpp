#include <windows.h>
#include <d3d11.h>
#include <algorithm>
#include <cstdio>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "projection.hpp"
using namespace visualizer;
static ID3D11Device* device{};
static ID3D11DeviceContext* context{};
static IDXGISwapChain* swapchain{};
static ID3D11RenderTargetView* target{};
static UINT resize_width{}, resize_height{};
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
static void drop_target() { if(target) { target->Release(); target=nullptr; } }
static bool make_target() {
    ID3D11Texture2D* buffer{};
    if(FAILED(swapchain->GetBuffer(0,IID_PPV_ARGS(&buffer)))) return false;
    HRESULT hr=device->CreateRenderTargetView(buffer,nullptr,&target);
    buffer->Release(); return SUCCEEDED(hr);
}
static void cleanup() {
    drop_target();
    if(swapchain) swapchain->Release();
    if(context) context->Release();
    if(device) device->Release();
}
static LRESULT WINAPI window_proc(HWND h,UINT msg,WPARAM w,LPARAM l) {
    if(ImGui_ImplWin32_WndProcHandler(h,msg,w,l)) return true;
    if(msg==WM_SIZE && w!=SIZE_MINIMIZED) { resize_width=LOWORD(l); resize_height=HIWORD(l); return 0; }
    if(msg==WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(h,msg,w,l);
}
static Player sample(float offset,int health) {
    Player p; p.health=health;
    p.bones={{{0,.65f,0},{0,.52f,0},{0,.3f,0},{0,-.05f,0},
      {-.14f,.46f,0},{-.23f,.2f,0},{-.29f,-.03f,0},
      {.14f,.46f,0},{.23f,.2f,0},{.29f,-.03f,0},
      {-.12f,-.35f,0},{-.15f,-.7f,0},{.12f,-.35f,0},{.15f,-.7f,0}}};
    for(auto& b:p.bones) { b.x=b.x*.65f+offset; b.y*=.75f; }
    return p;
}
static void render_ui() {
    static bool enabled=true, boxes=true, health=true, skeleton=true;
    static float thickness=2;
    static float color[4]={.1f,.85f,1,1};
    const auto display=ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos({0,0}); ImGui::SetNextWindowSize(display);
    ImGui::Begin("Visualizer",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextUnformatted("Player visualizer | SAMPLE DATA");
    ImGui::TextWrapped("Apex data source is not connected. Matrix and bone layouts require a verified adapter.");
    ImGui::Checkbox("Enable visuals",&enabled); ImGui::SameLine();
    ImGui::Checkbox("Boxes",&boxes); ImGui::SameLine();
    ImGui::Checkbox("Health",&health); ImGui::SameLine();
    ImGui::Checkbox("Skeleton",&skeleton);
    ImGui::SliderFloat("Line thickness",&thickness,1,4);
    ImGui::ColorEdit4("Visual color",color);
    ImVec2 origin=ImGui::GetCursorScreenPos(), size=ImGui::GetContentRegionAvail();
    if(size.x>0 && size.y>0) {
      auto* draw=ImGui::GetWindowDrawList();
      draw->AddRectFilled(origin,{origin.x+size.x,origin.y+size.y},IM_COL32(15,20,30,255));
      draw->PushClipRect(origin,{origin.x+size.x,origin.y+size.y},true);
      const Matrix matrix{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
      const ImU32 ink=ImGui::ColorConvertFloat4ToU32({color[0],color[1],color[2],color[3]});
      if(enabled) for(const auto& p:{sample(-.5f,100),sample(0,65),sample(.5f,25)}) {
        std::array<std::optional<Vec2>,14> points;
        float left=size.x, right=0, top=size.y, bottom=0; bool complete=true;
        for(size_t i=0;i<points.size();++i) {
          points[i]=project(p.bones[i],matrix,size.x,size.y);
          if(!points[i]) { complete=false; continue; }
          left=std::min(left,points[i]->x); right=std::max(right,points[i]->x);
          top=std::min(top,points[i]->y); bottom=std::max(bottom,points[i]->y);
        }
        if(skeleton) for(auto edge:links) if(points[edge[0]] && points[edge[1]]) {
          auto a=*points[edge[0]],b=*points[edge[1]];
          draw->AddLine({origin.x+a.x,origin.y+a.y},{origin.x+b.x,origin.y+b.y},ink,thickness);
        }
        if(!complete) continue;
        left+=origin.x-8; right+=origin.x+8; top+=origin.y-8; bottom+=origin.y+8;
        if(boxes) draw->AddRect({left,top},{right,bottom},ink,0,0,thickness);
        if(health) {
          float ratio=p.max_health>0 ? std::clamp(float(p.health)/p.max_health,0.f,1.f) : 0;
          draw->AddRectFilled({left-10,top},{left-5,bottom},IM_COL32(50,50,50,255));
          draw->AddRectFilled({left-10,bottom-(bottom-top)*ratio},{left-5,bottom},
            ImGui::ColorConvertFloat4ToU32({1-ratio,ratio,.15f,1}));
          char label[32]; std::snprintf(label,sizeof(label),"%d HP",p.health);
          draw->AddText({left,top-20},IM_COL32_WHITE,label);
        }
      }
      draw->PopClipRect();
    }
    ImGui::End();
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int) {
    WNDCLASSEXW wc{sizeof(wc),CS_CLASSDC,window_proc,0,0,instance,nullptr,nullptr,nullptr,nullptr,L"PlayerVisualizer",nullptr};
    if(!RegisterClassExW(&wc)) return 1;
    HWND window=CreateWindowW(wc.lpszClassName,L"Player Visualizer - Sample Data",WS_OVERLAPPEDWINDOW,100,100,1100,760,nullptr,nullptr,instance,nullptr);
    if(!window) { UnregisterClassW(wc.lpszClassName,instance); return 1; }
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount=2; desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT; desc.OutputWindow=window;
    desc.SampleDesc.Count=1; desc.Windowed=TRUE; desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
    HRESULT hr=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&desc,&swapchain,&device,nullptr,&context);
    if(FAILED(hr) || !make_target()) {
      MessageBoxW(window,L"Direct3D 11 initialization failed.",L"Visualizer",MB_OK|MB_ICONERROR);
      cleanup(); DestroyWindow(window); UnregisterClassW(wc.lpszClassName,instance); return 1;
    }
    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGui::StyleColorsDark();
    if(!ImGui_ImplWin32_Init(window)) { ImGui::DestroyContext(); cleanup(); DestroyWindow(window); UnregisterClassW(wc.lpszClassName,instance); return 1; }
    if(!ImGui_ImplDX11_Init(device,context)) { ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext(); cleanup(); DestroyWindow(window); UnregisterClassW(wc.lpszClassName,instance); return 1; }
    ShowWindow(window,SW_SHOWDEFAULT);
    bool running=true; int result=0;
    while(running) {
      MSG msg;
      while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {
        TranslateMessage(&msg); DispatchMessageW(&msg); if(msg.message==WM_QUIT) running=false;
      }
      if(!running) break;
      if(IsIconic(window)) { WaitMessage(); continue; }
      if(resize_width && resize_height) {
        context->OMSetRenderTargets(0,nullptr,nullptr); drop_target();
        hr=swapchain->ResizeBuffers(0,resize_width,resize_height,DXGI_FORMAT_UNKNOWN,0);
        resize_width=resize_height=0;
        if(FAILED(hr) || !make_target()) { result=1; break; }
      }
      ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame(); render_ui(); ImGui::Render();
      const float clear[4]={.04f,.04f,.06f,1};
      context->OMSetRenderTargets(1,&target,nullptr); context->ClearRenderTargetView(target,clear);
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
      hr=swapchain->Present(1,0); if(FAILED(hr)) { result=1; break; }
    }
    ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
    cleanup(); DestroyWindow(window); UnregisterClassW(wc.lpszClassName,instance); return result;
}
