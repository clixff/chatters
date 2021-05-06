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

void USessionWidget::OnKill(FString KillerName, FString VictimName, FLinearColor KillerColor, FLinearColor VictimColor)
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

	KillFeedElement->SetNicknameColors(KillerColor, VictimColor);

	if (!KillFeedElement)
	{
		return;
	}

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
}