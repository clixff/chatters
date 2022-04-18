// Fill out your copyright notice in the Description page of Project Settings.


#include "StreetLight.h"

// Sets default values
AStreetLight::AStreetLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MainComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(MainComponent);
	MainComponent->SetMobility(EComponentMobility::Static);


	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(MainComponent);
	StaticMesh->CastShadow = false;
	StaticMesh->SetMobility(EComponentMobility::Static);


	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	PointLight->SetupAttachment(StaticMesh);
	PointLight->SetMobility(EComponentMobility::Movable);
}

// Called when the game starts or when spawned
void AStreetLight::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStreetLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

