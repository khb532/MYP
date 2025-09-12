// ItemPickup.cpp
//
// 구현 설명
// - 구체 컴포넌트로 오버랩 감지, 스태틱 메시로 시각 표현.
// - 오버랩 대상이 캐릭터이고 인벤토리 컴포넌트를 보유하면 AddItem 후 Destroy().

#include "Items/ItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Components/InventoryComponent.h"
#include "MYP.h"

// 생성자: 충돌/시각 설정 및 오버랩 델리게이트 바인딩
AItemPickup::AItemPickup()
{
    PrimaryActorTick.bCanEverTick = false; // 틱 비활성화(픽업은 이벤트로만 동작)

    pSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere")); // 구체 콜리전 생성
    SetRootComponent(pSphere); // 루트로 설정
    pSphere->InitSphereRadius(60.f); // 반경 설정(접근 허용 거리)
    pSphere->SetCollisionResponseToAllChannels(ECR_Ignore); // 기본적으로 모든 채널 무시
    pSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Pawn에 대해서만 오버랩 감지

    pMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh")); // 시각용 메시 생성
    pMesh->SetupAttachment(pSphere); // 콜리전에 부착
    pMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시에는 충돌 없음(콜리전은 pSphere가 담당)

    // 델리게이트 바인딩(동적): 블루프린트에서 바인딩/해제 가능 형태
    pSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnSphereBeginOverlap); // 오버랩 시작 시 콜백 호출
}

// 오버랩 콜백: 플레이어 캐릭터가 들어오면 인벤토리에 아이템을 추가하고 픽업 제거
// @param OverlappedComponent 이 액터의 구체 컴포넌트(루트)
// @param OtherActor 오버랩한 상대 액터(플레이어 캐릭터 예상)
// @param OtherComp 상대 컴포넌트(미사용)
// @param OtherBodyIndex 상대 바디 인덱스(미사용)
// @param bFromSweep Sweep 기반 이동인지 여부(참고)
// @param SweepResult Sweep 세부 결과(참고)
void AItemPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 특정 캐릭터 타입에 의존하지 않고, 상대 액터에서 인벤토리 컴포넌트를 탐색
    UInventoryComponent* pInv = OtherActor ? OtherActor->FindComponentByClass<UInventoryComponent>() : nullptr; // 상대 액터에서 인벤토리 탐색
    if (!pInv) // 인벤토리 미보유 시 처리 중단
    {
        SHOWWARNF(TEXT("Character has no InventoryComponent")); // 경고 로그
        return;
    }

    // AddItem 전용 API만 유지: 구조체를 만들어 전달
    FInventoryItem Item; // 임시 아이템 구조체 생성
    Item.ItemId = ItemId; // 아이템 식별자 설정(병합 기준)
    Item.DisplayName = DisplayName; // UI 표시명 설정
    Item.Quantity = Quantity; // 수량 설정

    if (pInv->AddItem(Item)) // 인벤토리에 추가 성공 시
    {
        SHOWLOGF(TEXT("Picked up %s x%d"), *DisplayName.ToString(), Quantity); // 픽업 로그 출력
        Destroy(); // 픽업 액터 제거
    }
}
