# auto-recycle
시스템 프로그래밍 2조  
프로젝트명 : 잘버려  
팀원 : 강선규, 김지훈, 장은학  

# Build

```sh
> git clone https://git.ajou.ac.kr/gron1gh1/auto-recycle.git
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

```sh
> ./raspi-c_out <port>
```

- Rasberry Pi A

```sh
> ./raspi-a_out <ip> <port>
```

- Rasberry Pi C

```sh
> cd raspi-b
> python3 raspi-b <ip> <port>
```
