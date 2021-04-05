// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UCustomWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void Show();

	virtual void Hide();

	template<class T>
	static T* CreateUserWidget(TSubclassOf<T> WidgetSubclass);

	static APlayerController* GetPlayerController();
};

template<class T>
inline T* UCustomWidgetBase::CreateUserWidget(TSubclassOf<T> WidgetSubclass)
{
	APlayerController* PlayerController = UCustomWidgetBase::GetPlayerController();

	if (PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UCustomWidgetBase::CreateWidget] PlayerController was nullptr"));
		return nullptr;
	}

	T* Widget = CreateWidget<T>(PlayerController, WidgetSubclass);

	return Widget;
}
