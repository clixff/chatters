// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LeaderboardElement.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API ULeaderboardElement : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText NicknameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText ScoreText;
};
