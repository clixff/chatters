// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

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

