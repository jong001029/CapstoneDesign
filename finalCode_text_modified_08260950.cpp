#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <fstream>
#include <cmath>
#include <algorithm>  // min_element와 max_element를 사용하기 위해 필요
#include "SerialClass.h"

// 주위 이웃 값을 이용해 x 값을 계산하는 함수
float calculateX(const std::vector<std::vector<float>>& matrix, int i, int j, int width, int height) {
    float sum = 0.0f;
    int count = 0;

    // 8방향 이웃 탐색 (상, 하, 좌, 우, 대각선)
    for (int di = -1; di <= 1; ++di) {
        for (int dj = -1; dj <= 1; ++dj) {
            if (di == 0 && dj == 0) continue;  // 자기 자신은 제외
            int ni = i + di;
            int nj = j + dj;
            if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                sum += matrix[ni][nj];
                ++count;
            }
        }
    }

    if (count > 0) {
        return sum / count;  // 이웃 값들의 평균
    } else {
        return matrix[i][j];  // 이웃이 없으면 자기 자신의 값 반환
    }
}

// 선형 보간 함수
float linearInterpolate(float v0, float v1, float t) {
    return v0 * (1 - t) + v1 * t;
}

// 업스케일링 함수
void upscaleMatrix(const std::vector<std::vector<float>>& inputMatrix, std::vector<std::vector<float>>& outputMatrix, int oldWidth, int oldHeight, int newWidth, int newHeight) {
    for (int i = 0; i < newHeight; ++i) {
        for (int j = 0; j < newWidth; ++j) {
            float x = static_cast<float>(j) / (newWidth - 1) * (oldWidth - 1);
            float y = static_cast<float>(i) / (newHeight - 1) * (oldHeight - 1);

            int x0 = static_cast<int>(x);
            int y0 = static_cast<int>(y);
            int x1 = std::min(x0 + 1, oldWidth - 1);
            int y1 = std::min(y0 + 1, oldHeight - 1);

            float xFraction = x - x0;
            float yFraction = y - y0;

            float top = linearInterpolate(inputMatrix[y0][x0], inputMatrix[y0][x1], xFraction);
            float bottom = linearInterpolate(inputMatrix[y1][x0], inputMatrix[y1][x1], xFraction);
            outputMatrix[i][j] = linearInterpolate(top, bottom, yFraction);
        }
    }
}

// Jet 컬러맵에 따른 색상 변환 함수
sf::Color getJetColor(float value, float vmin, float vmax) {
    float ratio = 2 * (value - vmin) / (vmax - vmin);
    int r = std::min(255.0f, std::max(0.0f, 255 * (ratio - 0.5f)));
    int g = std::min(255.0f, std::max(0.0f, 255 * (1.0f - std::abs(ratio - 1.0f))));
    int b = std::min(255.0f, std::max(0.0f, 255 * (1.5f - ratio)));
    return sf::Color(r, g, b);
}



// 파일 경로를 생성하는 함수
std::string getFilePath(const std::string& directory, const std::string& fileName) {
    return "text/" + directory + "/" + fileName + ".txt";
}

int main() {
    // 시리얼 포트를 초기화하고 연결
    Serial serialPort("/dev/cu.usbmodem14201"); // 시리얼 포트의 경로를 지정(포트에 맞게 변경이 필요함. Windows의 경우 "COM3" 등으로 지정)

    // 시리얼 포트 연결 여부를 확인
    if (!serialPort.IsConnected()) {
        std::cerr << "Failed to connect to the serial port!" << std::endl;
        return -1;  // 연결 실패 시 프로그램 종료
    }

    const int width = 16;  // 행렬의 너비 (열의 수)
    const int height = 8;  // 행렬의 높이 (행의 수)
    const int newWidth = 32;  // 업스케일링된 행렬의 너비
    const int newHeight = 16;  // 업스케일링된 행렬의 높이

    // 16x8 크기의 데이터 배열 초기화, 모든 값을 0으로 설정
    std::vector<std::vector<int>> array(height, std::vector<int>(width, 0));

    // 시각화를 위한 SFML 윈도우 생성
    sf::RenderWindow window(sf::VideoMode(1600, 800), "Visualization Program");

    // 마지막 시간을 기록하기 위한 변수 초기화
    auto lastTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds interval(10);  // 10ms마다 데이터를 읽음

    std::string receivedData;  // 수신된 데이터를 저장하기 위한 문자열

    std::ofstream maskFile("text/maskFile/maskFile.txt");  // Mask 데이터를 기록할 파일
    std::ofstream finalMatrixFile("text/finalMatrix/finalMatrix.txt");  // 최종 행렬 데이터를 기록할 파일

    // 파일 경로 입력
    std::string directoryChoice, fileName;
    std::cout << "Enter directory (True/False): ";
    std::cin >> directoryChoice;
    std::cout << "Enter the file name (without extension): ";
    std::cin >> fileName;

    // 파일 경로 생성
    std::string filePath = getFilePath(directoryChoice, fileName);
    std::ofstream upscaledMatrixFile(filePath);  // 업스케일된 행렬을 기록할 파일

    // Re-sampling 횟수 설정
    int resamplingCount = 1;  // 원하는 만큼 설정 가능 (현재 1회만 수행)

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

                    // 터미널에 입력된 값을 출력하여 디버깅
                    std::cout << "Received matrix data:" << std::endl;
                    for (int i = 0; i < height; ++i) {
                        for (int j = 0; j < width; ++j) {
                            std::cout << array[i][j] << " ";
                        }
                        std::cout << std::endl;
                    }

                    // Re-sampling을 resamplingCount만큼 반복
                    std::vector<std::vector<float>> result = std::vector<std::vector<float>>(height, std::vector<float>(width, 0.0f));
                    
                    for (int r = 0; r < resamplingCount; ++r) {
                        std::cout << "Re-sampling Iteration: " << r + 1 << std::endl;

                        // Re-sampling Strategy 적용
                        float sum = 0.0f, sum_sq = 0.0f;
                        for (int i = 0; i < height; ++i) {
                            for (int j = 0; j < width; ++j) {
                                result[i][j] = array[i][j];  // 현재는 단순히 array 값을 사용
                                sum += result[i][j];
                                sum_sq += result[i][j] * result[i][j];
                            }
                        }
                        int n = width * height;
                        float mean = sum / n;
                        float std_dev = std::sqrt((sum_sq / n) - (mean * mean));

                        float Thup = mean + std_dev;
                        float Thdn = mean - std_dev;

                        std::cout << "Thup: " << Thup << ", Thdn: " << Thdn << std::endl;

                        std::vector<std::vector<int>> Mask(height, std::vector<int>(width, 0));

                        for (int i = 0; i < height; ++i) {
                            for (int j = 0; j < width; ++j) {
                                if (result[i][j] < Thup && result[i][j] > Thdn) {
                                    Mask[i][j] = 1;
                                } else {
                                    Mask[i][j] = 0;
                                }
                                maskFile << Mask[i][j] << " ";  // Mask 값을 파일에 기록
                            }
                            maskFile << std::endl;  // 줄바꿈
                        }
                        maskFile << std::endl;  // 추가 줄바꿈

                        // Mask를 이용해 최종 행렬을 계산
                        for (int i = 0; i < height; ++i) {
                            for (int j = 0; j < width; ++j) {
                                if (Mask[i][j] == 0) {
                                    finalMatrixFile << result[i][j] << " ";  // 최종 행렬 값을 파일에 기록
                                } else {
                                    result[i][j] = calculateX(result, i, j, width, height);  // 이웃 값들로 x 계산
                                    finalMatrixFile << result[i][j] << " ";  // 최종 행렬 값을 파일에 기록
                                }
                            }
                            finalMatrixFile << std::endl;  // 줄바꿈
                        }
                        finalMatrixFile << std::endl;  // 추가 줄바꿈
                    }

                    // 32x16 크기로 업스케일링된 행렬을 저장할 벡터
                    std::vector<std::vector<float>> upscaledMatrix(newHeight, std::vector<float>(newWidth, 0.0f));

                    // 업스케일링 수행
                    upscaleMatrix(result, upscaledMatrix, width, height, newWidth, newHeight);

                    // 업스케일링된 행렬을 텍스트 파일로 저장
                    for (int i = 0; i < newHeight; ++i) {
                        for (int j = 0; j < newWidth; ++j) {
                            upscaledMatrixFile << upscaledMatrix[i][j] << " ";
                        }
                        upscaledMatrixFile << std::endl;
                    }
                    upscaledMatrixFile << std::endl;

                    // 시각화를 위한 데이터 그리기
                    window.clear();  // 창을 지움 (새로고침)

                    float barWidth = window.getSize().x / static_cast<float>(newWidth);  // 각 바의 너비 계산
                    float barHeight = window.getSize().y / static_cast<float>(newHeight);  // 각 바의 높이 계산

                    // 업스케일링된 데이터의 최솟값과 최댓값을 찾음
                    float vmin = 0.0f;
                    float vmax = 2000.0f;

                    for (int i = 0; i < newHeight; ++i) {
                        for (int j = 0; j < newWidth; ++j) {
                            sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight - 1));  // 바의 크기를 설정
                            bar.setPosition(j * barWidth, i * barHeight);  // 바의 위치 설정

                            // Jet 컬러맵에 따라 색상 계산
                            sf::Color color = getJetColor(upscaledMatrix[i][j], vmin, vmax);
                            bar.setFillColor(color);  // 바의 색상을 설정
                            window.draw(bar);  // 바를 창에 그림
                        }
                    }

                    window.display();  // 창을 업데이트하여 변경 사항을 반영
                }
            }
        }
    }

    maskFile.close();  // 프로그램 종료 시 파일을 닫음
    finalMatrixFile.close();  // 최종 행렬 파일을 닫음
    upscaledMatrixFile.close();  // 업스케일링된 행렬 파일을 닫음
    return 0;
}

/*
컴파일 및 실행할 수 있게 하는 코드
g++ -std=c++17 finalCode_text_modified_08260950.cpp Serial.cpp -o finalCode_text_modified_08260950 -lsfml-graphics -lsfml-window -lsfml-system
./finalCode_text_modified_08260950
*/
