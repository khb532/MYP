#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProfilingChannel.generated.h"

UCLASS()
class MYP_API UProfilingChannel : public UUserWidget
{
	GENERATED_BODY()

	/* Method */
public:
	void SetChannelName(const FString& InName);
	void SetValue(float InValue);



private:



	/* Field */
public:



private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> Text_Name;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> Text_Value;

};
