// Fill out your copyright notice in the Description page of Project Settings.


#include "KillFeedElement.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBoxSlot.h"


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

void UKillFeedElement::SetIsHeadshot(bool bHeadshot)
{
	if (!bHeadshot)
	{
		UWidget* HeadshotWrapper = GetWidgetFromName(TEXT("Headshot_Wrapper"));

		if (HeadshotWrapper)
		{
			HeadshotWrapper->RemoveFromParent();
			if (HeadshotWrapper->IsValidLowLevel())
			{
				HeadshotWrapper->ConditionalBeginDestroy();
			}
		}
	}
}

void UKillFeedElement::HideKillerName()
{
	auto* Widget = Cast<UTextBlock>(GetWidgetFromName(TEXT("KillerWiddget")));

	if (!Widget)
	{
		return;
	}

	Widget->SetRenderOpacity(0.0f);
	Widget->SetText(FText::FromString(TEXT("")));
	
	auto* SlotRef = Cast<UHorizontalBoxSlot>(Widget->Slot);

	if (SlotRef)
	{
		SlotRef->SetPadding(0.0f);
	}
}

