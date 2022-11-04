// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionWidget.h"
#include "Internationalization/Text.h"
#include "../../Core/ChattersGameInstance.h"
#include "Session/LeaderboardElement.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"

void USessionWidget::Show()
{
	Super::Show();
}

void USessionWidget::Hide()
{
	if (this->SessionNotificationsContainer)
	{
		auto SessionNotificatonsList = this->SessionNotificationsContainer->GetAllChildren();

		for (auto* Notification : SessionNotificatonsList)
		{
			if (Notification)
			{
				auto SessionNotification = Cast<USessionNotification>(Notification);

				if (SessionNotification)
				{
					if (SessionNotification->bDestroying)
					{
						SessionNotification->Destroy();
					}
				}
			}
		}
	}

	UWidget* NewRoundWidget = this->GetWidgetFromName(TEXT("NewRoundTextWidget"));

	if (NewRoundWidget)
	{
		NewRoundWidget->SetVisibility(ESlateVisibility::Hidden);
	}


	Super::Hide();

}


void USessionWidget::UpdateAliveBotsText(int32 NumberOfAlive, int32 MaxPlayers)
{
	FString AliveBotsString = FString::Printf(TEXT("%d / %d"), NumberOfAlive, MaxPlayers);
	this->AliveBotsText = FText::FromString(AliveBotsString);
}

void USessionWidget::SetStartGameSessionTipVisibility(bool bVisible)
{
	FName TipWidgetName = FName(TEXT("TipsWrapper"));
	auto* TipWidget = this->GetWidgetFromName(TipWidgetName);

	if (TipWidget)
	{
		TipWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USessionWidget::SetSpectatorWidgetVisibility(bool bVisible)
{
	FName WidgetName = FName(TEXT("Spec_Wrapper"));

	auto* SpectatorWidget = this->GetWidgetFromName(WidgetName);

	if (SpectatorWidget)
	{
		SpectatorWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USessionWidget::UpdateSpectatorBotName(FString BotName)
{
	this->SpectatorBotName = FText::FromString(BotName);
}

void USessionWidget::UpdateSpectatorBotHealth(int32 HealthPoints)
{
	if (HealthPoints < 0)
	{
		HealthPoints = 0;
	}

	FString HealthText = FString::Printf(TEXT("%d"), HealthPoints);

	this->SpectatorBotHealthPoints = FText::FromString(HealthText);
}

void USessionWidget::UpdateSpectatorBotKills(int32 NumberOfKills)
{
	FString BotKillsString = FString::Printf(TEXT("%d"), NumberOfKills);

	this->SpectatorBotKillsText = FText::FromString(BotKillsString);
}

void USessionWidget::PlayNewRoundAnimation(int32 RoundNumber)
{
	if (!this->NewRoundAnimationRef)
	{
		return;
	}

	//FString NewRoundString = FString::Printf(TEXT("Раунд %d"), RoundNumber);
	this->NewRoundText = FText::FromStringTable(UChattersGameInstance::CoreStringTablePath, TEXT("NewRound"));
	this->NewRoundText = FText::Format(this->NewRoundText, RoundNumber);

	UWidget* TextWidget = this->GetWidgetFromName(TEXT("NewRoundTextWidget"));
	if (TextWidget)
	{
		TextWidget->SetVisibility(ESlateVisibility::Visible);
	}

	this->PlayAnimation(this->NewRoundAnimationRef, 0.0f, 1);
}

void USessionWidget::PlayWinnerAnimation(FString BotName, FLinearColor BotTeamColor)
{
	UCanvasPanel* WinnerWrapper = Cast<UCanvasPanel>(this->GetWidgetFromName(TEXT("Winner_Wrapper")));

	if (!WinnerWrapper)
	{
		return;
	}

	WinnerWrapper->SetVisibility(ESlateVisibility::Visible);

	this->WinnerNameText = FText::FromString(BotName);

	UTextBlock* WinnerNameTextWidget = Cast<UTextBlock>(this->GetWidgetFromName(TEXT("Winner_BotName")));

	if (WinnerNameTextWidget)
	{
		//WinnerNameTextWidget->SetColorAndOpacity(BotTeamColor);
	}

	if (this->WinnerAnimation)
	{
		this->PlayAnimation(this->WinnerAnimation, 0.0f, 1);
	}
}

void USessionWidget::SetFPS(int32 FPS)
{
	this->FPSText = FText::FromStringTable(UChattersGameInstance::CoreStringTablePath, TEXT("FPS"));
	this->FPSText = FText::Format(this->FPSText, FPS);
}

void USessionWidget::SetPlayCommandVisibility(bool bVisible)
{
	this->bPlayCommandVisible = bVisible;

	auto* PlayCommandTextWidget = this->GetWidgetFromName(TEXT("PlayCommand"));
	
	if (PlayCommandTextWidget)
	{
		PlayCommandTextWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USessionWidget::SetTeamAliveNumber(int32 BlueAlive, int32 RedAlive, int32 BlueMaxAlive, int32 RedMaxAlive)
{
	auto NumberToText = [](int32 Number)
	{
		FString AliveString = FString::Printf(TEXT("%d"), Number);
		return FText::FromString(AliveString);
	};

	this->BlueTeamAlive = NumberToText(BlueAlive);
	this->RedTeamAlive = NumberToText(RedAlive);

	const int32 TotalAlive = BlueAlive + RedAlive;

	const float BlueAlivePercent = FMath::Clamp(float(BlueAlive) / float(BlueMaxAlive), 0.0f, 1.0f);
	const float RedAlivePercent = FMath::Clamp(float(RedAlive) / float(RedMaxAlive), 0.0f, 1.0f);

	auto SetWidgetBarWidth = [this](UWidget** Widget, FName Name, float SizeX)
	{
		if (!*Widget)
		{
			*Widget = this->GetWidgetFromName(Name);
		}

		if (*Widget)
		{
			(*Widget)->SetRenderScale(FVector2D(SizeX, 1.0f));
		}
	};


	SetWidgetBarWidth(&this->BlueAliveBarWidget, TEXT("Blue_Score_Bg"), BlueAlivePercent);
	SetWidgetBarWidth(&this->RedAliveBarWidget, TEXT("Red_Score_Bg"), RedAlivePercent);
}

void USessionWidget::SetTeamsWrapperVisibility(bool bVisible)
{
	auto* TeamsWrapperWidget = this->GetWidgetFromName(TEXT("TeamsAliveWrapper"));

	if (TeamsWrapperWidget)
	{
		TeamsWrapperWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USessionWidget::SetRoundTimerVisibility(bool bVisible)
{
	auto* RoundTimerWidgdet = this->GetWidgetFromName(TEXT("RoundTimer"));

	if (RoundTimerWidgdet)
	{
		RoundTimerWidgdet->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USessionWidget::UpdateRoundSeconds(float Seconds)
{
	this->RoundSecondsFloat = Seconds;
	int32 RoundedSeconds = FMath::Floor(Seconds);

	if (RoundedSeconds == this->RoundSeconds)
	{
		return;
	}

	this->RoundSeconds = RoundedSeconds;

	int32 TotalMinutes = RoundedSeconds / 60;
	int32 TotalSeconds = RoundedSeconds % 60;

	auto SecondsText = TotalSeconds < 10 ? TEXT("%d") : TEXT("%d");

	FString SecondsString = FString::FromInt(TotalSeconds);

	if (TotalSeconds < 10)
	{
		SecondsString = FString(TEXT("0")) + SecondsString;
	}

	FString RoundTimerString = FString::Printf(TEXT("%d:%s"), TotalMinutes, *SecondsString);

	this->RoundTimerText = FText::FromString(RoundTimerString);
}

void USessionWidget::SetNotificationsContainerWidget()
{
	FName ContainerWidgetName = FName(TEXT("Notifications_Container"));
	this->SessionNotificationsContainer = Cast<UVerticalBox>(this->GetWidgetFromName(ContainerWidgetName));
}

UVerticalBoxSlot* USessionWidget::AddNotificationToContainer(USessionNotification* Notification)
{
	UVerticalBoxSlot* VerticalBoxSlot = Cast<UVerticalBoxSlot>(SessionNotificationsContainer->AddChildToVerticalBox(Notification));

	if (!VerticalBoxSlot)
	{
		return nullptr;
	}

	VerticalBoxSlot->SetHorizontalAlignment(this->KillFeedPosition == EKillFeedPosition::Right ? EHorizontalAlignment::HAlign_Right : EHorizontalAlignment::HAlign_Left);
	VerticalBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	Notification->VerticalBoxSlot = VerticalBoxSlot;

	return VerticalBoxSlot;
}

void USessionWidget::OnKill(FString KillerName, FString VictimName, FLinearColor KillerColor, FLinearColor VictimColor, FKillFeedIcon& Icon, bool bHeadshot, bool bHideKillerName)
{
	if (!this->KillFeedSubclass)
	{
		this->KillFeedSubclass = UKillFeedElement::StaticClass();
	}

	if (!this->SessionNotificationsContainer)
	{
		this->SetNotificationsContainerWidget();

		if (!this->SessionNotificationsContainer)
		{
			return;
		}
	}

	UKillFeedElement* KillFeedElement = this->WidgetTree->ConstructWidget<UKillFeedElement>(this->KillFeedSubclass, USessionNotification::GenerateName());

	if (!KillFeedElement)
	{
		return;
	}

	KillFeedElement->Init(KillerName, VictimName);
	KillFeedElement->SetNicknameColors(KillerColor, VictimColor);
	KillFeedElement->SetIcon(Icon);
	KillFeedElement->SetIsHeadshot(bHeadshot);

	if (bHideKillerName)
	{
		KillFeedElement->HideKillerName();
	}

	this->AddNotificationToContainer(KillFeedElement);
	SessionNotifications.Add(KillFeedElement);

	if (SessionNotifications.Num() > this->MaxKillFeedElements)
	{
		auto* FirstElement = SessionNotifications[0];

		if (FirstElement)
		{
			FirstElement->StartDestroying();
			SessionNotifications[0] = nullptr;
			SessionNotifications.RemoveAt(0, 1, true);
		}
	}
}

void USessionWidget::SetKillFeedPosition(EKillFeedPosition Position)
{
	this->KillFeedPosition = Position;

	EHorizontalAlignment HorizontalAlignment = this->KillFeedPosition == EKillFeedPosition::Right ? EHorizontalAlignment::HAlign_Right : EHorizontalAlignment::HAlign_Left;

	for (auto* Notification : SessionNotifications)
	{
		if (Notification && Notification->IsValidLowLevel() && Notification->VerticalBoxSlot)
		{
			Notification->VerticalBoxSlot->SetHorizontalAlignment(HorizontalAlignment);
		}
	}

	auto* LeaderboardWidget = this->GetWidgetFromName(TEXT("Deathmatch_Wrapper_2"));

	if (LeaderboardWidget)
	{
		auto* LeaderboardSlot = Cast<UCanvasPanelSlot>(LeaderboardWidget->Slot);

		if (LeaderboardSlot)
		{
			float AnchorValue = 0.0f;

			if (Position == EKillFeedPosition::Left)
			{
				AnchorValue = 1.0f;
			}

			LeaderboardSlot->SetAnchors(FAnchors(AnchorValue, 0.5f, AnchorValue, 0.5f));
			LeaderboardSlot->SetAlignment(FVector2D(AnchorValue, 0.5f));
		}
	}

}

void USessionWidget::ClearAllNotifications()
{
	for (auto* Notification : SessionNotifications)
	{
		if (Notification && Notification->IsValidLowLevel())
		{
			Notification->StartDestroying();
		}
	}

	this->SessionNotifications.Empty();
}

void USessionWidget::OnViewerJoined(FString Nickname, FLinearColor NameColor)
{
	if (!this->ViewerNotificationClass)
	{
		this->ViewerNotificationClass = UViewerJoinNotification::StaticClass();
	}

	if (!this->SessionNotificationsContainer)
	{
		this->SetNotificationsContainerWidget();

		if (!this->SessionNotificationsContainer)
		{
			return;
		}
	}

	UViewerJoinNotification* Notification = this->WidgetTree->ConstructWidget<UViewerJoinNotification>(this->ViewerNotificationClass, USessionNotification::GenerateName());

	if (!Notification)
	{
		return;
	}

	Notification->Init(Nickname, NameColor);

	this->AddNotificationToContainer(Notification);
	SessionNotifications.Add(Notification);

	if (SessionNotifications.Num() > this->MaxJoiningNotificationElements)
	{
		auto* FirstElement = SessionNotifications[0];

		if (FirstElement)
		{
			FirstElement->StartDestroying();
			SessionNotifications[0] = nullptr;
			SessionNotifications.RemoveAt(0, 1, true);
		}
	}
}

void USessionWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	for (int32 i = 0; i < SessionNotifications.Num(); i++)
	{
		auto* KillFeedElement = SessionNotifications[i];

		if (!KillFeedElement || KillFeedElement->bDestroying)
		{
			SessionNotifications[i] = nullptr;
			SessionNotifications.RemoveAt(i, 1, true);
			i--;
		}
		else
		{
			KillFeedElement->Tick(DeltaTime);
		}
	}

	FPSUpdateTimer.Add(DeltaTime);

	if (FPSUpdateTimer.IsEnded())
	{
		this->SetFPS(FMath::RoundToInt(1.0f / DeltaTime));
		FPSUpdateTimer.Reset();
	}

	if (this->bUpdateRoundTimer)
	{
		//this->RoundSecondsFloat += DeltaTime;
		//this->UpdateRoundSeconds(RoundSecondsFloat);
	}
}

void USessionWidget::SetLeaderboardVisibility(bool bVisible)
{
	auto* Widget = this->GetWidgetFromName(TEXT("Deathmatch_Leaderboard_Wrapper"));

	if (Widget)
	{
		Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

void USessionWidget::UpdateLeaderboard(TArray<FDeathmatchLeaderboardElement>& Leaders)
{
	auto* LeaderboardContainer = this->GetLeaderboardContainer();

	if (!LeaderboardContainer)
	{
		return;
	}

	auto ContainerChildren = LeaderboardContainer->GetAllChildren();

	for (int32 i = 0; i < 5; i++)
	{
		if (i >= ContainerChildren.Num())
		{
			continue;
		}

		auto* Widget = Cast<ULeaderboardElement>(ContainerChildren[i]);

		if (!Widget)
		{
			continue;
		}

		bool bVisible = i < Leaders.Num();

		if (bVisible)
		{
			Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
			Widget->NicknameText = FText::FromString(Leaders[i].Nickname);
			FString ScoreString = FString::FromInt(Leaders[i].Kills);
			Widget->ScoreText = FText::FromString(ScoreString);
		}
		else
		{
			Widget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

UVerticalBox* USessionWidget::GetLeaderboardContainer()
{
	if (!this->LeaderboardVerticalBox)
	{
		this->LeaderboardVerticalBox = Cast<UVerticalBox>(this->GetWidgetFromName(TEXT("Leaderboard_Container")));
	}

	return this->LeaderboardVerticalBox;
}

void USessionWidget::SetLeaderboardTipVisibility(bool bVisible)
{
	auto* Widget = this->GetWidgetFromName(TEXT("Leaderboard_Tip"));

	if (Widget)
	{
		Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}
