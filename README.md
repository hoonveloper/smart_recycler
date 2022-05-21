# README.MD

# ✅auto-recycle

시스템 프로그래밍 2조
프로젝트명 : **잘버려**
팀원 : 강선규, 김지훈, 장은학

설명: 라즈베리파이 3대를 이용한 스마트 쓰레기통 제작

✅ 쓰레기 버리기 


![2022-05-21 19;09;35](https://user-images.githubusercontent.com/77804950/169647208-e1546c51-136e-4fc5-9a18-468c1337867e.gif)


1. 버튼을 이용해 쓰레기의 종류(플라스틱/캔)을 선택한다.
2. 쓰레기를 올리면, 데이터를 학습한 Object detection 기술을 이용해 쓰레기의 종류를 분류한다.
    
    만약 초기 설정한 쓰레기 종류와, 올린 쓰레기의 종류가 같다면 LCD 화면에 SUCCESS
    
    만약 초기 설정한 쓰레기 종류와 올린 쓰레기의 종류가 다르다면 LCD화면에 FAIL을 출력한다.
    
3. 쓰레기 분류 성공 여부에 따라 라즈베리파이의 모터를 작동시켜 쓰레기통의 뚜껑을 열고 닫는다.

✅ 용량 체크


![2022-05-21 19;10;11 (1)](https://user-images.githubusercontent.com/77804950/169647203-82e77bd1-4c35-40bc-98d7-4c772708c1ff.gif)

1. 적외선 센서를 이용해 쓰레기통 바닥까지의 거리를 측정하고 측정된 길이가 빈 쓰레기통의 길이보다 짧을경우, 쓰레기통이 얼마나 차 있는지 계산하여 LCD에 쓰레기통의 용량을 표시한다. 

### 잘버려 결과물
<img src="https://user-images.githubusercontent.com/77804950/169647312-715ba109-582a-4409-beea-07d1bfeb1caa.jpg" width="400" height="400"/>
<img src="https://user-images.githubusercontent.com/77804950/169647438-28387cae-6b0a-47dd-9fda-d49b32e652d4.png" width="400" height="400"/>


# Build

```
> git clone <https://git.ajou.ac.kr/gron1gh1/auto-recycle.git>
> cd auto-recycle
> make

```

# Usage

- Sequence of Execution
1. build
2. run raspi-c_out (this server)
3. run raspi-a_out (this first client)
4. python raspi-b (this second client)
- Rasberry Pi C

```
> ./raspi-c_out <port>

```

- Rasberry Pi A

```
> ./raspi-a_out <ip> <port>

```

- Rasberry Pi C

```
> cd raspi-b
> python3 raspi-b <ip> <port>

```
