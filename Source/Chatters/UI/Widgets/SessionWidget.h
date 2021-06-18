// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "KillFeedElement.h"
#include "Components/VerticalBox.h"
#include "Animation/WidgetAnimation.h"
#include "../../Misc/Misc.h"
#include "ViewerJoinNotification.h"
#include "SessionNotification.h"
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

	UPROPERTY()
		UWidget* BlueAliveBarWidget = nullptr;

	UPROPERTY()
		UWidget* RedAliveBarWidget = nullptr;

	void SetTeamAliveNumber(int32 BlueAlive, int32 RedAlive);

	void SetTeamsWrapperVisibility(bool bVisible);

	bool bUpdateRoundTimer = false;

	float RoundSecondsFloat = 0.0f;
	int32 RoundSeconds = 0;

	void SetRoundTimerVisibility(bool bVisible);

	void UpdateRoundSeconds(float Seconds);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText RoundTimerText = FText::FromString(TEXT("0:00"));

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetStreamerJoinTipVisible(bool bVisible);

public:
	// Notifications

	UPROPERTY()
		TArray<USessionNotification*> SessionNotifications;

	void SetNotificationsContainerWidget();

	UVerticalBoxSlot* AddNotificationToContainer(USessionNotification* Notification);
public:
	// Kill feed

	UPROPERTY(EditDefaultsOnly, Category = "Notifications | KillFeed")
		TSubclassOf<UKillFeedElement> KillFeedSubclass;

	void OnKill(FString KillerName, FString VictimName, FLinearColor KillerColor, FLinearColor VictimColor, FKillFeedIcon& Icon);

	UPROPERTY(EditDefaultsOnly, Category = "Notifications | KillFeed")
		int32 MaxKillFeedElements = 7;

	UPROPERTY()
		UVerticalBox* SessionNotificationsContainer = nullptr;

	UPROPERTY()
		EKillFeedPosition KillFeedPosition = EKillFeedPosition::Right;

	void SetKillFeedPosition(EKillFeedPosition Position);

	void ClearAllNotifications();
public:
	// Viewer notifictaions

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notifications | Joining")
		TSubclassOf<UViewerJoinNotification> ViewerNotificationClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notifications | Joining")
		int32 MaxJoiningNotificationElements = 10;

	void OnViewerJoined(FString Nickname, FLinearColor NameColor);
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime);

	FManualTimer FPSUpdateTimer = FManualTimer(0.1f);
};