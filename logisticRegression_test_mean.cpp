#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// 시그모이드 함수
double sigmoid(double z) {
    return 1.0 / (1.0 + exp(-z));
}

// 예측 함수
double predict(const vector<double>& weights, const vector<double>& x) {
    double z = 0.0;
    for (size_t i = 0; i < weights.size(); ++i) { 
        z += weights[i] * x[i];
    }
    return sigmoid(z);
}

// 경사 하강법으로 로지스틱 회귀 학습 (SGD)
// 훈련 함수
void train(vector<double>& weights, const vector<vector<double>>& X, const vector<int>& y, double learning_rate, int epochs) {
    size_t m = X.size();
    size_t n = weights.size();

    for (int epoch = 0; epoch < epochs; ++epoch) {
        vector<double> gradients(n, 0.0);

        for (size_t i = 0; i < m; ++i) {
            double h = predict(weights, X[i]);
            for (size_t j = 0; j < n; ++j) {
                gradients[j] += (h - y[i]) * X[i][j]; // 기울기 계산
            }
        }

        for (size_t j = 0; j < n; ++j) {
            weights[j] -= learning_rate * gradients[j] / m; // 평균 기울기를 이용하여 가중치 업데이트
        }
    }
}

// 파일로부터 32x16 행렬들을 읽고, 이를 하나의 벡터로 평균화하는 함수
vector<double> read_and_aggregate_matrices_from_file(const string& file_path) {
    ifstream file(file_path);
    string line;

    vector<double> aggregated_matrix(512, 0.0);  // 32x16 = 512 크기의 벡터로 초기화
    int matrix_count = 0;

    int current_row = 0;

    while (getline(file, line)) { // 파일에서 한 줄씩 읽음
        if (!line.empty()) {
            stringstream ss(line);
            double value;
            size_t index = current_row * 16;  // 행의 인덱스 설정

            while (ss >> value && index < aggregated_matrix.size()) { // 한 줄에서 값을 읽음
                aggregated_matrix[index] += value; // 이후 행렬은 합산
                ++index;
            }
            current_row++;

            if (current_row == 32) {
                current_row = 0;  // 다음 행렬로 이동 (32줄 후)
                matrix_count++;
            }
        }
    }

    // 평균 계산
    if (matrix_count > 0) {
        for (double& val : aggregated_matrix) {
            val /= matrix_count;
        }
    }

    return aggregated_matrix;
}

// 지정된 디렉터리에서 데이터셋을 로드하는 함수
void load_dataset(const string& directory, vector<vector<double>>& X, vector<int>& y) {
    for (const auto& entry : fs::directory_iterator(directory + "/True")) { // True 디렉터리의 파일을 읽음
        if (entry.path().extension() == ".txt") { // 확장자가 .txt인 파일만 읽음
            vector<double> aggregated_matrix = read_and_aggregate_matrices_from_file(entry.path().string()); // 파일에서 행렬을 읽어 평균화
            X.push_back(aggregated_matrix); // 평균화된 행렬을 입력 데이터에 추가
            y.push_back(1);  // True는 1로 라벨링
        }
    }
    for (const auto& entry : fs::directory_iterator(directory + "/False")) { // False 디렉터리의 파일을 읽음
        if (entry.path().extension() == ".txt") { // 확장자가 .txt인 파일만 읽음
            vector<double> aggregated_matrix = read_and_aggregate_matrices_from_file(entry.path().string()); // 파일에서 행렬을 읽어 평균화
            X.push_back(aggregated_matrix); // 평균화된 행렬을 입력 데이터에 추가
            y.push_back(0);  // False는 0으로 라벨링
        }
    }
}

int main() {
    // 데이터셋 로드 (실제 경로로 수정 필요)
    string dataset_path = "text";
    vector<vector<double>> X;
    vector<int> y;
    load_dataset(dataset_path, X, y); // X: 입력 데이터, y: 라벨

    // 초기 가중치 설정
    vector<double> weights(X[0].size(), 0.0);

    // 학습률 및 반복 횟수 설정 (Hyperparameter)
    double learning_rate = 1e-5;
    int epochs = 1000;

    // 모델 학습
    train(weights, X, y, learning_rate, epochs);

    // 학습 데이터로 예측 수행 및 결과 출력
    int correct_predictions = 0;
    for (size_t i = 0; i < X.size(); ++i) {
        double prediction = predict(weights, X[i]);
        int classified_prediction = prediction >= 0.5 ? 1 : 0;
        cout << "Actual: " << y[i] << ", Predicted: " << classified_prediction << endl;
        if (classified_prediction == y[i]) {
            correct_predictions++;
        }
    }

    double accuracy = static_cast<double>(correct_predictions) / X.size();
    cout << "Accuracy: " << accuracy * 100.0 << "%" << endl;

    // 가중치 출력
    cout << "Weights (16x32):" << endl;
    for (size_t i = 0; i < 16; ++i) {
        for (size_t j = 0; j < 32; ++j) {
            cout << weights[i * 32 + j];
            if (j < 31) cout << ", ";  // 각 요소 사이에 콤마 추가, 마지막에는 제외
        }
        cout << endl;
    }

    // 사용자로부터 테스트 파일 경로 입력받기
    cout << "테스트할 텍스트 파일의 경로를 입력하세요: ";
    string test_file_path;
    cin >> test_file_path;
    //예외 처리
    if(!fs::exists(test_file_path)) {
        cout << "파일이 존재하지 않습니다." << endl;
        return 1;
    }

    // 테스트 파일에서 행렬을 읽어와서 예측 수행
    vector<double> test_data = read_and_aggregate_matrices_from_file(test_file_path);
    double test_prediction = predict(weights, test_data);
    int classified_test_prediction = test_prediction >= 0.5 ? 1 : 0;
    cout << "테스트 파일의 예측 결과: " << (classified_test_prediction == 1 ? "True" : "False") << endl;

    return 0;
}

//컴파일 및 실행 코드 (오현종의 경우)
/*
g++ -o logisticRegression_test_mean logisticRegression_test_mean.cpp -std=c++17
./logisticRegression_test_mean
*/
