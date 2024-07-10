#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

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
    char start;
    cout << "배열을 입력하시겠습니까? (y/n): ";
    cin >> start;

    if (start == 'y' || start == 'Y') {
        int rows, cols;
        cout << "배열 크기 입력 (32*8 또는 32*16):\n";
        cout << "행의 개수 입력 (8 또는 16): ";
        cin >> rows;
        cols = 32;

        if (rows != 8 && rows != 16) {
            cerr << "잘못된 배열 크기입니다.\n";
            return 1;
        }

        vector<vector<int>> array(rows, vector<int>(cols));

        cout << "배열 요소를 입력하세요:\n";
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cin >> array[i][j];
            }
        }

        string filepath;
        cout << "배열을 저장할 파일 경로를 입력하세요 (예: C:/path/to/file.txt): ";
        cin >> filepath;

        saveArrayToFile(array, filepath);
    } else {
        cout << "프로그램을 종료합니다.\n";
    }

    return 0;
}
