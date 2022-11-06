#include "Includes.h"

key_code TranslateWindowsKey(WPARAM WParam) {
    if(WParam >= 0x30 && WParam <= 0x39) return (key_code)(WParam - 48);
    if(WParam >= 0x41 && WParam <= 0x5A) return (key_code)(WParam - 55); 
    return key_none;
}

LRESULT CALLBACK WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) {
    LONG_PTR UserData = GetWindowLongPtr(WindowHandle, GWLP_USERDATA);
    platform *Platform = (platform *)UserData;
    
    if(Platform) {
        switch(Message) {
            case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
            case WM_KEYDOWN: {
                key_code Code = TranslateWindowsKey(WParam);
                if(Code != key_none) {
                    Platform->KeyPressed[Code] = Platform->KeyStates[Code] != 1;
                    Platform->KeyStates[Code] = 1;
                    if(GetKeyState(VK_SHIFT) & 0x8000) Platform->ShiftDown = true;
                    if(GetKeyState(VK_CONTROL) & 0x8000) Platform->CtrlDown = true;
                    if(GetKeyState(VK_MENU) & 0x8000) Platform->AltDown = true;
                }
            } break;
            
            case WM_KEYUP: {
                key_code Code = TranslateWindowsKey(WParam);
                if(Code != key_none) {
                    Platform->KeyStates[Code] = 0;
                    if(!(GetKeyState(VK_SHIFT) & 0x8000)) Platform->ShiftDown = false;
                    if(!(GetKeyState(VK_CONTROL) & 0x8000)) Platform->CtrlDown = false;
                    if(!(GetKeyState(VK_MENU) & 0x8000)) Platform->AltDown = false;
                }
            } break;
        }
    }
    
    return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

INT CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
    s8 *WindowClassName = "LibUtilsWindowClass";
    
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.lpfnWndProc = WindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = WindowClassName;
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    if(!RegisterClassEx(&WindowClass)) {
        PlatformFatalError("Failed to register window class: %s", GetWindowsErrorMessage());
    }
    
    s32 WindowWidth = 1366;
    s32 WindowHeight = 768;
    
    s32 ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    s32 ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    s32 WindowX = Max(0u, (ScreenWidth - WindowWidth) / 2);
    s32 WindowY = Max(0u, (ScreenHeight - WindowHeight) / 2);
    
    HWND WindowHandle = CreateWindowEx(0, WindowClassName, "LibUtils", WS_SYSMENU | WS_CAPTION,
                                       WindowX, WindowY,
                                       WindowWidth, WindowHeight,
                                       0, 0, Instance, 0);
    
    if(!WindowHandle) PlatformFatalError("Failed to create window: %s", GetWindowsErrorMessage());
    
    PIXELFORMATDESCRIPTOR PFD;
    memset(&PFD, 0, sizeof(PFD));
    
    PFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    PFD.nVersion = 1;
    PFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
    PFD.iPixelType = PFD_TYPE_RGBA;
    PFD.cColorBits = 24;
    PFD.cAlphaBits = 8;
    PFD.cDepthBits = 24;
    PFD.cStencilBits = 8;
    PFD.dwLayerMask = PFD_MAIN_PLANE;
    
    HDC DeviceContext = GetDC(WindowHandle);
    int Format = ChoosePixelFormat(DeviceContext, &PFD);
    if(!Format) PlatformFatalError("Failed to choose pixel format: %s", GetWindowsErrorMessage());
    SetPixelFormat(DeviceContext, Format, &PFD);
    
    HGLRC TempContext = wglCreateContext(DeviceContext);
    if(!TempContext) PlatformFatalError("Failed to create temp OpenGL context: %s", GetWindowsErrorMessage());
    
    wglMakeCurrent(DeviceContext, TempContext);
    
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    if(!wglCreateContextAttribsARB) PlatformFatalError("Failed to load wglCreateContextAttribsARB: %s", GetWindowsErrorMessage());
    
    int Attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB|WGL_CONTEXT_DEBUG_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC GLContext = wglCreateContextAttribsARB(DeviceContext, 0, Attributes);
    if(!GLContext) PlatformFatalError("Failed to create OpenGL context: %s", GetWindowsErrorMessage());
    
    wglMakeCurrent(0, 0);
    wglDeleteContext(TempContext);
    wglMakeCurrent(DeviceContext, GLContext);
    
    LoadOpenGLFunctions();
    
    platform Platform;
    SetWindowLongPtr(WindowHandle, GWLP_USERDATA, (LONG_PTR)&Platform);
    
    ShowWindow(WindowHandle, SW_SHOW);
    UpdateWindow(WindowHandle);
    
    TracyGpuContext;
    
    InitGame(&Platform);
    
    RECT Rect;
    GetWindowRect(WindowHandle, &Rect);
    
    bool Running = true;
    MSG Message = {};
    while(Running) {
        TracyBegin(PlatformFrame);
        
        TracyBegin(MessageLoop);
        if(PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
            if(Message.message == WM_QUIT) {
                Running = 0;
                break;
            }
            
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        TracyEnd(MessageLoop);
        
        f32 Now = PlatformTime();
        Platform.DeltaTime = Now - Platform.RealTime;
        Platform.RealTime = Now;
        Platform.GameTime = Now;
        
        POINT Pos;
        GetCursorPos(&Pos);
        Platform.MousePos = v2(Pos.x, Pos.y) - v2(Rect.left, Rect.top);
        
        TickGame(&Platform);
        
        
        SwapBuffers(DeviceContext);
        
        Zero(Platform.KeyPressed, sizeof(u8) * 256);
        
        TracyEnd(PlatformFrame);
        TracyGpuCollect;
        TracyCFrameMark;
    }
    
    return 0;
}

#ifndef TRACY_ENABLE
extern "C" int WinMainCRTStartup() {
    WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(0);
}
#endif