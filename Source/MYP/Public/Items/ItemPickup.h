// ItemPickup.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

// 레벨에 배치 가능한 간단한 픽업 액터
// - 플레이어가 구체 콜리전에 진입하면 인벤토리에 아이템을 추가하고 자기 자신을 제거
UCLASS()
class MYP_API AItemPickup : public AActor
{
    GENERATED_BODY()

public:
    /**
     * 기본 생성자
     * - 구체 콜리전(Overlap 전용) + 시각용 스태틱 메시 구성.
     * - BeginOverlap 델리게이트에 OnSphereBeginOverlap을 동적 바인딩.
     */
    AItemPickup();

protected:
    /** 플레이어와의 오버랩을 감지할 단순 구형 콜리전(루트) */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* pSphere;

    /** 픽업 오브젝트의 외형 표현용 메시(콜리전 없음) */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* pMesh;

    /** 인벤토리에 추가될 아이템 식별자(스택 병합 기준) */
    UPROPERTY(EditAnywhere, Category = "Item")
    FName ItemId = TEXT("Item.Sample");

    /** UI 표시용 아이템 이름(FText) */
    UPROPERTY(EditAnywhere, Category = "Item")
    FText DisplayName = FText::FromString(TEXT("Sample Item"));

    /** 인벤토리에 더해질 수량(>0 권장) */
    UPROPERTY(EditAnywhere, Category = "Item")
    int32 Quantity = 1;

    /**
     * 구체 컴포넌트의 BeginOverlap 콜백
     *
     * 기대/전제
     * - OtherActor가 플레이어 캐릭터(AMYPCharacter)이며, 인벤토리 컴포넌트를 보유해야 합니다.
     * - AddItemSimple 성공 시 자기 자신(픽업 액터)을 Destroy() 합니다.
     *
     * 파라미터
     * @param OverlappedComponent 오버랩 이벤트를 트리거한 컴포넌트(여기서는 pSphere)
     * @param OtherActor 오버랩에 들어온 상대 액터(플레이어 캐릭터 예상)
     * @param OtherComp 상대 액터의 컴포넌트(사용하지 않음)
     * @param OtherBodyIndex 상대 바디 인덱스(사용하지 않음, PhysX 잔재)
     * @param bFromSweep Sweep 이동으로 인한 오버랩인지 여부(참고용)
     * @param SweepResult Sweep에 의한 히트 상세(FromSweep=true일 때 유효)
     */
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
