// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "Components/VerticalBoxSlot.h"
#include "Animation/WidgetAnimation.h"
#include "KillFeedElement.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UKillFeedElement : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText KillerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText VictimName;

	static int32 NumberOfElements;

	void Init(FString KillerNameString, FString VictimNameString);

	static FName GenerateName();

	void Destroy();

	void Tick(float DeltaTime);

	float SecondsDIsplaying = 0.0f;

	UPROPERTY()
		UVerticalBoxSlot* VerticalBoxSlot = nullptr;

	UPROPERTY(EditDefaultsOnly)
		float MaxSeconds = 5.0f; 

	UPROPERTY(EditDefaultsOnly)
		float FadeInSeconds = 1.0f;

	UPROPERTY(EditDefaultsOnly)
		float FadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UWidgetAnimation* FadeInAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UWidgetAnimation* FadeOutAnimation;

	void StartDestroying();

	bool bDestroying = false;

protected:
	virtual void OnAnimationFinishedPlaying(UUMGSequencePlayer& Player) override;
};