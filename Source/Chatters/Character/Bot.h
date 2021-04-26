// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BotController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../UI/Widgets/BotNameWidget.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Equipment/Weapon/WeaponItem.h"
#include "Equipment/Weapon/Instances/WeaponInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Bot.generated.h"

UENUM(BlueprintType)
enum class ECombatAction : uint8
{
	IDLE,
	Shooting,
	Rotating,
	Moving
};

class UChattersGameSession;

UCLASS()
class CHATTERS_API ABot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABot();

	~ABot();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(VisibleAnywhere, Category="Bot")
		FString DisplayName = FString();

	int32 ID;

	UPROPERTY(VisibleAnywhere, Category = "Bot")
		int32 HealthPoints;

	UPROPERTY(VisibleAnywhere, Category = "Bot")
		int32 MaxHealthPoints;

	bool GetIsAlive();

	ABotController* GetAIController();

	void Init(FString NewName, int32 NewID);

	void ApplyDamage(int32 Damage, ABot* ByBot = nullptr, EWeaponType WeaponType = EWeaponType::None, FVector ImpulseVector = FVector(0.0f), FVector ImpulseLocation = FVector(0.0f), FName BoneHit = NAME_None, bool bCritical = false);

	float GetHeathValue();

	void Say(FString Message);

	void OnGameSessionStarted();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		 bool bPlayerAttached = false;

	UPROPERTY(BlueprintReadOnly)
		bool bAlive = true;

	UFUNCTION(BlueprintCallable)
		void OnFootstep();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void PlayFootstepSound(const FVector& Location, EPhysicalSurface Surface);

	UPROPERTY(EditAnywhere)
		UWeaponInstance* WeaponInstance = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
		ABot* TargetTo = nullptr;

	UPROPERTY(VisibleAnywhere)
		ECombatAction CombatAction = ECombatAction::IDLE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float AimingAngle = 50.0f;

	UPROPERTY(EditAnywhere)
		FVector AimAtTestLocation;
private:
	bool bReady = false;

	void SetEquipment();

	void MoveToRandomLocation();

	bool bMovingToRandomLocation = false;

	FVector RandomLocationTarget;

	UBotNameWidget* NameWidget = nullptr;

	UBotNameWidget* GetNameWidget();

	FString ChatBubbleMessage;

	void SayRandomMessage();

	bool bHatAttached = false;

	float SecondsAfterDeath = 0.0f;

	void TryDetachHat();

	void DeatachWeapon();

	UChattersGameSession* GetGameSession();

	void FindNewEnemyTarget();

	void SetNewEnemyTarget(ABot* Target);

	FVector MovingTarget;

	float UpdateMovingTargetTimeout = 0.0f;

	void MoveToTarget();

	void CombatTick(float DeltaTime);

	void FirearmCombatTick(float DeltaTime);

	void Shoot();

	void AimAt(FVector Location);

	void TestAimAt();
	
	bool bTestAiming = false;

	UPROPERTY(EditAnywhere)
		float MinAimingPitchRotation = -80.0f;

	UPROPERTY(EditAnywhere)
		float MaxAimingPitchRotation = 80.0f;

	static const float MinAimRotationValue;
	static const float MaxAimRotationValue;
	
	AActor* AimingTarget = nullptr;

	FVector GunSocketRelativeLocation;

	FVector CombatRandomLocation;

	bool bMovingToRandomCombatLocation = false;

	UCharacterMovementComponent* GetCharacterMovementComponent();
public:
	static ABot* CreateBot(UWorld* World, FString NameToSet, int32 IDToSet, TSubclassOf<ABot> Subclass, UChattersGameSession* GameSessionObject);
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		USkeletalMeshComponent* HeadMesh;

	/** UI Widget with display name and health points */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UWidgetComponent* NameWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* HatMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* BeardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* WeaponMesh;

	void OnDead(ABot* Killer = nullptr, EWeaponType WeaponType = EWeaponType::None, FVector ImpulseVector = FVector(0.0f), FVector ImpulseLocation = FVector(0.0f), FName BoneHit = NAME_None);


};
