// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CustomWidgetBase.h"
#include "Animation/WidgetAnimation.h"

#include "PlayerStatsWidget.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatsType : uint8
{
	MostKills,
	MostShots,
	MostHits,
	Accuracy,
	Barrels,
	Hats,
	DiedFirst,
	MostDamage
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UPlayerStatsWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	void Init();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UWidgetAnimation* InitAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FText WinnerName;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateStatsType(EPlayerStatsType StatsType, const FString& Name, const FString& Number);

	UFUNCTION(BlueprintCallable)
		void ExitToMainMenu();

};
