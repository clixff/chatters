// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionNotification.h"
#include "Animation/UMGSequencePlayer.h"

int32 USessionNotification::NumberOfElements = 0;

void USessionNotification::Init()
{
	this->DestroyingTimer.Max = this->MaxSeconds;

	this->SetAnimationsRefs();

	if (this->FadeInAnimation)
	{
		this->PlayAnimation(this->FadeInAnimation, 0.0f, 1);
	}

}

FName USessionNotification::GenerateName()
{
	FString WidgetNameString = FString::Printf(TEXT("Session_Notification_%d"), USessionNotification::NumberOfElements);

	USessionNotification::NumberOfElements++;
	return FName(*WidgetNameString);
}

void USessionNotification::Destroy()
{
	this->RemoveFromParent();

	if (this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}
}

void USessionNotification::Tick(float DeltaTime)
{
	if (this->bDestroying)
	{
		return;
	}

	this->DestroyingTimer.Add(DeltaTime);

	if (this->DestroyingTimer.IsEnded())
	{
		this->StartDestroying();
	}
}

void USessionNotification::StartDestroying()
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


void USessionNotification::OnAnimationFinishedPlaying(UUMGSequencePlayer& Player)
{
	Super::OnAnimationFinishedPlaying(Player);

	if (this->FadeOutAnimation && Player.GetAnimation() == this->FadeOutAnimation)
	{
		this->Destroy();
	}
}
