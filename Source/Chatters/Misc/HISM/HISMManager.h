// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "HISMManager.generated.h"

UCLASS()
class CHATTERS_API AHISMManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHISMManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Activate();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* MainComponent;

	UPROPERTY(VisibleAnywhere)
		TMap<FString, UHierarchicalInstancedStaticMeshComponent*> HISMMap;

	UPROPERTY(EditAnywhere)
		TArray<UStaticMesh*> StaticMeshes;
public:
	static FString GeyKeyForStaticMeshComponent(UStaticMeshComponent* Component);

	UHierarchicalInstancedStaticMeshComponent* GetHISMComponent(FString Key, UStaticMeshComponent* Component);
};
