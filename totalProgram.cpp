#include <SFML/Graphics.hpp> 
#include <vector>  
#include <iostream>  
#include <sstream>  
#include <chrono>  
#include <thread>  
#include <fstream>  
#include "SerialClass.h"

// 보간 함수: 16x8 행렬을 32x16 행렬로 업스케일링
std::vector<std::vector<float>> bilinearInterpolation(const std::vector<std::vector<float>>& input, int newWidth, int newHeight) {
    int width = input[0].size();
    int height = input.size();
    std::vector<std::vector<float>> output(newHeight, std::vector<float>(newWidth, 0));

    for (int i = 0; i < newHeight; ++i) {
        float y = static_cast<float>(i) * (height - 1) / (newHeight - 1);
        int y0 = static_cast<int>(std::floor(y));
        int y1 = std::min(y0 + 1, height - 1);
        float yLerp = y - y0;

        for (int j = 0; j < newWidth; ++j) {
            float x = static_cast<float>(j) * (width - 1) / (newWidth - 1);
            int x0 = static_cast<int>(std::floor(x));
            int x1 = std::min(x0 + 1, width - 1);
            float xLerp = x - x0;

            float top = (1 - xLerp) * input[y0][x0] + xLerp * input[y0][x1];
            float bottom = (1 - xLerp) * input[y1][x0] + xLerp * input[y1][x1];
            output[i][j] = (1 - yLerp) * top + yLerp * bottom;
        }
    }

    return output;
}

int main() {
    // 시리얼 포트를 초기화하고 연결
    Serial serialPort("/dev/cu.usbmodem14101"); // 시리얼 포트의 경로를 지정

    // 시리얼 포트 연결 여부를 확인
    if (!serialPort.IsConnected()) {
        std::cerr << "Failed to connect to the serial port!" << std::endl;
        return -1;  // 연결 실패 시 프로그램 종료
    }

    const int width = 16;  // 행렬의 너비 (열의 수)
    const int height = 8;  // 행렬의 높이 (행의 수)
    const int newWidth = 32;  // 업스케일된 행렬의 너비
    const int newHeight = 16;  // 업스케일된 행렬의 높이

    // 16x8 크기의 캘리브레이션 행렬, 각 요소는 0과 1 사이의 값(255/(max-min))
    std::vector<std::vector<float>> calibrationMatrix = {
        {0.447, 0.337, 0.306, 0.212, 0.400, 0.257, 0.234, 0.190, 0.218, 0.268, 0.188, 0.204, 0.253, 0.400, 0.257, 0.161},
        {0.222, 0.231, 0.962, 0.367, 0.349, 0.440, 0.242, 0.180, 0.206, 0.212, 0.246, 0.253, 0.213, 0.360, 0.269, 0.264},
        {0.363, 0.214, 0.219, 0.276, 0.293, 0.343, 0.386, 0.210, 0.413, 0.329, 0.167, 0.269, 0.227, 0.444, 0.436, 0.281},
        {0.210, 0.183, 0.150, 0.186, 0.250, 0.148, 0.163, 0.193, 0.186, 0.169, 0.148, 0.264, 0.151, 0.241, 0.232, 0.210},
        {0.249, 0.198, 0.196, 0.175, 0.313, 0.318, 0.187, 0.245, 0.204, 0.208, 0.142, 0.211, 0.139, 0.198, 0.190, 0.206},
        {0.153, 0.173, 0.219, 0.200, 0.220, 0.193, 0.157, 0.179, 0.206, 0.162, 0.142, 0.229, 0.144, 0.157, 0.165, 0.166},
        {0.183, 0.183, 0.157, 0.168, 0.223, 0.211, 0.189, 0.159, 0.256, 0.227, 0.195, 0.245, 0.201, 0.224, 0.172, 0.187},
        {0.209, 0.297, 0.235, 0.276, 0.250, 0.259, 0.221, 0.197, 0.222, 0.198, 0.213, 0.214, 0.176, 0.201, 0.201, 0.198}
    };

    // 16x8 크기의 Minium 행렬 생성
    std::vector<std::vector<float>> minimumMatrix = {
        {1100, 1110, 1156, 1140, 1444, 1700, 1069, 1183, 1260, 1490, 1240, 990, 1480, 1720, 1300, 1010},
        {1570, 1440, 1380, 1330, 1320, 1830, 1440, 1130, 1020, 1360, 1270, 1320, 1300, 1705, 1540, 1430},
        {1580, 1320, 1030, 1250, 1010, 1250, 1200, 1050, 1430, 1470, 560, 1420, 1510, 1850, 1250, 1490},
        {1580, 1530, 1230, 1490, 1830, 1300, 1050, 1320, 1130, 1200, 1220, 1570, 1000, 1280, 1220, 1170},
        {1590, 1440, 1440, 1145, 1560, 1990, 1200, 890, 1340, 1270, 820, 1570, 700, 1160, 1230, 1330},
        {1020, 1410, 1830, 1510, 1140, 1640, 1100, 880, 1480, 1270, 860, 1360, 970, 1080, 1130, 1380},
        {790, 850, 1010, 1010, 1600, 1520, 1260, 1270, 1570, 1280, 1060, 1580, 1030, 1100, 890, 1380},
        {1130, 1170, 1030, 1250, 1510, 1730, 1290, 1370, 1350, 1360, 1240, 1150, 1040, 1350, 1340, 1160}
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
    std::ofstream upscaledFile("upscaled_output.txt"); // 업스케일된 데이터를 기록할 파일

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
                            result[i][j] = (array[i][j] - minimumMatrix[i][j])*calibrationMatrix[i][j];
                            if (result[i][j] < 0) {
                                result[i][j] = 0;
                            }
                            outFile << result[i][j] << " ";  // 파일에 기록
                        }
                        outFile << std::endl;  // 줄바꿈
                    }
                    outFile << std::endl;  // 추가 줄바꿈

                    // 결과 행렬을 업스케일링
                    std::vector<std::vector<float>> upscaledMatrix = bilinearInterpolation(result, newWidth, newHeight);

                    // 업스케일된 데이터를 텍스트 파일에 기록
                    for (const auto& row : upscaledMatrix) {
                        for (float value : row) {
                            upscaledFile << value << " ";
                        }
                        upscaledFile << std::endl;
                    }

                    // 디버깅을 위해 결과를 콘솔에 출력
                    for (int i = 0; i < newHeight; ++i) {
                        for (int j = 0; j < newWidth; ++j) {
                            std::cout << upscaledMatrix[i][j] << " ";
                        }
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;  // 줄바꿈

                    // 시각화를 위한 데이터 그리기
                    window.clear();  // 창을 지움 (새로고침)

                    float barWidth = window.getSize().x / static_cast<float>(newWidth);  // 각 바의 너비 계산
                    float barHeight = window.getSize().y / static_cast<float>(newHeight);  // 각 바의 높이 계산

                    for (int i = 0; i < newHeight; ++i) {
                        for (int j = 0; j < newWidth; ++j) {
                            sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight - 1));  // 바의 크기를 설정
                            bar.setPosition(j * barWidth, i * barHeight);  // 바의 위치 설정

                            int value = static_cast<int>(upscaledMatrix[i][j] * 255);  // 값을 시각화에 사용
                            sf::Color color;
                            color.r = value;  // 값이 클수록 빨간색 강도 증가
                            color.g = 255 - value;  // 값이 작을수록 초록색 강도 증가
                            color.b = 0;  // 파란색은 고정

                            bar.setFillColor(color);  // 바의 색상을 설정
                            window.draw(bar);  // 바를 창에 그림
                        }
                    }

                    window.display();  // 창을 업데이트하여 변경 사항을 반영
                }
            }
        }
    }

    outFile.close();  // 프로그램 종료 시 파일을 닫음
    upscaledFile.close();  // 업스케일된 데이터 파일을 닫음
    return 0;
}

/*
컴파일 및 실행할 수 있게 하는 코드
g++ -std=c++17 totalProgram.cpp Serial.cpp -o totalProgram -lsfml-graphics -lsfml-window -lsfml-system
./totalProgram
*/