// Fill out your copyright notice in the Description page of Project Settings.


#include "AnchorField.h"

AAnchorField::AAnchorField()
{
	this->BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	this->BoxComponent->SetCanEverAffectNavigation(false);
	this->BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	this->BoxComponent->SetHiddenInGame(true);
	this->BoxComponent->SetupAttachment(this->GetRootComponent());

	this->Cube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	this->Cube->SetupAttachment(this->BoxComponent);
	this->Cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	this->Cube->SetHiddenInGame(true);

	this->CullingField = CreateDefaultSubobject<UCullingField>(TEXT("CullingField"));
	this->BoxFalloff = CreateDefaultSubobject<UBoxFalloff>(TEXT("BoxFalloff"));
	this->UniformInteger = CreateDefaultSubobject<UUniformInteger>(TEXT("UniformInteger"));

	this->Type = EObjectStateTypeEnum::Chaos_Object_Static;

	//this->SetFieldCommand();
}

AAnchorField::~AAnchorField()
{

}

void AAnchorField::SetFieldCommand(bool bEnabled)
{
	auto* FieldSystemComponentRef = this->GetFieldSystemComponent();
	auto* FieldSystem = FieldSystemComponentRef->GetFieldSystem();
	if (!FieldSystem)
	{
		return;
	}

	this->BoxFalloff->SetBoxFalloff(1.0f, 1.0f, 1.0f, 0.0f, this->GetActorTransform(), EFieldFalloffType::Field_FallOff_None);

	this->UniformInteger->SetUniformInteger(int32(this->Type));

	this->CullingField->SetCullingField(this->BoxFalloff, this->UniformInteger, EFieldCullingOperationType::Field_Culling_Outside);

	FieldSystemComponentRef->AddFieldCommand(bEnabled, EFieldPhysicsType::Field_DynamicState, nullptr, this->CullingField);
}

void AAnchorField::BeginPlay()
{
	Super::BeginPlay();
	//this->SetFieldCommand();
}
