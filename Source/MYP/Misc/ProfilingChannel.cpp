#include "ProfilingChannel.h"

#include "MYP_Graph.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UProfilingChannel::NativeConstruct()
{
	Super::NativeConstruct();
	
	GraphWidget = NewObject<UMYP_Graph>(this);
	if (Channel_Vertical && GraphWidget)
		Channel_Vertical->AddChild(GraphWidget);
}

void UProfilingChannel::SetChannelName(const FString& InName)
{
	if (Text_Name)
		Text_Name->SetText(FText::FromString(InName));
}

void UProfilingChannel::SetValue(float InValue)
{
	if (Text_Value)
		Text_Value->SetText(FText::AsNumber(InValue));
	
	if (IsValid(GraphWidget))
	{
		GraphWidget->AddSample(InValue);
		
	}
	
}
