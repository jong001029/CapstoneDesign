#include <SFML/Graphics.hpp>
#include <vector>
#include "SerialClass.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

int main() {
    Serial serialPort("/dev/tty.usbmodem142101");

    if (!serialPort.IsConnected()) {
        std::cerr << "Failed to connect to the serial port!" << std::endl;
        return -1;
    }

    const int width = 3;
    const int height = 3;

    std::vector<std::vector<int>> array(height, std::vector<int>(width, 0));

    sf::RenderWindow window(sf::VideoMode(800, 200), "Visualization Program");

    auto lastTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds interval(1000); // 1초

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

            // Read data from serial port
            char buffer[256];
            int bytesRead = serialPort.ReadData(buffer, sizeof(buffer) - 1);

            if (bytesRead > 0) {
                buffer[bytesRead] = '\0'; // Null-terminate the buffer
                std::string receivedData(buffer);

                std::istringstream ss(receivedData);
                for (int i = 0; i < height; ++i) {
                    for (int j = 0; j < width; ++j) {
                        if (!(ss >> array[i][j])) {
                            array[i][j] = 0; // Default value if parsing fails
                        }
                    }
                }

                // Print array for debugging
                for (int i = 0; i < height; ++i) {
                    for (int j = 0; j < width; ++j) {
                        std::cout << array[i][j] << " ";
                    }
                    std::cout << std::endl;
                }
            }
        }

        window.clear();

        // Draw the visualization
        float barWidth = window.getSize().x / width;
        float barHeight = window.getSize().y / height;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight - 1));
                bar.setPosition(j * barWidth, i * barHeight);

                int value = array[i][j];
                sf::Color color;
                color.r = value;               // 빨간색 (값이 클수록 강해짐)
                color.g = 255 - value;         // 초록색 (값이 작을수록 강해짐)
                color.b = 100;                 // 파란색은 고정 (느낌을 줄 수 있도록)

                bar.setFillColor(color);
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
