// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosionFieldSystem.h"

AExplosionFieldSystem::AExplosionFieldSystem()
{
	this->SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	this->SphereComponent->SetupAttachment(this->GetRootComponent());
	this->SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	this->SphereComponent->SetCanEverAffectNavigation(false);
	this->SphereComponent->SetHiddenInGame(true);

	this->OperatorField = CreateDefaultSubobject<UOperatorField>(TEXT("OperatorField"));
	this->ForceRadialFalloff = CreateDefaultSubobject<URadialFalloff>(TEXT("RadialFalloff"));
	this->ForceRadialVector = CreateDefaultSubobject<URadialVector>(TEXT("RadialVector"));


	this->StrainMagnitude = 1000000.0f;
	this->ForceMagnitude = 15000.0f;
}

AExplosionFieldSystem::~AExplosionFieldSystem()
{

}


void AExplosionFieldSystem::BreakChaosObject()
{
	auto* FieldSystemComponentRef = this->GetFieldSystemComponent();
	auto* FieldSystem = FieldSystemComponentRef->GetFieldSystem();
	if (!FieldSystem)
	{
		return;
	}

	FieldSystemComponentRef->ApplyStrainField(true, this->GetActorLocation(), this->SphereComponent->GetScaledSphereRadius(), this->StrainMagnitude, 0);
}

void AExplosionFieldSystem::ActivatePhysicsForce()
{
	auto* FieldSystemComponentRef = this->GetFieldSystemComponent();
	auto* FieldSystem = FieldSystemComponentRef->GetFieldSystem();

	if (!FieldSystem)
	{
		return;
	}

	this->ForceRadialVector->SetRadialVector(this->ForceMagnitude, this->GetActorLocation());
	this->ForceRadialFalloff->SetRadialFalloff(1.0f, 0.0f, 1.0f, 0.0f, this->SphereComponent->GetScaledSphereRadius(), this->GetActorLocation(), EFieldFalloffType::Field_Falloff_Linear);

	this->OperatorField->SetOperatorField(1.0f, this->ForceRadialVector, this->ForceRadialFalloff, EFieldOperationType::Field_Multiply);

	FieldSystemComponent->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearForce, nullptr, this->OperatorField);
}
