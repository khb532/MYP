#include "MYP_Graph.h"

// [1] ← SNew(SMyGraphWidget) 진입. 초기화 작업 여기서
void SMyGraphWidget::Construct(const FArguments& InArgs)
{

}

// [3] ← Slate 렌더 루프 진입. ValueHistory → FVector2D 배열 변환 → MakeLines로 그리기
//      OutDrawElements : 그리기 명령을 여기에 추가 (Slate가 나중에 실제 렌더)
//      LayerId         : 현재 레이어 깊이. 겹치는 요소 있으면 +1해서 위에 그려야 함
//      → return LayerId : 사용한 최고 레이어 반환 (부모가 다음 레이어 계산에 사용)
int32 SMyGraphWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                              const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                              const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 실제 할당된 위젯 크기 (px)
	FVector2D Size = AllottedGeometry.GetLocalSize();
	
	// X axis
	TArray<FVector2D> XAxis;
	XAxis.Add(FVector2D(0, Size.Y));
	XAxis.Add(FVector2D(Size.X, Size.Y));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), XAxis, ESlateDrawEffect::None, FColor::Blue, false, 2);
	
	// Y axis
	TArray<FVector2D> YAxis;
	YAxis.Add(FVector2D(0, 0));
	YAxis.Add(FVector2D(0, Size.Y));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), YAxis, ESlateDrawEffect::None, FColor::Blue, false, 2);
	
	// 샘플 2개 미만이면 선을 그을 수 없음
	if (ValueHistory.Num() < 2) return LayerId;

	// Max=0이면 나누기 불가
	float Max = FMath::Max(ValueHistory);
	if (FMath::IsNearlyZero(Max)) return LayerId;


	TArray<FVector2D> Points;
	for (int32 i = 0; i < ValueHistory.Num(); i++)
	{
		// X축: 샘플 N개를 너비 W에 균등 배분
		// 간격 = W / (N-1)  →  i번째 X = i * 간격 = (i / (N-1)) * W
		float ScreenX = (i / (float)(ValueHistory.Num() - 1)) * Size.X;

		// Y축: 값이 클수록 위에 표시 (화면은 위=0이라 반전 필요)
		// 비율 = Value / Max  (0.0~1.0)
		// 반전 = 1 - 비율      (1.0~0.0)
		// 픽셀 = 반전 * H
		float ScreenY = (1.0f - ValueHistory[i] / Max) * Size.Y;

		Points.Add(FVector2D(ScreenX, ScreenY));
	}

	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, FColor::Red, false, 2);

	return LayerId; // → 호출한 Slate 부모에게 반환
}

// [*] ← Slate 레이아웃 시스템 진입. "이 위젯은 이 크기가 필요해요" 알림
//      → return FVector2D : VerticalBox 등 부모 패널이 자식 배치 시 이 값 참고
FVector2D SMyGraphWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(200, 60);
}

// [4] ← UMYP_Graph::AddSample() 진입. ValueHistory에 값 추가
//      슬라이딩 윈도우: MaxSamples 초과 시 가장 오래된 값(앞) 제거
void SMyGraphWidget::AddSample(float Value)
{
	ValueHistory.Add(Value);
	// if (ValueHistory.Num() >= MaxSamples)
	// 	ValueHistory.RemoveAt(0);
}


/*******       UMYP_Graph : UWidget       *******/

// [2] ← UMG 빌드 시 진입. SMyGraphWidget 생성 후 MyGraphWidgetPtr에 캐싱
//      → return TSharedRef<SWidget> : UMG에게 실제 Slate 위젯 전달
TSharedRef<SWidget> UMYP_Graph::RebuildWidget()
{
	TSharedRef<SMyGraphWidget> MyGraphWidgetRef = SNew(SMyGraphWidget);
	MyGraphWidgetPtr = MyGraphWidgetRef;
	return MyGraphWidgetRef;
}

// [4] ← 외부(ProfilingChannel) 진입점. MyGraphWidgetPtr 유효하면 SMyGraphWidget::AddSample()으로 전달
void UMYP_Graph::AddSample(float value)
{
	if (MyGraphWidgetPtr.IsValid())
		MyGraphWidgetPtr->AddSample(value); // → [4] SMyGraphWidget::AddSample()
}
