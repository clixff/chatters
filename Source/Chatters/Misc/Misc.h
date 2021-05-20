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