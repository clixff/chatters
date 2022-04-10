// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Components/DecalComponent.h"
#include "BulletHolesManager.generated.h"

UCLASS()
class CHATTERS_API ABulletHolesManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletHolesManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Get singleton */
	static ABulletHolesManager* Get();

	static void AddDecal(FVector Location, FRotator Rotation, UMaterialInterface* Material);

	UPROPERTY()
		TArray<UDecalComponent*> DecalComponents;

	static int32 DecalsCounter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USceneComponent* MainComponent;
private:
	void AddDecalPrivate(FVector Location, FRotator Rotation, UMaterialInterface* Material);

	int32 MaxDecals = 60;
};
