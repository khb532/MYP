# Part 4: UDP 소켓의 동작 원리

UDP(User Datagram Protocol)는 **비연결형이고 신뢰성 없는** 전송 프로토콜입니다. TCP와는 정반대의 특성을 가지며, 속도와 단순성을 우선시합니다.

---

## 4.1 UDP의 특징

### 비연결형 (Connectionless)

**엽서와 비슷:**
- 주소만 쓰고 바로 발송
- 연결 설정/해제 과정 없음
- 각 메시지가 독립적

**TCP와의 비교:**
```
TCP: 전화 통화
  1. 전화를 걸어 연결
  2. 대화
  3. 통화 종료 인사
  4. 끊기

UDP: 엽서 우편
  1. 주소 쓰고 발송 (끝!)
```

**장점:**
- 빠른 시작 (handshake 불필요)
- 오버헤드 최소
- 상태 정보 유지 불필요

### 신뢰성 없음 (Unreliable)

**보장하지 않는 것:**
- ❌ 데이터 도착 보장 없음 (패킷 손실 가능)
- ❌ 순서 보장 없음
- ❌ 중복 제거 없음
- ❌ 흐름 제어 없음
- ❌ 혼잡 제어 없음

**"신뢰성 없음" = "나쁜 것"?**

아닙니다! 용도에 따라 장점이 됩니다.

**예: 실시간 영상 스트리밍**
```
프레임 1 [✓]
프레임 2 [✗] 손실
프레임 3 [✓]
프레임 4 [✓]

TCP라면?
→ 프레임 2 재전송 대기
→ 프레임 3, 4는 버퍼에서 대기
→ 지연 발생, 끊김 현상

UDP라면?
→ 프레임 2 무시하고 계속 진행
→ 사람이 거의 인지 못함
→ 부드러운 재생
```

### 메시지 지향 (Message-oriented)

**메시지 지향이란?**

데이터를 **바이트 스트림**이 아닌 **독립적인 메시지 단위**로 전송하는 방식입니다.

**핵심 개념: 메시지 경계(Message Boundary)**

"메시지 경계"란 각 메시지의 **시작과 끝이 명확히 구분**되는 것을 의미합니다.

**비유로 이해하기:**

**TCP (스트림) = 수도꼭지에서 나오는 물**
```
[컵1] [컵2] [컵3]을 연속으로 부음
→ 받는 쪽에서는 하나의 연속된 물줄기
→ 어디서 컵1이 끝나고 컵2가 시작하는지 모름
→ 경계가 없음!
```

**UDP (데이터그램) = 택배 상자**
```
[상자1] [상자2] [상자3]을 개별 전송
→ 받는 쪽에서는 각각의 독립된 상자
→ 각 상자가 명확히 구분됨
→ 경계가 보존됨!
```

**코드로 확인하기:**

**TCP의 경우 (경계 없음):**
```c
// 송신측
send(sock, "Hello", 5, 0);
send(sock, "World", 5, 0);
send(sock, "!!!", 3, 0);

// 수신측 - 다양한 경우 발생 가능
recv(sock, buf, 100, 0);  // 가능한 결과:
                          // "HelloWorld!!!" (13바이트, 한 번에)
                          // "Hell" (4바이트)
                          // "HelloWo" (7바이트)
                          // "H" (1바이트)
                          // ... 무수히 많은 조합 가능
```

**왜 이런가?**
- TCP는 바이트 스트림 프로토콜
- 네트워크 상황에 따라 패킷이 합쳐지거나 쪼개짐
- send() 호출 횟수와 recv() 호출 횟수가 일치하지 않음

**UDP의 경우 (경계 보존):**
```c
// 송신측
sendto(sock, "Hello", 5, 0, ...);
sendto(sock, "World", 5, 0, ...);
sendto(sock, "!!!", 3, 0, ...);

// 수신측 - 항상 동일
recvfrom(sock, buf, 100, 0, ...);  // "Hello" (정확히 5바이트)
recvfrom(sock, buf, 100, 0, ...);  // "World" (정확히 5바이트)
recvfrom(sock, buf, 100, 0, ...);  // "!!!" (정확히 3바이트)

// 절대로 "HelloWorld"로 합쳐지지 않음!
// 절대로 "Hel"과 "lo"로 쪼개지지 않음!
```

**왜 이런가?**
- 각 sendto() 호출이 하나의 독립적인 데이터그램 생성
- 데이터그램은 원자적(atomic) 단위
- recvfrom() 한 번에 데이터그램 하나씩만 수신

**실전 예제: 채팅 메시지**

**TCP 사용 시 (문제 발생):**
```c
// 송신측
send(sock, "Alice: Hi", 9, 0);
send(sock, "Bob: Hello", 10, 0);

// 수신측
char buf[100];
int n = recv(sock, buf, 100, 0);
// 결과: "Alice: HiBob: Hello" (19바이트)
// → 두 메시지가 합쳐짐!
// → 어디서 첫 번째 메시지가 끝나는지 알 수 없음!

// 해결 방법 필요:
// 1. 고정 길이 (예: 모든 메시지 100바이트)
// 2. 구분자 (예: '\n'으로 구분)
// 3. 길이 헤더 (예: 앞 4바이트에 메시지 길이 저장)
```

**UDP 사용 시 (자동 해결):**
```c
// 송신측
sendto(sock, "Alice: Hi", 9, 0, ...);
sendto(sock, "Bob: Hello", 10, 0, ...);

// 수신측
char buf[100];
int n = recvfrom(sock, buf, 100, 0, ...);
// 결과: "Alice: Hi" (9바이트만)
buf[n] = '\0';
printf("%s\n", buf);  // 출력: Alice: Hi

n = recvfrom(sock, buf, 100, 0, ...);
// 결과: "Bob: Hello" (10바이트만)
buf[n] = '\0';
printf("%s\n", buf);  // 출력: Bob: Hello

// 별도의 구분 처리 불필요!
// 메시지 경계가 자동으로 유지됨!
```

**메시지 경계 보존의 장점:**

**1. 프로그래밍 단순화**
```c
// TCP: 메시지 구분 로직 필요
struct Message {
    uint32_t length;  // 추가 필드 필요
    char data[...];
};

// UDP: 그냥 보내고 받으면 됨
sendto(sock, msg, msg_size, 0, ...);
recvfrom(sock, buf, buf_size, 0, ...);
```

**2. 독립적 처리**
```c
// 각 데이터그램을 독립적으로 처리 가능
while (1) {
    ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, ...);
    process_one_message(buf, n);  // 완전한 하나의 메시지
}
```

**3. 메시지 타입 구분 용이**
```c
struct Packet {
    uint8_t type;  // 메시지 타입
    char data[...];
};

// 받을 때마다 완전한 Packet 구조체 하나
recvfrom(sock, &pkt, sizeof(pkt), 0, ...);

switch (pkt.type) {
    case TYPE_CHAT: ...
    case TYPE_POSITION: ...
    case TYPE_ACTION: ...
}
```

**메시지 경계 보존의 제약:**

**1. 버퍼가 작으면 데이터 손실**
```c
// 송신측: 1000바이트 전송
sendto(sock, big_data, 1000, 0, ...);

// 수신측: 500바이트 버퍼
char buf[500];
recvfrom(sock, buf, 500, 0, ...);
// 결과: 앞 500바이트만 받고, 나머지 500바이트는 영원히 손실!
// 경고도 없음!

// 해결 1: 충분히 큰 버퍼 사용 (힙 할당 권장)
char *buf = malloc(65507);  // UDP 최대 페이로드 크기
if (buf == NULL) { /* 에러 처리 */ }
// ... 사용 ...
free(buf);

// 해결 2: 스택에 할당 (작은 크기 권장)
char buf[1500];  // MTU 크기 이하로 제한하는 경우
```

**2. 크기 제한**
- UDP는 데이터그램 하나의 크기가 제한됨
- 이론적 최대: 65,535바이트 (IP 패킷 최대 크기)
- UDP 페이로드 최대: 65,507바이트 (65,535 - 20(IP헤더) - 8(UDP헤더))
- 실용 권장: 1,472바이트 (MTU 1,500 - 20 - 8, 단편화 방지)
- 큰 데이터는 직접 분할 필요


**비교 정리:**

| 특성 | TCP (스트림) | UDP (메시지 지향) |
|------|-------------|-----------------|
| 메시지 경계 | ❌ 없음 | ✅ 보존 |
| send/recv 대응 | 불일치 | 일대일 대응 |
| 데이터 합쳐짐 | ✅ 가능 | ❌ 없음 |
| 데이터 쪼개짐 | ✅ 가능 | ❌ 없음 |
| 구분 로직 필요 | ✅ 필요 | ❌ 불필요 |
| 프로그래밍 복잡도 | 높음 | 낮음 |

### 헤더 오버헤드

**오버헤드란?**

실제 데이터 외에 **프로토콜 동작을 위해 추가되는 부가 정보**를 의미합니다. 헤더는 주소, 제어 정보 등을 담고 있지만, 실제 전송하려는 데이터는 아닙니다.

**비유:**
- 택배 상자에서 실제 물건 = 데이터
- 택배 송장, 포장재 = 오버헤드

**왜 오버헤드가 중요한가?**

**1. 대역폭 낭비**
```
1GB 영상을 1,000바이트 패킷으로 전송한다면:

TCP: 1,000,000개 패킷 × 20바이트 헤더 = 20MB 추가
UDP: 1,000,000개 패킷 × 8바이트 헤더 = 8MB 추가

→ 12MB 절약!
```

**2. 작은 메시지에서 더욱 비효율**
```
센서가 온도 데이터 2바이트를 매초 전송:

TCP: 2 + 20 = 22바이트 (오버헤드 90%!)
UDP: 2 + 8 = 10바이트 (오버헤드 80%)

→ UDP가 절반 이상 효율적
```

**3. 처리 속도**
- 헤더가 작을수록 파싱/검증 빠름
- CPU 사용량 감소

**UDP vs TCP 헤더 상세 비교:**

**UDP 헤더: 8바이트 (고정)**
```
┌─────────────────┬─────────────────┐
│  출발지 포트     │  목적지 포트     │  2 + 2 = 4바이트
├─────────────────┼─────────────────┤
│     길이        │    체크섬        │  2 + 2 = 4바이트
└─────────────────┴─────────────────┘

총 8바이트 - 단순하고 빠름
```

**TCP 헤더: 20~60바이트 (가변)**
```
┌─────────────────┬───────────────┐
│  출발지 포트     │  목적지 포트   │  4바이트
├─────────────────────────────────┤
│        시퀀스 번호               │  4바이트 (순서 보장)
├─────────────────────────────────┤
│      확인 응답 번호              │  4바이트 (신뢰성)
├───────┬──────────┬──────────────┤
│헤더길이│플래그    │  윈도우 크기  │  4바이트 (흐름 제어)
├─────────────────┬───────────────┤
│    체크섬        │ 긴급 포인터   │  4바이트
├─────────────────────────────────┤
│        옵션 (0~40바이트)         │  가변
└─────────────────────────────────┘

총 20~60바이트 - 복잡하지만 기능 많음
```

**왜 TCP 헤더가 더 클까?**

TCP가 제공하는 기능들을 위해 필요:
- 시퀀스 번호: 순서 보장
- 확인 응답 번호: 신뢰성
- 윈도우 크기: 흐름 제어
- 플래그: 연결 관리 (SYN, ACK, FIN 등)
- 옵션: 타임스탬프, MSS 등

**실제 영향 비교:**

**시나리오 1: IoT 센서 (작은 데이터)**
```
데이터: 10바이트 (온도, 습도)
초당 전송: 1회
하루 전송량:

TCP: (10 + 20) × 86,400 = 2.59MB
UDP: (10 + 8) × 86,400 = 1.56MB

→ 40% 대역폭 절약
```

**시나리오 2: 게임 위치 업데이트 (빈번한 전송)**
```
데이터: 16바이트 (x, y, z, rotation)
초당 전송: 60회 (60 FPS)
분당 전송량:

TCP: (16 + 20) × 60 × 60 = 129.6KB
UDP: (16 + 8) × 60 × 60 = 86.4KB

→ 33% 대역폭 절약
```

**시나리오 3: 영상 스트리밍 (큰 데이터)**
```
데이터: 1,400바이트 (비디오 프레임 조각)
초당 전송: 30회
분당 전송량:

TCP: (1,400 + 20) × 30 × 60 = 2.56MB
UDP: (1,400 + 8) × 30 × 60 = 2.53MB

→ 1% 차이 (큰 데이터에서는 오버헤드 비율 낮음)
```

**결론:**
- **작은 데이터, 빈번한 전송**: UDP 오버헤드 이점 큼
- **큰 데이터**: 오버헤드 차이 미미
- **실시간 통신**: 헤더 크기보다 지연이 더 중요

---

## 4.2 UDP 소켓 생명주기

### 전체 흐름도

```
서버                                클라이언트
─────────────────────────────────────────────────
socket()                            socket()
  ↓                                   ↓
bind()         (필수)                bind() (선택)
  ↓                                   ↓
  ↓                                   ↓
recvfrom() ←────────────────────── sendto()
  ↓           데이터 →                 ↓
  ↓                                   ↓
sendto() ──────────────────────────→ recvfrom()
  ↓         ← 데이터                   ↓
  ↓                                   ↓
close()                             close()

[연결 설정/해제 없음!]
```

### TCP와의 비교

| 단계 | TCP 서버 | UDP 서버 |
|------|---------|---------|
| 소켓 생성 | socket() | socket() |
| 주소 할당 | bind() | bind() |
| 연결 대기 | listen() | ❌ 없음 |
| 연결 수락 | accept() | ❌ 없음 |
| 데이터 송수신 | send()/recv() | sendto()/recvfrom() |

**핵심 차이:**
- listen(), accept() 불필요
- 바로 데이터 송수신 가능
- 훨씬 단순!

---

## 4.3 주요 함수

### 4.3.1 socket() - UDP 소켓 생성

**함수 원형:**
```c
int socket(int domain, int type, int protocol);
```

**UDP 소켓 생성:**
```c
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// 또는 명시적으로
int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

if (sockfd < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
}
```

**차이점:**
- `SOCK_STREAM` → `SOCK_DGRAM`
- 그 외는 TCP와 동일

---

### 4.3.2 bind() - 주소 할당

**함수 원형:**
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

**언제 필요한가?**

**서버:**
- **반드시 필요**
- 클라이언트가 어디로 보낼지 알아야 함
- 잘 알려진 포트 사용

**클라이언트:**
- **선택사항**
- 생략하면 OS가 임시 포트 자동 할당
- 특별한 이유가 없으면 생략

---

**왜 이런 차이가 있는가?**

이 차이는 서버와 클라이언트의 **역할 차이**와 **운영체제의 포트 할당 메커니즘**에서 비롯됩니다.

### 서버가 bind()를 반드시 해야 하는 이유

**1. 클라이언트가 찾을 수 있는 "주소"가 필요**

서버는 **수동적으로 요청을 기다리는** 역할입니다. 클라이언트가 서버를 찾을 수 있으려면:
- 서버의 IP 주소를 알아야 함
- 서버의 **포트 번호**를 알아야 함

**비유: 식당의 주소**
```
서버 = 식당
- 고정된 주소가 필요 (서울시 강남구 123번지)
- 손님(클라이언트)이 찾아올 수 있어야 함
- 주소가 없거나 매번 바뀌면 손님이 찾을 수 없음

클라이언트 = 손님
- 자기 집 주소는 식당에 알릴 필요 없음
- 그냥 식당으로 가면 됨
```

**2. "잘 알려진 포트(Well-known port)" 사용**

서버는 표준화된 포트를 사용합니다:
```
DNS 서버:    53번 포트
HTTP 서버:   80번 포트
HTTPS 서버:  443번 포트
게임 서버:   특정 포트 (예: 7777번)
```

**클라이언트는 어떻게 알까?**
```c
// 클라이언트 코드
struct sockaddr_in server_addr;
server_addr.sin_port = htons(53);  // DNS 서버는 53번 포트라고 "알고 있음"
sendto(sock, query, len, 0, (struct sockaddr*)&server_addr, ...);
```

**만약 서버가 bind()를 안 하면?**
```c
// 서버 (bind 생략)
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// bind()를 안 함!

char buf[1024];
recvfrom(sockfd, buf, sizeof(buf), 0, ...);
// ❌ 오류: "어떤 포트로 들어오는 데이터를 받을지 모름"
```

**실제 오류 상황:**
- `recvfrom()`은 실패하거나
- OS가 임의의 포트를 할당하지만, 클라이언트는 이 포트 번호를 모름

### 클라이언트가 bind()를 생략할 수 있는 이유

**1. 운영체제의 자동 포트 할당**

클라이언트가 bind()를 호출하지 않고 `sendto()`를 처음 호출하면:

**OS가 자동으로 하는 일:**
```
Step 1: 임시 포트(ephemeral port) 할당
        - IANA 권장 범위: 49152-65535
        - Linux 실제 기본값: 32768-60999
          (/proc/sys/net/ipv4/ip_local_port_range에 정의)
        - Windows: 49152-65535 (IANA 권장 따름)

Step 2: 내부적으로 bind() 수행
        - IP: 0.0.0.0 (모든 인터페이스)
        - 포트: 할당된 임시 포트

Step 3: sendto() 실행
        - 이제 출발지 포트가 정해짐
```

**코드 예제:**
```c
// 클라이언트 (bind 생략)
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// bind()를 호출하지 않음!

struct sockaddr_in server_addr;
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);  // 서버 포트
inet_pton(AF_INET, "192.168.1.10", &server_addr.sin_addr);

// 첫 sendto() 호출 시:
sendto(sockfd, "Hello", 5, 0,
       (struct sockaddr*)&server_addr, sizeof(server_addr));

// OS가 자동으로 수행:
// bind(sockfd, {0.0.0.0:54321}, ...);  ← 예: 54321번 포트 자동 할당
// 그 다음 실제로 sendto() 전송
```

**실제 패킷 모습:**
```
출발지 IP:포트    → 192.168.1.5:54321  (OS가 자동 할당)
목적지 IP:포트    → 192.168.1.10:8080  (서버의 잘 알려진 포트)
```

**2. 클라이언트는 "찾아갈 수 있으면" 됨**

클라이언트는 능동적으로 서버에 연결하는 역할:
- 서버의 주소를 알고 있음 (설정 파일, DNS 조회 등)
- 자신의 포트 번호는 서버에 자동으로 알려짐 (패킷의 출발지 포트)

**서버가 응답하는 방법:**
```c
// 서버
struct sockaddr_in client_addr;
socklen_t len = sizeof(client_addr);

// 클라이언트로부터 수신
recvfrom(sockfd, buf, sizeof(buf), 0,
         (struct sockaddr*)&client_addr, &len);
// client_addr에 클라이언트의 IP:포트가 자동으로 저장됨!

// 그 주소로 응답
sendto(sockfd, response, resp_len, 0,
       (struct sockaddr*)&client_addr, len);
// 서버는 client_addr (예: 192.168.1.5:54321)로 응답
```

### 클라이언트가 bind()를 사용하는 경우

**일반적으로는 불필요하지만, 다음 경우에는 사용:**

**1. 특정 포트 사용 요구**
```c
// 방화벽 정책: "클라이언트는 5000번 포트만 사용 가능"
struct sockaddr_in local_addr;
local_addr.sin_family = AF_INET;
local_addr.sin_port = htons(5000);
local_addr.sin_addr.s_addr = INADDR_ANY;

bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));
// 이제 이 소켓은 항상 5000번 포트 사용
```

**2. 특정 네트워크 인터페이스 지정**
```c
// 여러 네트워크 카드가 있는 경우
// eth0 (192.168.1.5): 내부 네트워크
// eth1 (203.0.113.5): 외부 네트워크

struct sockaddr_in local_addr;
local_addr.sin_family = AF_INET;
local_addr.sin_port = htons(0);  // 포트는 OS가 할당
inet_pton(AF_INET, "192.168.1.5", &local_addr.sin_addr);  // 특정 IP

bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));
// eth0 인터페이스만 사용
```

**3. P2P 애플리케이션**
```c
// 두 피어가 서로에게 연결
// 양쪽 모두 서버이자 클라이언트

// 포트 5555로 bind
bind(sockfd, {..., 5555}, ...);

// 상대방에게 "내 포트는 5555야"라고 알려줌
// 상대방도 이 포트로 데이터 전송
```

### 실제 동작 비교

**시나리오 1: 서버가 bind()를 안 한 경우**
```c
// 서버 (잘못된 예)
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// bind() 생략!

struct sockaddr_in client_addr;
socklen_t len = sizeof(client_addr);
recvfrom(sockfd, buf, sizeof(buf), 0,
         (struct sockaddr*)&client_addr, &len);

// 결과:
// - recvfrom()이 블로킹되거나
// - OS가 임의 포트(예: 54321) 할당
// - 하지만 클라이언트는 서버가 54321번 포트라는 걸 모름
// → 통신 불가능!
```

**시나리오 2: 클라이언트가 bind()를 안 한 경우 (정상)**
```c
// 클라이언트 (정상 예)
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// bind() 생략!

struct sockaddr_in server_addr;
server_addr.sin_port = htons(8080);  // 서버 포트는 알고 있음
inet_pton(AF_INET, "192.168.1.10", &server_addr.sin_addr);

sendto(sockfd, "Hello", 5, 0, ...);
// OS가 자동으로 포트 할당 (예: 54321)
// 패킷: 192.168.1.5:54321 → 192.168.1.10:8080

recvfrom(sockfd, buf, sizeof(buf), 0, ...);
// 서버의 응답을 54321번 포트로 수신
// 패킷: 192.168.1.10:8080 → 192.168.1.5:54321

// → 통신 성공!
```

### 핵심 원리 정리

**서버 = 수동적 대기자**
```
반드시 bind() 필요:
1. 클라이언트가 "찾을 수 있는" 고정 주소 필요
2. 잘 알려진 포트 사용
3. bind() 없이는 recvfrom()이 "어디서" 받을지 모름
```

**클라이언트 = 능동적 연결자**
```
bind() 선택사항:
1. 서버 주소는 클라이언트가 "알고 있음"
2. 자신의 포트는 OS가 자동 할당
3. 서버는 패킷의 출발지 주소로 응답 가능
4. 특별한 요구사항이 없으면 생략 권장
```

**비유로 정리:**
```
서버 (식당):
- 주소가 없으면 손님이 못 찾아옴 → bind() 필수
- "강남구 123번지"처럼 고정 주소 필요

클라이언트 (손님):
- 식당 주소만 알면 찾아갈 수 있음
- 자기 집 주소는 식당에 알릴 필요 없음 → bind() 불필요
- 식당에 도착하면 식당 주인이 손님을 "볼 수" 있음 (패킷 출발지)
```

**예제 - UDP 서버:**
```c
struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr));

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);
server_addr.sin_addr.s_addr = INADDR_ANY;

if (bind(sockfd, (struct sockaddr*)&server_addr,
         sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
}

printf("UDP server listening on port 8080\n");
```

**TCP bind()와 차이점:**
- 기능은 동일
- 하지만 UDP는 이후 listen() 없음!

---

### 4.3.3 sendto() - 데이터 전송

**함수 원형:**
```c
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
```

**목적:**
UDP 데이터그램을 지정된 주소로 전송합니다.

**매개변수:**
```c
sockfd:    UDP 소켓
buf:       보낼 데이터
len:       데이터 크기
flags:     옵션 플래그 (보통 0)
dest_addr: 목적지 주소 (IP + 포트)
addrlen:   dest_addr 크기
```

**반환값:**
- **성공**: 전송한 바이트 수
- **실패**: -1

**왜 매번 주소를 지정?**

UDP는 비연결형이므로 **소켓이 특정 상대방과 연결되어 있지 않습니다**. 따라서 매번 "누구에게 보낼지"를 명시해야 합니다.

**TCP와의 근본적 차이:**

**TCP (연결 지향):**
```c
// 1. 한 번 연결
connect(sock, &server_addr, ...);  // "이 소켓은 서버와 연결됨"

// 2. 이후 주소 불필요
send(sock, "msg1", 4, 0);  // 서버로 전송
send(sock, "msg2", 4, 0);  // 서버로 전송
send(sock, "msg3", 4, 0);  // 서버로 전송

// 소켓이 "누구와 연결되어 있는지" 기억함
```

**UDP (비연결):**
```c
// 연결 개념 없음!

// 매번 주소 지정 필요
sendto(sock, "msg1", 4, 0, &addr1, len);  // addr1로 전송
sendto(sock, "msg2", 4, 0, &addr2, len);  // addr2로 전송
sendto(sock, "msg3", 4, 0, &addr1, len);  // 다시 addr1로

// 소켓은 아무것도 기억하지 않음
```

**유연성의 실전 예:**

**1. 멀티캐스트 DNS (mDNS)**
```c
// 같은 소켓으로 여러 서비스 디스커버리
struct sockaddr_in multicast_addr;
struct sockaddr_in device1_addr;
struct sockaddr_in device2_addr;

// 전체 네트워크에 질의
sendto(sock, query, len, 0, &multicast_addr, ...);

// 개별 기기에게 직접 질의
sendto(sock, query, len, 0, &device1_addr, ...);
sendto(sock, query, len, 0, &device2_addr, ...);
```

**2. 게임 서버 (여러 클라이언트)**
```c
struct sockaddr_in clients[100];
int num_clients = 50;

// 모든 클라이언트에게 게임 상태 브로드캐스트
for (int i = 0; i < num_clients; i++) {
    sendto(sock, &game_state, sizeof(game_state), 0,
           &clients[i], sizeof(clients[i]));
}

// 같은 소켓으로!
```

**3. DNS 서버**
```c
// 하나의 UDP 소켓으로 수천 개의 클라이언트 처리
while (1) {
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    // 질의 수신
    recvfrom(sock, query, sizeof(query), 0, &client, &len);

    // 응답 (각 클라이언트에게 개별 전송)
    sendto(sock, response, resp_len, 0, &client, len);
}

// 매번 다른 클라이언트!
```

**장점:**
- 하나의 소켓으로 여러 상대방과 통신
- 동적으로 목적지 변경 가능
- 연결 설정/해제 오버헤드 없음

**단점:**
- 매번 주소 구조체 채워야 함 (코드 길어짐)
- 실수로 잘못된 주소로 보낼 위험

**예제:**
```c
struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);
inet_pton(AF_INET, "192.168.1.100", &server_addr.sin_addr);

const char *msg = "Hello, UDP Server!";
ssize_t sent = sendto(sockfd, msg, strlen(msg), 0,
                     (struct sockaddr*)&server_addr,
                     sizeof(server_addr));

if (sent < 0) {
    perror("sendto failed");
} else {
    printf("Sent %zd bytes\n", sent);
}
```

**중요한 특징:**

**1. 원자적 전송 (Atomic)**

**원자적(Atomic)이란?**

화학에서 원자(Atom)는 더 이상 쪼갤 수 없는 최소 단위입니다. 프로그래밍에서 "원자적"이란 **전체가 성공하거나 전체가 실패**하는, 중간 상태가 없는 동작을 의미합니다.

**UDP의 원자적 전송:**
```c
sendto(sock, data, 100, 0, ...);
```

이 호출의 결과는 **둘 중 하나**만 가능합니다:
- **성공**: 100바이트 **전체**가 하나의 데이터그램으로 전송됨
- **실패**: 아무것도 전송 안 됨 (에러 반환)

**절대 일어나지 않는 일:**
- ❌ 50바이트만 보내고 나머지는 나중에
- ❌ 일부는 성공, 일부는 실패

**TCP와의 차이:**

**TCP (부분 전송 가능):**
```c
// 1000바이트 전송 시도
ssize_t sent = send(sock, data, 1000, 0);

// 반환값 확인
printf("Sent: %zd bytes\n", sent);

// 가능한 결과:
// sent = 1000  (전부 전송)
// sent = 500   (절반만 전송!)
// sent = 1     (1바이트만!)
// sent = -1    (실패)
```

**왜 이런가?**
- TCP 송신 버퍼가 가득 찼을 수 있음
- 흐름 제어로 인한 제한
- 네트워크 혼잡

**해결:**
```c
// TCP는 전체 전송을 보장하는 루프 필요
size_t total_sent = 0;
while (total_sent < 1000) {
    ssize_t sent = send(sock, data + total_sent,
                       1000 - total_sent, 0);
    if (sent < 0) {
        // 에러 처리
        break;
    }
    total_sent += sent;
}
```

**UDP (항상 전체 또는 실패):**
```c
// 1000바이트 전송 시도
ssize_t sent = sendto(sock, data, 1000, 0, &addr, len);

// 가능한 결과:
// sent = 1000  (전부 전송)
// sent = -1    (실패)

// 불가능한 결과:
// sent = 500   (절대 일어나지 않음!)
```

**왜 이런가?**
- UDP는 데이터그램 단위로 동작
- 데이터그램은 쪼갤 수 없는 단위
- 커널이 전체를 보낼 수 있을 때만 전송

**장점:**
```c
// 별도의 루프 불필요!
if (sendto(sock, data, len, 0, &addr, addrlen) < 0) {
    perror("Failed");  // 전체가 실패
} else {
    printf("Success");  // 전체가 성공
}
```

**주의사항:**

**1. 송신 버퍼 크기 제한**
```c
// 송신 버퍼보다 큰 데이터는?
char huge_data[1000000];  // 1MB

sendto(sock, huge_data, 1000000, 0, ...);
// 결과: 실패 (EMSGSIZE 에러)
// 부분 전송은 하지 않음!
```

**2. 수신측도 원자적**
```c
// 송신측
sendto(sock, "Hello", 5, 0, ...);

// 수신측
char buf[100];
recvfrom(sock, buf, 100, 0, ...);
// 결과: 정확히 5바이트
// 절대 3바이트, 2바이트로 나뉘지 않음
```

**비교 정리:**

| 특성 | TCP | UDP |
|------|-----|-----|
| 전송 단위 | 바이트 스트림 | 데이터그램 (원자적) |
| 부분 전송 | ✅ 가능 | ❌ 불가능 |
| send 반환값 | 실제 전송된 바이트 | len 또는 -1 |
| 루프 필요 | ✅ 필요 | ❌ 불필요 |
| 코드 복잡도 | 높음 | 낮음 |

**2. 최대 크기 제한과 MTU**

**UDP 데이터그램 크기 제한:**
```
이론적 최대: 65,507바이트
  = 65,535 (IP 최대) - 20 (IP 헤더) - 8 (UDP 헤더)

실용적 권장: 512 ~ 1,472바이트
  = MTU(1,500) - 20 (IP) - 8 (UDP)
  = 단편화 방지
```

**MTU(Maximum Transmission Unit)란?**

네트워크에서 **한 번에 전송할 수 있는 최대 패킷 크기**입니다.

**비유:**
- 터널 높이 제한 = MTU
- 너무 높은 트럭 = 큰 패킷
- 트럭을 쪼개서 통과 = 단편화

**일반적인 MTU 값:**
```
이더넷:     1,500바이트 (가장 흔함)
WiFi:       1,500바이트 (이더넷과 동일)
PPPoE:      1,492바이트 (DSL 등)
VPN:        1,300~1,400바이트 (캡슐화 오버헤드)
인터넷 최소: 576바이트 (IPv4 표준)
```

**단편화(Fragmentation) 문제**

**단편화란?**

IP 패킷의 크기가 네트워크의 MTU보다 클 때, **IP 계층에서 자동으로 여러 조각으로 쪼개는 과정**입니다.

**왜 단편화가 발생하는가?**

네트워크는 계층 구조이며, 각 계층은 아래 계층의 세부사항을 모릅니다:

```
[응용 계층] "10,000바이트 보내!"
     ↓
[UDP 계층]  UDP 헤더(8) 추가 → 10,008바이트
     ↓
[IP 계층]   IP 헤더(20) 추가 → 10,028바이트
     ↓      "MTU는 1,500인데 10,028바이트를 어떻게?"
     ↓      → 단편화 수행!
[링크 계층] 각 조각을 1,500바이트 이하로 전송
```

**단편화 과정 상세 설명:**

**예시: 3,000바이트 데이터 전송**

```
Step 1: 응용 프로그램
┌────────────────────────────────────┐
│     3,000바이트 데이터              │
└────────────────────────────────────┘

Step 2: UDP가 헤더 추가
┌───┬────────────────────────────────┐
│ 8 │     3,000바이트 데이터          │
└───┴────────────────────────────────┘
UDP 헤더 (출발지/목적지 포트, 길이, 체크섬)
총 3,008바이트

Step 3: IP가 헤더 추가
┌────┬───┬────────────────────────────┐
│ 20 │ 8 │   3,000바이트 데이터        │
└────┴───┴────────────────────────────┘
IP 헤더 (주소, TTL, 프로토콜 등)
총 3,028바이트

Step 4: MTU 확인
MTU = 1,500바이트
3,028 > 1,500 → 단편화 필요!

Step 5: IP 단편화 (자동 수행)

조각 #1:
┌────┬──────────────────────┐
│IP헤더│   1,480바이트       │ = 1,500바이트
│ 20 │     페이로드          │
└────┴──────────────────────┘
Fragment Offset: 0
More Fragments: Yes (1)

조각 #2:
┌────┬──────────────────────┐
│IP헤더│   1,480바이트       │ = 1,500바이트
│ 20 │     페이로드          │
└────┴──────────────────────┘
Fragment Offset: 1480
More Fragments: Yes (1)

조각 #3:
┌────┬─────────┐
│IP헤더│ 48바이트│ = 68바이트
│ 20 │페이로드  │
└────┴─────────┘
Fragment Offset: 2960
More Fragments: No (0)

각 조각은 독립적으로 라우팅됨!
```

**IP 헤더의 단편화 관련 필드:**

```c
struct iphdr {
    // ...
    uint16_t id;         // 식별자 - 같은 원본 패킷의 조각들은 같은 ID
    uint16_t frag_off;   // 단편화 오프셋 및 플래그
                         // 비트 0-12: 오프셋 (8바이트 단위)
                         // 비트 13: Reserved
                         // 비트 14: Don't Fragment (DF)
                         // 비트 15: More Fragments (MF)
    // ...
};
```

**수신측 재조립 과정:**

```
조각 수신 순서는 보장 안 됨:

시간 T1: 조각 #2 도착
        → 버퍼에 보관, 대기

시간 T2: 조각 #1 도착
        → 버퍼에 보관, 대기

시간 T3: 조각 #3 도착
        → More Fragments = 0 (마지막 조각)
        → 모든 조각 수신 확인
        → 오프셋 순서로 재조립:
           조각#1 (offset:0) + 조각#2 (offset:1480) + 조각#3 (offset:2960)
        → 원본 3,008바이트 복원
        → UDP 계층으로 전달

만약 조각 하나라도 누락?
→ 타임아웃 (보통 30-60초)
→ 받은 모든 조각 폐기
→ 응용 프로그램은 아무것도 못 받음
```

**왜 문제인가?**

**1. 전체 손실 위험**
```
조각1: [✓] 도착
조각2: [✗] 손실!
조각3: [✓] 도착

결과: 전체 데이터그램 폐기
→ 조각2만 재전송 불가능 (UDP는 재전송 없음)
→ 응용 프로그램은 아무것도 못 받음
```

**2. 손실 확률 증가**
```
단일 패킷 손실률: 1%

단편화 없음 (1개 패킷): 손실 확률 = 1%
3개로 단편화: 손실 확률 = 1 - (0.99)³ = 2.97%
10개로 단편화: 손실 확률 = 1 - (0.99)¹⁰ = 9.6%

→ 조각 수가 많을수록 손실 확률 급증!
```

**3. 성능 저하**
```
- 단편화/재조립에 CPU 사용
- 메모리 버퍼 필요
- 중간 라우터 부담
```

**4. 방화벽/NAT 문제**

**첫 번째 조각만 포트 정보 포함:**
```
조각 #1: IP헤더 + UDP헤더(포트 포함) + 데이터
조각 #2: IP헤더 + 데이터 (포트 정보 없음!)
조각 #3: IP헤더 + 데이터 (포트 정보 없음!)
```

**문제:**
- 방화벽/NAT는 포트 번호로 패킷 필터링
- 첫 번째 조각 이후의 조각은 포트 정보 없음
- 두 가지 선택:
  1. 모든 단편화 패킷 통과 (보안 위험)
  2. 단편화 패킷 차단 (안전하지만 통신 실패)

**많은 네트워크가 선택 #2 채택:**
```
방화벽: "단편화된 패킷? 차단!"
→ 조각 #2, #3 드롭
→ 수신측은 영원히 완전한 패킷 못 받음
→ 응용 프로그램: 연결 실패
```

**실제 사례:**
```
클라이언트(집)  → 인터넷 → 회사 방화벽 → 서버
   3,000바이트 전송
      ↓
   단편화 (3개 조각)
      ↓
   조각 #1 통과
   조각 #2, #3 차단!
      ↓
   통신 실패
```

**5. 보안 취약점**

**Teardrop 공격:**
```
악의적으로 겹치는 오프셋 설정:

조각 #1: offset=0,   길이=100
조각 #2: offset=50,  길이=100  (50바이트 겹침!)

재조립 시:
→ 버퍼 오버플로우
→ 시스템 크래시
```

**Ping of Death:**
```
65,535바이트를 초과하는 ICMP 패킷 전송
→ 재조립 시 버퍼 오버플로우
→ 시스템 다운
(과거 Windows 95/NT 취약)
```

**이런 이유로 많은 네트워크가 단편화 패킷을 차단합니다.**

**6. 라우팅 문제**

**경로마다 다른 MTU:**
```
송신자 ─(MTU:1500)─ 라우터A ─(MTU:1400!)─ 라우터B ─(MTU:1500)─ 수신자
                              ↑
                        여기서 재단편화 필요!
```

**재단편화 문제:**
- 조각을 더 작은 조각으로 쪼갬
- 복잡도 증가
- 성능 저하
- 일부 라우터는 재단편화 안 함 → 패킷 드롭

**7. IPv6에서는 금지**

IPv6는 중간 라우터의 단편화를 **완전히 금지**했습니다:
```
IPv4: 라우터가 필요시 단편화 가능
IPv6: 송신자만 단편화 가능, 라우터는 불가

MTU 초과 패킷 수신 시:
→ ICMP "Packet Too Big" 에러 반환
→ 송신자가 크기 줄여서 재전송
```

**이유:**
- 라우터 부담 감소
- 성능 향상
- 보안 강화
- End-to-end 원칙

**실전 예제:**

**나쁜 예 (단편화 발생):**
```c
char big_data[10000];
fill_data(big_data);

// 10,000바이트 전송
// → 약 7개 조각으로 단편화!
sendto(sock, big_data, 10000, 0, &addr, len);

// 문제:
// - 조각 하나라도 손실 시 전체 손실
// - 손실 확률 7배 증가
```

**좋은 예 (단편화 방지):**
```c
#define MAX_SAFE_UDP 1472  // 1500 - 20(IP) - 8(UDP)

char big_data[10000];
fill_data(big_data);

// 직접 분할하여 전송
for (int offset = 0; offset < 10000; offset += MAX_SAFE_UDP) {
    int chunk_size = (offset + MAX_SAFE_UDP > 10000) ?
                     (10000 - offset) : MAX_SAFE_UDP;

    struct Packet {
        uint32_t seq;      // 시퀀스 번호
        uint32_t total;    // 전체 조각 수
        char data[MAX_SAFE_UDP];
    } pkt;

    pkt.seq = offset / MAX_SAFE_UDP;
    pkt.total = (10000 + MAX_SAFE_UDP - 1) / MAX_SAFE_UDP;
    memcpy(pkt.data, big_data + offset, chunk_size);

    sendto(sock, &pkt, sizeof(pkt), 0, &addr, len);
}

// 장점:
// - 각 패킷이 독립적
// - 하나 손실돼도 재요청 가능
// - 응용 레벨 제어
```

**권장 크기:**

**보수적 (안전):**
```c
#define UDP_SAFE_SIZE 512
// - 인터넷 최소 MTU 보장
// - DNS가 사용하는 크기
// - 단편화 절대 발생 안 함
```

**표준 (권장):**
```c
#define UDP_SAFE_SIZE 1472
// = 1500 (이더넷 MTU) - 20 (IP) - 8 (UDP)
// - 대부분의 환경에서 안전
// - 이더넷 표준
```

**공격적 (위험):**
```c
#define UDP_SAFE_SIZE 8192
// - 고속 로컬 네트워크 전용
// - 인터넷에서는 단편화 가능
// - Jumbo Frame 지원 필요
```

**Path MTU Discovery (PMTUD)**

**문제:**
경로상의 최소 MTU를 어떻게 알 수 있을까?

**해결: Path MTU Discovery**

동적으로 경로상의 최소 MTU를 찾는 기법입니다.

**동작 원리:**

```
Step 1: DF(Don't Fragment) 플래그 설정
송신자: "단편화하지 마!"

Step 2: 큰 패킷 전송 시도
송신자 ─(1500바이트, DF=1)→ 라우터A
                              MTU=1400
                              "1500 > 1400인데 DF=1?"
                              ↓
송신자 ←(ICMP Fragmentation Needed, MTU=1400)─ 라우터A

Step 3: 크기 줄여서 재시도
송신자 ─(1400바이트, DF=1)→ 라우터A → 라우터B → 수신자
                              ✓ 성공!

결과: Path MTU = 1400바이트 확인
```

**Linux에서 사용:**

```c
// 1. DF 플래그 설정 및 PMTUD 활성화
int val = IP_PMTUDISC_DO;
setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

// 2. 전송 시도
ssize_t sent = sendto(sock, data, 2000, 0, &addr, len);

if (sent < 0) {
    if (errno == EMSGSIZE) {
        // MTU 초과!

        // 3. 현재 Path MTU 확인
        int mtu;
        socklen_t mtu_len = sizeof(mtu);
        getsockopt(sock, IPPROTO_IP, IP_MTU, &mtu, &mtu_len);

        printf("Path MTU: %d bytes\n", mtu);

        // 4. 크기 조정
        int safe_size = mtu - 20 - 8;  // IP헤더 - UDP헤더
        sendto(sock, data, safe_size, 0, &addr, len);
    }
}
```

**PMTUD 옵션:**

```c
int val;

// 옵션 1: IP_PMTUDISC_WANT (기본값)
// - PMTUD 시도하되, 실패하면 단편화 허용
val = IP_PMTUDISC_WANT;
setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

// 옵션 2: IP_PMTUDISC_DO (엄격)
// - 절대 단편화 안 함
// - MTU 초과 시 EMSGSIZE 에러
val = IP_PMTUDISC_DO;
setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

// 옵션 3: IP_PMTUDISC_DONT (비활성화)
// - PMTUD 사용 안 함
// - 필요시 단편화
val = IP_PMTUDISC_DONT;
setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

// 옵션 4: IP_PMTUDISC_PROBE (고급)
// - DF 설정하되 EMSGSIZE 무시
// - 테스트용
val = IP_PMTUDISC_PROBE;
setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));
```

**PMTUD의 문제점:**

**1. ICMP 블랙홀**
```
일부 방화벽이 ICMP 메시지 차단
→ MTU 초과 알림을 못 받음
→ 패킷은 계속 드롭
→ 연결 실패 (블랙홀)
```

**2. 시간 지연**
```
처음 몇 번의 패킷은 실패할 수 있음
→ 재시도로 인한 지연
```

**3. 경로 변경**
```
네트워크 경로가 바뀌면
→ MTU도 바뀔 수 있음
→ 주기적으로 재확인 필요
```

**실용적 접근:**

```c
// 가장 안전한 방법: 보수적 크기 사용
#define SAFE_UDP_SIZE 512  // 인터넷 최소 MTU 보장

// 또는 이더넷 표준
#define SAFE_UDP_SIZE 1472  // 1500 - 20 - 8

// PMTUD는 선택사항
// 성능이 중요한 경우에만 사용
```

**결론:**
- **1,472바이트 이하 권장** (이더넷 기준)
- 큰 데이터는 응용 레벨에서 분할
- 단편화는 피하는 것이 최선

**3. 즉시 전송 시도**
```c
sendto(...);  // 즉시 전송 시도
// TCP처럼 버퍼에 모았다가 보내지 않음
```

---

### 4.3.4 recvfrom() - 데이터 수신

**함수 원형:**
```c
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

**목적:**
UDP 데이터그램을 수신하고, 송신자의 주소 정보도 함께 받습니다.

**매개변수:**
```c
sockfd:   UDP 소켓
buf:      수신 버퍼
len:      버퍼 크기
flags:    옵션 플래그 (보통 0)
src_addr: 송신자 주소 저장 (출력)
addrlen:  src_addr 버퍼 크기 (입출력)
```

**반환값:**
- **성공**: 수신한 바이트 수
- **실패**: -1

**왜 송신자 주소를 받는가?**

- 응답을 보내기 위해
- 누가 보냈는지 알기 위해
- UDP는 연결 개념이 없으므로 매번 확인 필요

**예제 - UDP 에코 서버:**
```c
char buffer[1024];
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);

while (1) {
    // 데이터그램 수신 (블로킹)
    ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&client_addr,
                                &client_len);

    if (recv_len < 0) {
        perror("recvfrom failed");
        continue;
    }

    // 송신자 정보 출력
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr,
             client_ip, sizeof(client_ip));
    printf("Received %zd bytes from %s:%d\n",
           recv_len, client_ip, ntohs(client_addr.sin_port));

    // 에코 - 받은 주소로 그대로 전송
    sendto(sockfd, buffer, recv_len, 0,
           (struct sockaddr*)&client_addr,
           client_len);
}
```

**중요한 특징:**

**1. 메시지 경계 보존**
```c
// 송신측
sendto(sock, "Hello", 5, 0, ...);
sendto(sock, "World", 5, 0, ...);

// 수신측
char buf[100];
recvfrom(sock, buf, 100, 0, ...);  // "Hello" (5바이트)
recvfrom(sock, buf, 100, 0, ...);  // "World" (5바이트)

// 절대 합쳐지지 않음!
```

**2. 버퍼가 작으면?**
```c
// 송신측: 100바이트 전송
sendto(sock, data, 100, 0, ...);

// 수신측: 50바이트 버퍼
char buf[50];
recvfrom(sock, buf, 50, 0, ...);  // 50바이트만 받음
// 나머지 50바이트는 버려짐!
```

**주의:** 충분히 큰 버퍼 사용 필요

**3. 블로킹 동작**
```c
recvfrom(sock, buf, len, 0, ...);
// 데이터그램이 도착할 때까지 대기
// (논블로킹 모드로 변경 가능)
```

---

### 4.3.5 send() / recv() - 연결된 UDP

**놀라운 사실:**
UDP 소켓도 `connect()` 가능합니다!

**"연결된" UDP?**

"연결"이라는 말이 혼란스럽지만, 실제로는:
- **논리적 연결**: 커널이 주소를 기억함
- **물리적 연결 없음**: 여전히 비연결형
- **신뢰성 없음**: 여전히 UDP

**왜 사용하는가?**

**1. 편리성**
```c
// 일반 UDP
sendto(sock, data, len, 0, &addr, addrlen);  // 매번 주소 지정

// 연결된 UDP
connect(sock, &addr, addrlen);  // 한 번만
send(sock, data, len, 0);       // 이후 주소 생략
send(sock, data, len, 0);       // 간편!
```

**2. 성능**
```c
// 커널이 주소를 미리 검증 및 캐시
// 매번 주소 변환 불필요
// 약간의 성능 향상
```

**3. ICMP 에러 수신**
```c
// 일반 UDP: ICMP 에러 무시
sendto(sock, data, len, 0, &addr, addrlen);  // 실패 감지 못함

// 연결된 UDP: ICMP 에러 수신 가능
connect(sock, &addr, addrlen);
send(sock, data, len, 0);  // ECONNREFUSED 등 에러 받을 수 있음
```

**예제:**
```c
// UDP 소켓 생성
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);
inet_pton(AF_INET, "192.168.1.100", &server_addr.sin_addr);

// "연결" (실제로는 주소 저장)
if (connect(sockfd, (struct sockaddr*)&server_addr,
            sizeof(server_addr)) < 0) {
    perror("connect failed");
    exit(EXIT_FAILURE);
}

// 이제 send/recv 사용 가능
send(sockfd, "Hello", 5, 0);
char buf[1024];
recv(sockfd, buf, sizeof(buf), 0);

close(sockfd);
```

**주의사항:**

**1. 하나의 주소만**
```c
connect(sock, &addr1, ...);  // addr1로 설정
send(sock, data, len, 0);    // addr1로 전송

// 다른 주소로 보내려면?
connect(sock, &addr2, ...);  // 재설정
send(sock, data, len, 0);    // addr2로 전송
```

**2. 여전히 UDP**
```c
// 신뢰성 없음, 순서 보장 없음
// 단지 주소 지정이 편리해졌을 뿐
```

---

## 4.4 UDP vs TCP 비교

### 기능 비교 표

| 특성 | UDP | TCP |
|------|-----|-----|
| 연결 설정 | ❌ 없음 | ✅ 3-way handshake |
| 신뢰성 | ❌ 없음 | ✅ 보장 |
| 순서 보장 | ❌ 없음 | ✅ 보장 |
| 중복 제거 | ❌ 없음 | ✅ 보장 |
| 흐름 제어 | ❌ 없음 | ✅ 있음 |
| 혼잡 제어 | ❌ 없음 | ✅ 있음 |
| 메시지 경계 | ✅ 보존 | ❌ 없음 |
| 헤더 크기 | 8바이트 | 20바이트 이상 |
| 속도 | 빠름 | 상대적으로 느림 |
| 오버헤드 | 낮음 | 높음 |
| 브로드캐스트 | ✅ 지원 | ❌ 불가 |
| 멀티캐스트 | ✅ 지원 | ❌ 불가 |

### 코드 복잡도 비교

**UDP 서버 (간단):**
```c
socket()
bind()
while (1) {
    recvfrom()
    sendto()
}
close()
```

**TCP 서버 (복잡):**
```c
socket()
bind()
listen()
while (1) {
    accept()
    fork() / pthread_create()

    while (1) {
        recv()
        if (연결 종료) break
        send()
    }
    close()
}
close()
```

**복잡도:**
- UDP: 약 20줄
- TCP: 약 50줄 이상

---

## 4.5 실전 패턴

### 패턴 1: 단순 요청-응답

**시나리오:**
- DNS 조회
- 간단한 RPC
- 상태 확인 (health check)

**특징:**
- 하나의 요청, 하나의 응답
- 타임아웃 처리
- 필요 시 재시도

**예제: DNS 스타일**
```c
// 클라이언트
struct sockaddr_in server_addr;
// ... server_addr 설정 ...

// 요청 전송
char query[] = "example.com";
sendto(sockfd, query, strlen(query), 0,
       (struct sockaddr*)&server_addr, sizeof(server_addr));

// 타임아웃 설정
struct timeval tv;
tv.tv_sec = 3;  // 3초
tv.tv_usec = 0;
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

// 응답 대기
char response[512];
struct sockaddr_in from_addr;
socklen_t from_len = sizeof(from_addr);

ssize_t n = recvfrom(sockfd, response, sizeof(response), 0,
                     (struct sockaddr*)&from_addr, &from_len);

if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        printf("Timeout - no response\n");
        // 재시도 로직
    } else {
        perror("recvfrom error");
    }
} else {
    printf("Response: %.*s\n", (int)n, response);
}
```

### 패턴 2: 스트리밍

**시나리오:**
- 실시간 비디오/오디오
- 게임 상태 업데이트
- 센서 데이터 스트림

**특징:**
- 연속적인 데이터 전송
- 손실 허용
- 최신 데이터가 중요

**예제: 게임 위치 업데이트**
```c
// 클라이언트 (게임 클라이언트)
struct Position {
    float x, y, z;
    uint32_t timestamp;
};

struct Position pos = {100.5, 200.3, 10.0, time(NULL)};

// 초당 60번 전송 (60 FPS)
while (game_running) {
    update_position(&pos);

    sendto(sockfd, &pos, sizeof(pos), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

    usleep(16666);  // ~60Hz (1초 / 60)
}
```

**서버 측:**
```c
// 최신 위치만 유지
struct Position latest_positions[MAX_PLAYERS];

while (1) {
    struct Position pos;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    recvfrom(sockfd, &pos, sizeof(pos), 0,
             (struct sockaddr*)&client_addr, &len);

    int player_id = identify_player(&client_addr);

    // 최신 정보로 갱신 (이전 것은 버림)
    latest_positions[player_id] = pos;

    // 다른 플레이어들에게 브로드캐스트
    broadcast_position(player_id, &pos);
}
```

### 패턴 3: 신뢰성 있는 UDP (응용 레벨)

**문제:**
UDP는 신뢰성이 없지만, 때로는 필요합니다.

**해결:**
애플리케이션 레벨에서 구현
- 시퀀스 번호
- 확인 응답 (ACK)
- 재전송
- 타임아웃

**예제: 간단한 신뢰성 구현**
```c
struct Packet {
    uint32_t seq_num;  // 시퀀스 번호
    uint32_t data_len;
    char data[1024];
};

// 송신측
uint32_t seq = 0;
struct Packet pkt;

while (has_data_to_send) {
    pkt.seq_num = htonl(seq);
    pkt.data_len = htonl(fill_data(pkt.data));

    int retry = 0;
    int acked = 0;

    while (!acked && retry < MAX_RETRIES) {
        // 패킷 전송
        sendto(sockfd, &pkt, sizeof(pkt), 0, ...);

        // ACK 대기 (타임아웃)
        struct timeval tv = {1, 0};  // 1초
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        uint32_t ack;
        if (recvfrom(sockfd, &ack, sizeof(ack), 0, ...) > 0) {
            if (ntohl(ack) == seq) {
                acked = 1;  // 확인 완료
            }
        } else {
            retry++;
            printf("Retry %d for seq %u\n", retry, seq);
        }
    }

    if (!acked) {
        printf("Failed to send seq %u\n", seq);
    }

    seq++;
}
```

**수신측:**
```c
uint32_t expected_seq = 0;

while (1) {
    struct Packet pkt;
    struct sockaddr_in sender;
    socklen_t len = sizeof(sender);

    recvfrom(sockfd, &pkt, sizeof(pkt), 0,
             (struct sockaddr*)&sender, &len);

    uint32_t seq = ntohl(pkt.seq_num);

    if (seq == expected_seq) {
        // 올바른 순서
        process_data(pkt.data, ntohl(pkt.data_len));
        expected_seq++;

        // ACK 전송
        uint32_t ack = htonl(seq);
        sendto(sockfd, &ack, sizeof(ack), 0,
               (struct sockaddr*)&sender, len);
    } else if (seq < expected_seq) {
        // 중복 패킷 - ACK 재전송
        uint32_t ack = htonl(seq);
        sendto(sockfd, &ack, sizeof(ack), 0,
               (struct sockaddr*)&sender, len);
    } else {
        // seq > expected_seq: 패킷 손실 감지
        printf("Packet loss detected: expected %u, got %u\n",
               expected_seq, seq);
    }
}
```

**이미 존재하는 솔루션:**
- QUIC (Quick UDP Internet Connections)
- DTLS (Datagram TLS)
- RTP/RTCP (Real-time Transport Protocol)

### 패턴 4: 멀티캐스트

**시나리오:**
- IPTV
- 주식 시세 방송
- 클러스터 서비스 디스커버리

**특징:**
- 하나의 패킷을 여러 수신자에게
- 네트워크 대역폭 절약

**예제: 멀티캐스트 송신**
```c
// 송신자
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(12345);
inet_pton(AF_INET, "239.255.0.1", &addr.sin_addr);  // 멀티캐스트 주소

// 멀티캐스트 TTL 설정
int ttl = 64;
setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

// 데이터 전송
while (1) {
    char *msg = "Multicast message";
    sendto(sockfd, msg, strlen(msg), 0,
           (struct sockaddr*)&addr, sizeof(addr));
    sleep(1);
}
```

**예제: 멀티캐스트 수신**
```c
// 수신자
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

// 재사용 허용
int reuse = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

// 포트 바인딩
struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(12345);
addr.sin_addr.s_addr = INADDR_ANY;

bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

// 멀티캐스트 그룹 가입
struct ip_mreq mreq;
inet_pton(AF_INET, "239.255.0.1", &mreq.imr_multiaddr);
mreq.imr_interface.s_addr = INADDR_ANY;

setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
           &mreq, sizeof(mreq));

// 수신
char buf[1024];
while (1) {
    ssize_t n = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
    if (n > 0) {
        printf("Received: %.*s\n", (int)n, buf);
    }
}
```

---

## 4.6 UDP의 함정과 해결책

### 함정 1: 패킷 손실

**문제:**
```c
// 100개 패킷 전송
for (int i = 0; i < 100; i++) {
    sendto(sock, &data[i], size, 0, ...);
}
// 모두 도착? 보장 없음!
```

**해결:**
- 중요한 데이터: TCP 사용
- UDP 필수: 응용 레벨 재전송 구현
- 손실 허용 가능: 그대로 사용

### 함정 2: 순서 뒤바뀜

**문제:**
```c
sendto(sock, "1", 1, 0, ...);
sendto(sock, "2", 1, 0, ...);
sendto(sock, "3", 1, 0, ...);

// 수신: "1", "3", "2" 가능
```

**해결:**
```c
struct Packet {
    uint32_t seq_num;
    char data[...];
};

// 수신측에서 재정렬
```

### 함정 3: 단편화로 인한 손실

**문제:**
```c
// 10KB 데이터그램 전송
char big_data[10000];
sendto(sock, big_data, 10000, 0, ...);

// IP 단편화 발생
// 조각 하나라도 손실 시 전체 손실!
```

**해결:**
```c
// MTU 이하로 분할
#define MAX_UDP_PAYLOAD 1472  // 1500 - 20(IP) - 8(UDP)

for (int offset = 0; offset < total_size; offset += MAX_UDP_PAYLOAD) {
    int chunk_size = min(MAX_UDP_PAYLOAD, total_size - offset);
    sendto(sock, data + offset, chunk_size, 0, ...);
}
```

### 함정 4: 버퍼 오버플로우

**문제:**
```c
char buf[100];
recvfrom(sock, buf, 100, 0, ...);
// 1000바이트가 왔다면? 나머지 900바이트 손실!
```

**해결:**
```c
// 방법 1: 충분히 큰 버퍼 (힙 할당)
char *buf = malloc(65507);  // UDP 최대 페이로드
if (buf == NULL) { /* 에러 처리 */ }
recvfrom(sock, buf, 65507, 0, ...);
free(buf);

// 방법 2: 애플리케이션 최대 크기 정의
#define MAX_MSG_SIZE 1472  // MTU 고려
char buf[MAX_MSG_SIZE];
recvfrom(sock, buf, MAX_MSG_SIZE, 0, ...);

// 방법 3: MSG_PEEK로 크기 먼저 확인 (비효율적, 권장 안 함)
```

### 함정 5: 방화벽 차단

**문제:**
많은 방화벽이 UDP 트래픽을 차단합니다.

**해결:**
- 잘 알려진 포트 사용 (DNS 53번 등)
- UDP 홀 펀칭 (NAT traversal)
- 폴백: TCP로 전환

---

## 4.7 UDP를 선택해야 할 때

### UDP가 적합한 경우

**✅ 1. 실시간 통신**
- 지연이 치명적
- 약간의 손실 허용
- 예: VoIP, 화상회의, 온라인 게임

**✅ 2. 간단한 요청-응답**
- 상태 없음
- 한 번의 메시지 교환
- 예: DNS, DHCP, SNMP

**✅ 3. 브로드캐스트/멀티캐스트**
- 여러 수신자에게 동시 전송
- TCP는 불가능
- 예: 서비스 디스커버리, IPTV

**✅ 4. 높은 처리량**
- 많은 수의 작은 메시지
- 오버헤드 최소화
- 예: 게임 서버, 센서 네트워크

**✅ 5. 커스텀 프로토콜**
- 특수한 신뢰성 메커니즘 필요
- 기존 프로토콜로 불충분
- 예: QUIC

### TCP가 더 나은 경우

**✅ 1. 데이터 무결성 필수**
- 손실 절대 불가
- 예: 파일 전송, 데이터베이스

**✅ 2. 순서 중요**
- 순차적 처리 필수
- 예: HTTP, FTP

**✅ 3. 큰 데이터 전송**
- 여러 패킷으로 분할 필요
- 재조립 자동 처리

**✅ 4. 흐름/혼잡 제어 필요**
- 네트워크 상태에 따른 조절
- TCP가 자동 처리

---

## 4.8 완전한 예제

### UDP 에코 서버

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_len = sizeof(client_addr);

    // 1. 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. 주소 구조체 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 3. 바인딩
    if (bind(sockfd, (struct sockaddr*)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Echo Server listening on port %d\n", PORT);

    // 4. 에코 루프
    while (1) {
        // 수신
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                            (struct sockaddr*)&client_addr,
                            &client_len);

        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }

        // 클라이언트 정보
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr,
                 client_ip, sizeof(client_ip));

        printf("Received %zd bytes from %s:%d: %.*s\n",
               n, client_ip, ntohs(client_addr.sin_port),
               (int)n, buffer);

        // 에코
        sendto(sockfd, buffer, n, 0,
               (struct sockaddr*)&client_addr,
               client_len);
    }

    close(sockfd);
    return 0;
}
```

### UDP 에코 클라이언트

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 1. 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 3. 메시지 송수신
    const char *message = "Hello, UDP Server!";

    // 전송
    ssize_t sent = sendto(sockfd, message, strlen(message), 0,
                         (struct sockaddr*)&server_addr,
                         sizeof(server_addr));

    if (sent < 0) {
        perror("sendto failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sent: %s\n", message);

    // 타임아웃 설정 (5초)
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 수신
    socklen_t server_len = sizeof(server_addr);
    ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                        (struct sockaddr*)&server_addr,
                        &server_len);

    if (n < 0) {
        perror("recvfrom failed or timeout");
    } else {
        buffer[n] = '\0';
        printf("Echo: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
```

---

## 핵심 요약

**UDP 소켓 생명주기:**
```
socket() → bind() → sendto()/recvfrom() → close()
         (서버만)

socket() → sendto()/recvfrom() → close()
         (클라이언트)
```

**UDP의 핵심 특징:**
1. **비연결형**: 연결 설정/해제 없음
2. **신뢰성 없음**: 손실, 순서 뒤바뀜 가능
3. **메시지 지향**: 경계 보존
4. **빠름**: 오버헤드 최소
5. **단순함**: API 간단

**언제 사용?**
- 실시간 통신 (속도 > 신뢰성)
- 간단한 요청-응답
- 브로드캐스트/멀티캐스트
- 손실 허용 가능

**주의사항:**
- 버퍼 크기 충분히
- MTU 고려 (1472바이트 권장)
- 필요시 응용 레벨 신뢰성 구현
- 방화벽 고려

---

다음: [Part 5: 소켓 옵션 →](05_socket_options.md)
