#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

int main() {
    // 배열 초기화
    const int width = 32;
    const int height = 8;
    std::vector<std::vector<int>> array(height, std::vector<int>(width, 0));

    // 임의의 데이터로 배열 채우기
    std::srand(std::time(0));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            array[i][j] = std::rand() % 256; // 0-255 범위의 값
        }
    }

    // SFML 윈도우 생성
    sf::RenderWindow window(sf::VideoMode(800, 200), "Array Visualization");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
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
/* 실행 및 컴파일 코드
g++ -std=c++17 testVisualized.cpp Serial.cpp -o visualizedProgram -lsfml-graphics -lsfml-window -lsfml-system
./visualizedProgram 
*/
