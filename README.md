# CapstoneDesign

* 2024 inha univ capstone Design에 사용될 logistic regression을 c++로 구현.
* 아직은 예제 데이터셋을 이용.
* 센서를 통해서 받은 3*3 예제 데이터 셋을 txt 파일로 받아와 이를 logistic Regression으로 돌려 예측을 진행시킨다.
* 현재 센서입력으로 받은 True, False간 값이 너무나 촘촘하고 별 차이가 없기 때문에 Learning Rate와 Epoch를 크게 증가시켜 거의 overfitting이 될 수 있도록 설정
* 그러나 추후 사람들의 다양한 몸무게를 직접 입력 받으며, 어느 정도 General한 값, 유의미한 값들이 입력이 된다면 적절한 Parameter를 찾는 Tuning을 진행할 예정.
