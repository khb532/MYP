# 소켓(Socket) 완전 가이드

## 1. 소켓의 정의와 역사

### 1.1 소켓이란 무엇인가?

컴퓨터를 두 개 연결하려면 어떻게 할까? 케이블을 꽂는다. 하지만 소프트웨어적으로는 어떻게 한 프로그램이 다른 프로그램과 대화할까?

**소켓(Socket)**이 그 답이다. 소켓은 **네트워크 통신을 위한 통로**라고 생각하면 된다. 마치 현실에서 "전화기"가 사람 간의 음성 통신을 가능하게 하듯이, 소켓은 프로그램 간의 데이터 통신을 가능하게 한다.

더 정확히 말하면, 소켓은 운영체제가 제공하는 **통신 종단점(endpoint)**이다. 게임 클라이언트와 게임 서버가 데이터를 주고받으려면 양쪽 모두 소켓을 열어야 하고, 그 소켓을 통해 통신한다.

### 1.2 역사적 배경

흥미롭게도, 소켓 개념은 1983년 UC Berkeley의 Bill Joy가 처음 만들었다. 당시 인터넷은 지금처럼 대중화되지 않았지만, 대학들이 TCP/IP 프로토콜을 사용해서 통신하기 시작했다.

"그런데 프로그래머들이 매번 TCP/IP 내부 구조를 다루기는 너무 복잡하다. 쉬운 인터페이스가 필요하지 않을까?"

이런 고민 끝에 **BSD 소켓(Berkeley Software Distribution Socket)**이 탄생했다. 이것이 정말 좋은 설계였는지, 40년이 지난 지금도 거의 모든 운영체제(Windows, Linux, macOS)가 같은 방식으로 소켓을 제공하고 있다.

### 1.3 왜 소켓이 필요한가?

네트워크 통신이 없던 시절에는 프로그램이 같은 컴퓨터 내에서만 돌아갔다. 하지만 점점 프로그램들이 다른 컴퓨터의 데이터가 필요해지기 시작했다:

- 게임 서버에서 플레이어 데이터를 받아야 함
- 웹 브라우저가 웹 서버에 웹 페이지를 요청해야 함
- 모바일 앱이 클라우드 서버와 동기화해야 함

**그런데 문제가 있다.** 네트워크 하드웨어는 복잡하고, 프로토콜도 많다 (TCP, UDP, IP 등). 매 번 프로그래머가 이 모든 것을 직접 다루기는 불가능하다.

**해결책이 바로 소켓이다.** 소켓이라는 **추상화된 인터페이스**를 사용하면, 복잡한 네트워크 통신의 세부사항은 운영체제가 담당하고, 프로그래머는 간단한 함수 몇 개만 호출하면 된다.

---

## 2. 소켓의 종류

소켓을 만들 때 선택해야 할 것이 두 가지 있다:

1. **어디와 통신할 것인가?** (도메인)
2. **어떤 방식으로 통신할 것인가?** (타입)

예를 들어, 게임에서 온라인 서버와 통신한다면:
- 도메인: 인터넷을 통해 통신 (AF_INET)
- 타입: 신뢰할 수 있는 연결 기반 통신 (SOCK_STREAM)

각각을 살펴보자.

### 2.1 도메인(Domain)별 분류: "누구와 통신할 건가?"

도메인이란 **어떤 네트워크를 사용해서 통신할 것인가**를 정의한다.

#### 2.1.1 AF_INET (IPv4) - 인터넷 통신의 표준

**상황:** 게임 서버가 다른 나라의 클라이언트와 통신하려고 한다면?

이 경우 인터넷을 통해 통신해야 한다. 인터넷에서는 각 컴퓨터를 구분하기 위해 **IP 주소**를 사용한다.

- IP 주소는 32비트로, 네 개의 숫자로 표현된다 (예: 192.168.1.1)
- 각 숫자는 0~255 범위를 가진다
- 전 세계 43억 개 정도의 주소를 가질 수 있다

**특징:** 가장 기본적이고 널리 사용된다. 대부분의 웹, 게임, 스트리밍이 이것을 사용한다.

#### 2.1.2 AF_INET6 (IPv6) - 미래의 인터넷 통신

**상황:** 하지만 43억 개의 주소로는 부족해졌다. 스마트폰, IoT 기기, 그리고 미래의 수십억 개 장치들이 모두 IP 주소가 필요하기 때문이다.

그래서 만들어진 것이 **IPv6**이다.

- IP 주소가 128비트로 훨씬 크다 (예: 2001:db8::1)
- 사실상 무한에 가까운 개수의 주소를 지원한다
- 아직 완전히 대체되지는 않았지만, 점차 보급되고 있다

**특징:** 미래를 대비하는 선택지. 새로운 서비스를 만든다면 IPv6도 지원하는 것이 좋다.

#### 2.1.3 AF_UNIX (Unix Domain Socket) - 같은 컴퓨터 내 통신

**상황:** 게임 서버가 같은 컴퓨터에서 실행 중인 데이터베이스와 통신하려고 한다면?

같은 컴퓨터 내에서는 굳이 TCP/IP 네트워크 스택을 거칠 필요가 없다. 더 간단하고 빠른 방법이 있다:

**AF_UNIX 소켓**은 인터넷을 사용하지 않고, **파일 시스템 경로**를 주소로 사용한다.

- 예: `/tmp/game_server.sock` 이런 식의 파일 경로를 "주소"로 사용
- 같은 호스트 내에서만 통신 가능
- TCP/IP보다 약 2배 빠르다 (네트워크 스택을 거치지 않으므로)

**특징:** 마이크로서비스 아키텍처에서 같은 서버의 여러 프로세스가 통신할 때 유용하다.

---

### 2.2 타입(Type)별 분류: "어떤 방식으로 통신할 건가?"

이제 **어떤 방식으로** 데이터를 보낼 것인가를 정해야 한다. 이것이 타입(Type)이다.

#### 2.2.1 SOCK_STREAM (TCP) - 신뢰할 수 있는 연결

**상황 1 - 파일 다운로드:**
A가 B에게 100MB의 파일을 보낸다. 도중에 1MB가 손실되면?
→ "전체 파일이 손상되었다! 다시 받아야 한다!"

**상황 2 - 게임 플레이어의 위치 정보:**
플레이어가 "점프" 명령을 서버로 보낸다. 중간에 손실되면?
→ "플레이어가 점프하지 않았는데 표시되지 않네?"

이런 상황들에서는 **모든 데이터가 정확하게, 순서대로 도착해야 한다.**

**SOCK_STREAM (TCP 프로토콜)의 특징:**

- **연결 지향적**: 데이터를 보내기 전에 반드시 먼저 "연결"을 맺어야 한다
- **신뢰성**: 보낸 데이터가 반드시 도착한다. 손실되면 자동으로 재전송
- **순서 보장**: 먼저 보낸 데이터가 먼저 도착한다
- **흐름 제어**: 수신자가 받을 수 있는 속도가 느리면 송신자가 속도를 늦춘다
- **오버헤드**: 신뢰성을 보장하려다 보니 약간의 성능 저하가 있다

**사용 사례:**
- HTTP/HTTPS (웹 페이지 요청)
- FTP (파일 전송)
- SSH (원격 접속)
- 이메일 (SMTP, POP3)

#### 2.2.2 SOCK_DGRAM (UDP) - 빠르지만 신뢰성 없는 통신

**상황 1 - 실시간 게임:**
플레이어가 움직인다. 1000분의 1초 마다 위치 정보를 보낸다.
→ 만약 몇 개 패킷이 손실되어도 다음 정보가 곧 온다
→ "완벽할 필요 없다. 빨라야 한다!"

**상황 2 - 유튜브 스트리밍:**
영상 프레임을 실시간으로 본다. 모든 프레임이 완벽할 필요가 있나?
→ 가끔 끊기거나 뚝뚝 끊기는 게 정상
→ 대신 지연 없이 빨리 받아야 한다

이런 상황들에서는 **속도가 신뢰성보다 중요하다.**

**SOCK_DGRAM (UDP 프로토콜)의 특징:**

- **비연결형**: 연결 과정이 없다. 그냥 데이터를 보낸다
- **신뢰성 없음**: 보낸 데이터가 도착하지 않을 수 있다
- **순서 보장 없음**: 먼저 보낸 데이터가 나중에 도착할 수 있다
- **빠름**: 연결 과정이 없고 재전송도 안 하니까 매우 빠르다
- **메시지 단위**: 데이터가 독립적인 메시지로 전송된다

**사용 사례:**
- DNS 조회 (빠르고 단순한 요청/응답)
- 실시간 게임 (플레이어 위치 업데이트)
- 음성/영상 스트리밍 (VoIP, YouTube)
- IoT 센서 데이터 수집

**TCP vs UDP 선택 기준:**

데이터의 "모든 것"이 도착해야 한다면 → TCP (SOCK_STREAM)
속도가 중요하고 약간의 손실은 괜찮다면 → UDP (SOCK_DGRAM)


---

## 3. TCP 소켓의 동작 원리

### 3.1 서버와 클라이언트의 역할 구분

TCP 소켓 통신은 두 가지 **서로 다른 역할**로 나뉩니다.

**서버의 역할**: "내가 여기 있으니 누군가 연락을 기다릴게"
- 특정 주소(`bind()`)와 포트에서 대기 (`listen()`)
- 들어오는 연결을 수락 (`accept()`)
- 여러 클라이언트를 동시에 처리할 수 있음

**클라이언트의 역할**: "나는 저 서버에 연결하고 싶어"
- 특정 서버의 주소와 포트로 연결 요청 (`connect()`)
- 서버와 데이터 주고받음
- 완료 후 연결 해제

이렇게 역할을 나누는 이유는 뭘까요? 게임 서버를 예시로 생각해봅시다.

**상황**: 플레이어 100명이 게임 서버에 접속하고 싶습니다.
- 만약 역할 구분이 없다면? "누가 누구를 기다려야 하지?"라는 혼란이 생깁니다.
- 역할을 나누면? 게임 서버는 계속 대기하고, 플레이어들은 차례대로 연결합니다.

### 3.2 TCP 연결 과정: 3-Way Handshake

이제 실제로 **클라이언트가 서버에 연결하는 과정**을 자세히 살펴봅시다.

처음에 클라이언트가 `connect()`를 호출하면, TCP는 내부적으로 **3번의 메시지 교환**을 합니다. 이를 "3-way handshake"라고 부릅니다.

```
클라이언트                                     서버
   |                                            |
   | 1. SYN 전송                                |
   |   ("안녕, 나랑 연결하고 싶어")            |
   |────────────────────────────────────────>|
   |                                            |
   |                        2. SYN+ACK 수신     |
   |                   ("좋아, 난 준비됐어")    |
   |<────────────────────────────────────────|
   |                                            |
   | 3. ACK 전송                                |
   |   ("고마워, 나도 준비됐어")              |
   |────────────────────────────────────────>|
   |                                            |
   |========== 연결 완료 (ESTABLISHED) =========|
   |                                            |
```

**각 단계의 의미:**

1. **SYN (동기화 신호)**: 클라이언트가 서버에 "나와 통신하려고 합니다"라는 신호를 보냅니다. 이때 클라이언트의 초기 시퀀스 번호를 함께 전송합니다.

2. **SYN+ACK (확인 응답)**: 서버가 "나는 너의 신호를 받았고, 나도 준비됐다"는 응답을 보냅니다. 서버도 자신의 초기 시퀀스 번호를 보냅니다.

3. **ACK (확인)**: 클라이언트가 "너의 응답을 받았다"는 최종 확인을 보냅니다.

**왜 3번이 필요할까요?**
- 첫 번째 메시지로 클라이언트가 존재함을 증명
- 두 번째 메시지로 서버가 존재함을 증명
- 세 번째 메시지로 양쪽 모두 "서로 대화할 준비가 됐다"는 것을 확인

이제 양쪽 모두 **시퀀스 번호**를 알았으므로, 이 번호를 사용해서 데이터 전송 시 순서를 보장할 수 있습니다.

### 3.3 데이터 송수신

연결이 완료된 후, 클라이언트와 서버는 자유롭게 데이터를 주고받습니다.

```cpp
// 서버 코드 (단순화)
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, &server_addr, sizeof(server_addr));
listen(server_fd, 5);

int client_fd = accept(server_fd, &client_addr, &addr_len);
// 이 지점에서 3-way handshake 완료 ✓

char buffer[1024];
read(client_fd, buffer, sizeof(buffer));  // 클라이언트 메시지 수신
write(client_fd, "Hello", 5);              // 응답 전송
```

```cpp
// 클라이언트 코드 (단순화)
int sock = socket(AF_INET, SOCK_STREAM, 0);
connect(sock, &server_addr, sizeof(server_addr));
// 이 지점에서 3-way handshake 완료 ✓

write(sock, "Hi Server", 9);  // 메시지 전송
char buffer[1024];
read(sock, buffer, sizeof(buffer));  // 응답 수신
```

### 3.4 연결 해제: 4-Way Handshake

이제 통신이 끝났으므로 연결을 끊어야 합니다. 재밌게도, **연결을 끊는 것도 "악수"**처럼 4번의 메시지 교환이 필요합니다.

```
클라이언트                                     서버
   |                                            |
   | 1. FIN 전송 (더 이상 보낼 게 없어)        |
   |────────────────────────────────────────>|
   |                                            |
   |                        2. ACK 수신        |
   |                    ("알겠어")             |
   |<────────────────────────────────────────|
   |                                            |
   |                     3. FIN 수신            |
   |                   (서버도 준비됐어)       |
   |<────────────────────────────────────────|
   |                                            |
   | 4. ACK 전송 (최종 확인)                   |
   |────────────────────────────────────────>|
   |                                            |
   |═════════ 연결 완전 해제 ═════════|
   |                                            |
```

**왜 4번이 필요할까요?**

클라이언트가 `close()`를 호출하면, "더 이상 보낼 데이터가 없다"는 뜻입니다. 하지만 서버가 아직 클라이언트에게 전송할 데이터가 있을 수 있으니까요.

1. 클라이언트가 "나는 끝났어" (FIN) 신호를 보냅니다.
2. 서버가 "알았어, 너의 신호를 받았어" (ACK)라고 응답합니다.
3. 서버도 준비가 되면 "나도 끝났어" (FIN)를 보냅니다.
4. 클라이언트가 "좋아, 이제 정말 끝이다" (ACK)라고 최종 확인합니다.

### 3.5 TIME_WAIT 상태와 SO_REUSEADDR

**문제 상황**: 서버가 크래시되어 재시작해야 합니다. 얼른 같은 포트번호로 다시 서버를 시작하면...

```
ERROR: Address already in use!
```

뭐라고? 아까 종료했는데 포트가 이미 사용 중이라니요?

**원인**: TCP의 **TIME_WAIT** 상태입니다.

연결이 완전히 종료되어도, 운영체제는 바로 그 포트를 재사용하도록 허용하지 않습니다. 왜냐하면, 지연된 패킷이 떠돌아다닐 수 있기 때문입니다.

예를 들어:
1. 이전 연결에서 보낸 패킷이 네트워크 어딘가에 지연되어 있음
2. 같은 포트번호로 새 서버를 시작
3. 그 지연된 패킷이 도착하면? → 혼란!

그래서 TCP는 연결 해제 후 약 2분(TCP 정의상) 동안 그 포트를 "예약"해둡니다. 이것이 **TIME_WAIT** 상태입니다.

**해결책**: `SO_REUSEADDR` 옵션

```cpp
int reuse = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
           (const char*)&reuse, sizeof(reuse));
```

이 옵션을 설정하면, 개발 중에 서버를 빠르게 재시작할 수 있습니다. ("이전 연결이 아직 정리되지 않았지만, 내가 이거 무시할게"라는 뜻)

## 4. UDP 소켓의 동작 원리

### 4.1 TCP는 너무 무거워: UDP의 등장

앞서 TCP를 배웠습니다. 3-way handshake, 데이터 신뢰성 검증, 순서 보장... 모든 것이 안전합니다. 하지만 **항상 안전한 것이 필요할까요?**

**실제 상황 1**: 온라인 게임에서 플레이어의 위치 업데이트를 서버에 전송합니다.
- TCP를 사용: "플레이어가 10,20,30 좌표에 있어. 이건 정말 확실해야 해."
- 실제 필요: "대략 플레이어가 저 근처에 있는 것 같아. 몇 프레임 뒤에 올 다음 위치 업데이트가 더 정확할 거야."
- TCP의 오버헤드(3-way handshake, 재전송 등)는 낭비입니다.

**실제 상황 2**: 라이브 스트리밍 서비스
- 영상 프레임이 1-2초 지연되는 건 괜찮습니다.
- 하지만 모든 프레임의 신뢰성을 검증하느라 더 지연되는 건 싫습니다.
- 몇 개 프레임이 손실되도 사람 눈에는 티가 안 납니다.

이럴 때는 **빠르고 간단한 통신**이 필요합니다. 바로 **UDP (User Datagram Protocol)**입니다.

### 4.2 UDP의 특징

TCP와 달리 UDP는:

1. **비연결형 (Connectionless)**
   - 연결 과정이 없습니다. 3-way handshake? 필요 없음.
   - 그냥 데이터를 보냅니다. "누가 받으려나?"라는 기대 없이.

2. **신뢰성 없음**
   - 패킷이 도착했는지 확인하지 않습니다.
   - 패킷이 손실되어도 신경 쓰지 않습니다.
   - 패킷이 중간에 변조되어도 모릅니다.

3. **순서 보장 안 함**
   - 순서대로 보낸 데이터가 순서대로 올 거라는 보장이 없습니다.
   - 패킷 1, 패킷 2, 패킷 3을 보냈는데, 2, 3, 1 순서로 도착할 수도 있습니다.

4. **빠름**
   - 위의 모든 검증 과정이 없으므로 **매우 빠릅니다.**

### 4.3 UDP의 동작

```
서버                                  클라이언트
|                                      |
| socket() 생성                        |
| bind() 포트에 바인드                 |
|                                      |
| recvfrom() 대기                      | socket() 생성
|                                      |
|                     sendto() 데이터 발송
| <────────────────────────────────────
|
| 데이터 수신 ✓
|
| sendto() 응답 데이터
|─────────────────────────────────────> recvfrom() 수신
|
| close() 종료
|                                      | close() 종료
```

TCP와 가장 큰 차이:
- **연결 과정이 없음**: 바로 데이터 송수신
- **socket → bind → recvfrom()** : TCP의 listen/accept 대신 바로 recvfrom()으로 대기
- **sendto/recvfrom**: 각 메시지마다 "어디서 왔는지" "어디로 갈 건지" 명시

### 4.4 실제 코드 예시

```cpp
// 서버 (UDP)
int server_fd = socket(AF_INET, SOCK_DGRAM, 0);  // SOCK_STREAM 대신 SOCK_DGRAM
bind(server_fd, &server_addr, sizeof(server_addr));

struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);
char buffer[1024];

// 클라이언트의 요청을 기다리지만, 연결하지 않음
ssize_t n = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                     (struct sockaddr*)&client_addr, &client_len);

// 그 클라이언트에게 응답
sendto(server_fd, "OK", 2, 0,
       (struct sockaddr*)&client_addr, client_len);
```

```cpp
// 클라이언트 (UDP)
int sock = socket(AF_INET, SOCK_DGRAM, 0);  // 연결하지 않음!

// 그냥 서버에 데이터를 보냄
sendto(sock, "Hello", 5, 0,
       (struct sockaddr*)&server_addr, sizeof(server_addr));

// 응답 대기
ssize_t n = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
```

### 4.5 TCP vs UDP: 선택 기준

| 상황 | 선택할 것 | 이유 |
|------|---------|------|
| 파일 다운로드 | TCP | 모든 바이트가 정확해야 함 |
| 게임 플레이어 위치 | UDP | 최신 위치만 중요함 |
| 은행 송금 | TCP | 돈이 정확하게 이동해야 함 |
| 라이브 영상 스트리밍 | UDP | 몇 프레임 손실 허용 |
| 웹 브라우저 요청 | TCP | HTML, CSS 다 받아야 함 |
| 온라인 게임 음성 | UDP | 약간의 노이즈는 괜찮음 |

## 5. 소켓 옵션으로 동작 조정하기

### 5.1 소켓 옵션이란?

지금까지 socket을 만들고 연결하는 것을 배웠습니다. 하지만 **그 동작을 미세하게 조정**하고 싶을 때가 있습니다.

예를 들어:
- "내 데이터는 정말 중요한데, 혹시 연결이 끊겨도 자동으로 재연결해야 해"
- "타임아웃이 걸려서 무한 대기하는 거 싫어. 1초 후 포기해줄래?"
- "받는 버퍼가 작은데, 더 크게 해줄 수 있어?"

이런 요청들을 **소켓 옵션**으로 처리합니다. `setsockopt()` 함수를 사용하죠.

### 5.2 자주 사용되는 소켓 옵션

**SO_REUSEADDR: 포트 빨리 재사용하기**

```cpp
int reuse = 1;
setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
           (const char*)&reuse, sizeof(reuse));
bind(sock, &addr, sizeof(addr));
```

**언제 필요한가?**: 앞서 배운 TIME_WAIT 상태 때문에, 종료한 서버를 바로 재시작할 수 없습니다. 이 옵션을 켜면 더 빨리 포트를 재사용할 수 있습니다.

**SO_KEEPALIVE: 좀비 연결 감지하기**

```cpp
int keepalive = 1;
setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
           (const char*)&keepalive, sizeof(keepalive));
```

**언제 필요한가?**: 클라이언트가 비정상 종료되면, 서버는 여전히 그 연결을 "활성"이라고 생각할 수 있습니다. SO_KEEPALIVE는 주기적으로 "너 살아있어?"라고 물어봅니다.

**SO_RCVTIMEO / SO_SNDTIMEO: 타임아웃 설정하기**

```cpp
struct timeval tv;
tv.tv_sec = 5;      // 5초
tv.tv_usec = 0;
setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
           (const char*)&tv, sizeof(tv));
```

**언제 필요한가?**: 데이터를 받으려고 대기 중인데, 상대방이 영원히 데이터를 안 보낸다면? 무한 대기합니다. 이 옵션으로 "5초 기다렸는데 아무것도 없으면, 포기해"라고 설정할 수 있습니다.


## 6. I/O 멀티플렉싱: 여러 클라이언트를 동시에 처리하기

### 6.1 문제: 서버가 100명의 플레이어를 어떻게 처리하나?

지금까지 배운 코드를 보면:

```cpp
// 서버 코드
int client_fd = accept(server_fd, &addr, &len);
char buffer[1024];
read(client_fd, buffer, 1024);  // 여기서 무한 대기 (데이터 올 때까지)
write(client_fd, "OK", 2);
close(client_fd);
```

**문제**: `read()` 함수는 **블로킹(blocking)** 입니다.
- 데이터가 올 때까지 기다립니다.
- 그 동안 다른 클라이언트는 접속할 수 없습니다.
- 100명 중 1명의 `read()`에 갇혀있으면, 나머지 99명은 대기합니다.

**상황**: 플레이어 1번이 게임을 시작했는데, 데이터를 보내지 않고 있습니다.
- 서버는 `read()` 함수에 갇혀 있습니다.
- 플레이어 2, 3, 4, 5... 는 접속하려고 기다리는데, 서버가 응답이 없습니다.
- 최악의 상황입니다!

### 6.2 해결책 1: 스레드 사용하기

한 가지 해결책은 **각 클라이언트마다 스레드를 만드는 것**입니다.

```cpp
// 스레드를 사용한 접근
while(true) {
    int client_fd = accept(server_fd, &addr, &len);

    // 이 클라이언트를 처리할 새로운 스레드 시작
    std::thread client_thread([client_fd]() {
        char buffer[1024];
        read(client_fd, buffer, 1024);  // 이 스레드에서만 블로킹
        write(client_fd, "OK", 2);
        close(client_fd);
    });
    client_thread.detach();  // 스레드를 백그라운드에서 실행
}
```

**장점**: 각 클라이언트가 독립적으로 처리됩니다.

**문제**: 100명이 접속하면 100개의 스레드가 생깁니다.
- 스레드는 메모리를 많이 소비합니다. (각 스레드마다 1-2MB)
- 100명 × 2MB = 200MB의 메모리만 스레드 관리에 사용됩니다.
- 1000명? 10000명? 불가능합니다.

### 6.3 해결책 2: I/O 멀티플렉싱 (Multiplexing)

더 나은 방법이 있습니다. **여러 소켓을 동시에 "감시"**하는 것입니다.

개념: "이 100개의 소켓 중에서, **지금 바로 읽을 수 있는 것**이 뭐가 있나?"라고 물어봅니다.

```
시간 T1:  "소켓 1, 5, 23이 읽을 준비됐어요!"
         → 그 3개만 읽어요.

시간 T2:  "소켓 3, 7, 11, 95가 읽을 준비됐어요!"
         → 그 4개만 읽어요.

시간 T3:  "아무것도 준비 안 됐어요."
         → 기다려요.
```

이렇게 하면:
- 스레드가 필요 없습니다.
- 읽을 준비가 된 것만 읽습니다.
- 1만 개의 연결도 처리 가능합니다!

### 6.4 epoll: 멀티플렉싱의 구현 (Linux)

Linux에서는 `epoll()`이라는 시스템 콜을 사용합니다. 이는 매우 효율적인 멀티플렉싱 방법입니다.

**3가지 함수:**

1. **epoll_create1()**: 감시 시작
2. **epoll_ctl()**: 감시 대상 추가/제거/수정
3. **epoll_wait()**: "지금 읽을 준비된 게 뭐가 있나?" 물어보기

**실제 코드:**

```cpp
// 1단계: 감시 객체 생성
int epfd = epoll_create1(0);

// 2단계: 클라이언트 소켓을 감시 대상에 추가
struct epoll_event ev;
ev.events = EPOLLIN;  // 읽기 가능 이벤트만 감시
ev.data.fd = client_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

// 3단계: 이벤트 루프
struct epoll_event events[MAX_EVENTS];  // 최대 1000개 이벤트 동시 수신
int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);  // 준비될 때까지 대기

for (int i = 0; i < nfds; i++) {
    if (events[i].events & EPOLLIN) {
        // 읽기 가능! 이제 read()는 블로킹되지 않습니다.
        int fd = events[i].data.fd;
        char buffer[1024];
        read(fd, buffer, 1024);
    }
}
```

### 6.5 게임 서버의 메인 루프 (epoll 활용)

실제 게임 서버는 이렇게 작동합니다:

```cpp
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, &addr, sizeof(addr));
listen(server_fd, 5);

int epfd = epoll_create1(0);

// 서버 소켓(신규 접속 감지용)도 감시 대상에 추가
struct epoll_event server_ev;
server_ev.events = EPOLLIN;
server_ev.data.fd = server_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &server_ev);

// 메인 루프 (항상 실행)
while(true) {
    struct epoll_event events[100];  // 최대 100개 이벤트 동시 처리
    int nfds = epoll_wait(epfd, events, 100, -1);

    for (int i = 0; i < nfds; i++) {
        if (events[i].data.fd == server_fd) {
            // 새로운 클라이언트 접속!
            int client_fd = accept(server_fd, NULL, NULL);

            // 이 새 클라이언트도 감시 대상에 추가
            struct epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.fd = client_fd;
            epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

        } else {
            // 기존 클라이언트에서 데이터 수신
            int fd = events[i].data.fd;
            char buffer[1024];
            int n = read(fd, buffer, 1024);

            if (n == 0) {
                // 클라이언트가 연결 종료
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
            } else {
                // 데이터 처리
                handleClientMessage(fd, buffer, n);
            }
        }
    }
}
```

### 6.6 왜 epoll이 특별한가?

**성능 비교:**

| 방식 | 시간 복잡도 | 1000명 처리 | 10000명 처리 |
|------|----------|----------|-----------|
| 스레드 | O(n) | 가능 | 메모리 부족 |
| select() | O(n) | 느림 | 매우 느림 |
| poll() | O(n) | 느림 | 매우 느림 |
| **epoll** | **O(1)** | **빠름** | **매우 빠름** |

**epoll이 빠른 이유:**
- 커널이 "준비된 소켓"만 반환합니다.
- 모든 소켓을 검사할 필요가 없습니다.
- 10000개 중 10개만 준비되면, 10개만 처리합니다!

### 6.7 Level-Triggered vs Edge-Triggered

epoll은 두 가지 모드를 지원합니다.

**Level-Triggered (LT) - 기본 모드:**
```
클라이언트가 "데이터 준비됐어요!" 상태 → 계속 알려줍니다.
- 읽을 데이터가 있으면, 계속 epoll_wait()에서 알려줍니다.
- 더 안전합니다. (데이터를 깜빡 놓쳐도 다시 알려줌)
```

**Edge-Triggered (ET) - 고성능 모드:**
```
클라이언트가 "데이터 준비됐어요!" 상태로 변할 때만 → 한 번만 알려줍니다.
- 상태가 변할 때(예: 데이터 없음→있음)만 알려줍니다.
- 더 빠르지만, 모든 데이터를 반드시 읽어야 합니다.
- 놓치면 그 클라이언트는 다시 데이터를 보낼 때까지 알려주지 않습니다!
```

---

## 7. 소켓 상태와 TCP 상태 머신

### 7.1 TCP의 인생 여정: 상태 머신 이해하기

TCP 연결은 **마치 사람의 인생처럼 여러 상태**를 거칩니다.

**서버의 관점:**
1. **CLOSED**: "난 소켓이 없어"
2. **LISTEN**: "난 포트를 열고 누가 올 때까지 기다리고 있어"
3. **SYN_RCVD**: "클라이언트가 "안녕?"이라고 했어. 난 "좋아, 난 준비됐어"라고 했어"
4. **ESTABLISHED**: "우리 연결됐어! 대화할 준비 완료!"
5. **FIN_WAIT_1**: "클라이언트가 "난 끝났어"라고 했어. 난 "알았어"라고 했어"
6. **FIN_WAIT_2**: "클라이언트의 최종 ACK를 기다리는 중..."
7. **TIME_WAIT**: "연결이 끝났지만, 혹시 늦게 도착한 패킷이 있을 수 있으니 잠깐 기다릴게"
8. **CLOSED**: "완전히 끝"

**클라이언트의 관점:**
1. **CLOSED**: "난 서버에 아직 안 연결했어"
2. **SYN_SENT**: "서버에 "안녕, 연결할래?"라고 보냈어"
3. **ESTABLISHED**: "서버가 "좋아"라고 했어! 연결됐다!"
4. **FIN_WAIT_1**: "난 "끝내자"라고 했어"
5. **CLOSED**: "완전히 끝"

```
클라이언트                      서버
                             CLOSED
                               ↓ (socket, bind, listen)
                             LISTEN
                               ↑
connect() →
  ↓                            ↓ (receive SYN)
SYN_SENT                    SYN_RCVD
  ↓                            ↓
  ← ← ← ← ← ← ← ← ← ← ← (receive SYN+ACK)
ESTABLISHED              ESTABLISHED
  ↓ (send ACK)               ↓
  → → → → → → → → → → → → (ACK)
  ↓                            ↓ (receive ACK)
[데이터 교환]             [데이터 교환]
  ↓                            ↓
close()                      close()
  ↓                            ↓ (receive FIN)
FIN_WAIT_1                FIN_WAIT_1
  ↓                            ↓
  ← ← ← ← ← ← ← ← ← ← ← (ACK)
FIN_WAIT_2                 CLOSING
  ↓                            ↓ (send FIN)
  ← ← ← ← ← ← ← ← ← ← ← (FIN)
TIME_WAIT                   CLOSED
  ↓ (2MSL timeout)
CLOSED
```

### 7.2 도대체 TIME_WAIT 상태가 뭐가 필요해?

앞서 배웠듯이, TCP 연결이 종료되면 **TIME_WAIT** 상태로 2분 정도 기다립니다. "뭐하는 거야?"라고 생각할 수 있습니다.

**시나리오**: 4-way handshake의 마지막 ACK가 손실되었다면?

```
클라이언트                                        서버
   | close() → FIN 전송                           |
   |────────────────────────────────────────────>|
   |                                              | ACK 전송
   |<────────────────────────────────────────────|
   |                                              | close() → FIN 전송
   |<────────────────────────────────────────────|
   | ACK 전송                                     |
   |────────────────────────────X (손실됨!)      |
   |                                              | "어? ACK를 못 받았는데?"
   |                                              | 다시 FIN 전송? (재시도)
   |<────────────────────────────────────────────|
   |
```

만약 TIME_WAIT 상태가 없다면?
- 클라이언트가 즉시 같은 포트로 새 서버를 시작
- 서버의 재시도 FIN이 도착 → 혼란!
- 새 연결로 착각할 수 있음

**TIME_WAIT의 역할:**
1. 마지막 ACK 손실 시, 서버의 재시도 FIN에 다시 ACK 보냄
2. 네트워크에 떠도는 지연된 패킷이 새 연결에 영향 주지 않도록 기다림

**기간:**
- 일반적으로 30초~2분 (운영체제마다 다름)
- `netstat -an` 명령으로 확인 가능

이것이 "Address already in use" 에러의 원인입니다. 개발 중에 서버를 빠르게 재시작하려면 `SO_REUSEADDR` 옵션을 켜야 합니다.

## 8. 오류 처리: "뭐가 잘못됐는지" 알아차리기

### 8.1 네트워크 프로그래밍은 실패로 가득 찼다

소켓 프로그래밍을 할 때, **모든 것이 실패할 수 있다**는 관점으로 시작해야 합니다:
- 서버가 실행되지 않았을 수도
- 네트워크가 끊겼을 수도
- 상대방이 갑자기 연결을 끊었을 수도
- 방화벽이 패킷을 차단했을 수도

이런 상황들은 **에러 코드**로 반환됩니다. 이를 제대로 처리하지 않으면, 버그를 찾을 수 없습니다.

### 8.2 주요 에러 코드와 대처법

**ECONNREFUSED: 연결 거부**
```
connect()가 이 에러를 반환했다?
→ "서버가 실행되지 않거나, 그 포트가 열려있지 않아요"
→ 대처: 서버 시작 확인, 포트 번호 확인
```

```cpp
if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    if (errno == ECONNREFUSED) {
        printf("서버가 없습니다. 서버를 먼저 시작하세요!\n");
    }
}
```

**EADDRINUSE: 주소가 이미 사용 중**
```
bind()가 이 에러를 반환했다?
→ "그 포트는 이미 누군가 사용 중이에요"
→ 원인 1: 같은 서버를 두 번 시작함
→ 원인 2: TIME_WAIT 상태의 포트 (서버 재시작 직후)
→ 대처: SO_REUSEADDR 옵션 사용
```

```cpp
int reuse = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
           (const char*)&reuse, sizeof(reuse));
bind(server_fd, &addr, sizeof(addr));
```

**ETIMEDOUT: 연결 시간 초과**
```
connect()가 시간 초과되었다?
→ "서버가 너무 오래 응답이 없어요"
→ 원인: 서버가 느리거나, 네트워크 지연이 심함
→ 대처: 네트워크 확인, SO_RCVTIMEO 설정
```

**ECONNRESET: 연결이 원격에 의해 리셋됨**
```
read()나 write() 중에 이 에러가 났다?
→ "상대방이 갑자기 연결을 끊었어요"
→ 원인: 상대 프로세스 크래시, 상대 네트워크 끊김
→ 대처: 연결 재시도, 예외 처리
```

**EPIPE: 이미 닫힌 소켓에 write**
```
write()가 이 에러를 반환했다?
→ "상대방이 먼저 연결을 닫았는데, 넌 아직 보내려고 해?"
→ 원인: 상대방이 먼저 close() 호출
→ 대처: write() 전에 항상 연결 상태 확인
```

**EMFILE / ENFILE: 파일 디스크립터 부족**
```
accept()가 이 에러를 반환했다?
→ "더 이상 소켓을 만들 수 없어요. 시스템 한계에 도달했어요"
→ 원인: 메모리 부족, accept한 소켓을 close() 하지 않음
→ 대처: 메모리 누수 찾기, 리소스 정리
```

**EAGAIN / EWOULDBLOCK: 논블로킹 소켓에서 데이터 없음**
```
논블로킹 소켓에서 read()를 호출했는데 이 에러?
→ "지금 읽을 데이터가 없어요. 나중에 다시 시도하세요"
→ 멀티플렉싱(epoll)과 함께 사용됨
```

### 8.3 올바른 에러 처리의 패턴

```cpp
// ❌ 나쁜 예: 에러 처리 없음
int client_fd = accept(server_fd, &addr, &len);
read(client_fd, buffer, 1024);

// ✓ 좋은 예: 모든 반환값 확인
int client_fd = accept(server_fd, &addr, &len);
if (client_fd == -1) {
    printf("accept 실패: %s\n", strerror(errno));
    continue;  // 다음 요청으로
}

ssize_t n = read(client_fd, buffer, 1024);
if (n == -1) {
    printf("read 실패: %s\n", strerror(errno));
    close(client_fd);
    continue;
} else if (n == 0) {
    printf("클라이언트가 연결을 닫았습니다\n");
    close(client_fd);
    continue;
}

// n > 0: 정상적으로 데이터 수신
```

## 9. 소켓 프로그래밍 마무리: 주의할 점들

### 9.1 "반드시" 확인해야 할 것들

**1. 모든 반환값 확인하기**

```cpp
// ❌ 나쁜 예
socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, &addr, sizeof(addr));

// ✓ 좋은 예
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
if (server_fd == -1) {
    perror("socket");
    exit(1);
}

if (bind(server_fd, &addr, sizeof(addr)) == -1) {
    perror("bind");
    close(server_fd);
    exit(1);
}
```

**2. 바이트 순서 변환**

컴퓨터는 **바이트 순서**를 어떻게 해석하는지에 따라 숫자가 달라집니다.

예: 포트 번호 8080을 어떻게 저장할까요?
- **Big-Endian**: `0x1F 0x90` (높은 자리부터)
- **Little-Endian**: `0x90 0x1F` (낮은 자리부터)

네트워크는 **Big-Endian** (호스트 바이트 순서)을 사용합니다. 따라서 변환이 필요합니다.

```cpp
// ❌ 나쁜 예: 변환 안 함
addr.sin_port = 8080;

// ✓ 좋은 예: htons() 사용 (host to network short)
addr.sin_port = htons(8080);

// 다른 변환 함수들:
htonl() // 32-bit 정수
ntohs() // 네트워크 → 호스트 (16-bit)
ntohl() // 네트워크 → 호스트 (32-bit)
```

**3. 리소스 정리**

소켓도 파일입니다. 사용 후 **반드시 닫아야 합니다.**

```cpp
// ❌ 나쁜 예: 소켓을 닫지 않음
int client_fd = accept(server_fd, &addr, &len);
read(client_fd, buffer, 1024);
// 함수 끝 → client_fd가 열린 상태로 남음 (메모리 누수!)

// ✓ 좋은 예: 항상 닫기
int client_fd = accept(server_fd, &addr, &len);
if (client_fd != -1) {
    read(client_fd, buffer, 1024);
    close(client_fd);  // ← 중요!
}
```

### 9.2 "자주 실수하는" 것들

**실수 1: 부분 전송/수신 미처리**

```cpp
// ❌ 나쁜 예: 5바이트를 보냈다고 가정
write(sock, "Hello", 5);

// 실제로는 3바이트만 전송되었을 수도!
// "Hel"만 보내지고 "lo"는 나중에?

// ✓ 좋은 예: 반환값 확인
ssize_t n = write(sock, "Hello", 5);
if (n == -1) {
    perror("write");
} else if (n < 5) {
    printf("%ld바이트만 전송됨\n", n);
}
```

**실수 2: IPv6 미지원 (IPv4 하드코딩)**

```cpp
// ❌ 나쁜 예: AF_INET만 지원
socket(AF_INET, SOCK_STREAM, 0);

// ✓ 좋은 예: getaddrinfo() 사용 (IPv4/IPv6 모두 지원)
struct addrinfo hints = {0};
hints.ai_family = AF_UNSPEC;  // IPv4/IPv6 자동 선택
// ...
```

**실수 3: 타임아웃 미설정**

```cpp
// ❌ 나쁜 예: 무한 대기
read(sock, buffer, 1024);  // 데이터가 안 오면 영원히 기다림

// ✓ 좋은 예: 타임아웃 설정
struct timeval tv;
tv.tv_sec = 5;   // 5초
tv.tv_usec = 0;
setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
           (const char*)&tv, sizeof(tv));
read(sock, buffer, 1024);  // 5초 후 타임아웃
```

**실수 4: SIGPIPE 처리 안 함 (Linux/Unix)**

```cpp
// ❌ 나쁜 예: SIGPIPE 미처리
write(sock, data, len);  // 닫힌 소켓에 write → SIGPIPE 시그널 → 프로세스 종료!

// ✓ 좋은 예: SIGPIPE 무시
signal(SIGPIPE, SIG_IGN);
write(sock, data, len);  // 에러 반환되지만, 프로세스 종료 안 함
```

**실수 5: TIME_WAIT 상태 미고려**

이미 배웠듯이, 서버를 재시작할 때 "Address already in use" 에러가 나면 답답합니다.

```cpp
// ✓ 서버 시작 시 항상 이것을 추가
int reuse = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
           (const char*)&reuse, sizeof(reuse));
bind(server_fd, &addr, sizeof(addr));
```

### 9.3 요약: 네트워크 프로그래밍의 황금 규칙

1. **모든 반환값을 확인하라.** (socket, bind, connect, accept, read, write, close)
2. **모든 에러를 처리하라.** (errno 확인, strerror() 사용)
3. **모든 리소스를 정리하라.** (close() 호출)
4. **바이트 순서를 변환하라.** (htons, htonl 등)
5. **타임아웃을 설정하라.** (SO_RCVTIMEO, SO_SNDTIMEO)
6. **부분 전송/수신을 처리하라.** (루프로 전체 데이터 보내기)
7. **플랫폼 차이를 고려하라.** (Windows vs Linux, IPv4 vs IPv6)
