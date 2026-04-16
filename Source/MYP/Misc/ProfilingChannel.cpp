#include "ProfilingChannel.h"
#include "Components/TextBlock.h"

void UProfilingChannel::SetChannelName(const FString& InName)
{
	if (Text_Name)
		Text_Name->SetText(FText::FromString(InName));
}

void UProfilingChannel::SetValue(float InValue)
{
	if (Text_Value)
		Text_Value->SetText(FText::AsNumber(InValue));
}
