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
void train(vector<double>& weights, const vector<vector<double>>& X, const vector<int>& y, double learning_rate, int epochs) {
    size_t m = X.size();
    size_t n = weights.size();

    for (int epoch = 0; epoch < epochs; ++epoch) {
        vector<double> gradients(n, 0.0);

        for (size_t i = 0; i < m; ++i) {
            double h = predict(weights, X[i]);
            for (size_t j = 0; j < n; ++j) {
                gradients[j] += (h - y[i]) * X[i][j];
            }
        }

        for (size_t j = 0; j < n; ++j) {
            weights[j] -= learning_rate * gradients[j] / m;
        }
    }
}

// 파일로부터 행렬을 읽는 함수
vector<vector<double>> read_matrices_from_file(const string& file_path) {
    vector<vector<double>> matrices;
    ifstream file(file_path);
    string line;

    while (getline(file, line)) { // 파일에서 한 줄씩 읽음
        stringstream ss(line);
        vector<double> matrix;
        double value;
        while (ss >> value) { // 한 줄에서 한 개씩 읽음
            matrix.push_back(value); // 읽은 값을 벡터에 추가
        }
        matrices.push_back(matrix); // 읽은 벡터를 행렬에 추가
    }

    return matrices;
}

// 지정된 디렉터리에서 데이터셋을 로드하는 함수
void load_dataset(const string& directory, vector<vector<double>>& X, vector<int>& y) {
    for (const auto& entry : fs::directory_iterator(directory + "/True")) { // True 디렉터리의 파일을 읽음
        if (entry.path().extension() == ".txt") { // 확장자가 .txt인 파일만 읽음
            vector<vector<double>> matrices = read_matrices_from_file(entry.path().string()); // 파일에서 행렬을 읽음
            X.insert(X.end(), matrices.begin(), matrices.end()); // 읽은 행렬을 입력 데이터에 추가
            y.insert(y.end(), matrices.size(), 1);  // True는 1로 라벨링
        }
    }
    for (const auto& entry : fs::directory_iterator(directory + "/False")) { // False 디렉터리의 파일을 읽음
        if (entry.path().extension() == ".txt") { // 확장자가 .txt인 파일만 읽음
            vector<vector<double>> matrices = read_matrices_from_file(entry.path().string()); // 파일에서 행렬을 읽음
            X.insert(X.end(), matrices.begin(), matrices.end()); // 읽은 행렬을 입력 데이터에 추가
            y.insert(y.end(), matrices.size(), 0);  // False는 0으로 라벨링
        }
    }
}

int main() {
    // 데이터셋 로드 (실제 경로로 수정 필요)
    string dataset_path = "/Users/godhyunjong/Desktop/Work/School/Capstone Design/07.08.mark7";
    vector<vector<double>> X;
    vector<int> y;
    load_dataset(dataset_path, X, y); // X: 입력 데이터, y: 라벨

    // 초기 가중치 설정
    vector<double> weights(X[0].size(), 0.0);

    // 학습률 및 반복 횟수 설정 (Hyperparameter)
    double learning_rate = 1e-10;
    int epochs = 100000;

    // 모델 학습
    train(weights, X, y, learning_rate, epochs);

    // 가중치 출력
    cout << "Weights: ";
    for (const auto& weight : weights) {
        cout << weight << " ";
    }
    cout << endl;

    // 사용자 입력을 받아서 예측 수행
    cout << "9개의 입력 값을 띄어쓰기로 구분하여 입력하시오: ";
    vector<double> user_input(X[0].size());
    for (double& value : user_input) {
        cin >> value;
    }

    // 예측 수행
    double prediction = predict(weights, user_input);
    int classified_prediction = prediction >= 0.5 ? 1 : 0;
    cout << "Prediction: " << classified_prediction << endl;

    return 0;
}
