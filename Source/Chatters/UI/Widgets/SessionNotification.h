// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "Components/VerticalBoxSlot.h"
#include "Animation/WidgetAnimation.h"
#include "../../Misc/Misc.h"
#include "SessionNotification.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API USessionNotification : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	static int32 NumberOfElements;

	void Init();

	static FName GenerateName();

	void Destroy();

	void Tick(float DeltaTime);

	FManualTimer DestroyingTimer = FManualTimer(8.0f);

	UPROPERTY()
		UVerticalBoxSlot* VerticalBoxSlot = nullptr;

	UPROPERTY(EditDefaultsOnly)
		float MaxSeconds = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UWidgetAnimation* FadeInAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UWidgetAnimation* FadeOutAnimation;

	void StartDestroying();

	bool bDestroying = false;

	UFUNCTION(BlueprintImplementableEvent)
		void SetAnimationsRefs();
protected:
	virtual void OnAnimationFinishedPlaying(UUMGSequencePlayer& Player) override;
};
