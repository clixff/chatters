// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletHolesManager.h"
#include "../Core/ChattersGameSession.h"

int32 ABulletHolesManager::DecalsCounter = 0;

// Sets default values
ABulletHolesManager::ABulletHolesManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(MainComponent);
}

// Called when the game starts or when spawned
void ABulletHolesManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABulletHolesManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

ABulletHolesManager* ABulletHolesManager::Get()
{
	auto* GameSession = UChattersGameSession::Get();

	if (!GameSession)
	{
		return nullptr;
	}

	if (!GameSession->BulletHolesManager)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GameSession->BulletHolesManager = GameSession->GetWorld()->SpawnActor<ABulletHolesManager>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	return GameSession->BulletHolesManager;
}

void ABulletHolesManager::AddDecal(FVector Location, FRotator Rotation, UMaterialInterface* Material)
{
	auto* Singleton = ABulletHolesManager::Get();

	if (Singleton)
	{
		Singleton->AddDecalPrivate(Location, Rotation, Material);
	}
}

void ABulletHolesManager::AddDecalPrivate(FVector Location, FRotator Rotation, UMaterialInterface* Material)
{
	if (DecalComponents.Num() && DecalComponents.Num() >= MaxDecals)
	{
		DecalComponents[0]->DetachFromParent(false, false);
		DecalComponents[0]->UnregisterComponent();
		DecalComponents[0]->DestroyComponent();
		DecalComponents.RemoveAt(0);
	}

	FString ComponentName = FString::Printf(TEXT("BulletHole_%d"), ABulletHolesManager::DecalsCounter);
	ABulletHolesManager::DecalsCounter++;

	auto* DecalComponent = NewObject<UDecalComponent>(this, *ComponentName);

	if (DecalComponent)
	{
		DecalComponents.Add(DecalComponent);
		DecalComponent->AttachTo(GetRootComponent(), NAME_None, EAttachLocation::SnapToTarget);
		DecalComponent->RegisterComponent();
		DecalComponent->SetWorldLocation(Location);
		DecalComponent->SetWorldRotation(Rotation);
		DecalComponent->SetDecalMaterial(Material);
		DecalComponent->DecalSize = FVector(3.0f);
		DecalComponent->SetFadeScreenSize(0.001f);
		AddInstanceComponent(DecalComponent);
	}

}

