// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
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

};
