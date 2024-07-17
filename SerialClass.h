#ifndef SERIALCLASS_H_INCLUDED
#define SERIALCLASS_H_INCLUDED

#define ARDUINO_WAIT_TIME 2000 // 아두이노와 연결되는 시간

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <fcntl.h> // 리눅스 환경에서는 이 라이브러리를 추가해야 함
#include <errno.h> 
#include <termios.h> // 시리얼 통신을 위한 라이브러리
#include <unistd.h> // 유닉스 표준
#include <string.h>
#endif

#include <stdio.h>
#include <stdlib.h>

class Serial
{
    private:
        #if defined(_WIN32) || defined(_WIN64) // 윈도우 환경일 경우에
        HANDLE hSerial;
        COMSTAT status;
        DWORD errors;
        #else
        int fd; // 리눅스 환경일 경우에
        #endif
        bool connected; // 연결 상태

    public:
        Serial(const char *portName); //생성자와 소멸자 정의 (Class이므로)
        ~Serial();
        int ReadData(char *buffer, unsigned int nbChar); // 데이터를 읽는 함수
        bool WriteData(const char *buffer, unsigned int nbChar); // 데이터를 쓰는 함수
        bool IsConnected(); // 연결 되었는지 안 되었는지 (Bool형)
};

#endif // SERIALCLASS_H_INCLUDED
