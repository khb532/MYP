# 스마트포인터 (Smart Pointers) 완전 가이드

## 1. 기본 개념: 포인터부터 스마트포인터까지

### 1.1 포인터란 무엇인가?

프로그래밍에서 **포인터**는 메모리 주소를 저장하는 변수다. 쉽게 말해, "이 데이터가 메모리 어디에 있는지를 가리키는 화살표"라고 생각하면 된다.

```cpp
int x = 42;
int* ptr = &x;  // ptr은 x가 저장된 메모리 주소를 가리킨다
```

포인터를 사용하면:
- 함수에서 변수를 수정할 수 있다
- 동적 메모리를 할당할 수 있다
- 복잡한 데이터 구조를 만들 수 있다

### 1.2 일반 포인터의 문제점: 메모리 누수

하지만 포인터에는 **큰 문제**가 있다. 바로 **메모리 관리의 책임이 프로그래머에게 있다**는 것이다.

**시나리오:**
```cpp
Player* player = new Player("Alice");  // 메모리 할당
player->Attack();
// ... 복잡한 코드 ...
delete player;  // 반드시 메모리 해제해야 함
```

문제가 뭘까?

1. **할당했는데 해제를 깜빡했다면?**
   - 메모리가 계속 차지하고 있음 (메모리 누수)
   - 게임을 오래 플레이하면 프레임 드롭

2. **예외가 발생했다면?**
   - delete에 도달하지 못함
   - 역시 메모리 누수

3. **함수를 여러 개 거쳐야 한다면?**
   - 누가 delete 할 책임이 있나?
   - 담당자 불명확 → 버그 발생

### 1.3 스마트포인터의 등장: 자동 메모리 관리

이런 문제들 때문에 **스마트포인터**가 만들어졌다.

**핵심 아이디어**: "포인터가 스스로 메모리를 관리하도록 하자"

스마트포인터는:
- 포인터처럼 메모리 주소를 가리킨다
- **동시에** 그 메모리의 생명주기를 자동으로 관리한다
- 더 이상 필요 없으면 자동으로 delete 한다

```cpp
std::unique_ptr<Player> player(new Player("Alice"));  // 메모리 할당
player->Attack();
// ... 복잡한 코드 ...
// 블록을 벗어나면 자동으로 메모리 해제!
// delete를 직접 쓸 필요 없음
```

---

## 2. 스마트포인터의 종류: 소유권 전략

스마트포인터는 **소유권**을 어떻게 관리하느냐에 따라 3가지로 나뉜다.

### 2.1 unique_ptr: "이건 나만의 것"

**unique_ptr**는 "이 메모리는 나만 소유한다"는 의미다.

**특징:**
- **한 개의 unique_ptr만** 이 메모리를 소유할 수 있다
- 소유권을 **이동만** 가능하고, 복사는 불가능하다
- 가장 가볍고 빠르다

**상황:**
게임의 플레이어 캐릭터를 생각해보자. 게임 중에 플레이어는 한 명이고, 딱 한 개의 객체만 관리하면 된다.

```cpp
std::unique_ptr<Player> mainPlayer(new Player("Alice"));
// mainPlayer는 이 Player 객체의 유일한 소유자
```

게임이 끝나거나 플레이어가 제거되면? mainPlayer가 스코프를 벗어나면서 자동으로 메모리가 해제된다.

### 2.2 shared_ptr: "우리가 함께 소유하자"

**shared_ptr**는 "이 메모리를 여러 개가 함께 소유한다"는 의미다.

**특징:**
- **여러 개의 shared_ptr이** 같은 메모리를 소유할 수 있다
- 자동으로 **참조 계수(Reference Count)**를 관리한다
- 모든 shared_ptr이 메모리를 해제할 때만, 실제 메모리가 삭제된다

**상황:**
게임에서 아이템을 여러 플레이어가 동시에 참조한다고 하자.

```cpp
std::shared_ptr<Item> sword(new Item("Iron Sword"));

Player player1;
Player player2;

player1.SetWeapon(sword);  // player1도 소유
player2.SetWeapon(sword);  // player2도 소유

// sword는 이제 3개의 shared_ptr로 관리된다:
// 원래의 sword 변수 + player1 + player2
```

이제 누구든 sword를 삭제해도, 다른 누군가가 여전히 참조 중이면 메모리는 살아있다.

모두가 sword를 손놓을 때만 메모리가 진정으로 해제된다.

### 2.3 weak_ptr: "나는 빌려만 쓸게"

**weak_ptr**은 "나는 이 메모리를 참조하지만, 소유하지는 않는다"는 의미다.

**특징:**
- shared_ptr을 **참조할 때만** 사용한다
- **참조 계수에 포함되지 않는다**
- 메모리가 삭제되었는지 확인할 수 있다

**왜 필요할까?** 나중에 배울 "순환 참조" 문제를 해결하기 위해서다.

---

## 3. 각 스마트포인터의 동작 원리

### 3.1 unique_ptr의 동작

**unique_ptr은 어떻게 "유일함"을 보장할까?**

C++은 **이동 의미론(Move Semantics)**이라는 기법을 사용한다.

```cpp
std::unique_ptr<Player> player1(new Player("Alice"));
std::unique_ptr<Player> player2 = player1;  // ❌ 컴파일 에러! 복사 불가
std::unique_ptr<Player> player3 = std::move(player1);  // ✓ 이동 가능

// 이제 player3이 메모리 소유, player1은 nullptr
```

**이동이란:**
- player1의 포인터를 player3으로 "옮긴다"
- player1은 더 이상 메모리를 소유하지 않는다
- 따라서 한 번에 한 개의 unique_ptr만 소유하게 된다

**언제 해제되나?**
```cpp
{
    std::unique_ptr<Player> player(new Player("Alice"));
    player->Attack();
}  // 블록 끝 → player의 소멸자 호출 → 메모리 자동 해제
```

### 3.2 shared_ptr의 참조 계수

**shared_ptr은 몇 개가 메모리를 공유하고 있는지 어떻게 알까?**

내부적으로 **참조 계수(Reference Count)**를 관리한다.

```cpp
{
    std::shared_ptr<Item> sword1(new Item("Iron Sword"));
    // 참조 계수: 1

    std::shared_ptr<Item> sword2 = sword1;
    // 참조 계수: 2 (같은 메모리를 2개가 가리킴)

    {
        std::shared_ptr<Item> sword3 = sword1;
        // 참조 계수: 3
    }  // sword3 소멸, 참조 계수: 2

}  // sword1, sword2 소멸, 참조 계수: 0 → 메모리 삭제!
```

**참조 계수는 어디에 저장되나?** 내부적으로 별도의 메모리 영역에 관리되며, 각 shared_ptr 변수가 이 정보를 공유한다.

### 3.3 weak_ptr의 필요성: 순환 참조 방지

**순환 참조란 뭘까?**

```cpp
class Player {
public:
    std::shared_ptr<Weapon> weapon;
};

class Weapon {
public:
    std::shared_ptr<Player> owner;  // ❌ 순환 참조!
};
```

이제 코드를 실행해보자:

```cpp
{
    auto player = std::make_shared<Player>();
    auto sword = std::make_shared<Weapon>();

    player->weapon = sword;     // player → sword
    sword->owner = player;      // sword → player (순환!)

}  // 블록 끝
// player 삭제? 아니다, sword가 아직 가리키고 있다
// sword 삭제? 아니다, player가 아직 가리키고 있다
// 둘 다 메모리에 남는다 → 메모리 누수!
```

**해결책: weak_ptr 사용**

```cpp
class Weapon {
public:
    std::weak_ptr<Player> owner;  // ✓ weak_ptr 사용
};
```

이제:
- player는 sword를 소유한다 (shared_ptr)
- sword는 player를 "참조만"한다 (weak_ptr, 소유 아님)
- player가 삭제되면, sword의 owner는 nullptr이 된다
- sword도 정리된다 → 메모리 누수 해결!

---

## 4. 사용 시나리오: 언제 뭘 쓸까?

### 4.1 unique_ptr을 쓰는 경우

**한 개 객체가 다른 객체를 "소유"할 때**

예: 플레이어가 무기를 소유한다

```cpp
class Player {
private:
    std::unique_ptr<Weapon> weapon;  // 플레이어만 무기를 소유
};
```

특징:
- 플레이어가 죽으면 무기도 자동 삭제
- 무기는 다른 누구도 소유할 수 없음
- 매우 명확한 소유 관계

### 4.2 shared_ptr을 쓰는 경우

**여러 개가 같은 객체를 "공유"할 때**

예: 게임에 떨어진 아이템을 여러 플레이어가 볼 수 있다

```cpp
class GameWorld {
private:
    std::vector<std::shared_ptr<Item>> items;  // 게임이 관리
};

class Player {
public:
    void PickupItem(std::shared_ptr<Item> item) {
        inventory.push_back(item);  // 플레이어도 아이템 소유
    }
};
```

특징:
- 게임 월드도 소유, 플레이어도 소유
- 누가 먼저 제거해도 상관없음
- 마지막 사람이 놓을 때 삭제됨

### 4.3 weak_ptr을 쓰는 경우

**"관찰만" 하고 싶을 때**

예: 플레이어의 현재 목표(타겟)를 추적한다

```cpp
class Player {
private:
    std::weak_ptr<Enemy> target;  // 타겟을 "관찰만"함

public:
    void SetTarget(std::shared_ptr<Enemy> enemy) {
        target = enemy;  // 타겟 설정
    }

    void AttackTarget() {
        if (auto enemy = target.lock()) {  // 타겟이 아직 살아있나?
            player_->Attack(enemy);
        } else {
            // 타겟이 죽었다
        }
    }
};
```

특징:
- 타겟을 소유하지 않음
- 타겟이 죽으면 자동으로 nullptr처럼 작동
- 메모리 누수 걱정 없음

---

## 5. 언리얼 엔진의 스마트포인터

### 5.1 언리얼의 스마트포인터 종류

언리얼 엔진은 C++ 표준 라이브러리가 아닌 **자신의 스마트포인터**를 사용한다.

- **TUniquePtr**: unique_ptr과 같음
- **TSharedPtr**: shared_ptr과 같음
- **TWeakPtr**: weak_ptr과 같음

네이밍 규칙:
- T = Template (템플릿)
- Unique, Shared, Weak = 종류

### 5.2 언리얼 스마트포인터의 사용법

```cpp
// TUniquePtr
TUniquePtr<ACharacter> character = MakeUnique<ACharacter>();

// TSharedPtr
TSharedPtr<AActor> actor = MakeShared<AActor>();

// TWeakPtr
TWeakPtr<AActor> weakActor = actor;
if (TSharedPtr<AActor> strongActor = weakActor.Pin()) {
    // 아직 살아있다!
    strongActor->DoSomething();
}
```

### 5.3 언리얼 액터와 스마트포인터

**중요한 주의:**

언리얼의 **AActor**는 이미 **자동 메모리 관리**가 되어있다!

```cpp
AActor* actor = GetWorld()->SpawnActor<ACharacter>();  // 포인터 사용 가능
// 월드가 자동으로 관리해줌
```

따라서:
- AActor 파생 클래스: 보통 raw 포인터 사용
- AActor가 아닌 데이터: TSharedPtr / TUniquePtr 사용

```cpp
class AMyCharacter : public ACharacter {
private:
    TUniquePtr<class AWeapon> weapon;  // 액터가 아닌 일반 객체
    ASkeletalMeshComponent* mesh;  // 액터 컴포넌트는 raw 포인터
};
```

---

## 6. 주의사항: 순환 참조와 메모리 누수

### 6.1 순환 참조 시나리오

**시나리오: 부모-자식 관계**

```cpp
class Node {
public:
    std::shared_ptr<Node> parent;    // ❌ 부모 참조
    std::shared_ptr<Node> child;     // 자식 소유 OK
};
```

```cpp
{
    auto parent = std::make_shared<Node>();
    auto child = std::make_shared<Node>();

    parent->child = child;    // parent → child (OK)
    child->parent = parent;   // child → parent (순환!)

}  // 메모리 누수!
```

**해결책: 부모는 weak_ptr로**

```cpp
class Node {
public:
    std::weak_ptr<Node> parent;     // ✓ 약한 참조
    std::shared_ptr<Node> child;    // 자식 소유
};
```

### 6.2 스마트포인터 오용

**실수 1: raw 포인터와 스마트포인터 혼용**

```cpp
auto smartPtr = std::make_shared<Player>();
Player* rawPtr = smartPtr.get();

// ... 어딘가에서 ...
delete rawPtr;  // ❌ 큰일! 스마트포인터도 delete하려고 한다

// 같은 메모리를 두 번 해제 → 크래시!
```

**실수 2: 복사와 이동 혼동**

```cpp
auto ptr1 = std::make_shared<Player>();
Player* rawPtr = ptr1.get();

delete rawPtr;  // ❌ 큰일! 스마트포인터도 delete하려고 함
// 같은 메모리를 두 번 해제 → 크래시!
```

해결책: raw 포인터와 스마트포인터를 혼용하지 않기.

---

## 정리: 스마트포인터 선택 가이드

| 상황 | 스마트포인터 | 이유 |
|------|------------|------|
| A가 B를 "소유"함. 다른 곳에서 참조 안 함 | unique_ptr | 빠르고 명확 |
| 여러 곳에서 같은 객체 필요 | shared_ptr | 자동 관리, 안전 |
| A가 B를 소유하는데, B도 A 참조 필요 | A→B는 shared_ptr, B→A는 weak_ptr | 순환 참조 방지 |
| 그냥 관찰하고 싶음 | weak_ptr | 소유하지 않음 |

**황금 규칙:**
- 기본값: **unique_ptr**
- 공유 필요: **shared_ptr**
- 순환 참조 방지: **weak_ptr**
