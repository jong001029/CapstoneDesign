#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "SerialClass.h"

// 선형 보간 함수
float linearInterpolate(float v0, float v1, float t) {
    return v0 * (1 - t) + v1 * t;
}

// 평탄화 작업: 센서가 안정적인 상태일 때와 비교하여 시각화
sf::Color calculateColor(float value, float baseline, float threshold) {
    if (std::abs(value - baseline) <= threshold) {
        return sf::Color(0, 255, 0); // 초록색
    } else {
        int intensity = std::min(static_cast<int>(std::abs(value - baseline) * 255 / threshold), 255);
        return sf::Color(intensity, 0, 0); // 빨간색
    }
}

int main() {
    Serial serialPort("/dev/cu.usbmodem14101");

    if (!serialPort.IsConnected()) {
        std::cerr << "Failed to connect to the serial port!" << std::endl;
        return -1;
    }

    const int width = 16;
    const int height = 8;
    const int newWidth = 32;
    const int newHeight = 16;
    const float threshold = 10.0f; // 임계값, 이 값을 조정하여 민감도를 설정

    std::vector<std::vector<int>> array(height, std::vector<int>(width, 0));
    std::vector<std::vector<float>> upscaledArray(newHeight, std::vector<float>(newWidth, 0));
    std::vector<std::vector<float>> baselineArray(newHeight, std::vector<float>(newWidth, 0)); // 초기 기준점을 저장

    sf::RenderWindow window(sf::VideoMode(1600, 800), "Upscaled Visualization");

    auto lastTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds interval(10);

    std::string receivedData;
    bool baselineSet = false; // 기준점이 설정되었는지 확인

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (now - lastTime >= interval) {
            lastTime = now;

            char buffer[1024];
            int bytesRead = serialPort.ReadData(buffer, sizeof(buffer) - 1);

            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                receivedData += std::string(buffer);

                if (receivedData.find("-1") != std::string::npos) {
                    size_t pos = receivedData.find("-1");
                    std::string matrixData = receivedData.substr(0, pos);
                    receivedData = receivedData.substr(pos + 2);

                    std::istringstream ss(matrixData);
                    for (int i = 0; i < height; ++i) {
                        for (int j = 0; j < width; ++j) {
                            if (!(ss >> array[i][j])) {
                                array[i][j] = 0;
                            }
                        }
                    }

                    // 업스케일링 작업
                    for (int i = 0; i < newHeight; ++i) {
                        for (int j = 0; j < newWidth; ++j) {
                            float x = static_cast<float>(j) * (width - 1) / (newWidth - 1);
                            float y = static_cast<float>(i) * (height - 1) / (newHeight - 1);

                            int x0 = static_cast<int>(x);
                            int y0 = static_cast<int>(y);
                            int x1 = std::min(x0 + 1, width - 1);
                            int y1 = std::min(y0 + 1, height - 1);

                            float xFraction = x - x0;
                            float yFraction = y - y0;

                            float top = linearInterpolate(array[y0][x0], array[y0][x1], xFraction);
                            float bottom = linearInterpolate(array[y1][x0], array[y1][x1], xFraction);
                            upscaledArray[i][j] = linearInterpolate(top, bottom, yFraction);
                        }
                    }

                    // 기준점이 설정되지 않았다면 현재 값을 기준점으로 설정
                    if (!baselineSet) {
                        baselineArray = upscaledArray;
                        baselineSet = true;
                    }

                    // 시각화
                    window.clear();

                    float barWidth = window.getSize().x / static_cast<float>(newWidth);
                    float barHeight = window.getSize().y / static_cast<float>(newHeight);

                    for (int i = 0; i < newHeight; ++i) {
                        for (int j = 0; j < newWidth; ++j) {
                            sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight - 1));
                            bar.setPosition(j * barWidth, i * barHeight);

                            // 기준점과 비교하여 색상 설정
                            sf::Color color = calculateColor(upscaledArray[i][j], baselineArray[i][j], threshold);
                            bar.setFillColor(color);
                            window.draw(bar);
                        }
                    }

                    window.display();
                }
            }
        }
    }

    return 0;
}


/*
컴파일 및 실행할 수 있게 하는 코드
g++ -std=c++17 upscale_dtw_time_series.cpp Serial.cpp -o upscale_dtw_time_series -lsfml-graphics -lsfml-window -lsfml-system
./upscale_dtw_time_series
*/