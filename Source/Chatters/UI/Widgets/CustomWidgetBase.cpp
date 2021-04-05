// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomWidgetBase.h"
#include "../../Core/ChattersGameInstance.h"


void UCustomWidgetBase::Show()
{
	UE_LOG(LogTemp, Display, TEXT("[UCustomWidgetBase] Show widget %s"), *this->GetName());
	if (this->GetIsVisible() == false)
	{
		this->AddToViewport();
	}
}


void UCustomWidgetBase::Hide()
{
	UE_LOG(LogTemp, Display, TEXT("[UCustomWidgetBase] Hide widget %s"), *this->GetName());
	if (this->GetIsVisible() == true)
	{
		this->RemoveFromViewport();
	}
}

APlayerController* UCustomWidgetBase::GetPlayerController()
{
	APlayerController* PlayerController = UChattersGameInstance::GetPlayerController();

	return PlayerController;
}
