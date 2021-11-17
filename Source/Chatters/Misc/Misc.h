// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc.generated.h"

enum class EAttachCameraToBotType : uint8
{
	NextBot,
	PrevBot
};

UENUM(BlueprintType)
enum class ESessionMode : uint8
{
	Combat,
	TestAiming
};

UENUM(BlueprintType)
enum class ESessionGameMode : uint8
{
	/** All against all */
	Default,
	Teams,
	/** Bot with more kills in 2 minutes wins  */
	Deathmatch
};

UENUM(BlueprintType)
enum class EBotTeam : uint8
{
	White,
	Blue,
	Red
};

USTRUCT(BlueprintType)
struct FManualTimer
{
	GENERATED_BODY()
public:
	FManualTimer()
	{

	}

	FManualTimer(float MaxSeconds)
	{
		this->Max = MaxSeconds;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float Current = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Max = 10.0f;

	void Add(float DeltaTime)
	{
		this->Current += DeltaTime;
		if (this->Current > this->Max)
		{
			this->Current = this->Max;
		}
	}

	bool IsEnded()
	{
		return this->Current >= this->Max;
	}

	void Reset()
	{
		this->Current = 0.0f;
	}
};

UENUM(BlueprintType)
enum class EGraphicsQualityLevel : uint8
{
	Low,
	Mid,
	High
};

UENUM(BlueprintType)
enum class EKillFeedPosition : uint8
{
	Left,
	Right
};

UENUM(BlueprintType)
enum class ESessionType : uint8
{
	/**
	 * Fill battleground with generated bots
	 */
	Generated	UMETA(DisplayName = "Generated"),

	/**
	 * Spawn twitch viewers
	 */
	 Twitch	UMETA(DisplayName = "Twitch")
};

class FStringHelper
{
public:
	FStringHelper() {  };

	static FString ReplaceAllNonNumbers(FString OriginalString)
	{
		FString OutString = TEXT("");

		TArray<TCHAR> NewStringChars;

		auto CharsList = OriginalString.GetCharArray();

		for (auto& Character : CharsList)
		{
			auto bIsDigit = iswdigit(Character);
			if (bIsDigit)
			{
				NewStringChars.Add(Character);
			}
		}

		OutString = NewStringChars;

		return OutString;
	};
};

USTRUCT(BlueprintType)
struct FTwitchAuthData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
		bool bSignedIn = false;

	UPROPERTY(BlueprintReadOnly)
		FString DisplayName = TEXT("");
};

UENUM(BlueprintType)
enum class EFPSLimitType : uint8
{
	None,
	L_30,
	L_60,
	L_120,
	L_240
};