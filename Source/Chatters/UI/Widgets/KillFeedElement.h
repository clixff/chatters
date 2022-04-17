// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "SessionNotification.h"
#include "KillFeedElement.generated.h"

UENUM(BlueprintType)
enum class EKillFeedIconType : uint8
{
	Default,
	Explosion,
	Train,
	Fall
};

USTRUCT(BlueprintType)
struct FKillFeedIcon
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		UTexture2D* Texture = nullptr;
	UPROPERTY(EditAnywhere)
		float SizeX = 8.0f;
	UPROPERTY(EditAnywhere)
		float SizeY = 8.0f;

	EKillFeedIconType IconType = EKillFeedIconType::Default;
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UKillFeedElement : public USessionNotification
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText KillerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText VictimName;

	static int32 NumberOfElements;

	void Init(FString KillerNameString, FString VictimNameString);

	void SetNicknameColors(FLinearColor KillerColor, FLinearColor VictimColor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FLinearColor KillerNameColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FLinearColor VictimNameColor;

	void SetIcon(FKillFeedIcon& Icon);
public:
	UPROPERTY(EditDefaultsOnly, Category="Icons")
		FKillFeedIcon ExplosionIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Icons")
		FKillFeedIcon TrainIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Icons")
		FKillFeedIcon FallIcon;
};
