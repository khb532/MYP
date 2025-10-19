// InventoryWidget.h
//
// 목적
// - 코드로 위젯 트리를 구성하는 인벤토리 그리드(UI 고정 슬롯 방식).
// - MaxSlots 개수만큼 고정 슬롯을 생성해 빈 칸/채운 칸을 표현.
// - 외부에서 인벤토리 컴포넌트를 주입 받아(OnInventoryChanged) 그리드를 갱신.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UUniformGridPanel;
class USizeBox;
class UImage;
class UTextBlock;
class UBorder;
class UCanvasPanel;
class UInventoryComponent;

UCLASS()
class MYP_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * 외부에서 인벤토리 컴포넌트를 주입(구독 시작)
     *
     * 기대/전제
     * - InInv가 유효한 UInventoryComponent 인스턴스여야 합니다(소유자: 플레이어나 UI 소유 객체).
     * - 위젯은 내부적으로 약참조(TWeakObjectPtr)로 유지하여 수명 문제를 회피합니다.
     * - 주입 즉시 OnInventoryChanged 델리게이트에 구독하고, 한 번 즉시 UI를 갱신합니다.
     *
     * @param InInv 연결하려는 인벤토리 컴포넌트 포인터(소유권 없음)
     */
    void SetInventoryComponent(UInventoryComponent* InInv);

protected:
    /** 위젯 트리 생성(캔버스 → 보더 → 세로 박스). UMG 자산 없이 코드로 구성 */
    virtual void NativeConstruct() override;

private:
    /** 인벤토리 컴포넌트 약참조(GC 안전) */
    TWeakObjectPtr<UInventoryComponent> pInventory;

    /** 슬롯을 배치할 UniformGridPanel(코드 생성). Transient: 저장 불필요 */
    UPROPERTY(Transient)
    UUniformGridPanel* pGrid = nullptr;

    /** 그리드 구성 파라미터: 열 수(>=1), 슬롯 픽셀 크기 */
    UPROPERTY(EditAnywhere, Category="Inventory|UI")
    int32 Columns = 5;

    UPROPERTY(EditAnywhere, Category="Inventory|UI")
    FVector2D SlotSize = FVector2D(72.f, 72.f);

    /** 현재 인벤토리 상태를 읽어 고정 슬롯을 다시 그림(전체 리빌드) */
    void RebuildInventoryUI();

    /** 단일 슬롯 위젯을 구성해 반환(아이콘/수량/빈 칸 처리) */
    UWidget* BuildSlotWidget(int32 SlotIndex, const struct FInventoryItem* ItemOrNull);
};
