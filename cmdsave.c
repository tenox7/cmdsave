/*++

Copyright (c) 2014 by Antoni Sawicki

Module Name:

    cmdsave.c

Abstract:

    Dump contents of cmd.exe console window buffer to a file.

Version:

    1.0 - initial release

Author:

    Antoni Sawicki <as@tenoware.com>

License:
   
    BSD

--*/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


void ERRPT(char *msg, ...) {
    va_list valist;
    char vaBuff[1024];
    char errBuff[1024];
    DWORD err;

    va_start(valist, msg);
    _vsnprintf(vaBuff, sizeof(vaBuff), msg, valist);
    va_end(valist);
    printf("ERROR: %s\n", vaBuff);
    err=GetLastError();
    if(err) {
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errBuff, sizeof(errBuff) , NULL );
        printf("%08X : %s\n\n", err, errBuff);
    }    
    ExitProcess(1);
}


int main(int argc, char **argv) {
    HANDLE hConsole;
    HANDLE hFile;
    COORD start;
    LPSTR buff;
    CONSOLE_SCREEN_BUFFER_INFO cbi;
    DWORD rchr, ochr, n;
    LONG l;

    if(argc!=2)
        ERRPT("\rUsage: %s <filename>\n", argv[0]);

    hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsole==INVALID_HANDLE_VALUE)
        ERRPT("GetStdHandle" );

    if(GetConsoleScreenBufferInfo(hConsole, &cbi)==0)
        ERRPT("GetConsoleScreenBufferInfo");

    buff=HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (cbi.dwSize.X+2)*sizeof(WCHAR));
    if(buff==NULL)
        ERRPT("HeapAlloc");

    hFile=CreateFile(argv[1], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile==INVALID_HANDLE_VALUE)
        ERRPT("CreateFile %s", argv[1]);

    start.X=0;
    start.Y=0;

    while(start.Y<cbi.dwSize.Y) {

        if(ReadConsoleOutputCharacter(hConsole, buff, cbi.dwSize.X, start, &rchr)==0)
            ERRPT("ReadConsoleOutputCharacter [rchr=%d, y:%d]", rchr, start.Y);

        // trim horizontal
        n=cbi.dwSize.X-1;
        while(n && buff[n]==' ') 
            n--;

        if(n==0)
            n=-1;

        buff[n+1]='\n';
        buff[n+2]='\0';

        if(WriteFile(hFile, buff, strlen(buff), &ochr, NULL)==0)
            ERRPT("WriteFile");

        // for vertical trim
        if(buff[0]!='\n')
            l=GetFileSize(hFile, NULL);
       
        start.Y++;
    }

    // trim vertical
    SetFilePointer(hFile, l, NULL, FILE_BEGIN);
    SetEndOfFile(hFile);

    CloseHandle(hFile);

    return 0;
}

