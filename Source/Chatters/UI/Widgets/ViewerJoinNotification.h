// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SessionNotification.h"
#include "ViewerJoinNotification.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UViewerJoinNotification : public USessionNotification
{
	GENERATED_BODY()
	
public:
	void Init(FString Name, FLinearColor Color);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FLinearColor NicknameColor;
	
};
