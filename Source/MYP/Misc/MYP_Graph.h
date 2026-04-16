#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "MYP_Graph.generated.h"

class SMyGraphWidget : public SLeafWidget
{
	SLATE_BEGIN_ARGS(SMyGraphWidget) {}

	SLATE_END_ARGS()

public:
	// [1] SNew(SMyGraphWidget) 호출 시 진입 → cpp Construct()
	void Construct(const FArguments& InArgs);

	// [3] Slate 렌더 루프가 매 프레임 호출 → cpp OnPaint()에서 선 그리기
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	// [*] 위젯 크기 요청 시 Slate 레이아웃 시스템이 호출 → cpp ComputeDesiredSize()
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	// [4] UMYP_Graph::AddSample() → 여기로 전달 → ValueHistory에 저장
	void AddSample(float Value);
	
	/* Field */
public:
	
	
	
private:
	TArray<float> ValueHistory;
	
	int32 MaxSamples = 100;
};


UCLASS()
class MYP_API UMYP_Graph : public UWidget
{
	GENERATED_BODY()

	/* Method */
public:
	// [2] UMG가 위젯 빌드 시 호출 → cpp RebuildWidget()에서 SMyGraphWidget 생성 & 캐싱
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// [4] 외부(ProfilingChannel)에서 값 전달 진입점 → cpp AddSample()에서 MyGraphWidgetPtr로 전달
	void AddSample(float value);
private:
	
	
	
	
	/* Field */
public:


	
	
private:
	TSharedPtr<SMyGraphWidget> MyGraphWidgetPtr;
	
	
	
};
