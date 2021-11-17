// Fill out your copyright notice in the Description page of Project Settings.


#include "BotHeadAnimInstance.h"

void UBotHeadAnimInstance::AnimTick(float DeltaTime)
{
	auto* Bot = this->GetBotRef();

	if (!Bot)
	{
		return;
	}

	this->BodyRef = Bot->GetMesh();

	this->EyesRotaion = Bot->GetEyesRotation();
}

ABot* UBotHeadAnimInstance::GetBotRef()
{
	if (!this->BotRef)
	{
		this->BotRef = Cast<ABot>(this->TryGetPawnOwner());
	}

	return this->BotRef;
}
