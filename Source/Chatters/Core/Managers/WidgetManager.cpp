// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetManager.h"

UWidgetManager::UWidgetManager()
{
	UE_LOG(LogTemp, Display, TEXT("[UWidgetManager] WidgetManager created"));
}

UWidgetManager::~UWidgetManager()
{
	UE_LOG(LogTemp, Display, TEXT("[UWidgetManager] WidgetManager destroyed"));
}

void UWidgetManager::CreateMainMenuWidget()
{
	if (this->MainMenuWidget == nullptr)
	{
		if (this->MainMenuClass == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UWidgetManager] MainMenuClass is nullptr"));
			return;
		}

		this->MainMenuWidget = UCustomWidgetBase::CreateUserWidget(this->MainMenuClass);
	}

}

void UWidgetManager::RemoveMainMenuWidget()
{
	if (this->MainMenuWidget != nullptr)
	{
		this->MainMenuWidget->Hide();

		if (this->MainMenuWidget->IsValidLowLevel())
		{
			this->MainMenuWidget->ConditionalBeginDestroy();
		}

		this->MainMenuWidget = nullptr;
	}
}
