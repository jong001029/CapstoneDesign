#include "SerialClass.h"

Serial::Serial(const char *portName) // 생성자
{
    this->connected = false;

    #if defined(_WIN32) || defined(_WIN64) //윈도우 환경일 경우에는 아래의 코드
    this->hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (this->hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            printf("ERROR: Handle was not attached. Reason: %s not available.\n", portName);
        }
        else
        {
            printf("ERROR!!!");
        }
    }
    else
    {
        DCB dcbSerialParams = {0};

        if (!GetCommState(this->hSerial, &dcbSerialParams))
        {
            printf("failed to get current serial parameters!");
        }
        else
        {
            dcbSerialParams.BaudRate = CBR_9600;
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

            if (!SetCommState(hSerial, &dcbSerialParams))
            {
                printf("ALERT: Could not set Serial Port parameters");
            }
            else
            {
                this->connected = true;
                PurgeComm(this->hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
                Sleep(ARDUINO_WAIT_TIME);
            }
        }
    }
    #else
    this->fd = open(portName, O_RDWR | O_NOCTTY | O_NDELAY); // 리눅스 환경일 경우에는 이 코드를 사용
    if (this->fd == -1) // 파일 디스크립터가 -1이면 오류
    {
        perror("시리얼 포트를 열 수 없습니다."); //오류 메시지 출력
    }
    else
    {
        fcntl(this->fd, F_SETFL, 0); // 파일 디스크립터의 플래그를 0으로 설정

        struct termios options; // 시리얼 통신을 위한 구조체
        tcgetattr(this->fd, &options); // 시리얼 포트의 속성을 가져옴
        cfsetispeed(&options, B9600); // 입력 속도를 9600으로 설정
        cfsetospeed(&options, B9600); // 출력 속도를 9600으로 설정
        options.c_cflag |= (CLOCAL | CREAD); // 로컬 연결, 읽기 활성화
        options.c_cflag &= ~PARENB; // 패리티 비활성화
        options.c_cflag &= ~CSTOPB; // 1비트 중지 비트
        options.c_cflag &= ~CSIZE; // 문자 크기 비트를 0으로 설정
        options.c_cflag |= CS8; // 8비트 문자 크기
        tcsetattr(this->fd, TCSANOW, &options); // 시리얼 포트의 속성을 설정

        this->connected = true; // 연결 상태를 true로 설정
        usleep(ARDUINO_WAIT_TIME * 1000); // 2000ms 대기
    }
    #endif
}

Serial::~Serial() // 소멸자
{
    if (this->connected)
    {
        this->connected = false;
        #if defined(_WIN32) || defined(_WIN64)
        CloseHandle(this->hSerial);
        #else
        close(this->fd);
        #endif
    }
}

int Serial::ReadData(char *buffer, unsigned int nbChar)
{
    #if defined(_WIN32) || defined(_WIN64)
    DWORD bytesRead;
    unsigned int toRead;

    ClearCommError(this->hSerial, &this->errors, &this->status);

    if (this->status.cbInQue > 0)
    {
        toRead = (this->status.cbInQue > nbChar) ? nbChar : this->status.cbInQue;

        if (ReadFile(this->hSerial, buffer, toRead, &bytesRead, NULL))
        {
            return bytesRead;
        }
    }

    return 0;
    #else
    int bytesRead = read(this->fd, buffer, nbChar); // 시리얼 포트로부터 데이터를 읽음
    return (bytesRead < 0) ? -1 : bytesRead; // 읽은 데이터의 바이트 수를 반환 (bytesRead가 0보다 작으면 -1을 반환)
    #endif
}

bool Serial::WriteData(const char *buffer, unsigned int nbChar)
{
    #if defined(_WIN32) || defined(_WIN64)
    DWORD bytesSend;

    if (!WriteFile(this->hSerial, (void *)buffer, nbChar, &bytesSend, 0))
    {
        ClearCommError(this->hSerial, &this->errors, &this->status);
        return false;
    }
    return true;
    #else
    int bytesSent = write(this->fd, buffer, nbChar);
    return (bytesSent < 0) ? false : true;
    #endif
}

bool Serial::IsConnected()
{
    return this->connected;
}
