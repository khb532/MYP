// 목적
// - 캐릭터(또는 어떤 액터든)에 부착해 사용할 수 있는 경량 인벤토리 컴포넌트.
// - 배열 기반 간단한 스택 인벤토리로 수량 합치기, 제거, 비우기, 변경 이벤트 제공.
// - 멀티캐스트 델리게이트를 사용해 UI 등 구독자에게 변경 알림 브로드캐스트.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

/**
 * 인벤토리 한 슬롯을 표현하는 데이터 구조체
 * - ItemId: 아이템의 논리적 식별자(스택 병합 기준). 비어 있으면 무효로 간주.
 * - DisplayName: UI 표시용 이름(현지화 가능 FText).
 * - Quantity: 현재 스택 수량(1 이상). 0 이하가 되면 슬롯 삭제 대상.
 */
USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText DisplayName;

    /**
     * UI 아이콘(선택)
     * - 바둑판형 그리드 슬롯에 표시할 텍스처. 비어 있으면 아이콘 미표시.
     * - 소프트 참조를 사용해 에디터에서 자산 경로만 유지하고 필요 시 로드할 수 있습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftObjectPtr<class UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Quantity = 1;
};

// 인벤토리 변경을 알리는 간단한 델리게이트(파라미터 없음)
DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYP_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**
     * 기본 생성자
     * - Tick 비활성화(로직이 이벤트 기반이므로 불필요한 틱 방지)
     */
    UInventoryComponent();

    /**
     * 아이템(구조체) 단위로 추가
     *
     * 기대/전제
     * - Item.ItemId != None 이어야 하며, Item.Quantity > 0 이어야 합니다.
     * - 동일 ItemId가 이미 존재하면 해당 슬롯의 Quantity를 누적(스택 병합)합니다.
     * - 새 슬롯 추가가 필요한 경우 MaxSlots 초과 시 실패합니다.
     *
     * 내부 동작
     * - 선행 유효성 검사 → 동일 Id 탐색 → (존재) 수량 누적 / (부재) 슬롯 여유 확인 후 추가.
     * - 변경 성공 시 OnInventoryChanged 델리게이트 브로드캐스트.
     *
     * @param Item 추가하려는 인벤토리 항목(표시명/수량 포함)
     * @return true: 추가/누적 성공, false: 무효 입력 또는 공간 없음 등으로 실패
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(const FInventoryItem& Item);

    /**
     * 수량만큼 제거, 0 이하가 되면 슬롯 삭제
     *
     * 기대/전제
     * - ItemId != None, Quantity > 0
     * - 존재하지 않는 Id 제거 요청 시 false 반환
     *
     * 내부 동작
     * - Id로 슬롯 검색 → 수량 차감 → 0 이하이면 슬롯 제거 → 변경 브로드캐스트
     *
     * @param ItemId 제거할 대상 아이템 식별자
     * @param Quantity 제거 수량(>0)
     * @return true: 존재/차감/정리 성공, false: 미존재/무효 입력
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemId, int32 Quantity);

    /**
     * 전체 비우기
     * - 내부 배열을 Reset하고 변경 브로드캐스트
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void Clear();

    /**
     * 현재 인벤토리 목록을 상수 참조로 반환(읽기 전용)
     * - 대용량 복사를 피하기 위해 const TArray& 형태로 제공합니다.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FInventoryItem>& GetItems() const { return Items; }

    /** 최대 슬롯 수를 조회 */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetMaxSlots() const { return MaxSlots; }

    /**
     * 최대 슬롯 수 설정
     * - 최소 1 이상으로 강제(FMath::Max).
     * - 현재 보유 슬롯 수가 초과되어도 즉시 잘라내지는 않으며, 이후 Add 시에만 제한이 적용됩니다.
     *
     * @param InMax 설정하려는 최대 슬롯 수(1 이상)
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetMaxSlots(int32 InMax) { MaxSlots = FMath::Max(1, InMax); }

    /**
     * 인벤토리 변경 델리게이트(파라미터 없음)
     * - 목적: AddItem/RemoveItem/Clear 등으로 내부 상태가 바뀔 때 UI/시스템에 통지.
     * - 바인딩: C++에서만 사용(비동적 멀티캐스트). 예) pInventory->OnInventoryChanged.AddUObject(this, &UInventoryWidget::RebuildInventoryUI);
     *   해제: pInventory->OnInventoryChanged.RemoveAll(this); // 또는 AddUObject가 반환한 FDelegateHandle로 Remove
     * - 발신: 내부에서 BroadcastChanged()가 호출될 때 OnInventoryChanged.Broadcast() 실행.
     * - 스레드: 게임 스레드에서 사용 권장(일반적인 UMG/UI 패턴).
     * - 블루프린트: 현재는 BP에서 바인드할 수 없음. 필요 시 DECLARE_DYNAMIC_MULTICAST_DELEGATE(…)와
     *   UPROPERTY(BlueprintAssignable)로 교체하여 BP 노출 가능(성능/오버헤드 증가 고려).
     * - 페이로드: 파라미터가 필요하면 시그니처를 확장(예: 변경된 ItemId/Delta)하여 설계 가능.
     */
    FOnInventoryChanged OnInventoryChanged;

private:
    /** 최대 슬롯 수(간단 구현: 각 아이템 1 슬롯, 같은 Id는 스택 병합) */
    UPROPERTY(EditAnywhere, Category = "Inventory")
    int32 MaxSlots = 24;

    /** 인벤토리 내부 저장소(슬롯 목록) */
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FInventoryItem> Items;

    /**
     * ItemId로 슬롯 인덱스를 찾습니다.
     * - 내부 탐색 유틸, 미존재 시 INDEX_NONE 반환.
     *
     * @param ItemId 찾을 아이템 식별자
     * @return int32: 발견 시 인덱스, 실패 시 INDEX_NONE
     */
    int32 FindItemIndexById(FName ItemId) const;

    /**
     * 변경 브로드캐스트 도우미
     * - OnInventoryChanged를 호출합니다.
     */
    void BroadcastChanged();
};
