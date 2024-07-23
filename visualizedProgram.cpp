#include <SFML/Graphics.hpp>
#include <vector>
#include "SerialClass.h"

int main() {
    // 시리얼 포트 설정 (Mac에서는 "/dev/tty.usbmodem14101" 같은 포트를 사용합니다)
    Serial serialPort("/dev/tty.usbmodem14101");

    if (!serialPort.IsConnected()) {
        printf("Failed to connect to the serial port!\n");
        return -1;
    }

    // 배열 초기화 (배열 크기는 여기서 바꿀 수 있음)
    const int width = 32;
    const int height = 8;

    std::vector<std::vector<int>> array(height, std::vector<int>(width, 0));

    // SFML 윈도우 생성
    sf::RenderWindow window(sf::VideoMode(800, 200), "Array Visualization");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // 시리얼 포트로부터 데이터 읽기
        char buffer[width * height];
        int bytesRead = serialPort.ReadData(buffer, sizeof(buffer));

        if (bytesRead > 0) {
            // 데이터를 배열로 변환
            for (int i = 0; i < bytesRead; ++i) {
                int row = i / width;
                int col = i % width;
                array[row][col] = static_cast<unsigned char>(buffer[i]); // 값이 음수가 되지 않도록 변환
            }
        }

        window.clear();

        // 배열을 시각화
        float barWidth = window.getSize().x / width;
        float barHeight = window.getSize().y / height;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight - 1));
                bar.setPosition(j * barWidth, i * barHeight);

                // 색상 설정 (여기서는 값을 0-255 범위로 가정)
                int value = array[i][j];
                bar.setFillColor(sf::Color(value, 255 - value, 100));

                window.draw(bar);
            }
        }

        window.display();
    }

    return 0;
}

/* 
컴파일 및 실행할 수 있게 하는 코드
g++ -std=c++17 visualizedProgram.cpp Serial.cpp -o visualizedProgram -lsfml-graphics -lsfml-window -lsfml-system
./visualizedProgram
*/