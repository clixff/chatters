// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "KillFeedElement.h"
#include "Components/VerticalBox.h"
#include "Animation/WidgetAnimation.h"
#include "../../Misc/Misc.h"
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

	/** "Press space" text */
	void SetStartGameSessionTipVisibility(bool bVisible = false);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText SpectatorBotName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText SpectatorBotHealthPoints;

	void SetSpectatorWidgetVisibility(bool bVisible);

	void UpdateSpectatorBotName(FString BotName);

	void UpdateSpectatorBotHealth(int32 HealthPoints);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText SpectatorBotKillsText;

	void UpdateSpectatorBotKills(int32 NumberOfKills);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FLinearColor SpectatorNicknameColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText NewRoundText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UWidgetAnimation* NewRoundAnimationRef = nullptr;

	void PlayNewRoundAnimation(int32 RoundNumber);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UWidgetAnimation* WinnerAnimation = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText WinnerNameText;

	void PlayWinnerAnimation(FString BotName, FLinearColor BotTeamColor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText FPSText;

	void SetFPS(int32 FPS);

	UPROPERTY(BlueprintReadOnly)
		bool bPlayCommandVisible = true;

	void SetPlayCommandVisibility(bool bVisible);

	UPROPERTY(BlueprintReadOnly)
		FText BlueTeamAlive;

	UPROPERTY(BlueprintReadOnly)
		FText RedTeamAlive;

	void SetTeamAliveNumber(EBotTeam Team, int32 Number);

	void SetTeamsWrapperVisibility(bool bVisible);
public:
	// Kill feed

	UPROPERTY(EditDefaultsOnly, Category = "KillFeed")
		TSubclassOf<UKillFeedElement> KillFeedSubclass;

	UPROPERTY()
		TArray<UKillFeedElement*> KillFeedElements;

	void OnKill(FString KillerName, FString VictimName, FLinearColor KillerColor, FLinearColor VictimColor, FKillFeedIcon& Icon);

	UPROPERTY(EditDefaultsOnly, Category = "KillFeed")
		int32 MaxKillFeedElements = 7;

	UPROPERTY()
		UVerticalBox* KillFeedContainer = nullptr;
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime);

	FManualTimer FPSUpdateTimer = FManualTimer(0.1f);
};