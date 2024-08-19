#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include "SerialClass.h"

using namespace std;
using namespace std::chrono;

void saveArrayToFile(const vector<vector<int>>& array, ofstream& file) {
    if (file.is_open()) {
        for (const auto& row : array) {
            for (const auto& element : row) {
                file << element << " ";
            }
            file << "\n";
        }
        file << "\n"; // 행렬 간 줄 바꿈 추가
        file.flush(); // 버퍼를 비워서 데이터를 즉시 파일에 씁니다.
        cout << "데이터가 실시간으로 저장되었습니다.\n";
    } else {
        cerr << "파일을 열 수 없습니다.\n";
    }
}

int main() {
    int rows = 8, cols = 16;
    vector<vector<int>> array(rows, vector<int>(cols));
    string portName;

    portName = "/dev/cu.usbmodem142101"; // 시리얼 포트 이름 입력

    Serial arduino(portName.c_str()); // 아두이노 객체 생성

    if (!arduino.IsConnected()) { // 예외처리
        cerr << "아두이노에 연결할 수 없습니다.\n";
        return 1;
    }

    cout << "20초 동안 아두이노에서 데이터를 실시간으로 읽어와 저장합니다.\n";

    auto start_time = steady_clock::now(); // 시작 시간 (현재 시간)
    char buffer[16]; // 충분한 크기로 버퍼를 설정
    int value;

    string filepath = "/Users/godhyunjong/Desktop/Work/School/Capstone_Design/text/text.txt";
    ofstream file(filepath);

    if (!file.is_open()) {
        cerr << "파일을 열 수 없습니다.\n";
        return 1;
    }

    while (duration_cast<seconds>(steady_clock::now() - start_time).count() < 20) { // 20초 동안 데이터를 읽어옴
        int element_count = 0;
        while (element_count < rows * cols) {
            int bytesRead = arduino.ReadData(buffer, sizeof(buffer) - 1); // 데이터를 읽어옴
            if (bytesRead > 0) { // 데이터를 읽었을 때
                buffer[bytesRead] = '\0'; // NULL 문자 추가
                char* ptr = buffer;
                while (*ptr) {
                    if (sscanf(ptr, "%d", &value) == 1) { // 숫자로 변환
                        array[element_count / cols][element_count % cols] = value; // 배열에 저장
                        element_count++; // 읽은 데이터 개수 증가
                        if (element_count == rows * cols) {
                            break;
                        }
                    }
                    while (*ptr && *ptr != ' ') ptr++; // 다음 숫자로 이동
                    if (*ptr) ptr++;
                }
            }
        }
        saveArrayToFile(array, file); // 읽은 데이터를 파일에 저장
    }

    file.close();
    cout << "20초 동안의 데이터 저장이 완료되었습니다.\n";

    return 0;
}

/**
 *  아두이노에서 데이터를 읽어와 16x8 배열로 저장하는 프로그램입니다.
 * 
 *  이 프로그램은 아두이노와 시리얼 통신을 통해 데이터를 읽어와 2차원 배열에 저장합니다.
 *  아두이노로부터 20초 동안 데이터를 실시간으로 읽어와 배열에 저장하고, 이를 지정된 파일 경로에 실시간으로 저장합니다.
 * 
 *  이 프로그램은 SerialClass.h와 SerialClass.cpp 파일을 사용합니다.
 * 
 *  사용 방법 (macOS 기준):
 *    1. 시리얼 포트 확인: 아두이노가 연결된 시리얼 포트를 확인합니다.
 *       예를 들어, /dev/tty.usbmodemXXXX 또는 /dev/tty.usbserial-XXXX와 같은 형태입니다.
 * 
 *   2. 파일 경로 확인: 데이터를 저장할 파일의 경로를 확인합니다. 
 *       (포트 이름: /dev/tty.usbmodem142101)
 *       (파일 경로: /Users/godhyunjong/Desktop/Work/School/Capstone_Design/text/textArray4.txt)
 * 
 *  3. 프로그램 실행: 터미널에서 프로그램을 실행합니다 
 *   g++ -std=c++17 textChanger_live.cpp Serial.cpp -o textChanger_live 
 ./textChanger_live
 */
