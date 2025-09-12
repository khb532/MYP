// InventoryWidget.cpp
//
// 구현 설명
// - WidgetTree API로 런타임에 UI를 구성(UMG 에셋 없이).
// - 델리게이트를 통해 인벤토리 변경 시 목록을 다시 그린다.

#include "UI/InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/InventoryComponent.h"

// 인벤토리 컴포넌트를 주입하고 변경 이벤트에 구독한 뒤 즉시 UI를 1회 갱신
// @param InInv 연결할 인벤토리 컴포넌트(소유권 없음)
void UInventoryWidget::SetInventoryComponent(UInventoryComponent* InInv)
{
    pInventory = InInv; // 약참조에 주입(소유권 없음)
    if (pInventory.IsValid()) // 유효성 확인
    {
        // 인벤토리 변경 시 그리드를 다시 구축하도록 델리게이트 바인딩
        pInventory->OnInventoryChanged.AddUObject(this, &UInventoryWidget::RebuildInventoryUI);
        RebuildInventoryUI(); // 최초 1회 즉시 갱신
    }
}

// 위젯 트리 구성: Canvas → Border(반투명 배경) → VerticalBox(아이템 행)
void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas")); // 최상위 캔버스 생성
    WidgetTree->RootWidget = RootCanvas; // 루트로 설정

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel")); // 반투명 배경 보더 생성
    Panel->SetPadding(FMargin(16.f));                   // 안쪽 여백 설정
    Panel->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.7f)); // 반투명 검정 배경색

    pGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("Grid")); // 그리드 패널 생성
    Panel->SetContent(pGrid); // 보더의 콘텐츠로 그리드 설정

    // 로컬 변수명을 'Slot'이 아닌 'CanvasSlot'으로 사용해 UWidget::Slot 멤버와 이름 충돌을 피함
    if (UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(Panel)) // 루트 캔버스에 보더 추가
    {
        CanvasSlot->SetAutoSize(true);                        // 내용 크기에 맞춤
        CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));         // 화면 중앙 정렬(앵커)
        CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));      // 정렬 축 기준 중앙
    }

    RebuildInventoryUI();
}

// 고정 그리드 전체를 다시 그림(간단/안정 구현)
// - MaxSlots 개수만큼 슬롯을 매번 재생성(교육/간단 구현). 대규모일 경우 슬롯 재사용 최적화 가능.
void UInventoryWidget::RebuildInventoryUI()
{
    if (!pGrid)
    {
        return;
    }
    pGrid->ClearChildren();

    const int32 NumColumns = FMath::Max(1, Columns);
    int32 MaxSlots = 0;
    TArray<FInventoryItem> ItemsCopy;

    if (pInventory.IsValid())
    {
        MaxSlots = pInventory->GetMaxSlots();
        ItemsCopy = pInventory->GetItems();
    }

    // 안전 장치: 슬롯 수 최소 1
    MaxSlots = FMath::Max(1, MaxSlots);

    for (int32 SlotIndex = 0; SlotIndex < MaxSlots; ++SlotIndex)
    {
        const FInventoryItem* ItemPtr = (SlotIndex < ItemsCopy.Num()) ? &ItemsCopy[SlotIndex] : nullptr;
        UWidget* SlotWidget = BuildSlotWidget(SlotIndex, ItemPtr);
        if (UUniformGridSlot* GridSlot = pGrid->AddChildToUniformGrid(SlotWidget))
        {
            const int32 Row = SlotIndex / NumColumns;
            const int32 Col = SlotIndex % NumColumns;
            GridSlot->SetRow(Row);
            GridSlot->SetColumn(Col);
            GridSlot->SetHorizontalAlignment(HAlign_Fill);
            GridSlot->SetVerticalAlignment(VAlign_Fill);
        }
    }
}

// 단일 슬롯 위젯 구축: SizeBox(고정 크기) → Border(테두리/배경) → (아이콘 + 수량 텍스트)
// @param SlotIndex 표시할 슬롯 인덱스(그리드 배치용)
// @param ItemOrNull 아이템이 있으면 포인터, 없으면 nullptr(빈 칸)
UWidget* UInventoryWidget::BuildSlotWidget(int32 SlotIndex, const FInventoryItem* ItemOrNull)
{
    USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass()); // 고정 크기 컨테이너
    Size->SetWidthOverride(SlotSize.X);  // 슬롯 가로 크기 설정
    Size->SetHeightOverride(SlotSize.Y); // 슬롯 세로 크기 설정

    UBorder* Box = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass()); // 배경/테두리 보더
    Box->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.85f)); // 빈칸 배경색(연한 회색)
    Box->SetPadding(FMargin(2.f)); // 내부 여백
    Size->SetContent(Box); // 보더를 SizeBox 콘텐츠로 설정

    if (ItemOrNull) // 해당 슬롯에 아이템이 있는 경우
    {
        // 아이콘 + 수량 텍스트를 세로로 배치(간단 구현): 상단 아이콘, 하단 수량
        UVerticalBox* VB = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass()); // 세로 레이아웃 컨테이너
        Box->SetContent(VB); // 보더 콘텐츠로 배치

        if (ItemOrNull->Icon.IsValid()) // 아이콘 소프트 참조가 유효하면
        {
            UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass()); // 이미지 위젯 생성
            // 소프트 참조는 동기 로드 시 에디터에서만 안전. 런타임 최적화는 비동기 로드 고려.
            UTexture2D* Tex = ItemOrNull->Icon.LoadSynchronous(); // 텍스처 동기 로드 시도
            if (Tex) // 로드 성공 시 브러시 구성
            {
                FSlateBrush Brush; // Slate 브러시 생성
                Brush.SetResourceObject(Tex); // 리소스 할당
                Brush.ImageSize = FVector2D(SlotSize.X - 8.f, SlotSize.Y - 28.f); // 수량 텍스트 공간을 조금 남김
                Icon->SetBrush(Brush); // 이미지에 브러시 적용
            }
            VB->AddChildToVerticalBox(Icon); // 세로 박스에 아이콘 추가
        }
        else
        {
            // 아이콘이 없으면 이름 텍스트를 간단히 표시
            UTextBlock* Name = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); // 텍스트 생성
            Name->SetText(ItemOrNull->DisplayName); // 표시 이름 설정
            VB->AddChildToVerticalBox(Name); // 세로 박스에 추가
        }

        UTextBlock* Qty = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); // 수량 텍스트 생성
        Qty->SetText(FText::FromString(FString::Printf(TEXT("x%d"), ItemOrNull->Quantity))); // "xN" 형식
        VB->AddChildToVerticalBox(Qty); // 세로 박스에 추가(하단)
    }
    else
    {
        // 빈 칸: 보더 배경만 보이고 콘텐츠는 비움(완전 빈 표시를 원하면 브러시 투명 처리 가능)
        UTextBlock* Empty = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); // 빈 텍스트 생성
        Empty->SetText(FText::FromString(TEXT(""))); // 내용 없음
        Box->SetContent(Empty); // 빈 콘텐츠 배치(시각적 간격 유지)
    }

    return Size; // 슬롯 루트 위젯 반환
}

// OnInventoryChanged 래퍼 제거(간소화)
