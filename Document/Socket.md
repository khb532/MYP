소켓(Socket) 완전 가이드
1. 소켓의 정의와 역사
1.1 정의
**소켓(Socket)**은 네트워크 상에서 프로세스 간 통신(IPC, Inter-Process Communication)을 가능하게 하는 통신 종단점(endpoint)이다. 소켓은 응용 프로그램과 전송 계층(Transport Layer) 사이의 인터페이스 역할을 하며, BSD(Berkeley Software Distribution) 소켓 API로 처음 표준화되었다.
1.2 역사적 배경

1983년: 4.2BSD Unix에서 처음 도입
개발자: Bill Joy와 UC Berkeley 연구팀
목적: TCP/IP 프로토콜을 사용하기 위한 표준 API 제공
현재: POSIX 표준의 일부로 모든 주요 운영체제에서 지원

1.3 소켓의 필요성

네트워크 하드웨어의 복잡성 은닉
다양한 프로토콜에 대한 통일된 인터페이스 제공
운영체제 수준의 자원 관리 및 보안 제공
이식성(portability) 향상


2. 소켓의 종류
2.1 도메인(Domain)별 분류
2.1.1 AF_INET (IPv4)
```c
struct sockaddr_in {
    sa_family_t    sin_family;  // AF_INET
    in_port_t      sin_port;    // 포트 번호
    struct in_addr sin_addr;    // IPv4 주소
    char           sin_zero[8]; // 패딩
};
```
2.1.2 AF_INET6 (IPv6)
```c
struct sockaddr_in6 {
    sa_family_t     sin6_family;   // AF_INET6
    in_port_t       sin6_port;     // 포트 번호
    uint32_t        sin6_flowinfo; // 플로우 정보
    struct in6_addr sin6_addr;     // IPv6 주소
    uint32_t        sin6_scope_id; // 스코프 ID
};
```
2.1.3 AF_UNIX (Unix Domain Socket)

같은 호스트 내 프로세스 간 통신
네트워크 스택을 거치지 않아 매우 빠름
파일 시스템 경로를 주소로 사용

```c
struct sockaddr_un {
    sa_family_t sun_family;  // AF_UNIX
    char        sun_path[];  // 경로명
};
```

### 2.2 타입(Type)별 분류

#### 2.2.1 SOCK_STREAM (스트림 소켓)
**특징:**
- 연결 지향적(Connection-oriented)
- 신뢰성 있는 양방향 바이트 스트림
- 순서 보장(in-order delivery)
- 오류 검출 및 재전송
- 흐름 제어(flow control)
- 프로토콜: TCP

**사용 사례:**
- HTTP, HTTPS
- FTP, SSH
- SMTP, POP3
- 데이터 무결성이 중요한 모든 응용

#### 2.2.2 SOCK_DGRAM (데이터그램 소켓)
**특징:**
- 비연결형(Connectionless)
- 메시지 단위 전송
- 순서 보장 없음
- 신뢰성 없음 (패킷 손실 가능)
- 오버헤드 낮음
- 프로토콜: UDP

**사용 사례:**
- DNS 쿼리
- 실시간 스트리밍 (비디오, 음성)
- 온라인 게임
- IoT 센서 데이터

#### 2.2.3 SOCK_RAW (Raw 소켓)
**특징:**
- IP 헤더에 직접 접근
- ICMP, IGMP 등 저수준 프로토콜 구현 가능
- 관리자 권한 필요
- 패킷 스니핑, 네트워크 진단 도구 개발

**사용 사례:**
- ping 유틸리티
- traceroute
- 네트워크 모니터링 도구
- 커스텀 프로토콜 구현

#### 2.2.4 SOCK_SEQPACKET
- 순서가 보장되는 메시지 기반 연결
- SCTP(Stream Control Transmission Protocol)에서 사용

---

## 3. TCP 소켓의 동작 원리

### 3.1 서버-클라이언트 모델
```
서버                           클라이언트
socket()                       socket()
  ↓                              ↓
bind()                         
  ↓
listen()
  ↓
accept() ←─────────────────── connect()
  ↓         [3-way handshake]    ↓
  ↓                              ↓
read() ←───────────────────── write()
  ↓                              ↓
write() ─────────────────────→ read()
  ↓                              ↓
close() ←─────────────────────→ close()
        [4-way handshake]
3.2 주요 시스템 콜 상세 설명
3.2.1 socket()
cint socket(int domain, int type, int protocol);
매개변수:

domain: 주소 체계 (AF_INET, AF_INET6, AF_UNIX)
type: 소켓 타입 (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW)
protocol: 프로토콜 (보통 0으로 자동 선택)

반환값:

성공: 소켓 디스크립터 (양수)
실패: -1 (errno 설정됨)

내부 동작:

커널에 소켓 데이터 구조 생성
파일 디스크립터 테이블에 항목 추가
초기 상태는 CLOSED

3.2.2 bind()
cint bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
목적:

소켓에 로컬 주소(IP + 포트) 할당
서버에서 필수, 클라이언트는 선택적

Well-known Ports:

0-1023: 시스템 포트 (root 권한 필요)
1024-49151: 등록된 포트
49152-65535: 동적/임시 포트

특수 주소:

INADDR_ANY (0.0.0.0): 모든 네트워크 인터페이스
INADDR_LOOPBACK (127.0.0.1): 루프백
특정 IP: 특정 인터페이스만 바인딩

3.2.3 listen()
cint listen(int sockfd, int backlog);
매개변수:

backlog: 대기 큐의 최대 크기

완전히 연결된 소켓들의 큐
SYN_RCVD 상태 소켓들의 큐



상태 전환:

CLOSED → LISTEN

큐 관리:

클라이언트 연결 요청이 들어오면 큐에 저장
accept()가 호출될 때까지 대기
큐가 가득 차면 새 연결 거부

3.2.4 accept()
cint accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
동작:

연결 큐에서 하나의 연결 요청 꺼냄
새로운 소켓 디스크립터 생성 (connected socket)
클라이언트 주소 정보 반환
원래 소켓(listening socket)은 계속 listen 상태 유지

블로킹 vs 논블로킹:

블로킹 모드: 연결 요청이 올 때까지 대기
논블로킹 모드: 즉시 반환 (EWOULDBLOCK)

3.2.5 connect()
cint connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
동작:

TCP 3-way handshake 시작

SYN 전송
SYN-ACK 수신 대기
ACK 전송


연결 성공 시 ESTABLISHED 상태로 전환
로컬 포트가 바인딩되지 않았으면 자동 할당

타임아웃:

기본적으로 약 75초 (시스템 의존적)
SO_SNDTIMEO 옵션으로 조정 가능

3.2.6 send() / recv()
cssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
flags 옵션:

MSG_DONTWAIT: 논블로킹 모드
MSG_PEEK: 데이터를 읽되 버퍼에서 제거하지 않음
MSG_WAITALL: 요청한 바이트 수만큼 다 받을 때까지 대기
MSG_OOB: Out-of-band 데이터 (긴급 데이터)

부분 전송/수신:

TCP는 스트림이므로 요청한 크기만큼 전송/수신 보장 안 됨
반환값 확인하여 루프 처리 필요

c// 완전 전송 예제
ssize_t send_all(int sockfd, const void *buf, size_t len) {
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t sent = send(sockfd, buf + total_sent, 
                           len - total_sent, 0);
        if (sent < 0) return -1;
        total_sent += sent;
    }
    return total_sent;
}
3.2.7 close() / shutdown()
cint close(int sockfd);
int shutdown(int sockfd, int how);
close():

소켓 디스크립터 참조 카운트 감소
0이 되면 연결 종료 (4-way handshake)
양방향 모두 종료

shutdown():

SHUT_RD (0): 수신 종료
SHUT_WR (1): 송신 종료 (half-close)
SHUT_RDWR (2): 양방향 종료

Half-close 패턴:
cshutdown(sockfd, SHUT_WR);  // 더 이상 보낼 데이터 없음
// 하지만 여전히 받을 수 있음
while (recv(sockfd, buf, sizeof(buf), 0) > 0) {
    // 처리
}
close(sockfd);

4. UDP 소켓의 동작 원리
4.1 특징

연결 설정/해제 과정 없음
connect(), accept(), listen() 불필요
sendto(), recvfrom() 사용

4.2 주요 함수
cssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

### 4.3 서버-클라이언트 구조
```
서버                           클라이언트
socket()                       socket()
  ↓                              ↓
bind()                         bind() (선택)
  ↓                              ↓
recvfrom() ←────────────────── sendto()
  ↓                              ↓
sendto() ──────────────────────→ recvfrom()
  ↓                              ↓
close()                        close()
4.4 Connected UDP 소켓
c// UDP 소켓도 connect() 가능
connect(sockfd, &server_addr, sizeof(server_addr));
// 이후 send()/recv() 사용 가능
send(sockfd, buf, len, 0);
recv(sockfd, buf, len, 0);
장점:

커널이 주소를 기억하여 매번 지정할 필요 없음
ICMP 에러 메시지 수신 가능
성능 향상 (주소 검증 한 번만)


5. 소켓 옵션
5.1 setsockopt() / getsockopt()
cint setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen);
5.2 주요 옵션
5.2.1 SOL_SOCKET 레벨
SO_REUSEADDR
cint optval = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

TIME_WAIT 상태의 주소 재사용 허용
서버 재시작 시 "Address already in use" 에러 방지

SO_REUSEPORT

여러 소켓이 같은 포트에 바인딩 가능
로드 밸런싱 (커널이 자동 분배)

SO_KEEPALIVE

TCP 연결 유지 확인 (keepalive probe)
죽은 연결 감지

SO_RCVBUF / SO_SNDBUF

수신/송신 버퍼 크기 조정

cint bufsize = 65536;
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
SO_RCVTIMEO / SO_SNDTIMEO

수신/송신 타임아웃 설정

cstruct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 0;
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
SO_LINGER
cstruct linger lng;
lng.l_onoff = 1;
lng.l_linger = 10;  // 10초 대기
setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &lng, sizeof(lng));

close() 시 동작 제어
l_linger > 0: 데이터 전송 대기
l_linger = 0: 즉시 RST 전송 (abortive close)

5.2.2 IPPROTO_TCP 레벨
TCP_NODELAY
cint flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

Nagle 알고리즘 비활성화
작은 패킷 즉시 전송 (지연 감소)
실시간 응용에 유용

TCP_CORK (Linux)

패킷 모아서 한 번에 전송
HTTP 헤더+본문을 하나의 패킷으로

TCP_KEEPIDLE / TCP_KEEPINTVL / TCP_KEEPCNT

Keepalive 세부 설정

5.2.3 IPPROTO_IP 레벨
IP_TTL

Time-To-Live 값 설정

IP_TOS

Type of Service (QoS)

IP_MULTICAST_TTL / IP_ADD_MEMBERSHIP

멀티캐스트 관련 옵션


6. 논블로킹 소켓과 I/O 멀티플렉싱
6.1 블로킹 vs 논블로킹
블로킹 소켓:

시스템 콜이 완료될 때까지 프로세스 대기
단순하지만 동시성 처리 어려움

논블로킹 소켓:
cint flags = fcntl(sockfd, F_GETFL, 0);
fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

즉시 반환 (EWOULDBLOCK / EAGAIN)
주기적으로 폴링 필요 (CPU 낭비)

6.2 select()
cint select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
사용 예:
cfd_set readfds;
struct timeval tv;

FD_ZERO(&readfds);
FD_SET(sockfd, &readfds);

tv.tv_sec = 5;
tv.tv_usec = 0;

int ret = select(sockfd + 1, &readfds, NULL, NULL, &tv);
if (ret > 0 && FD_ISSET(sockfd, &readfds)) {
    // 읽을 데이터 있음
}
한계:

최대 1024개 디스크립터 (FD_SETSIZE)
O(n) 시간 복잡도
fd_set 복사 오버헤드

6.3 poll()
cint poll(struct pollfd *fds, nfds_t nfds, int timeout);

struct pollfd {
    int   fd;        // 파일 디스크립터
    short events;    // 관심 이벤트
    short revents;   // 발생한 이벤트
};
장점:

디스크립터 개수 제한 없음
events/revents 분리

단점:

여전히 O(n)

6.4 epoll() (Linux)
cint epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events,
               int maxevents, int timeout);
사용 예:
cint epfd = epoll_create1(0);

struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = sockfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

struct epoll_event events[MAX_EVENTS];
int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

for (int i = 0; i < nfds; i++) {
    if (events[i].events & EPOLLIN) {
        // 읽기 가능
    }
}
```

**모드:**
- **Level-triggered (LT)**: 기본, 조건이 만족되는 한 계속 알림
- **Edge-triggered (ET)**: 상태 변화 시점에만 한 번 알림
  - 더 효율적이지만 모든 데이터를 읽어야 함
  - 논블로킹 소켓 필수

**장점:**
- O(1) 시간 복잡도
- 수십만 개 연결 처리 가능
- 고성능 서버에 필수

### 6.5 kqueue() (BSD, macOS)
- epoll과 유사한 메커니즘
- 더 일반화된 이벤트 알림 시스템

### 6.6 IOCP (Windows)
- I/O Completion Port
- 완료 기반 모델 (completion-based)

---

## 7. 고급 주제

### 7.1 소켓 버퍼 관리

**송신 버퍼 (Send Buffer):**
```
응용 → write() → 커널 송신 버퍼 → 네트워크
```
- write()는 버퍼에 복사만 하고 반환
- 실제 전송은 커널이 비동기적으로 처리
- 버퍼가 가득 차면 블로킹

**수신 버퍼 (Receive Buffer):**
```
네트워크 → 커널 수신 버퍼 → read() → 응용
```
- 도착한 데이터는 버퍼에 저장
- read()는 버퍼에서 복사
- 버퍼가 비어있으면 블로킹

**윈도우 크기와 흐름 제어:**
- TCP 헤더의 Window Size 필드
- 수신 버퍼의 남은 공간 광고
- 송신자는 이를 초과하여 전송 불가

### 7.2 Zero-copy 기술

**문제점:**
```
디스크 → 커널 버퍼 → 소켓 버퍼 → NIC
       [복사 1]      [복사 2]
sendfile() (Linux):
c#include <sys/sendfile.h>
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);

커널 공간에서 직접 전송
사용자 공간 복사 제거

splice() (Linux):
cssize_t splice(int fd_in, loff_t *off_in, int fd_out,
               loff_t *off_out, size_t len, unsigned int flags);

파이프를 이용한 zero-copy

7.3 멀티캐스트와 브로드캐스트
브로드캐스트:
cint broadcast = 1;
setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, 
           &broadcast, sizeof(broadcast));

struct sockaddr_in addr;
addr.sin_addr.s_addr = inet_addr("255.255.255.255");
sendto(sockfd, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
멀티캐스트:
cstruct ip_mreq mreq;
mreq.imr_multiaddr.s_addr = inet_addr("239.255.0.1");
mreq.imr_interface.s_addr = INADDR_ANY;
setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
           &mreq, sizeof(mreq));

멀티캐스트 주소: 224.0.0.0 ~ 239.255.255.255
IGMP 프로토콜 사용

7.4 Raw 소켓 프로그래밍
cint sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
IP 헤더 직접 구성:
cint on = 1;
setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));

struct ip {
    u_char  ip_hl:4;        // 헤더 길이
    u_char  ip_v:4;         // 버전
    u_char  ip_tos;         // Type of Service
    u_short ip_len;         // 전체 길이
    u_short ip_id;          // 식별자
    u_short ip_off;         // 플래그 + 오프셋
    u_char  ip_ttl;         // Time To Live
    u_char  ip_p;           // 프로토콜
    u_short ip_sum;         // 체크섬
    struct  in_addr ip_src; // 출발지 주소
    struct  in_addr ip_dst; // 목적지 주소
};
응용:

패킷 스니핑
포트 스캐닝
네트워크 진단 도구

7.5 Unix Domain Socket
특징:

같은 호스트 내 IPC
파일 시스템 경로 사용
TCP보다 2배 빠름
네트워크 오버헤드 없음

예제:
cstruct sockaddr_un addr;
memset(&addr, 0, sizeof(addr));
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, "/tmp/mysocket", sizeof(addr.sun_path)-1);

bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
SCM_RIGHTS:

파일 디스크립터 전달

cstruct msghdr msg;
struct cmsghdr *cmsg;
int fd_to_send = open("file.txt", O_RDONLY);

// ancillary data로 fd 전달
sendmsg(sockfd, &msg, 0);
7.6 소켓 보안
SSL/TLS:
c#include <openssl/ssl.h>

SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
SSL *ssl = SSL_new(ctx);
SSL_set_fd(ssl, sockfd);
SSL_connect(ssl);

SSL_write(ssl, buf, len);
SSL_read(ssl, buf, len);
SO_PEERCRED (Unix):
cstruct ucred cred;
socklen_t len = sizeof(cred);
getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &len);
// cred.pid, cred.uid, cred.gid
```

---

## 8. 소켓 상태와 TCP 상태 머신

### 8.1 TCP 연결 상태
```
CLOSED
  ↓ (passive open)
LISTEN
  ↓ (receive SYN)
SYN_RCVD
  ↓ (receive ACK)
ESTABLISHED ←── (active open, 3-way handshake 완료)
  ↓ (close)
FIN_WAIT_1
  ↓ (receive ACK)
FIN_WAIT_2
  ↓ (receive FIN)
TIME_WAIT
  ↓ (2MSL timeout)
CLOSED
```

### 8.2 TIME_WAIT 상태

**목적:**
1. 마지막 ACK가 손실되었을 경우 재전송
2. 지연된 패킷이 새 연결에 영향 방지

**기간:**
- 2MSL (Maximum Segment Lifetime)
- 일반적으로 1~4분

**문제:**
- 같은 주소/포트 즉시 재사용 불가
- 해결: SO_REUSEADDR 옵션

### 8.3 Half-close 상태
```
클라이언트         서버
ESTABLISHED   ESTABLISHED
    ↓
shutdown(WR)
FIN_WAIT_1 →──FIN──→ CLOSE_WAIT
FIN_WAIT_2 ←──ACK──←
    ↓                   ↓
  (수신만)          (송신 가능)
    ↓                   ↓
          ←──data──←
          ─→─ACK─→
          ←──FIN───←   (close)
TIME_WAIT ─→─ACK─→  LAST_ACK
    ↓                   ↓
  CLOSED            CLOSED

9. 성능 최적화
9.1 Nagle 알고리즘
동작:

작은 패킷 모아서 전송
MSS 크기가 되거나 ACK 받을 때까지 대기

문제:

지연 발생 (interactive 응용에 부적합)

해결:
cint flag = 1;
setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
9.2 Delayed ACK
동작:

ACK를 즉시 보내지 않고 지연 (보통 200ms)
데이터와 함께 piggyback

문제:

Nagle + Delayed ACK = 상호작용 지연
요청-응답 패턴에서 심각

9.3 버퍼 크기 튜닝
c// 대역폭-지연 곱(BDP) 계산
// BDP = Bandwidth × RTT
// 예: 100Mbps × 100ms = 1.25MB

int bufsize = 1310720;  // 1.25MB
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
9.4 TCP Fast Open (TFO)
cint qlen = 5;
setsockopt(sockfd, IPPROTO_TCP, TCP_FASTOPEN, &qlen, sizeof(qlen));
장점:

3-way handshake 중 데이터 전송
RTT 절약

9.5 Concurrent Server 패턴
1. Iterative Server:

한 번에 하나의 클라이언트만 처리
단순하지만 확장성 없음

2. Forking Server:
cwhile (1) {
    int connfd = accept(listenfd, ...);
    pid_t pid = fork();
    if (pid == 0) {  // 자식
        close(listenfd);
        handle_client(connfd);
        exit(0);
    }
    close(connfd);  // 부모
}
3. Threading Server:
cwhile (1) {
    int *connfd = malloc(sizeof(int));
    *connfd = accept(listenfd, ...);
    pthread_t tid;
    pthread_create(&tid, NULL, handle_client, connfd);
    pthread_detach(tid);
}
4. Pre-forked/Pre-threaded:

미리 worker 생성
accept() 경쟁 (thundering herd)
해결: accept mutex 또는 SO_REUSEPORT

5. Event-driven (Reactor Pattern):
cepoll_wait() {
    for each ready socket:
        if (listenfd) {
            accept();
        } else {
            read/write();
        }
}
6. Proactor Pattern:

비동기 I/O (aio, io_uring)
완료 통지


10. 오류 처리
10.1 주요 에러 코드
ECONNREFUSED:

연결 거부 (서버 미실행)

ETIMEDOUT:

연결 시간 초과

ECONNRESET:

연결이 원격에 의해 리셋됨
RST 패킷 수신

EPIPE:

이미 닫힌 소켓에 write
SIGPIPE 시그널 발생

EADDRINUSE:

주소가 이미 사용 중
TIME_WAIT 상태 또는 다른 프로세스

EMFILE / ENFILE:

파일 디스크립터 부족
프로세스/시스템 한계

EAGAIN / EWOULDBLOCK:

논블로킹 소켓에서 데이터 없음

10.2 SIGPIPE 처리
c// 방법 1: 시그널 무시
signal(SIGPIPE, SIG_IGN);

// 방법 2: MSG_NOSIGNAL 플래그
send(sockfd, buf, len, MSG_NOSIGNAL);
10.3 Graceful Shutdown
cvoid graceful_shutdown(int sockfd) {
    // 송신 종료
    shutdown(sockfd, SHUT_WR);
    
    // 남은 데이터 수신
    char buf[1024];
    while (recv(sockfd, buf, sizeof(buf), 0) > 0) {
        // 버림
    }
    
    close(sockfd);
}

11. 디버깅과 모니터링
11.1 netstat
bashnetstat -anp | grep 8080
# -a: 모든 소켓
# -n: 숫자로 표시
# -p: 프로세스 정보
11.2 ss (socket statistics)
bashss -tapn
# netstat보다 빠름
11.3 tcpdump
bashtcpdump -i eth0 port 80 -w capture.pcap
11.4 Wireshark

GUI 패킷 분석기
프로토콜 디코딩

11.5 strace
bashstrace -e trace=network ./program
# 시스템 콜 추적
11.6 lsof
bashlsof -i :8080
# 포트 사용 프로세스 확인

12. 소켓 프로그래밍 베스트 프랙티스
12.1 체크리스트

항상 반환값 확인

cif (socket(...) < 0) {
    perror("socket");
    exit(1);
}

바이트 순서 변환

cuint16_t port = htons(8080);        // host to network short
uint32_t addr = htonl(INADDR_ANY);  // host to network long
uint16_t host_port = ntohs(port);   // network to host short

타임아웃 설정

cstruct timeval tv = {.tv_sec = 10};
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

리소스 정리

cif (sockfd >= 0) close(sockfd);

시그널 처리

c// SIGPIPE 무시
signal(SIGPIPE, SIG_IGN);

// EINTR 재시도
while ((n = recv(...)) < 0 && errno == EINTR);
12.2 일반적인 실수

부분 전송/수신 미처리
바이트 순서 변환 누락
버퍼 오버플로우
메모리 누수 (accept된 소켓 미닫기)
TIME_WAIT 상태 미고려
IPv4 하드코딩 (IPv6 호환성 부족)
에러 처리 누락


13. 실전 예제
13.1 간단한 TCP 에코 서버
c#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    // 2. SO_REUSEADDR 설정
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 3. 주소 구조체 초기화
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // 4. 바인딩
    if (bind(server_fd, (struct sockaddr*)&server_addr, 
             sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    // 5. 리스닝
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Server listening on port %d\n", PORT);
    
    // 6. 클라이언트 처리 루프
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, 
                          &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, 
                 client_ip, sizeof(client_ip));
        printf("Client connected: %s:%d\n", 
               client_ip, ntohs(client_addr.sin_port));
        
        // 에코 루프
        ssize_t n;
        while ((n = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            send(client_fd, buffer, n, 0);
        }
        
        close(client_fd);
        printf("Client disconnected\n");
    }
    
    close(server_fd);
    return 0;
}
13.2 멀티스레드 TCP 서버
c#include <pthread.h>

void *handle_client(void *arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    char buffer[1024];
    ssize_t n;
    
    while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        send(client_fd, buffer, n, 0);
    }
    
    close(client_fd);
    return NULL;
}

int main() {
    // ... (소켓 생성, 바인딩, 리스닝)
    
    while (1) {
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, ...);
        
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);
    }
}
13.3 epoll 기반 서버
c#include <sys/epoll.h>

#define MAX_EVENTS 100

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // ... (바인딩, 리스닝)
    
    // 논블로킹 설정
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    // epoll 생성
    int epfd = epoll_create1(0);
    
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);
    
    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                // 새 연결 수락
                int client_fd = accept(server_fd, NULL, NULL);
                
                // 논블로킹 설정
                flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
                
                // epoll에 추가
                ev.events = EPOLLIN | EPOLLET;  // Edge-triggered
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
            } else {
                // 클라이언트 데이터 처리
                char buffer[1024];
                ssize_t n;
                
                while ((n = recv(events[i].data.fd, buffer, 
                                sizeof(buffer), 0)) > 0) {
                    send(events[i].data.fd, buffer, n, 0);
                }
                
                if (n == 0 || (n < 0 && errno != EAGAIN)) {
                    close(events[i].data.fd);
                }
            }
        }
    }
    
    return 0;
}

14. 참고 문헌 및 추가 학습 자료
14.1 표준 문서

POSIX.1-2008 (IEEE Std 1003.1-2008)
RFC 793 (TCP)
RFC 768 (UDP)
RFC 791 (IP)

14.2 권장 도서

"Unix Network Programming" - W. Richard Stevens
"TCP/IP Illustrated, Volume 1" - W. Richard Stevens
"The Linux Programming Interface" - Michael Kerrisk
"Computer Networks" - Andrew S. Tanenbaum

14.3 온라인 리소스

Beej's Guide to Network Programming
Linux man pages (man 2 socket, man 7 tcp)
POSIX specification

