// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


struct FMathHelper
{
	static FVector SphericalCoordsToCartesian(float Radius, FRotator SphericalCoords)
	{
		FVector OutVector;
		float PitchRadians = FMath::DegreesToRadians(SphericalCoords.Pitch);
		float YawRadians = FMath::DegreesToRadians(SphericalCoords.Yaw);

		OutVector.X = Radius * cosf(YawRadians) * cosf(PitchRadians);

		OutVector.Y = -(Radius * sinf(YawRadians) * cosf(PitchRadians));

		OutVector.Z = Radius * -(sinf(PitchRadians));
		
		return OutVector;
	}
};