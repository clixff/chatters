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
#include "Equipment/Weapon/FirearmWeaponItem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "../Props/ExplodingBarrel.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../Misc/Misc.h"
#include "Components/BoxComponent.h"
#include "Bot.generated.h"


UENUM(BlueprintType)
enum class EYawRotatingType : uint8
{
	Clockwise,
	CounterClockwise
};

UENUM(BlueprintType)
enum class ECombatAction : uint8
{
	IDLE,
	Shooting,
	Rotating,
	Moving
};

UENUM(BlueprintType)
enum class ECombatStyle : uint8
{
	/**
	 * When in gunfight, stands at one place and shoots
	 */
	Defense,
	/**
	 * When in gunfight, moving around target and shoots
	 */
	 Attack
};

UENUM()
enum class ETargetType : uint8
{
	None,
	Bot,
	ExplodingBarrel
};

USTRUCT()
struct FSmoothRotation
{
	GENERATED_BODY()
public:
	bool bActive = false;
	FRotator Target = FRotator(0.0f);
	EYawRotatingType YawType = EYawRotatingType::Clockwise;
	/** Yaw rotation without limits */
	float CurrentYaw = 0.0f;
};

USTRUCT()
struct FBulletHitResult
{
	GENERATED_BODY()
public:
	FHitResult HitResult;
	ABot* BotToDamage = nullptr;
	AExplodingBarrel* ExplodingBarrel = nullptr;
};

USTRUCT()
struct FBotTarget
{
	GENERATED_BODY()
public:
	AActor* Actor = nullptr;
	ABot* Bot = nullptr;
	ETargetType TargetType = ETargetType::None;
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
	UPROPERTY(VisibleAnywhere, Category = "Bot")
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

	void OnGameSessionStarted(ESessionMode SessionMode);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bPlayerAttached = false;

	UPROPERTY(BlueprintReadOnly)
		bool bAlive = true;

	UFUNCTION(BlueprintCallable)
		void OnFootstep();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void PlayFootstepSound(const FVector& Location, EPhysicalSurface Surface);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UWeaponInstance* WeaponInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ECombatAction CombatAction = ECombatAction::IDLE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float AimingAngle = 50.0f;

	UPROPERTY(EditAnywhere)
		FVector AimAtTestLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EBotTeam Team = EBotTeam::White;

	UBotNameWidget* GetNameWidget();

	void SayRandomMessage();

	EWeaponType GetWeaponType();

	bool IsEnemy(ABot* BotToCheck);
private:
	bool bReady = false;

	void SetEquipment();

	void MoveToRandomLocation();

	bool bMovingToRandomLocation = false;

	FVector RandomLocationTarget;

	UBotNameWidget* NameWidget = nullptr;

	FString ChatBubbleMessage;

	bool bHatAttached = false;

	float SecondsAfterDeath = 0.0f;

	void TryDetachHat();

	void DeatachWeapon();

	UChattersGameSession* GetGameSession();

	void FindNewEnemyTarget();

	void SetNewEnemyTarget(ABot* TargetBot);

	FVector MovingTarget;

	float UpdateMovingTargetTimeout = 0.0f;

	void MoveToTarget();

	void CombatTick(float DeltaTime);

	ECombatStyle CombatStyle = ECombatStyle::Defense;

	void FirearmCombatTick(float DeltaTime, float TargetDist);

	void MeleeCombatTick(float DeltaTime, float TargetDist);

	void Shoot(bool bBulletOffset = true);

	void MeleeHit();

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

	FSmoothRotation SmoothRotation;

	void SmoothRotatingTick(float DeltaTime);

	FVector RandomPointToMoveWhileAiming = FVector(0.0f);

	bool bTestAimingMovingToCenter = true;

	void TestAimingTick(float DeltaTime);

	FVector GetFirearmOutBulletWorldPosition(FRotator GunRotation = FRotator(0.0f), bool bShouldRecalculateGunLocation = true);

	FVector GetFirearmBulletTargetWorldPosition(FVector OutBulletWorldPosition, float BulletDistance, FRotator GunRotation = FRotator(0.0f), bool bShouldRecalculateGunLocation = true, bool bBulletOffset = true);

	FRotator GetGunRotation();

	FBulletHitResult LineTraceFromGun(UFirearmWeaponItem* FirearmRef, bool bBulletOffset, bool bDrawDebugLines = false);

	float TimeSinceStartedMovingInCombat = 0.0f;

	FVector GunAnimationRotationPoint = FVector(0.0f, 0.0f, 10.094025);

	bool bSmoothRotatingBeforeMoving = false;

	float SecondsAimingWithoutHitting = 0.0f;
	float MaxSecondsAimingWithoutHitting = 5.0f;

	UPROPERTY(VisibleAnywhere)
		float DefenderSecondsWithoutMoving = 0.0f;
	/** Allow defender to move after 20s */
	float DefenderMaxSecondsWithoutMoving = 10.0f;

	UFUNCTION(BlueprintCallable)
		void MeleeCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


private:
	FBotTarget Target;

	float SecondsSinceLastBarrelsCheck = 0.0f;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UBoxComponent* MeleeCollision = nullptr;

	void OnDead(ABot* Killer = nullptr, EWeaponType WeaponType = EWeaponType::None, FVector ImpulseVector = FVector(0.0f), FVector ImpulseLocation = FVector(0.0f), FName BoneHit = NAME_None);

	UFUNCTION(BlueprintCallable)
		float GetSpeed();

	UFUNCTION(BlueprintCallable)
		float GetSpeedForAnimationBlueprint();

	UFUNCTION(BlueprintCallable)
		UWeaponItem* GetWeaponRef();

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* BloodParticle = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bShouldApplyGunAnimation = false;

	UPROPERTY()
		int32 Kills = 0;

	UFUNCTION(BlueprintCallable)
		float GetGunPitchRotation();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float AimingCenterBoneRotation = 2.918813f;

	UFUNCTION(BlueprintCallable)
		bool ShouldPlayWeaponHitAnimation();

	UFUNCTION(BlueprintCallable)
		bool ShouldPlayWeaponReloadingAnimation();

	void ResetOnNewRound();

	FLinearColor GetTeamColor();

	void UpdateNameColor();

	void StopMovementAfterRound();
	
	void StopMovement();

	/** Check for bots around */
	bool CanExplodeBarrel(AExplodingBarrel* Barrel);
};