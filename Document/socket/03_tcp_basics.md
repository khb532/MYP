# Part 3: TCP 소켓의 동작 원리

TCP(Transmission Control Protocol)는 **연결 지향적이고 신뢰성 있는** 전송 프로토콜입니다. 이 장에서는 TCP 소켓의 동작 원리와 주요 함수들을 상세히 다룹니다.

---

## 3.1 TCP의 특징 (복습)

### 연결 지향 (Connection-oriented)

**전화 통화와 비슷:**
1. 전화를 걸어 연결을 맺음 (3-way handshake)
2. 대화를 주고받음 (데이터 송수신)
3. 통화를 종료함 (4-way handshake)

**왜 연결이 필요한가?**
- 양쪽이 준비되었는지 확인
- 초기 시퀀스 번호 교환
- 통신 매개변수 협상 (윈도우 크기 등)

### 신뢰성 (Reliability)

**보장 사항:**
- ✅ 데이터 손실 없음 (손실 시 재전송)
- ✅ 순서 보장 (패킷 재정렬)
- ✅ 중복 제거
- ✅ 오류 검출 (체크섬)

### 바이트 스트림

**메시지 경계 없음:**
```c
// 송신측
send(sock, "Hello", 5, 0);
send(sock, "World", 5, 0);

// 수신측 - 다양한 경우 가능
recv(sock, buf, 10, 0);  // "HelloWorld" (10바이트)
// 또는
recv(sock, buf, 10, 0);  // "Hell" (4바이트)
recv(sock, buf, 10, 0);  // "oWorld" (6바이트)
```

---

## 3.2 서버-클라이언트 모델

TCP 통신은 **비대칭적**입니다. 역할이 명확히 구분됩니다.

### 전체 흐름도

```
서버                                클라이언트
─────────────────────────────────────────────────
socket()                            socket()
  ↓                                   ↓
bind()         (주소 할당)            (주소는 자동 할당)
  ↓
listen()       (대기 상태 전환)
  ↓
accept() ←──────────────────────── connect()
  ↓         [3-way handshake]         ↓
  ↓           SYN →                   ↓
  ↓         ← SYN+ACK                 ↓
  ↓           ACK →                   ↓
  ↓         [연결 완료]                ↓
  ↓                                   ↓
recv() ←────────────────────────── send()
  ↓           데이터 →                 ↓
send() ──────────────────────────→ recv()
  ↓         ← 데이터                   ↓
  ↓                                   ↓
close() ←──────────────────────────→ close()
       [4-way handshake]
         FIN →
       ← ACK
       ← FIN
         ACK →
```

### 역할 구분

**서버 (Passive Open):**
- 특정 포트에서 **대기**
- 클라이언트의 연결 요청을 **수락**
- 여러 클라이언트 **동시 처리** 가능
- 수동적(passive) 역할

**클라이언트 (Active Open):**
- 서버에 **연결 요청**
- 능동적(active) 역할
- 보통 임시 포트 사용

---

## 3.3 주요 시스템 콜 상세

### 3.3.1 socket() - 소켓 생성

**함수 원형:**
```c
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

**목적:**
새로운 소켓을 생성합니다. "전화기를 하나 구입"하는 것과 같습니다.

**매개변수:**
```c
domain:   AF_INET (IPv4), AF_INET6 (IPv6)
type:     SOCK_STREAM (TCP)
protocol: 0 (자동 선택) 또는 IPPROTO_TCP
```

**반환값:**
- **성공**: 소켓 디스크립터 (양의 정수, ≥0)
- **실패**: -1 (errno 설정됨)

**내부 동작:**
1. **커널 메모리 할당**: 소켓 데이터 구조 생성
2. **파일 디스크립터 할당**: 프로세스 FD 테이블에 추가
3. **초기 상태 설정**: CLOSED 상태

**예제:**
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
}
printf("Socket created: fd=%d\n", sockfd);
```

**주요 에러:**
- `EACCES`: 권한 부족 (Raw 소켓의 경우)
- `EMFILE`: 프로세스의 FD 한계 초과
- `ENFILE`: 시스템 전체 FD 한계 초과
- `EPROTONOSUPPORT`: 프로토콜 미지원

**소켓 디스크립터:**
- 파일 디스크립터와 동일한 개념
- 0, 1, 2는 stdin, stdout, stderr
- 소켓은 보통 3부터 시작

---

### 3.3.2 bind() - 주소 할당

**함수 원형:**
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

**목적:**
소켓에 로컬 주소(IP + 포트)를 할당합니다. "전화기에 전화번호 부여"하는 것과 같습니다.

**언제 필요한가?**
- **서버**: 반드시 필요 (클라이언트가 접속할 주소)
- **클라이언트**: 선택사항 (보통 생략, OS가 자동 할당)

**매개변수:**
```c
sockfd:  socket()에서 반환받은 디스크립터
addr:    바인딩할 주소 정보 (IP + 포트)
addrlen: addr 구조체의 크기
```

**주소 구조체 설정:**
```c
struct sockaddr_in server_addr;

// 1. 초기화 (중요!)
memset(&server_addr, 0, sizeof(server_addr));

// 2. 주소 체계
server_addr.sin_family = AF_INET;

// 3. 포트 번호 (네트워크 바이트 순서로 변환!)
server_addr.sin_port = htons(8080);

// 4. IP 주소
server_addr.sin_addr.s_addr = INADDR_ANY;  // 모든 인터페이스
// 또는 특정 IP:
// inet_pton(AF_INET, "192.168.1.100", &server_addr.sin_addr);

// 5. 바인딩
if (bind(sockfd, (struct sockaddr*)&server_addr, 
         sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
}
```

**포트 번호 체계:**

| 범위 | 이름 | 용도 | 권한 |
|------|------|------|------|
| 0-1023 | Well-known | HTTP(80), HTTPS(443), SSH(22) | root 필요 |
| 1024-49151 | Registered | MySQL(3306), PostgreSQL(5432) | 일반 사용자 가능 |
| 49152-65535 | Dynamic | 클라이언트 임시 포트 | 자동 할당 |

**특수 주소:**

**INADDR_ANY (0.0.0.0):**
```c
server_addr.sin_addr.s_addr = INADDR_ANY;
```
- **의미**: "모든 네트워크 인터페이스에서 수신"
- **사용 시나리오**:
  ```
  컴퓨터에 여러 IP가 있는 경우:
  - 192.168.1.10 (내부망)
  - 10.0.0.5 (또 다른 네트워크)
  - 127.0.0.1 (루프백)
  
  INADDR_ANY 사용 시: 모든 IP로 접속 가능
  ```

**INADDR_LOOPBACK (127.0.0.1):**
```c
server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
```
- **의미**: "루프백, 자기 자신"
- **용도**: 로컬 테스트, 외부 접속 불가

**특정 IP 지정:**
```c
inet_pton(AF_INET, "192.168.1.100", &server_addr.sin_addr);
```
- **의미**: "이 IP 주소로만 수신"
- **용도**: 보안, 네트워크 분리

**바이트 순서 변환 (중요!):**

#### 엔디안(Endianness)이란?

**엔디안**은 다중 바이트 데이터를 메모리에 저장하는 순서입니다.

**예시: 숫자 0x12345678을 메모리에 저장**

이 숫자는 4바이트입니다:
- 최상위 바이트(MSB): 0x12
- 0x34
- 0x56
- 최하위 바이트(LSB): 0x78

**빅 엔디안 (Big-Endian)** - 사람이 읽는 순서:
```
주소:   0x1000  0x1001  0x1002  0x1003
값:       12      34      56      78
        [높음 ←----------→ 낮음]
```
- 높은 자리(Big) 바이트가 낮은 주소에 먼저(End) 저장
- 직관적: 책을 읽듯 왼쪽에서 오른쪽으로
- **네트워크 표준**: 모든 네트워크 프로토콜이 사용

**리틀 엔디안 (Little-Endian)** - CPU 효율적:
```
주소:   0x1000  0x1001  0x1002  0x1003
값:       78      56      34      12
        [낮음 ←----------→ 높음]
```
- 낮은 자리(Little) 바이트가 낮은 주소에 먼저(End) 저장
- CPU 효율적: 연산 시 낮은 바이트부터 처리
- **Intel/AMD x86**: 대부분의 PC CPU가 사용

#### 왜 변환이 필요한가?

**문제 상황:**
```
PC (리틀 엔디안)            네트워크 (빅 엔디안)

포트 8080 (0x1F90)
메모리: [90 1F]       →     수신측이 0x901F (36895)로 해석!
                            ❌ 완전히 다른 포트!
```

**같은 숫자, 다른 해석:**
- 보내는 쪽: 8080 → 리틀엔디안 메모리에 [90 1F]
- 네트워크로 그대로 전송
- 받는 쪽: [90 1F]를 빅엔디안으로 읽음 → 36895 ❌

**왜 네트워크는 빅엔디안인가?**

**역사적 이유:**
1. **1980년대 초 인터넷 설계 당시** - 빅엔디안 시스템이 주류
   - Sun, IBM, HP 등 서버들이 빅엔디안 사용
2. **직관성** - 사람이 읽고 쓰는 순서와 동일
3. **디버깅 용이** - 패킷 덤프 시 그대로 읽기 가능

**한 번 정하면 바꿀 수 없음:**
- 전 세계 모든 시스템이 같은 규칙을 따라야 통신 가능
- 표준이 정해진 후에는 변경 불가능

**문제:**
```
Intel x86 (Little Endian):
    0x1234 → [34] [12] (낮은 바이트가 낮은 주소)

네트워크 (Big Endian):
    0x1234 → [12] [34] (높은 바이트가 낮은 주소)
```

**해결: 항상 네트워크 바이트 순서로 변환**
```c
// 보낼 때
uint16_t port = 8080;                    // 호스트 순서
uint16_t net_port = htons(port);         // 네트워크 순서로 변환

// 받을 때
uint32_t addr = 0x0A000001;              // 10.0.0.1
uint32_t net_addr = htonl(addr);         // 네트워크 순서로 변환
```

**변환 함수:**
- `htons()`: Host TO Network Short (16비트) - 포트 번호용
- `htonl()`: Host TO Network Long (32비트) - IP 주소용
- `ntohs()`: Network TO Host Short - 받은 포트 변환
- `ntohl()`: Network TO Host Long - 받은 IP 변환

**중요: 빅엔디안 시스템에서는?**
```c
// ARM(빅엔디안 모드), SPARC 등에서는
htons(8080);  // 실제로는 아무것도 안 함 (이미 빅엔디안)
              // 하지만 항상 써야 함! (이식성)
```

**주요 에러:**

**EADDRINUSE:**
```
원인: 주소(포트)가 이미 사용 중
해결: SO_REUSEADDR 옵션 사용
```

```c
int opt = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
bind(sockfd, ...);
```

**EACCES:**
```
원인: 권한 부족 (1024 이하 포트)
해결: root 권한으로 실행 또는 높은 포트 사용
```

**EADDRNOTAVAIL:**
```
원인: 주소를 사용할 수 없음 (존재하지 않는 IP)
해결: 올바른 IP 주소 사용
```

---

### 3.3.3 listen() - 연결 대기

**함수 원형:**
```c
int listen(int sockfd, int backlog);
```

**목적:**
소켓을 **수동 모드(passive mode)**로 전환하여 들어오는 연결 요청을 받을 수 있게 합니다.

**매개변수:**
```c
sockfd:  바인딩된 서버 소켓
backlog: 대기 큐의 최대 크기
```

**동작:**

**상태 전환:**
```
CLOSED → LISTEN
```

이제 소켓은:
- 능동적 연결(connect) 불가
- 수동적으로 연결 수락만 가능

**대기 큐 (Backlog):**

실제로는 **두 개의 큐**가 있습니다:

```
1. Incomplete Connection Queue (SYN 큐)
   - SYN 받았지만 3-way handshake 미완료
   - 상태: SYN_RCVD

2. Complete Connection Queue (Accept 큐)
   - 3-way handshake 완료
   - accept() 대기 중
   - 상태: ESTABLISHED
```

#### 왜 두 개의 큐로 분리되어 있는가?

**이유 1: 3-way Handshake의 비동기성**

3-way handshake는 시간이 걸립니다:
```
클라이언트                서버
    |                      |
    |-------- SYN -------->|  1단계: SYN 큐에 삽입
    |                      |  (아직 미완료)
    |<----- SYN+ACK -------|  2단계
    |                      |  (RTT만큼 시간 소요)
    |------- ACK --------->|  3단계: Accept 큐로 이동
    |                      |  (이제 완료)
```

**RTT(Round Trip Time)** 동안:
- 클라이언트와 서버 간 왕복 시간 (예: 100ms)
- 이 시간 동안 연결은 "진행 중" 상태
- 다른 연결 요청들을 계속 받아야 함

**만약 큐가 하나라면:**
```
❌ 단일 큐 방식:
- SYN 받음 → 큐 삽입
- 3-way handshake 완료 대기 (blocking)
- 이 시간 동안 다른 연결 요청 처리 못함 → 성능 저하!
```

**두 개 큐의 장점:**
```
✅ 이중 큐 방식:
SYN 큐: 진행 중인 핸드셰이크들 관리
   ↓ (비동기적으로 완료되면)
Accept 큐: 완료된 연결들 대기
   ↓
accept() 호출 시 즉시 반환
```

**이유 2: SYN Flood 공격 방어**

**SYN Flood 공격이란?**

공격자가 대량의 SYN 패킷을 보내지만 ACK를 보내지 않는 공격:
```
공격자                  서버
  |------ SYN --------->|  SYN 큐 +1
  |------ SYN --------->|  SYN 큐 +1
  |------ SYN --------->|  SYN 큐 +1
  | (ACK 안 보냄!)      |  ...
  |------ SYN --------->|  SYN 큐 가득 참!
                        |  → 정상 연결 불가 ❌
```

**왜 큐를 분리하면 도움이 되는가?**

**분리 전 (단일 큐):**
```
공격 SYN들이 큐를 가득 채움
→ 완료된 정상 연결도 큐에 들어갈 공간 없음
→ accept()가 가져갈 연결이 없음
→ 서비스 완전 마비 ❌
```

**분리 후 (이중 큐):**
```
SYN 큐: 공격 SYN들로 가득 참
  ↓ (하지만)
Accept 큐: 완료된 정상 연결은 여기 저장
  ↓
accept()는 Accept 큐에서 가져감
→ 공격받아도 일부 정상 연결은 처리 가능 ✅
```

**추가 방어 메커니즘: SYN Cookies**

SYN 큐가 가득 차면 활성화:
```
1. SYN 큐에 저장하지 않음
2. SYN+ACK의 시퀀스 번호에 연결 정보 인코딩
3. ACK 받으면 시퀀스 번호 디코딩하여 연결 복원
→ 메모리 사용 없이 연결 처리 가능!
```

**backlog의 의미:**

```
클라이언트 연결 요청들
        ↓  ↓  ↓
    [SYN 큐] → [Accept 큐] ← backlog 크기
                    ↓
              accept()로 하나씩 꺼냄
```

**역사적으로:**
- 예전: backlog = SYN 큐 + Accept 큐 전체 크기
- 현대 Linux: backlog = Accept 큐 크기만 지정
- SYN 큐 크기는 별도 커널 파라미터로 관리

**큐가 가득 차면?**

**Accept 큐가 가득 찬 경우:**
- 새로 완료된 연결을 Accept 큐에 넣을 수 없음
- SYN 큐의 연결들이 이동 불가
- **동작:** 새 SYN 무시 또는 거부
- **클라이언트:** 연결 실패 또는 타임아웃
- **의미:** "서버가 바쁨" 신호

**SYN 큐가 가득 찬 경우:**
- SYN Cookies 활성화 (커널 설정 시)
- 또는 SYN 패킷 드롭

**적절한 backlog 값:**
```c
listen(sockfd, 5);      // 작은 서버
listen(sockfd, 128);    // 일반적
listen(sockfd, 1024);   // 고성능 서버
```

**고려사항:**
- 너무 작으면: 동시 접속 시 거부됨
- 너무 크면: 메모리 낭비, SYN Flood 공격 취약
- 시스템 한계: `/proc/sys/net/core/somaxconn` (Linux)

**예제:**
```c
if (listen(sockfd, 10) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
}
printf("Server is listening...\n");
```

**주요 에러:**
- `EADDRINUSE`: 다른 소켓이 이미 listening
- `EOPNOTSUPP`: 소켓 타입이 listen 미지원 (예: SOCK_DGRAM)

---

### 3.3.4 accept() - 연결 수락

**함수 원형:**
```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

**목적:**
대기 큐에서 완료된 연결 하나를 꺼내서 수락합니다. "전화벨이 울릴 때 수화기를 드는" 것과 같습니다.

**핵심 개념: 새로운 소켓 생성**

```
원래 소켓 (listening socket)
      ↓
   accept()
      ↓
새 소켓 (connected socket) ← 이 소켓으로 통신!
```

#### 왜 새 소켓을 만드는가?

**원리: 하나의 소켓은 하나의 연결만 표현 가능**

소켓은 **5-튜플(5-tuple)**로 연결을 식별합니다:
```
(프로토콜, 로컬 IP, 로컬 포트, 원격 IP, 원격 포트)
```

**문제 상황: 만약 새 소켓을 만들지 않는다면?**

```
서버 (192.168.1.100:8080)

클라이언트 A (192.168.1.10:50000) 연결
→ 원래 소켓: (TCP, 192.168.1.100, 8080, 192.168.1.10, 50000)

클라이언트 B (192.168.1.11:50001) 연결 시도
→ 원래 소켓: (TCP, 192.168.1.100, 8080, ???, ???)
   ❌ 이미 A와 연결됨! 어떻게 B와 동시에 연결?
```

**하나의 소켓은 하나의 연결 상태만 가질 수 있음:**
- TCP 소켓은 연결 상태 정보를 가짐 (시퀀스 번호, 윈도우 크기 등)
- 여러 클라이언트의 상태를 하나의 소켓에 저장 불가능
- 각 클라이언트마다 독립적인 소켓 필요

**해결: 각 연결마다 새 소켓 생성**

```
Listening 소켓 (server_fd)
- 상태: LISTEN
- 역할: 새 연결 요청만 받음
- 5-튜플: (TCP, 192.168.1.100, 8080, *, *)
           (어떤 클라이언트든 연결 가능)

accept() 호출 시:

Connected 소켓 1 (client_fd_1)
- 상태: ESTABLISHED
- 역할: 클라이언트 A와 통신
- 5-튜플: (TCP, 192.168.1.100, 8080, 192.168.1.10, 50000)

Connected 소켓 2 (client_fd_2)
- 상태: ESTABLISHED
- 역할: 클라이언트 B와 통신
- 5-튜플: (TCP, 192.168.1.100, 8080, 192.168.1.11, 50001)
```

#### 포트 번호는 어떻게 관리되는가?

**의문: 서버 포트 8080을 여러 소켓이 사용?**

```
서버 (모두 포트 8080 사용)
- Listening 소켓: 8080
- Connected 소켓 1: 8080
- Connected 소켓 2: 8080
→ 포트 충돌 아닌가? ❌
```

**답: 5-튜플이 다르면 포트 재사용 가능!**

**식별 원리:**
```
연결 1: (TCP, 서버IP, 8080, 클라이언트A_IP, 클라이언트A_포트)
연결 2: (TCP, 서버IP, 8080, 클라이언트B_IP, 클라이언트B_포트)
         └─ 같음 ─┘            └──── 다름 ────┘
                              → 다른 연결로 구별됨 ✅
```

**OS 커널의 동작:**
1. 패킷 도착
2. 5-튜플 확인
3. 일치하는 소켓 찾기
4. 해당 소켓의 수신 버퍼로 전달

**예시:**
```
패킷 도착: [출발: 192.168.1.10:50000] [도착: 서버:8080]

커널이 찾는 소켓:
(TCP, 서버IP, 8080, 192.168.1.10, 50000)
→ client_fd_1로 전달 ✅
```

#### 실제 동작 시나리오

**서버 코드:**
```c
int server_fd = socket(...);      // 소켓 생성
bind(server_fd, ..., 8080);       // 포트 8080 바인딩
listen(server_fd, 10);            // LISTEN 상태
```

**현재 상태:**
```
server_fd: (TCP, *, 8080, *, *) - LISTEN
```

**클라이언트 A 접속:**
```c
int client_fd_1 = accept(server_fd, ...);  // 새 소켓 생성
```

**현재 상태:**
```
server_fd:   (TCP, *, 8080, *, *) - LISTEN (여전히 대기 중)
client_fd_1: (TCP, 서버IP, 8080, A_IP, A_포트) - ESTABLISHED
```

**클라이언트 B 접속:**
```c
int client_fd_2 = accept(server_fd, ...);  // 또 다른 새 소켓
```

**현재 상태:**
```
server_fd:   (TCP, *, 8080, *, *) - LISTEN (계속 대기)
client_fd_1: (TCP, 서버IP, 8080, A_IP, A_포트) - ESTABLISHED
client_fd_2: (TCP, 서버IP, 8080, B_IP, B_포트) - ESTABLISHED
```

**각 소켓의 역할:**
- **server_fd**: 계속 LISTEN 상태 유지, 새 연결 요청 받음
- **client_fd_1**: A와만 통신, 독립적인 TCP 상태 유지
- **client_fd_2**: B와만 통신, 독립적인 TCP 상태 유지

**왜 새 소켓?**
1. **원래 소켓**: 계속 LISTEN 상태 유지 (다른 클라이언트 받기)
2. **새 소켓**: 이 클라이언트와만 통신
3. **한 서버가 여러 클라이언트 동시 처리 가능**
4. **각 연결은 독립적인 TCP 상태 관리** (시퀀스 번호, 버퍼 등)

**매개변수:**
```c
sockfd:  listening 소켓
addr:    클라이언트 주소 정보 저장 (출력)
addrlen: addr 버퍼 크기 (입출력)
```

**반환값:**
- **성공**: 새로운 소켓 디스크립터 (connected socket)
- **실패**: -1

**블로킹 동작:**
- 기본적으로 **블로킹**
- 대기 큐에 연결이 없으면 새 연결 올 때까지 대기
- 논블로킹 모드 설정 가능 (즉시 반환, EWOULDBLOCK)

**예제:**
```c
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);

int client_fd = accept(server_fd, 
                      (struct sockaddr*)&client_addr, 
                      &client_len);

if (client_fd < 0) {
    perror("accept failed");
    exit(EXIT_FAILURE);
}

// 클라이언트 정보 출력
char client_ip[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &client_addr.sin_addr, 
         client_ip, sizeof(client_ip));
printf("Client connected: %s:%d\n", 
       client_ip, ntohs(client_addr.sin_port));

// 클라이언트와 통신
char buf[1024];
recv(client_fd, buf, sizeof(buf), 0);
send(client_fd, buf, strlen(buf), 0);

// 연결 종료
close(client_fd);  // 이 클라이언트만 종료
// server_fd는 계속 살아있음!
```

**여러 클라이언트 처리 패턴:**

**1. 순차 처리 (Iterative):**
```c
while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    handle_client(client_fd);  // 완료될 때까지 블로킹
    close(client_fd);
}
```

**2. 프로세스 기반 (Forking):**
```c
while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    
    if (fork() == 0) {  // 자식 프로세스
        close(server_fd);  // 자식은 listening 소켓 불필요
        handle_client(client_fd);
        exit(0);
    }
    
    close(client_fd);  // 부모는 connected 소켓 불필요
}
```

**3. 스레드 기반:**
```c
while (1) {
    int *client_fd = malloc(sizeof(int));
    *client_fd = accept(server_fd, NULL, NULL);
    
    pthread_t tid;
    pthread_create(&tid, NULL, handle_client, client_fd);
    pthread_detach(tid);
}
```

**주의사항:**

**1. 반드시 소켓 닫기:**
```c
close(client_fd);  // 메모리 누수 및 FD 고갈 방지
```

**2. 멀티프로세스/스레드 시:**
```c
// 부모 프로세스
close(client_fd);  // connected socket 닫기

// 자식 프로세스  
close(server_fd);  // listening socket 닫기
```

**주요 에러:**
- `EAGAIN/EWOULDBLOCK`: 논블로킹 모드에서 대기 큐 비어있음
- `EINTR`: 시그널에 의해 중단 (재시도 필요)
- `EMFILE`: 프로세스 FD 한계
- `ENFILE`: 시스템 전체 FD 한계

---

### 3.3.5 connect() - 연결 요청

**함수 원형:**
```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

**목적:**
클라이언트가 서버에 연결을 요청합니다. "전화를 거는" 것과 같습니다.

**매개변수:**
```c
sockfd: 클라이언트 소켓
addr:   서버 주소 (IP + 포트)
addrlen: addr 크기
```

**내부 동작 (3-way handshake):**

```
클라이언트                    서버
CLOSED                     LISTEN
  ↓
[1] SYN 전송
SYN_SENT  ──[SYN]──→      SYN_RCVD
          (seq=x)
  ↓
[2] SYN+ACK 수신
          ←[SYN+ACK]─     
          (seq=y, ack=x+1)
  ↓
[3] ACK 전송
ESTABLISHED ─[ACK]──→     ESTABLISHED
          (ack=y+1)
  ↓
connect() 반환            accept() 반환 가능
```

**자세한 설명:**

1. **클라이언트 → 서버: SYN**
   - "연결하고 싶습니다"
   - 초기 시퀀스 번호(ISN) 전송

2. **서버 → 클라이언트: SYN + ACK**
   - "알았어요, 저도 연결할게요"
   - 서버의 ISN + 클라이언트 SYN 확인

3. **클라이언트 → 서버: ACK**
   - "네, 연결되었습니다"
   - 서버의 SYN 확인

#### 시퀀스 번호와 ACK 번호의 의미

**시퀀스 번호(Sequence Number):**
- TCP는 바이트 스트림의 각 바이트에 번호를 부여
- 시퀀스 번호 = "내가 지금 보내는 데이터의 시작 번호"
- 수신자가 순서대로 재조립할 수 있게 함

**ACK 번호(Acknowledgement Number):**
- "다음에 받고 싶은 바이트 번호"
- 즉, "여기까지 잘 받았다"는 의미

**3-way Handshake에서의 번호:**

```
클라이언트                           서버
   |                                  |
   |---- SYN, seq=100 -------------->|
   |     "내 시작 번호는 100"          |
   |                                  |
   |<--- SYN+ACK, seq=200, ack=101 ---|
   |     "내 시작 번호는 200"          |
   |     "너의 101번부터 받을게"       |
   |                                  |
   |---- ACK, ack=201 --------------->|
   |     "너의 201번부터 받을게"       |
```

**왜 초기 번호가 0이 아닌가?**

ISN(Initial Sequence Number)은 랜덤 값:
```
예: 1000000, 3456789 등
```

**이유:**
1. **보안**: 예측 가능하면 세션 하이재킹 공격 가능
2. **재연결 구분**: 이전 연결의 지연 패킷 구분

**시퀀스 번호 예시:**

```
클라이언트가 "HELLO" (5바이트) 전송:
seq=101, data="HELLO"
→ 서버가 응답: ack=106 (101 + 5)

서버가 "WORLD" (5바이트) 전송:
seq=201, data="WORLD"
→ 클라이언트가 응답: ack=206 (201 + 5)
```

#### 왜 3단계가 필요한가? 2단계는 안 되나?

**2-way Handshake를 시도하면?**

```
❌ 2-way 방식:
클라이언트 → 서버: SYN (seq=x)
서버 → 클라이언트: SYN+ACK (seq=y, ack=x+1)
→ 연결 완료?
```

**문제 1: 지연된 중복 SYN**

```
시나리오:
1. 클라이언트가 SYN 전송 (seq=100)
2. 네트워크 지연으로 오래 걸림
3. 클라이언트가 타임아웃 → 재전송 (seq=200)
4. 새 SYN(seq=200)으로 연결 성공
5. 연결 종료
6. 옛날 SYN(seq=100)이 이제 도착!
   → 서버가 SYN+ACK 응답
   → 2-way라면 연결 완료로 간주
   → 서버는 연결 대기, 클라이언트는 모름
   → 자원 낭비! ❌
```

**3-way는 이를 방지:**
```
✅ 3-way 방식:
6. 옛날 SYN(seq=100) 도착
   → 서버가 SYN+ACK 응답
   → 클라이언트가 ACK를 보내지 않음 (모르는 연결)
   → 서버가 타임아웃 후 정리
   → 자원 낭비 방지 ✅
```

**문제 2: 양방향 시퀀스 번호 확인**

TCP는 **양방향 통신**:
- 클라이언트 → 서버: 시퀀스 번호 필요
- 서버 → 클라이언트: 별도의 시퀀스 번호 필요

**2-way라면:**
```
클라이언트 → 서버: SYN (클라이언트 seq 전달)
서버 → 클라이언트: SYN+ACK (서버 seq 전달)
→ 서버의 seq를 클라이언트가 확인했다는 보장 없음! ❌
```

**3-way라면:**
```
클라이언트 → 서버: SYN (클라이언트 seq=x)
서버 → 클라이언트: SYN+ACK (서버 seq=y, ack=x+1)
클라이언트 → 서버: ACK (ack=y+1)
→ 양쪽 모두 상대방의 시퀀스 번호 확인 완료 ✅
```

#### 초기 시퀀스 번호(ISN)의 중요성

**ISN 생성 방식 (RFC 6528):**

```
ISN = M + F(localhost, localport, remotehost, remoteport, secret)

M: 4 마이크로초마다 증가하는 카운터
F: 암호화 해시 함수
secret: 서버만 아는 비밀 값
```

**왜 랜덤해야 하는가?**

**공격 시나리오 (ISN이 예측 가능하면):**

```
1. 공격자가 서버의 ISN 패턴 관찰
   서버 ISN: 100, 101, 102, 103... (순차적)

2. 공격자가 희생자 IP로 위장
   SYN 전송 (출발 IP: 희생자, 목적 IP: 서버)

3. 서버가 희생자에게 SYN+ACK 전송
   서버 → 희생자: SYN+ACK (seq=104, ack=...)

4. 공격자는 SYN+ACK를 못 받지만 ISN 예측 가능!
   다음 ISN = 104일 것으로 예상

5. 공격자가 ACK 전송 (출발 IP: 희생자)
   공격자 → 서버: ACK (ack=105)

6. 서버는 희생자와 연결되었다고 착각!
   → 세션 하이재킹 성공 ❌
```

**랜덤 ISN으로 방어:**
```
ISN이 랜덤하면 공격자가 예측 불가
→ 올바른 ACK 번호를 보낼 수 없음
→ 공격 실패 ✅
```

**ISN의 또 다른 역할:**

**1. 재연결 구분**
```
연결 1: ISN=1000000
  → 데이터 송수신
  → 종료

연결 2: ISN=5000000 (다른 값)
  → 연결 1의 지연 패킷이 도착해도
  → 시퀀스 번호가 완전히 달라서 구분 가능
```

**2. 네트워크 지연 패킷 필터링**
```
옛날 연결의 패킷이 나중에 도착
→ 시퀀스 번호가 현재 윈도우 범위 밖
→ 자동으로 폐기
```

**자동 바인딩:**

connect() 호출 시:
1. 소켓이 바인딩 안 되었으면 자동으로 바인딩
2. 로컬 포트: OS가 임시 포트 자동 할당 (49152-65535)
3. 로컬 IP: 라우팅 테이블 기반 자동 선택

**블로킹 동작:**
- 기본적으로 **블로킹**
- 3-way handshake 완료될 때까지 대기
- 타임아웃: 보통 75초 (시스템 의존적)

**예제:**
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(8080);

// IP 주소 변환
if (inet_pton(AF_INET, "192.168.1.100", 
              &server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    exit(EXIT_FAILURE);
}

// 연결 시도
if (connect(sockfd, (struct sockaddr*)&server_addr, 
            sizeof(server_addr)) < 0) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
}

printf("Connected to server!\n");

// 데이터 송수신
send(sockfd, "Hello", 5, 0);
char buf[1024];
recv(sockfd, buf, sizeof(buf), 0);

close(sockfd);
```

**inet_pton() - 주소 변환:**
```c
// Presentation (문자열) TO Network (바이너리)
int inet_pton(int af, const char *src, void *dst);

// 반환값:
//  1: 성공
//  0: src가 유효한 주소 형식 아님
// -1: af가 지원되지 않는 주소 체계

// 반대: Network TO Presentation
const char *inet_ntop(int af, const void *src, 
                      char *dst, socklen_t size);
```

**연결 실패 원인:**

**1. ECONNREFUSED**
```
원인: 서버가 해당 포트에서 listen하지 않음
해결: 
  - 서버 프로그램 실행 여부 확인
  - 올바른 포트 번호 확인
  - 방화벽 설정 확인
```

**2. ETIMEDOUT**
```
원인: 네트워크 경로 문제 또는 서버 무응답
해결:
  - 네트워크 연결 확인
  - 서버 IP 주소 확인
  - ping으로 서버 도달 가능 여부 확인
```

**3. EHOSTUNREACH**
```
원인: 호스트에 도달할 수 없음
해결: 라우팅 테이블, 네트워크 설정 확인
```

**4. ENETUNREACH**
```
원인: 네트워크에 도달할 수 없음
해결: 기본 게이트웨이 설정 확인
```

**타임아웃 설정:**
```c
struct timeval timeout;
timeout.tv_sec = 5;   // 5초
timeout.tv_usec = 0;

setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, 
          &timeout, sizeof(timeout));

connect(sockfd, ...);  // 5초 후 타임아웃
```

**논블로킹 connect():**
```c
// 1. 논블로킹 모드 설정
int flags = fcntl(sockfd, F_GETFL, 0);
fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

// 2. connect() 호출 (즉시 반환)
if (connect(sockfd, ...) < 0) {
    if (errno == EINPROGRESS) {
        // 연결 진행 중
        
        // 3. select()로 완료 확인
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sockfd, &writefds);
        
        struct timeval tv = {5, 0};  // 5초 타임아웃
        
        if (select(sockfd + 1, NULL, &writefds, NULL, &tv) > 0) {
            // 4. 연결 결과 확인
            int error;
            socklen_t len = sizeof(error);
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
            
            if (error == 0) {
                printf("Connected!\n");
            } else {
                fprintf(stderr, "Connect error: %s\n", 
                       strerror(error));
            }
        } else {
            fprintf(stderr, "Connection timeout\n");
        }
    }
}
```

---

### 3.3.6 send() / recv() - 데이터 송수신

**함수 원형:**
```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

**목적:**
연결된 소켓을 통해 데이터를 주고받습니다.

**매개변수:**
```c
sockfd: 연결된 소켓 (connected socket)
buf:    데이터 버퍼 (send: 보낼 데이터, recv: 받을 공간)
len:    버퍼 크기 (send: 보낼 바이트, recv: 최대 받을 바이트)
flags:  옵션 플래그
```

**반환값:**
- **성공**: 실제로 전송/수신한 바이트 수
  - ⚠️ **중요**: 요청한 크기와 다를 수 있음!
- **실패**: -1
- **연결 종료**: 0 (recv만)

**핵심 개념: TCP는 스트림이다**

이것이 가장 중요한 개념입니다!

```c
// 보내는 쪽
send(sock, "Hello", 5, 0);  // 5바이트 전송
send(sock, "World", 5, 0);  // 5바이트 전송

// 받는 쪽 - 어떻게 받을까?
// 경우 1: 한 번에
recv(sock, buf, 100, 0);  // "HelloWorld" (10바이트)

// 경우 2: 나뉘어서
recv(sock, buf, 100, 0);  // "Hel" (3바이트)
recv(sock, buf, 100, 0);  // "loWor" (5바이트)
recv(sock, buf, 100, 0);  // "ld" (2바이트)

// 경우 3: 그 외 임의의 조합
```

**왜 이런가?**
- TCP는 메시지 경계를 유지하지 않음
- 네트워크 패킷 크기, 버퍼 상태 등에 따라 쪼개지거나 합쳐짐
- **애플리케이션이 직접 메시지 경계를 관리해야 함**

#### 왜 부분 송수신이 발생하는가?

**부분 전송 (Partial Send) 발생 원인:**

**1. 송신 버퍼 공간 부족**

TCP 소켓은 커널에 송신 버퍼를 가지고 있습니다:
```
애플리케이션            커널 송신 버퍼           네트워크
    send() → [버퍼에 복사] → [TCP가 전송]
```

**시나리오:**
```
송신 버퍼 현재 상태:
[이미 사용 중인 8KB] [남은 공간 2KB]

애플리케이션: send(sock, data, 10KB, 0);

커널 동작:
- 10KB 요청했지만 버퍼에 2KB만 남음
- 2KB만 버퍼에 복사
- send() 반환: 2000 (2KB만 전송)
- 나머지 8KB는 전송 안 됨!

애플리케이션이 다시 send() 해야 함:
send(sock, data+2000, 8KB, 0);  // 남은 부분 전송
```

**왜 버퍼가 가득 찰까?**
```
송신 속도 > 네트워크 처리 속도

애플리케이션이 빠르게 send()
  ↓
버퍼에 데이터 쌓임
  ↓
네트워크가 천천히 전송
  ↓
버퍼 가득 참!
```

**2. 네트워크 혼잡 제어 (Congestion Control)**

TCP는 네트워크 상태에 따라 전송 속도를 조절합니다:
```
네트워크 혼잡 감지
  ↓
송신 윈도우 크기 축소
  ↓
한 번에 보낼 수 있는 데이터 양 감소
  ↓
부분 전송 발생
```

**3. 수신측 윈도우 크기 (Flow Control)**

수신측이 받을 수 있는 양을 통보합니다:
```
수신측: "나는 1KB만 받을 수 있어" (윈도우 크기 1KB)
  ↓
송신측: send(sock, 10KB, ...)
  ↓
실제로는 1KB만 전송
  ↓
send() 반환: 1000
```

**예시:**
```
수신 버퍼 상태:
[이미 받은 데이터 15KB] [남은 공간 1KB]

수신측이 송신측에게:
"TCP 윈도우: 1KB" (헤더에 포함)

송신측 send(10KB):
→ 실제로 1KB만 전송 (수신측 윈도우 크기 제한)
```

**부분 수신 (Partial Recv) 발생 원인:**

**1. 수신 버퍼에 데이터가 충분하지 않음**

```
수신 버퍼 현재 상태:
[도착한 데이터 3KB] [비어있음]

애플리케이션: recv(sock, buf, 10KB, 0);

커널 동작:
- 10KB 요청했지만 버퍼에 3KB만 있음
- 3KB를 애플리케이션에 복사
- recv() 반환: 3000 (3KB만 수신)
- 나머지는 아직 네트워크에서 오는 중!
```

**2. 패킷이 아직 도착하지 않음**

네트워크 경로에서 패킷이 지연:
```
송신측이 10KB 전송:
[패킷1 2KB] [패킷2 2KB] [패킷3 2KB] [패킷4 2KB] [패킷5 2KB]

수신측 상황:
패킷1, 2 도착 (4KB)
패킷3, 4, 5는 네트워크 지연으로 아직 미도착

recv(10KB) 호출:
→ 버퍼에 4KB만 있음
→ 4KB만 반환
```

**3. TCP 세그먼트 크기 제한 (MSS)**

Maximum Segment Size - 한 번에 보낼 수 있는 최대 크기:
```
애플리케이션: send(100KB)

TCP 동작:
- MSS = 1460바이트 (일반적)
- 100KB를 1460바이트씩 여러 패킷으로 분할
  패킷1: 1460바이트
  패킷2: 1460바이트
  ...
  패킷69: 1460바이트

수신측은 패킷이 도착할 때마다 recv() 가능
→ 한 번에 1460바이트만 받을 수도 있음
```

**실제 시나리오:**

```
송신측: send(sock, 10000바이트)
  ↓
송신 버퍼에 5000바이트 공간만 남음
  ↓
send() 반환: 5000 ← 부분 전송!

---

수신측: recv(sock, buf, 10000)
  ↓
수신 버퍼에 3000바이트만 도착
  ↓
recv() 반환: 3000 ← 부분 수신!
```

**버퍼 크기 확인:**
```c
// 송신 버퍼 크기 확인
int sndbuf;
socklen_t len = sizeof(sndbuf);
getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sndbuf, &len);
printf("송신 버퍼: %d바이트\n", sndbuf);

// 수신 버퍼 크기 확인
int rcvbuf;
getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &len);
printf("수신 버퍼: %d바이트\n", rcvbuf);
```

**일반적인 버퍼 크기 (Linux):**
```
송신 버퍼: 16KB ~ 256KB (시스템 설정)
수신 버퍼: 16KB ~ 256KB
MSS: 1460바이트 (이더넷 MTU 1500 - IP 20 - TCP 20)
```

**부분 전송/수신 처리:**

**문제:**
```c
char msg[1000];
// ... msg 채움 ...
send(sock, msg, 1000, 0);  // 1000바이트 다 보냈다고 가정? NO!
// 실제로는 500바이트만 보내졌을 수 있음!

이유:
1. 송신 버퍼에 500바이트 공간만 남음
2. 네트워크 혼잡으로 윈도우 크기 축소
3. 수신측 버퍼 공간 부족
```

**해결: 전체 전송 보장 함수**
```c
ssize_t send_all(int sockfd, const void *buf, size_t len) {
    size_t total_sent = 0;
    const char *ptr = buf;
    
    while (total_sent < len) {
        ssize_t sent = send(sockfd, ptr + total_sent, 
                           len - total_sent, 0);
        if (sent < 0) {
            if (errno == EINTR) continue;  // 시그널 재시도
            return -1;  // 실제 에러
        }
        if (sent == 0) break;  // 이상한 경우
        
        total_sent += sent;
    }
    
    return total_sent;
}
```

**메시지 구분 방법:**

#### 1. 고정 길이 (Fixed-Length)

```c
// 항상 정확히 100바이트씩
char msg[100];
send_all(sock, msg, 100);

// 받는 쪽도 정확히 100바이트
char buf[100];
recv_exact(sock, buf, 100);
```

**장점:**
- 구현이 가장 간단
- 파싱 오버헤드 없음
- 빠른 처리 속도

**단점:**
- 데이터 크기가 다양하면 비효율적
- 짧은 데이터도 패딩 필요 → 대역폭 낭비
- 유연성 부족

**사용 예:**
```
고정 크기 레코드 전송:
[이름 20바이트][나이 4바이트][주소 100바이트] = 총 124바이트

"Kim"을 보낼 때:
"Kim\0\0\0\0..." (20바이트로 패딩)
```

**실제 프로토콜:**
- **TFTP** (Trivial File Transfer Protocol): 512바이트 고정 블록
- **고정 크기 바이너리 프로토콜**: 금융 시스템, 레거시 프로토콜

#### 2. 길이 헤더 (Length-Prefixed)

```c
// 보내는 쪽
uint32_t len = htonl(strlen(data));
send_all(sock, &len, 4);           // 먼저 길이 전송
send_all(sock, data, strlen(data)); // 그 다음 데이터

// 받는 쪽
uint32_t len;
recv_exact(sock, &len, 4);  // 먼저 길이 수신
len = ntohl(len);

char *buf = malloc(len + 1);
recv_exact(sock, buf, len);  // 그 다음 데이터
buf[len] = '\0';
```

**장점:**
- 가변 길이 데이터 효율적 처리
- 미리 메모리 할당 가능 (길이를 알기 때문)
- 바이너리 데이터 전송 가능 (구분자 충돌 없음)
- 빠른 파싱 (길이만큼 읽으면 됨)

**단점:**
- 헤더 오버헤드 (보통 2~4바이트)
- 길이 필드가 손상되면 전체 스트림 손상 가능
- 구현이 고정 길이보다 복잡

**변형:**
```
1. 2바이트 길이 (최대 65535바이트):
   [uint16_t len][데이터]

2. 4바이트 길이 (최대 4GB):
   [uint32_t len][데이터]

3. 가변 길이 인코딩 (Protobuf):
   [varint len][데이터]
   작은 숫자는 1바이트, 큰 숫자는 여러 바이트
```

**실제 프로토콜:**
- **HTTP/2**: 길이 필드로 프레임 구분
- **TLS/SSL**: 레코드 헤더에 길이 포함
- **Protobuf, Thrift**: 메시지 길이 prefix
- **WebSocket**: 페이로드 길이 필드
- **Redis RESP**: Bulk String에 길이 prefix

**예시 - Redis RESP:**
```
$5\r\n       ← 길이: 5바이트
Hello\r\n    ← 데이터: "Hello"
```

#### 3. 구분자 (Delimiter-Based)

```c
// 줄바꿈으로 구분
send(sock, "Hello\n", 6, 0);
send(sock, "World\n", 6, 0);

// 받는 쪽: 줄바꿈까지 읽기
char buf[1000];
int i = 0;
while (i < 999) {
    char c;
    if (recv(sock, &c, 1, 0) <= 0) break;
    if (c == '\n') break;
    buf[i++] = c;
}
buf[i] = '\0';
```

**장점:**
- 사람이 읽기 쉬움 (텍스트 프로토콜)
- 디버깅 용이 (telnet으로 테스트 가능)
- 구현 간단 (텍스트 기반)
- 길이 제한 없음

**단점:**
- 바이너리 데이터 전송 어려움 (구분자와 충돌)
- 1바이트씩 읽으면 성능 저하 (시스템 콜 오버헤드)
- 메시지 끝까지 읽어야 길이 파악 가능
- 이스케이프 처리 필요 (데이터에 구분자 포함 시)

**일반적인 구분자:**
```
1. 줄바꿈 (LF): \n
   - Unix 스타일
   - 예: Redis, SMTP

2. CR+LF: \r\n
   - Windows/네트워크 표준
   - 예: HTTP, FTP, POP3

3. Null 문자: \0
   - C 문자열 스타일
   - 바이너리 프로토콜에서 드물게 사용

4. 다중 문자: \r\n\r\n
   - HTTP 헤더 종료
```

**최적화 - 버퍼링:**
```c
// ❌ 비효율적: 1바이트씩 읽기
recv(sock, &c, 1, 0);  // 매번 시스템 콜!

// ✅ 효율적: 버퍼링
char buffer[4096];
int buf_pos = 0;
int buf_len = 0;

// 버퍼에서 줄 읽기
while (1) {
    if (buf_pos >= buf_len) {
        // 버퍼 재충전
        buf_len = recv(sock, buffer, sizeof(buffer), 0);
        buf_pos = 0;
    }

    if (buffer[buf_pos] == '\n') break;
    line[line_pos++] = buffer[buf_pos++];
}
```

**이스케이프 처리:**
```
문제: 데이터에 구분자 포함 시

"Hello\nWorld"를 전송하고 싶은데 \n이 구분자!

해결 1: 이스케이프
  "Hello\\nWorld\n"  (\\n은 리터럴 개행)

해결 2: 다른 프레이밍 사용
  길이 헤더 방식으로 전환
```

**실제 프로토콜:**
- **HTTP/1.1**: `\r\n`으로 헤더 라인 구분, `\r\n\r\n`으로 헤더 종료
- **SMTP, POP3**: `\r\n`으로 명령 구분
- **Redis RESP**: `\r\n`으로 요소 구분
- **FTP**: `\r\n`으로 명령 구분
- **IRC**: `\r\n`으로 메시지 구분

**HTTP 예시:**
```
GET / HTTP/1.1\r\n
Host: example.com\r\n
\r\n
← 헤더 끝

Body 시작...
```

#### 패턴 비교표

| 패턴 | 구현 | 효율성 | 바이너리 | 디버깅 | 실제 사용 |
|------|------|--------|----------|--------|-----------|
| 고정 길이 | 매우 쉬움 | 중간 | ✅ | ✅ | TFTP, 금융 |
| 길이 헤더 | 중간 | 높음 | ✅ | 중간 | HTTP/2, TLS, Protobuf |
| 구분자 | 쉬움 | 낮음* | ❌ | ✅ | HTTP/1.1, SMTP, Redis |

*버퍼링으로 개선 가능

#### 조합 패턴

**HTTP/1.1 방식 (혼합):**
```
1. 헤더: 구분자 기반 (\r\n)
   GET / HTTP/1.1\r\n
   Content-Length: 1234\r\n
   \r\n

2. Body: 길이 헤더 기반
   Content-Length 값만큼 읽기 (1234바이트)
```

**장점:**
- 헤더는 사람이 읽기 쉬움 (디버깅)
- Body는 효율적 전송 (바이너리 가능)

**flags 옵션:**

**MSG_DONTWAIT**
```c
recv(sock, buf, len, MSG_DONTWAIT);
```
- 이 호출만 논블로킹
- 데이터 없으면 즉시 EAGAIN 반환

**MSG_PEEK**
```c
recv(sock, buf, len, MSG_PEEK);
```
- 데이터를 읽되 버퍼에서 제거하지 않음
- "훔쳐보기"
- 다음 recv()에서 같은 데이터를 다시 읽을 수 있음

**MSG_WAITALL**
```c
recv(sock, buf, len, MSG_WAITALL);
```
- 요청한 바이트를 모두 받을 때까지 대기
- 또는 에러/연결 종료/시그널까지

**MSG_NOSIGNAL**
```c
send(sock, buf, len, MSG_NOSIGNAL);
```
- 닫힌 소켓에 write 시 SIGPIPE 방지
- 시그널 대신 EPIPE 에러 반환

**연결 종료 감지:**
```c
ssize_t n = recv(sock, buf, len, 0);

if (n > 0) {
    // 정상 수신
    printf("Received %zd bytes\n", n);
} else if (n == 0) {
    // 상대방이 연결 종료 (FIN 받음)
    printf("Connection closed by peer\n");
    close(sock);
} else {
    // 에러 (n == -1)
    perror("recv error");
}
```

**예제: 에코 서버**
```c
char buffer[1024];
ssize_t n;

while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
    // 받은 데이터를 그대로 전송
    if (send_all(client_fd, buffer, n) < 0) {
        perror("send failed");
        break;
    }
}

if (n == 0) {
    printf("Client disconnected\n");
} else if (n < 0) {
    perror("recv failed");
}

close(client_fd);
```

---

### 3.3.7 close() / shutdown() - 연결 종료

**함수 원형:**
```c
int close(int sockfd);
int shutdown(int sockfd, int how);
```

**목적:**
소켓 연결을 종료합니다.

**close() - 완전 종료**

**동작:**
1. 소켓 디스크립터의 **참조 카운트 감소**
2. 참조 카운트가 0이 되면:
   - TCP 4-way handshake 시작 (FIN 전송)
   - 송신 버퍼의 남은 데이터 전송 시도
   - 소켓 리소스 해제
3. 양방향 모두 종료

**참조 카운트란?**
```c
int fd = socket(...);     // ref_count = 1

pid_t pid = fork();
// 부모와 자식이 같은 fd 공유 → ref_count = 2

close(fd);  // 부모 → ref_count = 1
// 소켓은 아직 닫히지 않음!

close(fd);  // 자식 → ref_count = 0
// 이제 실제로 닫힘
```

**예제:**
```c
close(sockfd);
// 이후 sockfd는 사용 불가
```

**shutdown() - 부분 종료**

**함수 원형:**
```c
int shutdown(int sockfd, int how);
```

**how 옵션:**

**SHUT_RD (0) - 수신 종료**
```c
shutdown(sockfd, SHUT_RD);
```
- 더 이상 수신하지 않겠다고 선언
- recv()는 0 반환 (EOF)
- 상대방의 send()는 여전히 작동
- 실제로는 거의 사용 안 함

**SHUT_WR (1) - 송신 종료 (Half-close)**
```c
shutdown(sockfd, SHUT_WR);
```
- 더 이상 보내지 않겠다고 선언
- FIN 패킷 전송
- send()는 에러 (EPIPE)
- recv()는 여전히 가능!
- **매우 유용한 패턴**

**SHUT_RDWR (2) - 양방향 종료**
```c
shutdown(sockfd, SHUT_RDWR);
```
- close()와 비슷하지만:
  - close(): 디스크립터 해제
  - shutdown(): 연결만 종료, 디스크립터는 유지

#### Half-close가 필요한 이유

**문제: close()만 사용하면?**

```c
// 클라이언트: 요청 전송
send(sock, "GET /file.txt", 13, 0);

// 요청 다 보냈으니 close()?
close(sock);  // ❌ 양방향 모두 종료!

// 이제 recv() 불가능!
// 서버의 응답을 못 받음!
```

**문제 상황:**
```
클라이언트                        서버
   |                               |
   |---- 요청 전송 ---------------->|
   |                               |
   |---- close() ------------------->|  FIN 전송
   |                               |
   |                               | 응답 준비 중...
   |                               |
   |<---- FIN -------------------|  서버도 종료 시작
   ❌ 응답 못 받음!
```

**해결: shutdown(SHUT_WR) 사용**

```
클라이언트                        서버
   |                               |
   |---- 요청 전송 ----------------->|
   |                               |
   |-- shutdown(SHUT_WR) --------->|  FIN 전송
   |    "더 안 보냄"                | recv() = 0 (EOF 감지)
   |                               |
   |                               | 아! 클라이언트 요청 끝났구나
   |                               | 응답 생성...
   |                               |
   |<---- 응답 계속 수신 ------------|  send() 가능
   |<---- 응답 계속 수신 ------------|
   |<---- 응답 계속 수신 ------------|
   |                               |
   |<---- FIN ----------------------|  서버 응답 완료
   ✅ 모든 응답 수신!
```

#### 실제 사용 사례

**1. HTTP/1.0 요청**

HTTP/1.0에서는 Content-Length가 없을 수 있습니다:
```c
// 클라이언트
send(sock, "GET / HTTP/1.0\r\n\r\n", 18, 0);
shutdown(sock, SHUT_WR);  // 요청 끝 신호

// 서버가 보내는 모든 응답 수신
while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
    write(STDOUT_FILENO, buf, n);
}
// recv() = 0 → 서버가 응답 다 보냄

close(sock);
```

**왜 필요한가?**
```
문제: Content-Length 없는 응답

서버:
HTTP/1.0 200 OK\r\n
\r\n
<html>...</html>

클라이언트가 언제까지 읽어야 할까?
→ shutdown(SHUT_WR) 후 recv() = 0 될 때까지!
```

**2. 대용량 파일 업로드**

```c
// 클라이언트: 파일 업로드
FILE *fp = fopen("large_file.dat", "rb");

// 파일 크기를 모르거나 스트리밍
while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
    send_all(sock, buf, n);
}

// 파일 전송 완료 알림
shutdown(sock, SHUT_WR);

// 서버의 확인 메시지 수신
char response[100];
recv(sock, response, sizeof(response), 0);
printf("서버 응답: %s\n", response);

close(sock);
```

**서버 측:**
```c
// 파일 수신
FILE *fp = fopen("received.dat", "wb");

while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
    fwrite(buf, 1, n, fp);
}

// recv() = 0 → 클라이언트가 전송 완료
fclose(fp);

// 확인 메시지 전송
send(sock, "OK: File received", 17, 0);
close(sock);
```

**장점:**
```
서버가 파일 끝을 정확히 알 수 있음
- 클라이언트의 shutdown(SHUT_WR) → 서버 recv() = 0
- 크기를 미리 몰라도 됨
- 스트리밍 가능
```

**3. 데이터베이스 쿼리 결과 스트리밍**

```c
// 클라이언트: 쿼리 전송
send(sock, "SELECT * FROM huge_table", 24, 0);
shutdown(sock, SHUT_WR);  // 쿼리 끝

// 서버가 보내는 모든 결과 수신
while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
    process_results(buf, n);
}
// recv() = 0 → 모든 결과 수신 완료
```

**서버:**
```c
char query[1024];
int n = recv(sock, query, sizeof(query), 0);

if (n == 0) {
    // 클라이언트가 쿼리 전송 완료 (FIN 받음)

    // 쿼리 실행
    while (has_more_rows()) {
        get_next_row(buf);
        send(sock, buf, row_size, 0);
    }

    // 결과 전송 완료
    close(sock);
}
```

**4. Unix 명령어 파이프라인 에뮬레이션**

```c
// 명령어 전송 후 출력 수신
send(sock, "ls -la\n", 7, 0);
shutdown(sock, SHUT_WR);

// 명령어 출력 전체 수신
while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
    write(STDOUT_FILENO, buf, n);
}
```

#### Half-close 없이는?

**대안 1: Content-Length 사용**
```
문제: 미리 크기를 알아야 함
- 파일 크기를 먼저 계산
- 동적 데이터는 어려움
- 스트리밍 불가
```

**대안 2: 타임아웃**
```c
// ❌ 나쁜 방법
while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
    process(buf, n);
}

// 언제까지 기다려야 할까?
// 서버가 느리면? 네트워크 지연이면?
// → 타임아웃으로는 정확한 종료 감지 불가
```

**대안 3: 특수 종료 마커**
```c
// 데이터에 EOF 마커 포함
send(sock, "data...", len, 0);
send(sock, "<<<EOF>>>", 9, 0);

// 받는 쪽: EOF 마커까지 읽기
// 문제: 데이터에 EOF 마커가 포함되면?
// 이스케이프 필요 → 복잡도 증가
```

**Half-close의 장점:**
```
✅ TCP 프로토콜 레벨 지원
✅ 크기를 미리 몰라도 됨
✅ 타임아웃 불필요
✅ 특수 마커 불필요
✅ 명확한 종료 신호
```

**Half-close 패턴:**

**시나리오:**
1. 클라이언트: 요청 전송 완료
2. 클라이언트: shutdown(SHUT_WR) - "더 안 보냄"
3. 서버: FIN 받음 → recv() = 0 → 클라이언트 요청 끝났다는 것을 알게 됨
4. 서버: 응답 계속 전송 가능
5. 클라이언트: 응답 계속 수신 가능

**예제: HTTP 요청**
```c
// 클라이언트
send(sock, "GET / HTTP/1.1\r\n\r\n", 18, 0);

// 더 이상 보낼 것 없음
shutdown(sock, SHUT_WR);

// 서버의 응답은 계속 받을 수 있음
while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
    process(buf, n);
}
// recv() = 0 → 서버 응답 완료

close(sock);
```

**예제: 파일 전송**
```c
// 파일 내용 전송
while ((n = read(file_fd, buf, sizeof(buf))) > 0) {
    send_all(sock, buf, n);
}

// 파일 전송 완료를 알림
shutdown(sock, SHUT_WR);

// 서버의 확인 메시지 수신
recv(sock, ack, sizeof(ack), 0);

close(sock);
```

**실제 프로토콜 사용:**
- **HTTP/1.0**: half-close로 응답 끝 감지
- **FTP 데이터 연결**: 파일 전송 완료 신호
- **SMTP**: 메일 본문 전송 완료

**4-way handshake (정상 종료):**
```
클라이언트                  서버
ESTABLISHED              ESTABLISHED
    ↓
close() 또는 shutdown(WR)
FIN_WAIT_1  ──[FIN]──→  CLOSE_WAIT
            (seq=x)
FIN_WAIT_2  ←─[ACK]──   
            (ack=x+1)
    ↓                       ↓
  (대기)                 close()
TIME_WAIT  ←─[FIN]──    LAST_ACK
            (seq=y)
CLOSED     ─→[ACK]──→   CLOSED
            (ack=y+1)
    ↓
 (2MSL 대기)
    ↓
  CLOSED
```

---

## 핵심 요약

**TCP 소켓 생명주기:**
```
socket() → bind() → listen() → accept() → send/recv() → close()
         (서버만)   (서버만)    (서버만)
         
socket() → connect() → send/recv() → close()
         (클라이언트)
```

**핵심 개념:**
1. **연결 지향**: 3-way handshake로 연결 설정
2. **신뢰성**: 손실 없음, 순서 보장
3. **바이트 스트림**: 메시지 경계 없음
4. **새 소켓**: accept()는 새 소켓 생성
5. **부분 송수신**: 항상 반환값 확인 필요

---

다음: [Part 4: UDP 소켓의 동작 원리 →](04_udp_basics.md)
