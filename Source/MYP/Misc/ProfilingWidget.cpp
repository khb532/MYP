#include "Misc/ProfilingWidget.h"
#include "Misc/ProfilingChannel.h"
#include "Components/VerticalBox.h"

void UProfilingWidget::PrintValue(float value, FString name, AActor* owner)
{
	// Owner 소멸 체크 — 기존 채널이 있고 Owner가 죽었으면 제거
	if (Channels.Contains(name))
	{
		if (!ChannelOwners[name].IsValid())
		{
			RemoveChannelByName(name);
		}
	}

	// 채널 없으면 새로 생성
	if (!Channels.Contains(name))
	{
		if (Channels.Num() >= MaxChannels) return;
		if (!ChannelClass || !ChannelBox) return;

		UProfilingChannel* NewChannel = CreateWidget<UProfilingChannel>(this, ChannelClass);
		if (!NewChannel) return;

		NewChannel->SetChannelName(name);
		ChannelBox->AddChild(NewChannel);
		Channels.Add(name, NewChannel);
		ChannelOwners.Add(name, owner);
	}
	
	Channels[name]->SetValue(value);
}

void UProfilingWidget::RemoveChannelByName(const FString& name)
{
	if (UProfilingChannel* Channel = Channels.FindRef(name))
	{
		ChannelBox->RemoveChild(Channel);
		Channels.Remove(name);
		ChannelOwners.Remove(name);
	}
}
