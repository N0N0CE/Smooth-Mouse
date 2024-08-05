// Smooth Mouse.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "interception.h"
#include <windows.h>
#include <stdlib.h>
#include <fstream>
#include <string>

struct average {
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
average amy(30);

int mx = 0;
int my = 0;
int smooth_shift = -1;
int query = -1;

InterceptionContext context;
InterceptionDevice v_device;


DWORD WINAPI mouse_thread(LPVOID lpParam) {



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
    const double targetFrequency = 119.9975; //99.946;
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
            int x = mx;
            int y = my;
            mx = 0;
            my = 0;
            
            amx.add_int(x);
            amy.add_int(y);
            //std::cout << y << std::endl;

            mstroke.x = x;
            mstroke.y = y;

            interception_send(context, v_device, &stroke, 1);
        }

        else {
            SleepEx(50, FALSE);
            QueryPerformanceCounter(&startCount);
            continue;
        }

        if (query == 1) {
            QueryPerformanceCounter(&startCount);
            query *= -1;
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
}

int main()
{





    InterceptionDevice device;
    InterceptionStroke stroke;

    

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE | INTERCEPTION_MOUSE_BUTTON_2_UP | INTERCEPTION_MOUSE_BUTTON_2_DOWN | INTERCEPTION_MOUSE_BUTTON_1_DOWN);

    v_device = interception_wait(context);
    
    DWORD mouse_thread_id;
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
    }

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {


        if (interception_is_mouse(device))
        {
            InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;

            if (mstroke.state == INTERCEPTION_MOUSE_BUTTON_1_DOWN) {
                query = 1;
            }
            if (mstroke.state == INTERCEPTION_MOUSE_BUTTON_2_DOWN) {
                smooth_shift = 1;
            }
            else if (mstroke.state == INTERCEPTION_MOUSE_BUTTON_2_UP) {
                smooth_shift = 0;
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


        if (interception_is_keyboard(device))
        {
            InterceptionKeyStroke& kstroke = *(InterceptionKeyStroke*)&stroke;

            interception_send(context, device, &stroke, 1);

            //std::cout << "key:" << kstroke.code << std::endl;
                                                                                      
            if (kstroke.code == 57) {
                //std::cout << "state:" << kstroke.state;
                if (kstroke.state ==  INTERCEPTION_KEY_DOWN) {
                    smooth_shift = 1;
                    mx = 0;
                    my = 0;
                                                            
                }
                else if (kstroke.state == INTERCEPTION_KEY_UP) {
                    smooth_shift = 0;

                }
            }


            //if (kstroke.code == SCANCODE_ESC) break;
        }
    }

    CloseHandle(mouse_thread_handle);
    interception_destroy_context(context);

    return 0;
}

