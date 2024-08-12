# CapstoneDesign

* 2024 inha univ capstone Design에 사용될 logistic regression을 c++로 구현.
* totalProgram을 통해서 데이터셋을 수집함과 동시에 시각화를 할 수 있게 구현하였으며, 이는 센서를 통해 입력된 행렬과 캘리브레이션된 행렬을 곱하여 최종 result를 시각화를 하며,
동시에 텍스트로 저장을 하여 이후 logistic regression을 진행할 예정이다.
* 이렇게 logistic regression을 통해 1명인지 아닌지 구분하여 한 명이 아니라면 시동이 걸리지 않게 할 예정

* 이때 최종 result와 센서와 싱크를 맞게 하기 위해 아두이노 쪽에서 하나의 행렬이 다 보내지면 -1을 출력하게 하였고, totalProgram을 보면 알 수 있듯 -1이 들어오게 된다면
하나의 행렬의 입력이 끝났다고 판단을 할 수 있게 하여 싱크를 맞추었다.

