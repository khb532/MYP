// InventoryComponent.cpp
//
// 구현 설명
// - 배열 기반 스택 인벤토리: 동일 ItemId는 수량만 증가.
// - 최대 슬롯 수 초과 시 추가 거부.
// - 변경 시 OnInventoryChanged 멀티캐스트 델리게이트 브로드캐스트.

#include "Components/InventoryComponent.h"
#include "MYP.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 틱 비활성화(이벤트 기반 동작이므로 불필요한 틱 방지)
}

// 내부 헬퍼: Id로 슬롯 인덱스를 찾는다(없으면 INDEX_NONE)
// 내부 헬퍼: Id로 슬롯 인덱스를 찾는다(없으면 INDEX_NONE)
// @param ItemId 찾을 아이템 식별자
// @return 발견 시 인덱스, 실패 시 INDEX_NONE
int32 UInventoryComponent::FindItemIndexById(FName ItemId) const
{
    // 배열에서 동일 ItemId를 가진 첫 슬롯의 인덱스를 찾음(없으면 INDEX_NONE)
    return Items.IndexOfByPredicate([&](const FInventoryItem& It){ return It.ItemId == ItemId; }); // 람다로 비교 조건 정의
}

// 내부 헬퍼: 변경 알림 브로드캐스트
// 내부 헬퍼: 변경 알림 브로드캐스트
void UInventoryComponent::BroadcastChanged()
{
    OnInventoryChanged.Broadcast(); // 구독자들에게 "인벤토리가 변경됨" 신호 전파
}

// AddItem
// @param Item 추가할 항목(유효한 ItemId, 1 이상 Quantity 필요)
// @return true: 병합/추가 성공, false: 무효 입력/슬롯 부족 등 실패
bool UInventoryComponent::AddItem(const FInventoryItem& Item)
{
    // 입력 유효성 검사: 비어있는 Id 또는 0/음수 수량이면 실패
    if (Item.ItemId.IsNone() || Item.Quantity <= 0)
    {
        SHOWWARNF(TEXT("Invalid item: Id=%s, Qty=%d"), *Item.ItemId.ToString(), Item.Quantity); // 경고 로그 출력
        return false; // 유효하지 않은 요청 거부
    }

    // 동일 Id 슬롯 검색(있으면 스택 수량 증가)
    int32 Index = FindItemIndexById(Item.ItemId); // 병합 대상 인덱스 조회
    if (Index != INDEX_NONE) // 이미 같은 Id의 슬롯이 존재
    {
        Items[Index].Quantity += Item.Quantity; // 스택 누적
        BroadcastChanged(); // 변경 알림
        return true; // 성공 반환
    }

    // 새 슬롯 필요: 현재 슬롯 수가 MaxSlots를 초과하는지 확인
    if (Items.Num() >= MaxSlots)
    {
        SHOWWARNF(TEXT("Inventory full")); // 공간 부족 경고
        return false; // 실패 반환
    }

    // 신규 슬롯 추가(구조체 복사)
    Items.Add(Item); // 마지막 인덱스에 추가
    BroadcastChanged(); // 변경 알림
    return true; // 성공
}

// AddItemSimple
// @param ItemId 병합 기준 식별자(None 금지)
// @param DisplayName UI 표시명(FText)
// @param Quantity 추가 수량(>0)
// @return AddItem과 동일
// RemoveItem
// @param ItemId 제거 대상 식별자
// @param Quantity 제거 수량(>0)
// @return true: 존재/차감/정리, false: 미존재/무효 입력
bool UInventoryComponent::RemoveItem(FName ItemId, int32 Quantity)
{
    // 입력 유효성 검사: 비어있는 Id 또는 0/음수 수량이면 실패
    if (ItemId.IsNone() || Quantity <= 0)
    {
        return false; // 무효 입력
    }

    // 대상 슬롯 검색
    int32 Index = FindItemIndexById(ItemId); // 인덱스 조회
    if (Index == INDEX_NONE) // 인벤토리에 해당 Id가 없음
    {
        return false; // 실패 반환
    }

    // 수량 차감 및 0 이하일 경우 슬롯 제거
    FInventoryItem& It = Items[Index]; // 참조 획득(직접 수정)
    It.Quantity -= Quantity; // 수량 감소
    if (It.Quantity <= 0) // 소진되었으면
    {
        Items.RemoveAt(Index); // 슬롯 제거(뒤 요소들이 앞으로 당겨짐)
    }
    BroadcastChanged(); // 변경 알림
    return true; // 성공
}

// Clear: 전체 비우기 후 변경 브로드캐스트
void UInventoryComponent::Clear()
{
    Items.Reset(); // 모든 슬롯 제거(용량 유지)
    BroadcastChanged(); // 변경 알림
}
