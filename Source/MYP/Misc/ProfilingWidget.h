#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProfilingWidget.generated.h"

UCLASS()
class MYP_API UProfilingWidget : public UUserWidget
{
	GENERATED_BODY()

	/* Method */
public:
	void PrintValue(float value, FString name, AActor* owner);

private:
	void RemoveChannel(const FString& name);



	/* Field */
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Profiling")
	TSubclassOf<class UProfilingChannel> ChannelClass;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UVerticalBox> ChannelBox;

	// Name → Channel 매핑
	UPROPERTY()
	TMap<FString, TObjectPtr<class UProfilingChannel>> Channels;

	// Name → Owner 추적
	TMap<FString, TWeakObjectPtr<AActor>> ChannelOwners;

	static constexpr int32 MaxChannels = 5;

};

template<typename T>
void PrintProfile(UProfilingWidget* widget, T value, FString name, AActor* owner)
{
	if (!IsValid(widget)) return;
	widget->PrintValue(static_cast<float>(value), name, owner);
}
