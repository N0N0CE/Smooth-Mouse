// Smooth Mouse.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "interception.h"
#include <windows.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cmath>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <sstream>
#include <string>
#include <unordered_map>


#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

#define MAX_LOADSTRING 100

using namespace DirectX;

// Fonction pour charger le fichier de configuration dans un tableau (unordered_map)
std::unordered_map<std::string, float> loadConfig(const std::string& filename) {
    std::unordered_map<std::string, float> config;
    std::ifstream infile(filename);

    if (!infile.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier " << filename << std::endl;
        return config;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string key;
        float value;

        if (std::getline(iss, key, '=') && (iss >> value)) {
            config[key] = value;
        }
        else {
            std::cerr << "Erreur: ligne de format incorrect dans le fichier de configuration: " << line << std::endl;
        }
    }

    infile.close();
    return config;
}


// Structures
struct Vertex {
    XMFLOAT3 position;
};

// Variables globales :
HINSTANCE hInst;                                // instance actuelle
WCHAR szTitle[MAX_LOADSTRING];                  // Texte de la barre de titre
WCHAR szWindowClass[MAX_LOADSTRING];            // nom de la classe de fenêtre principale

// Déclarations anticipées des fonctions incluses dans ce module de code :
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Variables globales DirectX
IDXGISwapChain* swapChain;
ID3D11Device* device;
ID3D11DeviceContext* D3Dcontext;
ID3D11RenderTargetView* renderTargetView;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

void InitD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = 12;
    scd.BufferDesc.Height = 12;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &D3Dcontext);

    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
    pBackBuffer->Release();

    D3Dcontext->OMSetRenderTargets(1, &renderTargetView, nullptr);
}

void CleanD3D() {
    swapChain->Release();
    renderTargetView->Release();
    device->Release();
    D3Dcontext->Release();
}

void RenderFrame() {
    float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    D3Dcontext->ClearRenderTargetView(renderTargetView, clearColor);

    // Appel de la fonction à chaque rafraîchissement d'écran
    //MyFunction();

    swapChain->Present(1, 0);
}

/*struct average {
    int size;
    int ap = 0;
    float* array;
    float int_remainder = 0;

    average(int s) {
        array = new float[s]();
        size = s;
        ap = 0;
    }

    void add(float& x) {
        array[ap] = x;

        ap++;
        if (ap > size - 1) ap = 0;

        float ax = 0;

        for (int i = 0; i < size; i++) {
            ax += array[i];
        }

        x = ax / size;

        return;
    }

    void add_int(int& x) {
        float fx = x;
        this->add(fx);
        int ix = fx;
        int_remainder += fx - ix;
        if (int_remainder >= 1) {
            ix++;
            int_remainder--;
        }
        else if (int_remainder <= -1) {
            ix--;
            int_remainder++;
        }
        //if (abs(int_remainder) > 1) int_remainder = 0;
        x = ix;
    }
};

average amx(30);
average amy(30);*/

float mx = 0;
float my = 0;
int smooth_shift = -1;
int query = -1;
float decay_factor = 0.94;

InterceptionContext context;
InterceptionDevice v_device;


DWORD WINAPI D3D_thread(LPVOID lpParam) {
    float remainder_x = 0;
    float remainder_y = 0;


    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    timeBeginPeriod(1);
    InterceptionStroke stroke;

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = NULL;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    HWND hWnd = CreateWindowEx(0, L"WindowClass", L"DirectX Example", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, NULL, nullptr);
    ShowWindow(hWnd, SW_SHOW);

    InitD3D(hWnd);

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (smooth_shift == 1) {
                InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;

                float inc_x, inc_y;
                //float decaay = 0.94;// 0.95;//0.97;// -0.03;

                //std::cout << mx << "," << my << std::endl;

                inc_x = mx;
                inc_y = my;

                float dist = sqrt(mx * mx + my * my);
                if (dist == 0) {
                    SleepEx(1, FALSE);
                    continue;
                }
                float dist2 = dist * decay_factor;
                dist = dist2 / dist;

                mx *= dist;//0.97 - 0.03;
                my *= dist;//0.97-0.03;

                inc_x -= mx;
                inc_y -= my;

                remainder_x += inc_x;
                remainder_y += inc_y;

                int int_remainder_x = (int)remainder_x;
                int int_remainder_y = (int)remainder_y;


                mstroke.x = int_remainder_x;
                mstroke.y = int_remainder_y;

                remainder_x -= int_remainder_x;
                remainder_y -= int_remainder_y;

                interception_send(context, v_device, &stroke, 1);
            }

            else {
                SleepEx(60, FALSE);

                continue;
            }


            RenderFrame();

        }

          
    }

    CleanD3D();
    return msg.wParam;
}

/*
DWORD WINAPI mouse_thread(LPVOID lpParam) {
    float remainder_x = 0;
    float remainder_y = 0;


    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    timeBeginPeriod(1);
    InterceptionStroke stroke;

    LARGE_INTEGER liDueTime;
    liDueTime.QuadPart = -0LL;

    // Create an unnamed waitable timer.
    HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (NULL == hTimer)
    {
        printf("CreateWaitableTimer failed (%d)\n", GetLastError());
        return 1;
    }


    // Define the target frequency (120 Hz)
    const double targetFrequency = 120.001; //119.9975; //99.946; //59.951;
    // Calculate the target period in seconds
    const double targetPeriod = 1.0 / targetFrequency;

    // Get the frequency of the high-resolution performance counter
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    // Calculate the target period in counts
    const LONGLONG targetPeriodCounts = static_cast<LONGLONG>(targetPeriod * frequency.QuadPart);

    // Get the starting count
    LARGE_INTEGER startCount, currentCount;
    QueryPerformanceCounter(&startCount);

    while (true) {



        if (smooth_shift == 1) {
            InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;

            float inc_x, inc_y;
            float decay = 0.94;// 0.95;//0.97;// -0.03;

            //std::cout << mx << "," << my << std::endl;

            inc_x = mx;
            inc_y = my;

            float dist = sqrt(mx * mx + my * my);
            if (dist == 0) {
                SleepEx(1, FALSE);
                continue;
            }
            float dist2 = dist * decay;
            dist = dist2 / dist;

            mx *= dist;//0.97 - 0.03;
            my *= dist;//0.97-0.03;

            inc_x -= mx;
            inc_y -= my;

            remainder_x += inc_x;
            remainder_y += inc_y;

            int int_remainder_x = (int)remainder_x;
            int int_remainder_y = (int)remainder_y;


            mstroke.x = int_remainder_x;
            mstroke.y = int_remainder_y;

            remainder_x -= int_remainder_x;
            remainder_y -= int_remainder_y;

            //int x = mx;
            //int y = my;
            //mx = 0;
            //my = 0;
            
            //amx.add_int(x);
            //amy.add_int(y);
            ////std::cout << y << std::endl;

            //mstroke.x = x;
            //mstroke.y = y;

            interception_send(context, v_device, &stroke, 1);
        }

        else {
            SleepEx(60, FALSE);
            QueryPerformanceCounter(&startCount);

            continue;
        }

        if (query == 1) {
            SleepEx(50, FALSE);
            QueryPerformanceCounter(&startCount);
            query *= -1;
            continue;
        }

        // Get the current count
        QueryPerformanceCounter(&currentCount);

        // Calculate the elapsed time in counts
        LONGLONG elapsedCounts = currentCount.QuadPart - startCount.QuadPart;

        // Calculate the remaining time in counts
        LONGLONG remainingCounts = targetPeriodCounts - elapsedCounts;

        // Convert remaining counts to milliseconds
        double remainingMilliseconds = (remainingCounts * 1000.0) / frequency.QuadPart;

        if (remainingMilliseconds > 0) {
            // Sleep for the remaining time
            //SleepEx(static_cast<DWORD>(remainingMilliseconds),FALSE);
            liDueTime.QuadPart = remainingMilliseconds * -10000LL;
            SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
            WaitForSingleObject(hTimer, INFINITE);
        }

        // Update the start count for the next iteration
        //QueryPerformanceCounter(&startCount);
        startCount.QuadPart = startCount.QuadPart + targetPeriodCounts;

    }


    return 0;
}*/



int main()
{
    // Nom du fichier de configuration
    const std::string configFilename = "config.cfg";

    // Charger le fichier de configuration
    std::unordered_map<std::string, float> config = loadConfig(configFilename);

    // Afficher les paires clé-valeur chargées
    for (const auto& pair : config) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }

    int activate_key = config["activate_key_code"];
    int activate_mouse_code = config["activate_mouse_code"];
    decay_factor = config["decay_factor"];
    int start_on = config["start_on"];
    int keep_pressed = config["keep_pressed"];
    int activate_with_mouse = config["activate_with_mouse"]; 
    int activate_with_keyboard = config["activate_with_keyboard"];

    if (start_on == 1) smooth_shift = 1; else smooth_shift = 0;

    float fmx = 0;
    float fmy = 0;





    InterceptionDevice device;
    InterceptionStroke stroke;

    

    context = interception_create_context();

    if(activate_with_keyboard == 1) interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL);

    v_device = interception_wait(context);
    
    
    /*DWORD mouse_thread_id;
    HANDLE  mouse_thread_handle;
    mouse_thread_handle = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        mouse_thread,       // thread function name
        0,          // argument to thread function 
        0,                      // use default creation flags 
        &mouse_thread_id);   // returns the thread identifier 


    // Check the return value for success.
    // If CreateThread fails, terminate execution. 
    // This will automatically clean up threads and memory. 

    if (mouse_thread_handle == NULL)
    {
        //ErrorHandler(TEXT("CreateThread"));
        ExitProcess(3);
    }*/

    DWORD D3D_thread_id;
    HANDLE  D3D_thread_handle;
    D3D_thread_handle = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        D3D_thread,       // thread function name
        0,          // argument to thread function 
        0,                      // use default creation flags 
        &D3D_thread_id);   // returns the thread identifier 


    // Check the return value for success.
    // If CreateThread fails, terminate execution. 
    // This will automatically clean up threads and memory. 

    if (D3D_thread_handle == NULL)
    {
        //ErrorHandler(TEXT("CreateThread"));
        std::cout << "Could not create thread" << std::endl;
        ExitProcess(3);
    }

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {


        if (interception_is_mouse(device))
        {
            InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;

            if (mstroke.state != 0) std::cout << "Last pressed mouse button code:" << mstroke.state << std::endl;

            if (activate_with_mouse == 1) {
                if (mstroke.state == activate_mouse_code) {
                    smooth_shift = 1;
                }
                else if (mstroke.state == activate_mouse_code * 2) {
                    smooth_shift = 0;
                }
            }

            if (!(mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE) && smooth_shift == 1) {
                mx += mstroke.x;
                my += mstroke.y;
                mstroke.x = 0;
                mstroke.y = 0;




            }

            
            //else {
                interception_send(context, device, &stroke, 1);

            //}
            


            
        }


        if (activate_with_keyboard == 1 && interception_is_keyboard(device))
        {
            InterceptionKeyStroke& kstroke = *(InterceptionKeyStroke*)&stroke;

            interception_send(context, device, &stroke, 1);

            if (kstroke.state == INTERCEPTION_KEY_UP) std::cout << "Last pressed keyboard key code:" << kstroke.code << std::endl;
                                                                                      
            if (kstroke.code == activate_key) {
                //std::cout << "state:" << kstroke.state;
                if (kstroke.state ==  INTERCEPTION_KEY_DOWN) {
                    if(keep_pressed==1) if (start_on == 0) {
                        smooth_shift = 1;
                    }
                    else {
                        smooth_shift = 0;
                    }
                    
                    //mx = 0;
                    //my = 0;
                                                            
                }
                else if (kstroke.state == INTERCEPTION_KEY_UP) {
                    
                    if (keep_pressed == 1) if (start_on == 0) {
                        smooth_shift = 0;
                    }
                    else {
                        smooth_shift = 1;
                    }
                    if (keep_pressed == 0) {
                        if (smooth_shift == 1) smooth_shift = 0; else smooth_shift = 1;
                    }

                }
            }


            //if (kstroke.code == SCANCODE_ESC) break;
        }
        if (smooth_shift == 0) {
            mx = 0;
            my = 0;
        }
    }

    //CloseHandle(mouse_thread_handle);
    CloseHandle(D3D_thread_handle);
    interception_destroy_context(context);

    return 0;
}

