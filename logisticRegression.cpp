#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

// 시그모이드 함수
double sigmoid(double z) {
    return 1.0 / (1.0 +  exp(-z));
}

// 예측 함수
double predict(const  vector<double>& weights, const  vector<double>& x) {
    double z = 0.0;
    for (size_t i = 0; i < weights.size(); ++i) {
        z += weights[i] * x[i];
    }
    return sigmoid(z);
}

// 경사 하강법으로 로지스틱 회귀 학습 (SGD)
void train( vector<double>& weights, const  vector< vector<double>>& X, const  vector<int>& y, double learning_rate, int epochs) {
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
    // 예제 데이터셋
     vector< vector<double>> X = {
        {1.0, 2.0, 3.0}, {1.0, 3.0, 5.0}, {1.0, 4.0, 6.0}
    };
     vector<int> y = {0, 1, 1};
    
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
     cout <<  endl;
    
    // 예측
     vector<double> new_data = {1.0, 3.5};
    double prediction = predict(weights, new_data);
     cout << "Prediction for new data: " << prediction <<  endl;
    
    return 0;
}
