#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

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

int main() {
    // 예제 데이터셋: 3x3 행렬 10개
    vector<vector<double>> X = {
        {1, 2, 3, 4, 5, 6, 7, 8, 9},
        {22, 13, 4, 15, 6, 7, 8, 19, 10},
        {1, 22, 3, 14, 5, 6, 17, 18, 19},
        {13, 12, 23, 24, 15, 16, 17, 18, 19},
        {2, 3, 24, 15, 6, 27, 18, 19, 20},
        {1, 12, 3, 4, 5, 26, 27, 18, 9},
        {12, 3, 14, 25, 26, 27, 18, 9, 10},
        {1, 22, 3, 14, 25, 16, 7, 8, 19},
        {2, 13, 4, 5, 26, 7, 28, 19, 20},
        {1, 12, 23, 4, 15, 16, 7, 18, 9}
    };

    // 각 행렬에 대한 참 값(레이블)
    vector<int> y = {0, 1, 1, 0, 1, 1, 0, 1, 1, 0};

    // 초기 가중치 설정
    vector<double> weights(X[0].size(), 0.0);

    // 학습률 및 반복 횟수 설정
    double learning_rate = 0.001;
    int epochs = 10000;

    // 모델 학습
    train(weights, X, y, learning_rate, epochs);

    // 가중치 출력
    cout << "Weights: ";
    for (const auto& weight : weights) {
        cout << weight << " ";
    }
    cout << endl;

    // 예측
    for (size_t i = 0; i < X.size(); ++i) {
        double prediction = predict(weights, X[i]);
        cout << "Prediction for matrix " << i+1 << ": " << prediction << endl;
    }

    return 0;
}
