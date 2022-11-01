// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotAnimInstance.h"

void URobotAnimInstance::AnimTick(float DeltaTime)
{
	auto* Robot = GetRobotRef();

	if (!Robot)
	{
		return;
	}

	auto* Bot = Robot->BotOwner;

	if (!Bot)
	{
		return;
	}

	Speed = Robot->GetVelocity().Size();

	AimingAngle = Bot->AimingAngle;

	if (Bot->CombatAction != ECombatAction::Shooting)
	{
		AimingAngle = 50.0f;
	}

	GunPitchRotation = Bot->GetGunPitchRotation();

}

ARobot* URobotAnimInstance::GetRobotRef()
{
	if (!RobotRef)
	{
		auto* PawnOwner = TryGetPawnOwner();

		RobotRef = Cast<ARobot>(PawnOwner);
	}

	return RobotRef;
}
