#include <windows.h>
#include <stdio.h>

int main(void)
{
    HANDLE hSerial;
    FILE *file=NULL;
    errno_t err;
    char buffer[256];
    DWORD bytesRead;

    // Open serial port COM14
    hSerial = CreateFileA(
        "\\\\.\\COM14",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("Error opening COM14\n");
        return 1;
    }

    // Configure UART settings
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(hSerial, &dcb))
    {
        printf("Error reading serial settings\n");
        CloseHandle(hSerial);
        return 1;
    }

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hSerial, &dcb))
    {
        printf("Error configuring UART\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Configure UART timeout
    COMMTIMEOUTS timeout = { 0 };

    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 100;
    timeout.ReadTotalTimeoutMultiplier = 10;

    SetCommTimeouts(hSerial, &timeout);

    // Open text file
    err = fopen_s(&file, "C:\\Documentos\\Sprint5_23.txt", "a");

    if (err != 0)
    {
        printf("Error opening text file\n");
        CloseHandle(hSerial);
        return 1;
    }

    printf("Listening to STM32 on COM14...\n");

    while (1)
    {
        if (ReadFile(
            hSerial,
            buffer,
            sizeof(buffer),
            &bytesRead,
            NULL
        ))
        {
            if (bytesRead > 0)
            {
                // Save received bytes to file
                fwrite(buffer, 1, bytesRead, file);

                // Display on terminal
                fwrite(buffer, 1, bytesRead, stdout);

                // Ensure data is written to disk
                fflush(file);
            }
        }
        else
        {
            printf("UART reading error\n");
            break;
        }
    }

    fclose(file);
    CloseHandle(hSerial);

    return 0;
}
