#include <iostream> 
#include <fstream> 
#include <vector> 
#include <thread> // 스레드 사용을 위한 헤더 파일
#include <chrono> // 시간 측정을 위한 헤더 파일

using namespace std;
using namespace std::chrono; // 

void saveArrayToFile(const vector<vector<int>>& array, const string& filename) { 
    ofstream file(filename); // 주어진 파일 이름으로 파일 열기
    if (file.is_open()) { // 파일이 성공적으로 열렸는지 확인
        for (const auto& row : array) { // 배열의 각 행에 대해 반복
            for (const auto& element : row) { // 행의 각 요소에 대해 반복
                file << element << " "; // 요소를 파일에 쓰고 공백으로 구분
            }
            file << "\n"; // 각 행 뒤에 새 줄 문자 쓰기
        }
        file.close(); // 파일 닫기
        cout << "파일이 성공적으로 저장되었습니다.\n"; // 성공 메시지 출력
    } else {
        cerr << "파일을 열 수 없습니다.\n"; // 예외처리
    }
}

int main() {
    int rows = 16, cols = 32; // 배열의 행과 열 개수 정의
    vector<vector<int>> array(rows, vector<int>(cols)); // 지정된 크기로 2차원 벡터 배열 생성

    cout << "20초 동안 배열 요소를 입력하세요 (각 요소는 공백으로 구분됨):\n"; // 사용자에게 배열 요소 입력을 요청

    auto start_time = steady_clock::now(); // 현재 시간 가져오기
    int element_count = 0; // 요소 개수 초기화

    while (duration_cast<seconds>(steady_clock::now() - start_time).count() < 20 && element_count < rows * cols) { 
        //20초 동안 입력을 받기 위함. (현재 시간을 가져오고, 현제[ 시간을 기준으로 경과 시간 초 단위로 계산)
        if (cin.peek() != EOF) { // 입력이 있는지 확인 (표준 입력 스트림에 데이터 있는지 확인 후)
            cin >> array[element_count / cols][element_count % cols]; // 입력을 읽고 배열에 저장
            element_count++; // 요소 개수 증가
        }
    }

    if (element_count != rows * cols) {
        cerr << "제한 시간 내에 충분한 데이터를 입력하지 않았습니다.\n"; // 예외처리
        return 1; // 오류로 종료
    }

    string filepath; // 파일 경로를 저장할 변수 선언
    cout << "배열을 저장할 파일 경로를 입력하세요 (예: C:/path/to/file.txt): "; // 파일 경로 입력
    cin >> filepath; // 사용자로부터 파일 경로 읽기

    saveArrayToFile(array, filepath); // 지정된 파일에 배열 저장하는 함수 호출

    return 0;
}
