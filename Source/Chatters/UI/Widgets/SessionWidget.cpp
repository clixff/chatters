// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionWidget.h"

void USessionWidget::UpdateAliveBotsText(int32 NumberOfAlive, int32 MaxPlayers)
{
	FString AliveBotsString = FString::Printf(TEXT("%d / %d"), NumberOfAlive, MaxPlayers);
	this->AliveBotsText = FText::FromString(AliveBotsString);
}

void USessionWidget::HideStartGameSessionTip()
{
	FName TipWidgetName = FName(TEXT("StartGameSessionTip_Wrapper"));
	auto* TipWidget = this->GetWidgetFromName(TipWidgetName);

	if (TipWidget)
	{
		TipWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
