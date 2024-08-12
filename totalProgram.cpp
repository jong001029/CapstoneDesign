#include <SFML/Graphics.hpp> 
#include <vector>  
#include <iostream>  
#include <sstream>  
#include <chrono>  
#include <thread>  
#include <fstream>  
#include "SerialClass.h" 

//using namespace std;

int main() {
    // 시리얼 포트를 초기화하고 연결
    Serial serialPort("/dev/cu.usbmodem14201"); // 시리얼 포트의 경로를 지정

    // 시리얼 포트 연결 여부를 확인
    if (!serialPort.IsConnected()) {
        std::cerr << "Failed to connect to the serial port!" << std::endl;
        return -1;  // 연결 실패 시 프로그램 종료
    }

    const int width = 16;  // 행렬의 너비 (열의 수)
    const int height = 8;  // 행렬의 높이 (행의 수)

    // 16x8 크기의 캘리브레이션 행렬, 각 요소는 0과 1 사이의 값
    std::vector<std::vector<float>> calibrationMatrix = {
        {0.118, 0.147, 0.193, 0.153, 0.178, 0.145, 0.126, 0.110, 0.119, 0.156, 0.144, 0.199, 0.119, 0.128, 0.243, 0.144},
        {0.214, 0.376, 0.322, 0.301, 0.211, 0.289, 0.120, 0.114, 0.479, 0.157, 0.254, 0.173, 0.225, 0.201, 0.266, 0.377},
        {0.123, 0.092, 0.139, 0.148, 0.267, 0.233, 0.228, 0.192, 0.353, 0.244, 0.195, 0.358, 0.280, 0.191, 0.328, 0.139},
        {0.169, 0.410, 0.309, 0.224, 0.180, 0.235, 0.151, 0.258, 0.090, 0.189, 0.208, 0.146, 0.121, 0.142, 0.203, 0.381},
        {0.254, 0.239, 0.152, 0.301, 0.319, 0.179, 0.324, 0.181, 0.320, 0.214, 0.188, 0.188, 0.266, 0.220, 0.176, 0.153},
        {0.185, 0.166, 0.170, 0.155, 0.129, 0.177, 0.123, 0.173, 0.221, 0.201, 0.158, 0.191, 0.122, 0.136, 0.161, 0.218},
        {0.139, 0.164, 0.198, 0.143, 0.182, 0.133, 0.159, 0.118, 0.125, 0.147, 0.129, 0.169, 0.165, 0.189, 0.146, 0.147},
        {0.183, 0.149, 0.142, 0.215, 0.177, 0.195, 0.188, 0.125, 0.127, 0.117, 0.132, 0.129, 0.107, 0.102, 0.101, 0.117}
    };

    // 16x8 크기의 데이터 배열 초기화, 모든 값을 0으로 설정
    std::vector<std::vector<int>> array(height, std::vector<int>(width, 0));

    // 시각화를 위한 SFML 윈도우 생성
    sf::RenderWindow window(sf::VideoMode(1600, 800), "Visualization Program");

    // 마지막 시간을 기록하기 위한 변수 초기화
    auto lastTime = std::chrono::steady_clock::now();  
    const std::chrono::milliseconds interval(10);  // 10ms마다 데이터를 읽음

    std::string receivedData;  // 수신된 데이터를 저장하기 위한 문자열

    std::ofstream outFile("output.txt");  // 파일을 열어 데이터를 기록할 준비

    // 메인 루프, 창이 열려 있는 동안 실행
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();  // 창을 닫는 이벤트가 발생하면 창을 닫음
            }
        }

        auto now = std::chrono::steady_clock::now();  // 현재 시간을 가져옴
        if (now - lastTime >= interval) {  // 설정된 간격(10ms)이 경과했는지 확인
            lastTime = now;  // 마지막 시간 업데이트

            // 시리얼 포트에서 데이터를 읽어옴
            char buffer[1024];  // 읽어들일 데이터를 저장할 버퍼
            int bytesRead = serialPort.ReadData(buffer, sizeof(buffer) - 1);  // 데이터를 읽음

            if (bytesRead > 0) {  // 데이터를 읽어들였는지 확인
                buffer[bytesRead] = '\0';  // 버퍼의 마지막에 널 문자를 추가하여 문자열로 만듦
                receivedData += std::string(buffer);  // 읽은 데이터를 저장

                // 데이터에서 "-1"을 찾음, 이는 행렬의 끝을 나타냄
                if (receivedData.find("-1") != std::string::npos) {
                    size_t pos = receivedData.find("-1");  // "-1"의 위치를 찾음
                    std::string matrixData = receivedData.substr(0, pos);  // "-1" 이전의 데이터를 분리
                    receivedData = receivedData.substr(pos + 2);  // "-1" 이후의 데이터를 남김

                    // 받은 데이터를 스트림으로 변환하여 행렬에 저장
                    std::istringstream ss(matrixData);
                    for (int i = 0; i < height; ++i) {  // 8줄에 대해 반복
                        for (int j = 0; j < width; ++j) {  // 각 줄에서 16개의 데이터를 처리
                            if (!(ss >> array[i][j])) {  // 데이터를 읽어들임, 실패하면 0으로 초기화
                                array[i][j] = 0;
                            }
                        }
                    }

                    // 결과 행렬을 계산, 캘리브레이션 행렬과 곱함
                    std::vector<std::vector<float>> result(height, std::vector<float>(width, 0.0f));

                    for (int i = 0; i < height; ++i) {
                        for (int j = 0; j < width; ++j) {
                            result[i][j] = array[i][j] * calibrationMatrix[i][j];  // 결과값 계산
                            outFile << result[i][j] << " ";  // 파일에 기록
                        }
                        outFile << std::endl;  // 줄바꿈
                    }
                    outFile << std::endl;  // 추가 줄바꿈

                    // 디버깅을 위해 결과를 콘솔에 출력
                    for (int i = 0; i < height; ++i) {
                        for (int j = 0; j < width; ++j) {
                            std::cout << result[i][j] << " ";
                        }
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;  // 줄바꿈
                }
            }
        }

        window.clear();  // 창을 지움 (새로고침)

        // 데이터를 시각화
        float barWidth = window.getSize().x / width;  // 각 바의 너비 계산
        float barHeight = window.getSize().y / height;  // 각 바의 높이 계산
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight - 1));  // 바의 크기를 설정
                bar.setPosition(j * barWidth, i * barHeight);  // 바의 위치 설정

                int value = static_cast<int>(array[i][j] * calibrationMatrix[i][j]);  // 캘리브레이션 행렬과 곱한 값을 시각화에 사용
                sf::Color color;
                if (value == 0) {
                    color = sf::Color(128, 128, 128);  // 값이 0일 경우 회색
                } else {
                    color.r = value;  // 값이 클수록 빨간색 강도 증가
                    color.g = 255 - value;  // 값이 작을수록 초록색 강도 증가
                    color.b = 0;  // 파란색은 고정
                }

                bar.setFillColor(color);  // 바의 색상을 설정
                window.draw(bar);  // 바를 창에 그림
            }
        }

        window.display();  // 창을 업데이트하여 변경 사항을 반영
    }

    outFile.close();  // 프로그램 종료 시 파일을 닫음
    return 0;
}
