#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include "SerialClass.h"

using namespace std;
using namespace std::chrono;

void saveArrayToFile(const vector<vector<int>>& array, const string& filename) {
    ofstream file(filename);
    if (file.is_open()) {
        for (const auto& row : array) {
            for (const auto& element : row) {
                file << element << " ";
            }
            file << "\n";
        }
        file.close();
        cout << "파일이 성공적으로 저장되었습니다.\n";
    } else {
        cerr << "파일을 열 수 없습니다.\n";
    }
}

int main() {
    int rows = 20, cols = 9;
    vector<vector<int>> array(rows, vector<int>(cols));
    string portName;

    portName = "/dev/tty.usbmodem142101"; // 시리얼 포트 이름 입력

    Serial arduino(portName.c_str()); // 아두이노 객체 생성

    if (!arduino.IsConnected()) { //예외처리
        cerr << "아두이노에 연결할 수 없습니다.\n";
        return 1;
    }

    cout << "20초 동안 아두이노에서 데이터를 읽어옵니다.\n";

    auto start_time = steady_clock::now(); // 시작 시간 (현재 시간)
    int element_count = 0;
    char buffer[4]; // 3자리 숫자 + NULL 문자 (최대 3자리 숫자까지)
    int value;

    while (duration_cast<seconds>(steady_clock::now() - start_time).count() < 20 && element_count < rows * cols) { // 20초 동안 데이터를 읽어옴
        int bytesRead = arduino.ReadData(buffer, sizeof(buffer) - 1); // NULL 문자를 위해 1을 빼줌
        if (bytesRead > 0) { // 데이터를 읽었을 때
            buffer[bytesRead] = '\0'; // NULL 문자 추가
            if (sscanf(buffer, "%d", &value) == 1) { // 숫자로 변환
                array[element_count / cols][element_count % cols] = value; // 배열에 저장
                element_count++; // 읽은 데이터 개수 증가
            }
        }
    }

    if (element_count != rows * cols) { // 예외처리
        cerr << "제한 시간 내에 충분한 데이터를 읽지 못했습니다.\n";
        return 1;
    }

    string filepath;
    filepath = "/Users/godhyunjong/Desktop/Work/School/Capstone_Design/text/textArray.txt";

    saveArrayToFile(array, filepath);

    return 0;
}

/**
 *  아두이노에서 데이터를 읽어와 배열로 저장하는 프로그램입니다.
 * 
 *  이 프로그램은 아두이노와 시리얼 통신을 통해 데이터를 읽어와 2차원 배열에 저장합니다.
 *          사용자로부터 아두이노의 시리얼 포트 이름과 배열을 저장할 파일 경로를 입력받습니다.
 *          아두이노로부터 20초 동안 데이터를 읽어오며, 제한 시간 내에 충분한 데이터를 읽지 못하면 오류가 발생합니다.
 *          읽어온 배열은 입력받은 파일 경로에 저장됩니다.
 * 
 *  이 프로그램은 SerialClass.h와 SerialClass.cpp 파일을 사용합니다.
 *  macOS에서 사용 방법
    시리얼 포트 확인: 아두이노가 연결된 시리얼 포트를 확인합니다. 예를 들어, /dev/tty.usbmodemXXXX 또는 /dev/tty.usbserial-XXXX와 같은 형태입니다.
    컴파일: g++ -std=c++17 textChanger_live.cpp Serial.cpp -o textChanger_live
    ./textChanger_live 

    포트 이름 : /dev/tty.usbmodem142101
    filepath : /Users/godhyunjong/Desktop/Work/School/Capstone Design/text
 */