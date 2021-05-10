// Fill out your copyright notice in the Description page of Project Settings.


#include "Skybox.h"

// Sets default values
ASkybox::ASkybox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	this->SetRootComponent(this->Root);
	
	this->SphereComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	this->SphereComponent->SetupAttachment(this->Root);

	this->SphereComponent->SetWorldScale3D(FVector(400.0f));
}

// Called when the game starts or when spawned
void ASkybox::BeginPlay()
{
	Super::BeginPlay();
	
	this->UpdateDynamicMaterial();
}

// Called every frame
void ASkybox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
#if WITH_EDITOR
void ASkybox::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


void ASkybox::SetDynamicMaterial()
{
	if (this->BaseMaterial)
	{
		this->DynamicMaterial = UMaterialInstanceDynamic::Create(this->BaseMaterial, this);
		this->SphereComponent->SetMaterial(0, this->DynamicMaterial);
		this->UpdateDynamicMaterial();
	}
}

void ASkybox::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	this->SetDynamicMaterial();
}

void ASkybox::UpdateDynamicMaterial()
{
	if (!this->DynamicMaterial)
	{
		return;
	}

	this->DynamicMaterial->SetVectorParameterValue(TEXT("ZenithColor"), this->ZenithColor);
	this->DynamicMaterial->SetVectorParameterValue(TEXT("HorizonColor"), this->HorizonColor);

	if (this->DirectionalLight)
	{
		this->DynamicMaterial->SetVectorParameterValue(TEXT("Light direction"), FLinearColor(this->DirectionalLight->GetActorRotation().Vector()));
	}
}

