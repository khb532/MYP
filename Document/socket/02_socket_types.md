# Part 2: 소켓의 종류

소켓을 분류하는 방법은 크게 두 가지입니다:
1. **도메인(Domain)**: 어떤 주소 체계를 사용할 것인가?
2. **타입(Type)**: 어떤 방식으로 데이터를 전송할 것인가?

이 두 가지를 조합하여 원하는 종류의 소켓을 만들 수 있습니다.

---

## 2.1 도메인(Domain)별 분류

### 도메인이란?

소켓 도메인(Socket Domain)은 소켓이 어떤 **주소 체계(Address Family)**를 사용하여 통신할 것인지를 정의합니다.

**핵심 질문:** "통신 상대를 어떻게 식별하고 찾을 것인가?"

**왜 도메인으로 분류하는가?**

통신 환경에 따라 주소를 표현하는 방식이 다르기 때문입니다:
- 인터넷 통신: IP 주소 + 포트 번호 필요
- 같은 컴퓨터 내 통신: 파일 경로만으로 충분
- 다른 네트워크 (예: Bluetooth): 고유한 주소 체계 필요

**비유:**
- 국제 전화: 국가 코드 + 지역 번호 + 전화번호
- 내선 전화: 내선 번호만
- 소켓 도메인: 어떤 "전화 체계"를 사용할지 결정

### 2.1.1 AF_INET (IPv4)

**무엇인가?**

인터넷 프로토콜 버전 4를 사용하는 소켓입니다. 현재 인터넷에서 가장 널리 사용되는 주소 체계입니다.

**주소 구성:**
- **IP 주소**: 32비트 (예: 192.168.0.1)
  - 4개의 숫자, 각각 0-255
  - 약 43억 개의 주소 가능
- **포트 번호**: 16비트 (0~65535)
  - 한 컴퓨터에서 여러 서비스 구분

**구조체:**
```c
struct sockaddr_in 
{
    sa_family_t    sin_family;  // AF_INET (항상 이 값)
    in_port_t      sin_port;    // 포트 번호 (16비트)
    struct in_addr sin_addr;    // IPv4 주소 (32비트)
    char           sin_zero[8]; // 패딩 (크기 맞추기용)
};
```

**왜 패딩이 필요한가?**
- 모든 소켓 주소 구조체를 `struct sockaddr`로 캐스팅 가능하게 하기 위함
- 크기를 통일해야 일관된 인터페이스 제공 가능

**사용 예:**
```c
struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(8080);
addr.sin_addr.s_addr = INADDR_ANY;  // 모든 인터페이스
```

**실제 사용 사례:**
- 웹 브라우저 → 웹 서버
- 이메일 클라이언트 → 메일 서버
- 게임 클라이언트 → 게임 서버
- 대부분의 인터넷 통신

**제한:**
- IPv4 주소 고갈 (43억 개로 부족)
- NAT로 임시 해결 중
- IPv6로 전환 중

**NAT(Network Address Translation)란?**

IPv4 주소 부족 문제를 해결하기 위한 임시 방편입니다.

**개념:**
```
집 안 (사설 IP)              인터넷 (공인 IP)
192.168.0.2 ─┐
192.168.0.3 ─┤
192.168.0.4 ─┤─→ [NAT 공유기] ─→ 1.2.3.4 (공인 IP 1개)
192.168.0.5 ─┤
192.168.0.6 ─┘

여러 기기가 하나의 공인 IP를 공유
```

**동작 원리:**
```
내부 → 외부:
  출발지: 192.168.0.2:5000
  NAT 변환: 1.2.3.4:50000 (포트 번호로 구분)

외부 → 내부:
  목적지: 1.2.3.4:50000
  NAT 역변환: 192.168.0.2:5000
```

**장점:**
- ✅ 공인 IP 1개로 여러 기기 사용
- ✅ 내부 네트워크 보안 (외부에서 직접 접근 불가)

**단점:**
- ❌ P2P 통신 어려움
- ❌ 서버 운영 복잡 (포트 포워딩 필요)
- ❌ 일부 프로토콜 호환성 문제

**사설 IP 범위:**
- 10.0.0.0 ~ 10.255.255.255
- 172.16.0.0 ~ 172.31.255.255
- 192.168.0.0 ~ 192.168.255.255

### 2.1.2 AF_INET6 (IPv6)

**무엇인가?**

인터넷 프로토콜 버전 6을 사용하는 소켓입니다. IPv4 주소 고갈 문제를 해결하기 위해 만들어졌습니다.

**주소 구성:**
- **IP 주소**: 128비트 (예: 2001:0db8:85a3::8a2e:0370:7334)
  - 16진수 8그룹, 콜론으로 구분
  - 약 340언데실리언(3.4×10^38) 개의 주소
- **포트 번호**: 16비트 (동일)
- **추가 정보**: 플로우 레이블, 스코프 ID

**IPv4와의 주요 차이:**

| 특성 | IPv4 | IPv6 |
|------|------|------|
| 주소 크기 | 32비트 | 128비트 |
| 주소 개수 | 43억 | 사실상 무제한 |
| 표기법 | 점(.) | 콜론(:) |
| 헤더 | 가변 길이 | 고정 길이 (단순) |
| 자동 설정 | DHCP 필요 | 자동 설정 내장 |
| 보안 | 선택사항 | IPsec 내장 |

**구조체:**
```c
struct sockaddr_in6 {
    sa_family_t     sin6_family;   // AF_INET6
    in_port_t       sin6_port;     // 포트 번호
    uint32_t        sin6_flowinfo; // 플로우 정보 (QoS 용)
    struct in6_addr sin6_addr;     // IPv6 주소 (128비트)
    uint32_t        sin6_scope_id; // 인터페이스 식별자
};
```

**주소 표기 예:**
```
완전한 형식: 2001:0db8:0000:0000:0000:0000:0000:0001
압축 형식:   2001:db8::1
링크 로컬:   fe80::1
루프백:      ::1
```

**사용 예:**
```c
struct sockaddr_in6 addr;
memset(&addr, 0, sizeof(addr));
addr.sin6_family = AF_INET6;
addr.sin6_port = htons(8080);
inet_pton(AF_INET6, "2001:db8::1", &addr.sin6_addr);
```

**언제 사용하는가?**
- 새로운 서비스 개발 시
- IPv4 주소가 부족한 환경
- IPv6 전용 네트워크
- 듀얼 스택 지원 (IPv4 + IPv6 동시)

**현실:**
- 아직 완전히 전환되지 않음
- 많은 시스템이 듀얼 스택 운영
- 점진적으로 IPv6 비중 증가 중

### 2.1.3 AF_UNIX (Unix Domain Socket)

**무엇인가?**

같은 컴퓨터 내에서 프로세스 간 통신(IPC)을 위한 소켓입니다. **네트워크를 전혀 사용하지 않습니다.**

**왜 필요한가?**

같은 컴퓨터 내 통신인데 왜 굳이 TCP/IP를 사용할까요?

**문제점:**
```
프로세스 A ─→ TCP/IP 스택 ─→ 네트워크 카드 ─→ 루프백 ─→ 
─→ 네트워크 카드 ─→ TCP/IP 스택 ─→ 프로세스 B

불필요한 오버헤드!
```

**Unix Domain Socket의 해결:**
```
프로세스 A ─→ 커널 메모리 복사 ─→ 프로세스 B

네트워크 스택 우회, 매우 빠름!
```

**성능 비교:**
- Unix Domain Socket: 약 2배 빠름
- 체크섬, 라우팅 등 불필요한 처리 생략
- 단순 메모리 복사만

**주소 체계:**

파일 시스템의 경로를 주소로 사용합니다.

```c
struct sockaddr_un 
{
    sa_family_t sun_family;  // AF_UNIX
    char        sun_path[108];  // 경로명
};
```

**예:**
```c
struct sockaddr_un addr;
memset(&addr, 0, sizeof(addr));
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, "/tmp/my_socket", sizeof(addr.sun_path)-1);
```

**파일 시스템에 나타남:**
```bash
$ ls -l /tmp/my_socket
srwxr-xr-x 1 user user 0 Nov 19 10:00 /tmp/my_socket
```
- `s`: 소켓 파일
- 파일 권한으로 접근 제어 가능

**특징:**

1. **빠름** - TCP보다 약 2배
2. **안전** - 파일 권한으로 접근 제어
3. **로컬 전용** - 네트워크 방화벽 영향 없음
4. **파일 디스크립터 전달 가능** - SCM_RIGHTS

**파일 디스크립터 전달이란?**

Unix Domain Socket의 특별한 기능으로, 프로세스 간에 **열린 파일이나 소켓**을 공유할 수 있습니다.

**왜 필요한가?**

일반적으로 파일 디스크립터는 프로세스 고유의 자원입니다:
```
프로세스 A의 fd=5 ≠ 프로세스 B의 fd=5
→ 같은 번호여도 완전히 다른 파일!
```

하지만 Unix Domain Socket을 통해 실제 파일/소켓 자체를 전달할 수 있습니다.

**사용 예시:**

**시나리오: 웹 서버 아키텍처**
```
마스터 프로세스 (root 권한)
  ↓ (80번 포트 바인딩, 클라이언트 accept)
  ↓
  ↓ [클라이언트 소켓 fd를 전달] ← SCM_RIGHTS
  ↓
워커 프로세스 (일반 사용자 권한)
  → 클라이언트와 직접 통신
```

**장점:**
- 마스터만 root 권한 필요
- 워커는 일반 권한으로 안전하게 실행
- 워커가 크래시해도 마스터는 안전

**코드 예시 (개념):**
```c
// 마스터 프로세스
int client_fd = accept(server_fd, ...);  // 클라이언트 연결

// Unix Domain Socket으로 워커에게 client_fd 전달
struct msghdr msg;
struct cmsghdr *cmsg;
// ... SCM_RIGHTS 설정 ...
sendmsg(unix_sock, &msg, 0);  // fd 전달!

// 워커 프로세스
recvmsg(unix_sock, &msg, 0);  // fd 수신!
// 이제 client_fd를 사용할 수 있음
read(client_fd, buf, size);
```

**실제 사용 사례 (fd 전달):**
- Nginx: 마스터-워커 아키텍처
- systemd: 소켓 활성화(socket activation)
- Chrome: 렌더러 프로세스 샌드박싱

**실제 사용 사례 (일반 IPC):**
- Docker 데몬 ↔ Docker CLI
- X11 서버 ↔ X 클라이언트
- MySQL 로컬 연결 (TCP보다 빠름)
- systemd ↔ 서비스들
- 웹 서버 ↔ FastCGI/WSGI 백엔드

**추상 네임스페이스 (Linux):**
```c
// sun_path[0] = '\0'으로 시작
addr.sun_path[0] = '\0';
strncpy(addr.sun_path + 1, "my_socket", sizeof(addr.sun_path)-2);
```

**장점:**
- 파일 시스템에 파일 생성 안 됨
- unlink() 불필요
- 자동 정리

### 도메인 선택 가이드

| 상황 | 권장 도메인 | 이유 |
|------|------------|------|
| 인터넷 통신 | AF_INET | 가장 널리 사용 |
| 새 서비스 개발 | AF_INET6 | 미래 대비 |
| 듀얼 스택 | 둘 다 | 호환성 최대화 |
| 로컬 IPC | AF_UNIX | 성능, 보안 |

---

## 2.2 타입(Type)별 분류

### 타입이란?

소켓 타입은 데이터를 **어떤 방식으로 전송할 것인지**를 정의합니다.

**결정 요소:**
- 신뢰성이 필요한가?
- 순서가 중요한가?
- 메시지 경계가 필요한가?
- 연결이 필요한가?
- 속도가 중요한가?

### 2.2.1 SOCK_STREAM (스트림 소켓)

**무엇인가?**

신뢰성 있는 양방향 연결 기반의 바이트 스트림 통신을 제공하는 소켓입니다.

**"스트림"의 의미:**

데이터가 경계 없이 흐르는 강물처럼 전송됩니다.

```
송신: send("Hel"), send("lo")
수신: recv() → "Hello" (한 번에)
또는: recv() → "He", recv() → "llo" (나뉘어서)

메시지 경계가 없음!
```

**핵심 특징:**

**1. 연결 지향적(Connection-oriented)**
- 데이터 전송 전에 연결 설정 필요
- 3-way handshake 수행
- 전화 통화와 비슷: 먼저 전화 걸고 연결한 후 대화

**2. 신뢰성 있는 전송(Reliable)**
- 데이터 손실 없음
  - 손실 감지 시 자동 재전송
- 중복 제거
  - 같은 패킷이 두 번 오면 하나만 전달
- 오류 검출
  - 체크섬으로 데이터 무결성 검증

**3. 순서 보장(In-order delivery)**
```
송신: [1] [2] [3] [4]
네트워크에서 순서 뒤바뀜: [3] [1] [4] [2]
수신: [1] [2] [3] [4] (재정렬됨)
```

**4. 흐름 제어(Flow control)**
```
수신자: "버퍼 가득참, 천천히 보내!"
송신자: 속도 조절
```

**5. 혼잡 제어(Congestion control)**
```
네트워크: "혼잡함"
TCP: 전송 속도 자동 감소
```

**사용 프로토콜: TCP**

Transmission Control Protocol

**생성:**
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
// 또는
int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
```

**사용 사례:**

**데이터 정확성이 중요:**
- 파일 전송 (FTP)
- 웹 페이지 (HTTP/HTTPS)
- 이메일 (SMTP, POP3, IMAP)
- 데이터베이스 통신
- SSH, Telnet

**특징:**
- 약간의 지연 허용
- 손실은 절대 허용 안 됨
- 순서가 중요

**장단점:**

**장점:**
- ✅ 신뢰성: 데이터 손실 없음
- ✅ 순서 보장
- ✅ 흐름/혼잡 제어
- ✅ 프로그래밍 단순

**단점:**
- ❌ 오버헤드: 연결 설정, 재전송 등
- ❌ 지연: 재전송으로 인한 지연 가능
- ❌ 헤드 오브 라인 블로킹 (Head-of-Line Blocking)

**헤드 오브 라인 블로킹이란?**

앞쪽 패킷이 지연되면 뒤쪽 패킷들도 모두 대기해야 하는 현상입니다.

```
송신: [패킷1] [패킷2] [패킷3] [패킷4]
네트워크: [패킷1 손실!] [패킷2 도착] [패킷3 도착] [패킷4 도착]

TCP 동작:
- 패킷2, 3, 4는 이미 도착했지만
- 패킷1을 기다리며 버퍼에서 대기
- 패킷1이 재전송되어 도착할 때까지 애플리케이션에 전달 안 됨
→ 뒤쪽 데이터가 앞쪽 때문에 "블로킹"됨
```

**실제 영향:**
```
웹 페이지 로딩 시:
이미지1 [손실] → 이미지2, 3, 4 모두 대기
→ 페이지 전체가 멈춘 것처럼 보임
```

**해결:**
- HTTP/2: 멀티플렉싱으로 완화
- HTTP/3 (QUIC): UDP 기반으로 문제 해결

**비유:**

등기우편
- 반드시 도착
- 순서대로 배달
- 느리지만 확실

### 2.2.2 SOCK_DGRAM (데이터그램 소켓)

**무엇인가?**

비연결형 메시지 기반 통신을 제공하는 소켓입니다.

**"데이터그램"의 의미:**

하나의 독립적인 메시지 단위입니다.

```
송신: sendto("Hello"), sendto("World")
수신: recvfrom() → "Hello" (정확히 5바이트)
     recvfrom() → "World" (정확히 5바이트)

메시지 경계 보존!
```

**핵심 특징:**

**1. 비연결형(Connectionless)**
```
TCP: 전화 - 먼저 연결, 통화, 끊기
UDP: 엽서 - 주소 쓰고 바로 발송
```

**2. 메시지 단위 전송**
- 각 메시지가 독립적
- 경계 보존
- 쪼개지거나 합쳐지지 않음

**3. 순서 보장 없음**
```
송신: [1] [2] [3]
수신: [2] [3] [1] 가능
```

**4. 신뢰성 없음**
```
송신: [1] [2] [3] [4] [5]
수신: [1] [3] [5] (2, 4 손실)
```

**5. 오버헤드 낮음**
- 연결 설정/해제 없음
- 재전송 없음
- 헤더 작음 (TCP 20바이트 vs UDP 8바이트)

**사용 프로토콜: UDP**

User Datagram Protocol

**생성:**
```c
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// 또는
int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

**사용 사례:**

**속도가 중요, 약간의 손실 허용:**
- DNS 조회 (간단한 요청/응답)
- 실시간 영상/음성 (VoIP, 화상회의)
- 온라인 게임 (플레이어 위치 업데이트)
- IoT 센서 데이터
- 스트리밍 (YouTube, Netflix)

**왜 손실을 허용?**

**영상 스트리밍 예:**
```
프레임 1, 2, 3, 4, 5, 6, ...
프레임 3 손실?
→ 사람이 거의 인지 못함
→ 재전송하면 이미 지나간 시간 (무의미)
→ 계속 진행하는 게 나음
```

**게임 예:**
```
플레이어 위치: (10, 20) 손실
다음 위치: (11, 21) 도착
→ 최신 정보가 중요, 과거는 무의미
```

**장단점:**

**장점:**
- ✅ 빠름: 오버헤드 최소
- ✅ 단순: 연결 관리 불필요
- ✅ 브로드캐스트/멀티캐스트 지원
- ✅ 실시간 적합

**단점:**
- ❌ 신뢰성 없음
- ❌ 순서 보장 없음
- ❌ 흐름 제어 없음
- ❌ 혼잡 제어 없음

**비유:**

일반 엽서
- 빠르고 간단
- 분실 가능
- 순서 뒤바뀔 수 있음
- 중복 배달 가능

### 2.2.3 SOCK_RAW (Raw 소켓)

**무엇인가?**

운영체제의 TCP/IP 스택을 우회하여 IP 계층 또는 그 이하에 직접 접근할 수 있는 소켓입니다.

**"Raw"의 의미:**

가공되지 않은 날것의 데이터를 다룬다는 뜻입니다.

```
일반 소켓: OS가 헤더 자동 처리
Raw 소켓: 프로그래머가 직접 헤더 구성
```

**핵심 특징:**

**1. IP 헤더 직접 접근**
```c
struct iphdr 
{
    uint8_t  version:4, ihl:4;  // 버전, 헤더 길이
    uint8_t  tos;               // Type of Service
    uint16_t tot_len;           // 전체 길이
    uint16_t id;                // 식별자
    uint16_t frag_off;          // 플래그, Fragment
    uint8_t  ttl;               // Time To Live
    uint8_t  protocol;          // 프로토콜
    uint16_t check;             // 체크섬
    uint32_t saddr;             // 출발지 IP
    uint32_t daddr;             // 목적지 IP
};
```

**2. 저수준 프로토콜 구현 가능**
- ICMP (ping)
- IGMP (멀티캐스트 그룹 관리)
- 커스텀 프로토콜

**3. 관리자 권한 필요**
```bash
$ ./my_raw_socket_program
Permission denied

$ sudo ./my_raw_socket_program
Success!
```

**왜?** 보안상의 이유
- IP 스푸핑 가능 (출발지 주소 위조)
- DoS 공격 가능
- 패킷 스니핑 가능

**생성:**
```c
// ICMP 소켓
int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

// 모든 IP 패킷
int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
```

**사용 사례:**

**네트워크 도구:**
- **ping**: ICMP Echo Request/Reply
- **traceroute**: TTL 조작하여 경로 추적
- **Wireshark**: 모든 패킷 캡처
- **nmap**: 다양한 패킷 조작으로 포트 스캔

**보안 도구:**
- 침입 탐지 시스템 (IDS)
- 방화벽
- 패킷 필터

**연구/교육:**
- 새로운 프로토콜 실험
- 네트워크 프로토콜 학습

**위험성:**

Raw 소켓으로 할 수 있는 악의적 행위:
- ❌ IP 스푸핑
- ❌ DDoS 공격
- ❌ 패킷 스니핑 (도청)
- ❌ ARP 스푸핑

따라서 일반 사용자에게는 권한을 주지 않습니다.

**비유:**

자동차의 엔진룸
- 일반 운전: 핸들, 페달만 사용 (일반 소켓)
- 정비사: 엔진을 직접 만짐 (Raw 소켓)
- 강력하지만 위험함

### 2.2.4 SOCK_SEQPACKET

**무엇인가?**

SOCK_STREAM과 SOCK_DGRAM의 장점을 결합한 소켓입니다.

**특징:**
- ✅ 연결 지향 (SOCK_STREAM처럼)
- ✅ 신뢰성 (SOCK_STREAM처럼)
- ✅ 순서 보장 (SOCK_STREAM처럼)
- ✅ 메시지 경계 보존 (SOCK_DGRAM처럼)

**TCP vs SOCK_SEQPACKET 비교:**

```
TCP (SOCK_STREAM):
  송신: send("AB"), send("CD")
  수신: recv() → "ABCD" (합쳐질 수 있음)

SOCK_SEQPACKET:
  송신: send("AB"), send("CD")
  수신: recv() → "AB" (첫 번째)
       recv() → "CD" (두 번째)
  메시지 경계가 보존됨!
```

**프로토콜: SCTP**

Stream Control Transmission Protocol

**SCTP란?**

TCP의 대안으로 설계된 전송 계층 프로토콜입니다. TCP의 단점을 보완하면서 추가 기능을 제공합니다.

**SCTP의 주요 특징:**

**1. 멀티스트리밍 (Multi-streaming)**
```
TCP:
  [데이터1 손실!] → [데이터2, 3, 4 모두 대기] (헤드 오브 라인 블로킹)

SCTP:
  스트림1: [데이터1 손실!] → [재전송 대기]
  스트림2: [데이터2] → 정상 전달 (독립적!)
  스트림3: [데이터3] → 정상 전달 (독립적!)
```

**2. 멀티호밍 (Multi-homing)**
```
하나의 연결이 여러 네트워크 경로를 동시에 사용:

서버 (IP: A, B)  ←→  클라이언트 (IP: C, D)
   A ←→ C  (주 경로)
   A ←→ D  (예비)
   B ←→ C  (예비)
   B ←→ D  (예비)

경로 A-C 장애 시 자동으로 A-D로 전환
→ 연결 끊김 없음!
```

**3. 메시지 지향**
- TCP처럼 신뢰성 있지만
- UDP처럼 메시지 경계 보존

**4. 향상된 보안**
- 4-way handshake (SYN flooding 공격 방어)
- 쿠키 메커니즘

**왜 잘 안 쓰이는가?**

**기술적 이유:**
- TCP가 이미 널리 사용됨
- 방화벽/NAT가 SCTP를 모름 (차단되는 경우 많음)
- 애플리케이션 레벨에서 메시지 경계 구현 가능

**생태계 이유:**
- 운영체제 지원 제한적
- 라이브러리/프레임워크 지원 부족
- 학습 자료 부족

**사용 사례:**

현재 주로 특수 목적으로만 사용됩니다:
- **VoIP**: 음성 통화 (멀티스트리밍 활용)
- **통신사 인프라**: SS7 over IP (Signaling System 7)
- **WebRTC 데이터 채널**: 브라우저 간 P2P 통신

**Unix Domain Socket에서는?**

Unix Domain Socket에서 SOCK_SEQPACKET을 사용할 수 있습니다:
```c
socket(AF_UNIX, SOCK_SEQPACKET, 0);
```

이 경우:
- SCTP가 아닌 커널 내부 구현 사용
- 메시지 경계 보존 + 신뢰성
- 로컬 IPC에서는 실용적으로 사용 가능

---

## 2.3 도메인과 타입의 조합

### 조합 가능한 경우

```c
// IPv4 + TCP
socket(AF_INET, SOCK_STREAM, 0);

// IPv4 + UDP
socket(AF_INET, SOCK_DGRAM, 0);

// IPv6 + TCP
socket(AF_INET6, SOCK_STREAM, 0);

// IPv6 + UDP
socket(AF_INET6, SOCK_DGRAM, 0);

// Unix Domain + Stream
socket(AF_UNIX, SOCK_STREAM, 0);

// Unix Domain + Datagram
socket(AF_UNIX, SOCK_DGRAM, 0);

// Raw ICMP
socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```

### 세 번째 매개변수 (프로토콜)

보통 0으로 지정하면 자동 선택됩니다:
- SOCK_STREAM + 0 → TCP
- SOCK_DGRAM + 0 → UDP

명시적 지정:
```c
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
```

---

## 2.4 선택 가이드

### 플로우차트

```
네트워크 통신인가?
├─ Yes: 인터넷인가?
│   ├─ Yes: IPv4/IPv6
│   └─ No: 특수 네트워크
└─ No: 같은 컴퓨터? → AF_UNIX

신뢰성이 필요한가?
├─ Yes: SOCK_STREAM (TCP)
└─ No: 속도가 중요? → SOCK_DGRAM (UDP)

특수한 제어 필요?
└─ Yes: SOCK_RAW (관리자 권한 필요)
```

### 상황별 권장

| 상황 | 권장 조합 | 이유 |
|------|----------|------|
| 웹 서버 | AF_INET + SOCK_STREAM | HTTP는 신뢰성 필요 |
| 실시간 게임 | AF_INET + SOCK_DGRAM | 속도 > 신뢰성 |
| 로컬 IPC | AF_UNIX + SOCK_STREAM | 빠르고 안전 |
| DNS 서버 | AF_INET + SOCK_DGRAM | 간단한 요청/응답 |
| 파일 전송 | AF_INET + SOCK_STREAM | 데이터 무결성 |
| 비디오 스트리밍 | AF_INET + SOCK_DGRAM | 지연 최소화 |
| ping 구현 | AF_INET + SOCK_RAW | ICMP 접근 |

---

## 핵심 요약

### 도메인 (주소 체계)
- **AF_INET**: IPv4, 인터넷 표준
- **AF_INET6**: IPv6, 미래 표준
- **AF_UNIX**: 로컬 IPC, 가장 빠름

### 타입 (전송 방식)
- **SOCK_STREAM**: TCP, 신뢰성, 순서 보장
- **SOCK_DGRAM**: UDP, 빠름, 메시지 단위
- **SOCK_RAW**: 직접 제어, 관리자 권한

### 선택 기준
1. 어디와 통신? → 도메인 결정
2. 어떻게 통신? → 타입 결정
3. 신뢰성 vs 속도 트레이드오프 고려

