#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <fstream>
#include <cmath>
#include "SerialClass.h"

// 선형 보간 함수
float linearInterpolate(float v0, float v1, float t) {
    return v0 * (1 - t) + v1 * t;
}

// DTW 알고리즘을 사용하여 두 행렬 간의 유사성을 측정
float calculateDTW(const std::vector<std::vector<float>>& matrix1, const std::vector<std::vector<float>>& matrix2) {
    int n = matrix1.size();
    int m = matrix2.size();
    int d = matrix1[0].size(); // Assuming both matrices have the same width
    
    // DP 테이블 초기화
    std::vector<std::vector<float>> dtw(n, std::vector<float>(m, std::numeric_limits<float>::infinity()));
    dtw[0][0] = 0;

    for (int i = 1; i < n; ++i) {
        for (int j = 1; j < m; ++j) {
            float cost = 0;
            for (int k = 0; k < d; ++k) {
                cost += std::abs(matrix1[i][k] - matrix2[j][k]);
            }
            dtw[i][j] = cost + std::min({dtw[i-1][j], dtw[i][j-1], dtw[i-1][j-1]});
        }
    }

    return dtw[n-1][m-1];
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

    std::vector<std::vector<int>> array(height, std::vector<int>(width, 0));
    std::vector<std::vector<float>> upscaledArray(newHeight, std::vector<float>(newWidth, 0));
    std::vector<std::vector<float>> previousUpscaledArray(newHeight, std::vector<float>(newWidth, 0));

    sf::RenderWindow window(sf::VideoMode(1600, 800), "Upscaled Visualization");

    auto lastTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds interval(10);

    std::string receivedData;
    std::ofstream finalMatrixFile("upscaled_matrix_output.txt");

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

                            finalMatrixFile << upscaledArray[i][j] << " ";
                        }
                        finalMatrixFile << std::endl;
                    }
                    finalMatrixFile << std::endl;

                    // 이전 행렬과 현재 행렬 간의 DTW 유사도 계산
                    float dtwDistance = calculateDTW(previousUpscaledArray, upscaledArray);
                    std::cout << "DTW Distance: " << dtwDistance << std::endl;

                    // 현재 업스케일된 행렬을 이전 행렬로 업데이트
                    previousUpscaledArray = upscaledArray;

                    // 시각화
                    window.clear();

                    float barWidth = window.getSize().x / static_cast<float>(newWidth);
                    float barHeight = window.getSize().y / static_cast<float>(newHeight);

                    for (int i = 0; i < newHeight; ++i) {
                        for (int j = 0; j < newWidth; ++j) {
                            sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight - 1));
                            bar.setPosition(j * barWidth, i * barHeight);

                            int value = static_cast<int>(upscaledArray[i][j] * 255 / *std::max_element(upscaledArray[i].begin(), upscaledArray[i].end()));
                            sf::Color color;
                            color.r = value;
                            color.g = 255 - value;
                            color.b = 0;

                            bar.setFillColor(color);
                            window.draw(bar);
                        }
                    }

                    window.display();
                }
            }
        }
    }

    finalMatrixFile.close();
    return 0;
}

/*
컴파일 및 실행할 수 있게 하는 코드
g++ -std=c++17 masktest.cpp Serial.cpp -o masktest -lsfml-graphics -lsfml-window -lsfml-system
./masktest
*/