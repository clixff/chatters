// Fill out your copyright notice in the Description page of Project Settings.


#include "HISMManager.h"
#include "../../Core/ChattersGameSession.h"
#include "Kismet/GameplayStatics.h"



// Sets default values
AHISMManager::AHISMManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Main")); 
	SetRootComponent(MainComponent);
	MainComponent->SetMobility(EComponentMobility::Static);
}

// Called when the game starts or when spawned
void AHISMManager::BeginPlay()
{
	Super::BeginPlay();
	
	auto* GameSession = UChattersGameSession::Get();

	if (!GameSession)
	{
		return;
	}

	Activate();
}

// Called every frame
void AHISMManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHISMManager::Activate()
{
	auto* GameSession = UChattersGameSession::Get();
	bool bNightMode = GameSession->TimeOfDay == ETimeOfDay::Night;

	TArray<AActor*> Actors;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), Actors);

	for (auto* Actor : Actors)
	{
		auto* SMActor = Cast<AStaticMeshActor>(Actor);

		if (!SMActor)
		{
			continue;
		}

		auto* StaticMeshComponent = SMActor->GetStaticMeshComponent();

		auto* StaticMesh = StaticMeshComponent->GetStaticMesh();

		if (!StaticMesh || !StaticMeshes.Contains(StaticMesh))
		{
			continue;
		}

		if (bNightMode)
		{
			for (auto WindowMesh : NightModeWindowMashes)
			{
				if (StaticMesh != WindowMesh.Mesh)
				{
					continue;
				}

				if (FMath::RandRange(0, 2) == 0)
				{
					continue;
				}

				if (!WindowMesh.NightRandomMaterials.Num())
				{
					break;
				}

				auto* NightMaterial = WindowMesh.NightRandomMaterials[FMath::RandRange(0, WindowMesh.NightRandomMaterials.Num()-1)];

				StaticMeshComponent->SetMaterial(WindowMesh.MaterialIndex, NightMaterial);
				
				break;
			}
		}

		TArray<UMaterialInterface*> Materials = StaticMeshComponent->GetMaterials();

		FString KeyName = AHISMManager::GeyKeyForStaticMeshComponent(StaticMeshComponent);

		auto* HISMComponent = GetHISMComponent(KeyName, StaticMeshComponent);
		
		FTransform InstanceTransform = StaticMeshComponent->GetComponentTransform();

		HISMComponent->AddInstance(InstanceTransform);

		SMActor->Destroy();
	}
}

FString AHISMManager::GeyKeyForStaticMeshComponent(UStaticMeshComponent* Component)
{
	auto* StaticMesh = Component->GetStaticMesh();
	FString StaticMeshName = StaticMesh->GetName();
	TArray<FString> KeyArray = { StaticMeshName };

	TArray<UMaterialInterface*> Materials = Component->GetMaterials();

	for (auto* Material : Materials)
	{
		FString MaterialName = TEXT("NULL");

		if (Material)
		{
			MaterialName = Material->GetName();
		}

		KeyArray.Add(MaterialName);
	}

	FString KeyName = FString::Join(KeyArray, TEXT("_"));

	return KeyName;
}

UHierarchicalInstancedStaticMeshComponent* AHISMManager::GetHISMComponent(FString Key, UStaticMeshComponent* Component)
{
	UHierarchicalInstancedStaticMeshComponent* Result;

	if (HISMMap.Contains(Key))
	{
		Result = HISMMap[Key];
	}
	else
	{
		Result = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, *Key);
		HISMMap.Add(Key, Result);
		Result->SetStaticMesh(Component->GetStaticMesh());

		auto Materials = Component->GetMaterials();

		for (int32 i = 0; i < Materials.Num(); i++)
		{
			Result->SetMaterial(i, Materials[i]);
		}

		Result->SetCanEverAffectNavigation(Component->CanEverAffectNavigation());
		Result->SetCastShadow(Result->CastShadow);
		Result->SetMobility(EComponentMobility::Static);
		Result->AttachTo(GetRootComponent(), NAME_None, EAttachLocation::SnapToTarget);
		Result->RegisterComponent();
		AddInstanceComponent(Result);
	}

	return Result;
}

