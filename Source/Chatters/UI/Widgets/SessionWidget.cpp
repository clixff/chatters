// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionWidget.h"
#include "Internationalization/Text.h"
#include "../../Core/ChattersGameInstance.h"
#include "Blueprint/WidgetTree.h"

void USessionWidget::UpdateAliveBotsText(int32 NumberOfAlive, int32 MaxPlayers)
{
	FString AliveBotsString = FString::Printf(TEXT("%d / %d"), NumberOfAlive, MaxPlayers);
	this->AliveBotsText = FText::FromString(AliveBotsString);
}

void USessionWidget::SetStartGameSessionTipVisibility(bool bVisible)
{
	FName TipWidgetName = FName(TEXT("StartGameSessionTip_Wrapper"));
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

	//FString NewRoundString = FString::Printf(TEXT("����� %d"), RoundNumber);
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

void USessionWidget::SetTeamAliveNumber(EBotTeam Team, int32 Number)
{
	FString AliveString = FString::Printf(TEXT("%d"), Number);
	FText AliveText = FText::FromString(AliveString);

	if (Team == EBotTeam::Blue)
	{
		this->BlueTeamAlive = AliveText;
	}
	else if (Team == EBotTeam::Red)
	{
		this->RedTeamAlive = AliveText;
	}
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

void USessionWidget::OnKill(FString KillerName, FString VictimName, FLinearColor KillerColor, FLinearColor VictimColor, FKillFeedIcon& Icon)
{
	if (!this->KillFeedSubclass)
	{
		this->KillFeedSubclass = UKillFeedElement::StaticClass();
	}

	if (!this->KillFeedContainer)
	{
		FName KillFeedContainerName = FName(TEXT("KillFeed_Container"));
		this->KillFeedContainer = Cast<UVerticalBox>(this->GetWidgetFromName(KillFeedContainerName));
		if (!this->KillFeedContainer)
		{
			return;
		}
	}

	UKillFeedElement* KillFeedElement = this->WidgetTree->ConstructWidget<UKillFeedElement>(this->KillFeedSubclass, UKillFeedElement::GenerateName());

	if (!KillFeedElement)
	{
		return;
	}

	KillFeedElement->SetNicknameColors(KillerColor, VictimColor);
	KillFeedElement->SetIcon(Icon);

	UVerticalBoxSlot* VerticalBoxSlot = Cast<UVerticalBoxSlot>(KillFeedContainer->AddChildToVerticalBox(KillFeedElement));

	if (!VerticalBoxSlot)
	{
		return;
	}

	VerticalBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
	VerticalBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	KillFeedElement->VerticalBoxSlot = VerticalBoxSlot;
	KillFeedElement->Init(KillerName, VictimName);
	KillFeedElements.Add(KillFeedElement);

	if (this->KillFeedElements.Num() > this->MaxKillFeedElements)
	{
		auto* FirstElement = this->KillFeedElements[0];

		if (FirstElement)
		{
			FirstElement->StartDestroying();
			this->KillFeedElements[0] = nullptr;
			this->KillFeedElements.RemoveAt(0, 1, true);
		}
	}
}

void USessionWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	for (int32 i = 0; i < this->KillFeedElements.Num(); i++)
	{
		auto* KillFeedElement = this->KillFeedElements[i];

		if (!KillFeedElement || KillFeedElement->bDestroying)
		{
			this->KillFeedElements[i] = nullptr;
			this->KillFeedElements.RemoveAt(i, 1, true);
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
		this->RoundSecondsFloat += DeltaTime;
		this->UpdateRoundSeconds(RoundSecondsFloat);
	}
}