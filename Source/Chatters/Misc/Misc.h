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