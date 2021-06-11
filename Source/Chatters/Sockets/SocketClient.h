// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class CHATTERS_API FSocketClient
{
public:
	FSocketClient();
	~FSocketClient();

	static FSocketClient* Singleton;

	static FSocketClient* Create();

	static void Destroy();
};
