// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewerJoinNotification.h"

void UViewerJoinNotification::Init(FString Name, FLinearColor Color)
{
	this->DisplayName = FText::FromString(Name);
	this->NicknameColor = Color;

	USessionNotification::Init();
}