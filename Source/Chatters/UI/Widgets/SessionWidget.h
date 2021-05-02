// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "KillFeedElement.h"
#include "Components/VerticalBox.h"
#include "SessionWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API USessionWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:

	void UpdateAliveBotsText(int32 NumberOfAlive, int32 MaxPlayers);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText AliveBotsText;

	/** Hide "Press space" text */
	void HideStartGameSessionTip();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText SpectatorBotName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText SpectatorBotHealthPoints;

	void SetSpectatorWidgetVisibility(bool bVisible);

	void UpdateSpectatorBotName(FString BotName);

	void UpdateSpectatorBotHealth(int32 HealthPoints);

	FText SpectatorBotKillsText;

	void UpdateSpectatorBotKills(int32 NumberOfKills);
public:
	// Kill feed

	UPROPERTY(EditDefaultsOnly, Category="KillFeed")
		TSubclassOf<UKillFeedElement> KillFeedSubclass;

	UPROPERTY()
		TArray<UKillFeedElement*> KillFeedElements;

	void OnKill(FString KillerName, FString VictimName);

	UPROPERTY(EditDefaultsOnly, Category = "KillFeed")
		int32 MaxKillFeedElements = 7;

	UPROPERTY()
		UVerticalBox* KillFeedContainer = nullptr;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime);
};
