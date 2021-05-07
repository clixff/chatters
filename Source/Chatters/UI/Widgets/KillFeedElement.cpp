// Fill out your copyright notice in the Description page of Project Settings.


#include "KillFeedElement.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Animation/UMGSequencePlayer.h"

int32 UKillFeedElement::NumberOfElements = 0;

void UKillFeedElement::Init(FString KillerNameString, FString VictimNameString)
{
	this->KillerName = FText::FromString(KillerNameString);
	this->VictimName = FText::FromString(VictimNameString);
	this->SecondsDIsplaying = this->MaxSeconds;
	

	if (this->FadeInAnimation)
	{
		this->PlayAnimation(this->FadeInAnimation, 0.0f, 1);
	}

}

FName UKillFeedElement::GenerateName()
{
	FString WidgetNameString = FString::Printf(TEXT("KillFeed_Element_%d"), UKillFeedElement::NumberOfElements);

	UKillFeedElement::NumberOfElements++;
	return FName(*WidgetNameString);
}

void UKillFeedElement::Destroy()
{
	this->RemoveFromParent();

	if (this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}
}

void UKillFeedElement::Tick(float DeltaTime)
{
	this->SecondsDIsplaying -= DeltaTime;

	if (this->SecondsDIsplaying <= 0.0f)
	{
		this->SecondsDIsplaying = 0.0f;
		this->StartDestroying();
	}
}

void UKillFeedElement::StartDestroying()
{
	if (this->bDestroying)
	{
		return;
	}

	this->bDestroying = true;
	if (this->FadeOutAnimation)
	{
		this->PlayAnimation(this->FadeOutAnimation, 0.0f, 1);
	}
	else
	{
		this->Destroy();
	}
}

void UKillFeedElement::SetNicknameColors(FLinearColor KillerColor, FLinearColor VictimColor)
{
	this->KillerNameColor = KillerColor;
	this->VictimNameColor = VictimColor;
}

void UKillFeedElement::SetIcon(FKillFeedIcon& Icon)
{
	if (Icon.IconType == EKillFeedIconType::Explosion)
	{
		Icon = this->ExplosionIcon;
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

void UKillFeedElement::OnAnimationFinishedPlaying(UUMGSequencePlayer& Player)
{
	Super::OnAnimationFinishedPlaying(Player);

	if (this->FadeOutAnimation && Player.GetAnimation() == this->FadeOutAnimation)
	{
		this->Destroy();
	}
}
