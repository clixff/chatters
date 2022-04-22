// Fill out your copyright notice in the Description page of Project Settings.


#include "KillFeedElement.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"


int32 UKillFeedElement::NumberOfElements = 0;

void UKillFeedElement::Init(FString KillerNameString, FString VictimNameString)
{
	this->KillerName = FText::FromString(KillerNameString);
	this->VictimName = FText::FromString(VictimNameString);

	USessionNotification::Init();

}
void UKillFeedElement::SetNicknameColors(FLinearColor KillerColor, FLinearColor VictimColor)
{
	this->KillerNameColor = KillerColor;
	this->VictimNameColor = VictimColor;
}

void UKillFeedElement::SetIcon(FKillFeedIcon& Icon)
{
	switch (Icon.IconType)
	{
	case EKillFeedIconType::Explosion:
		Icon = this->ExplosionIcon;
		break;
	case EKillFeedIconType::Train:
		Icon = this->TrainIcon;
		break;
	case EKillFeedIconType::Fall:
		Icon = this->FallIcon;
		break;
	case EKillFeedIconType::Walker:
		Icon = this->WalkerIcon;
		break;
	case EKillFeedIconType::Bomber:
		Icon = this->BomberIcon;
		break;
	}

	if (!Icon.Texture)
	{
		return;
	}

	auto* ImageWidget = Cast<UImage>(this->GetWidgetFromName(TEXT("Icon")));

	if (!ImageWidget)
	{
		return;
	}

	ImageWidget->SetBrushFromTexture(Icon.Texture, false);

	auto* CanvasSlot = Cast<UCanvasPanelSlot>(ImageWidget->Slot);

	if (CanvasSlot)
	{
		CanvasSlot->SetSize(FVector2D(Icon.SizeX, Icon.SizeY));
	}
}

