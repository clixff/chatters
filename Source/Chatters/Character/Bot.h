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
#include "../Combat/FirearmProjectile.h"
#include "../Misc/BloodDecal.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Bot.generated.h"

DECLARE_STATS_GROUP(TEXT("BOTS_Game"), STATGROUP_BOTS, STATCAT_Advanced);

class APlayerPawn;

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
struct FBotTarget
{
	GENERATED_BODY()
public:
	UPROPERTY()
		AActor* Actor = nullptr;
	UPROPERTY()
		ABot* Bot = nullptr;
	UPROPERTY()
	ETargetType TargetType = ETargetType::None;
};

USTRUCT(BlueprintType)
struct FEyesRotation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator LeftEye = FRotator(0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FRotator RightEye = FRotator(0.0f);
};

USTRUCT(BlueprintType)
struct FBotDamagedBonesData
{
	GENERATED_BODY()
public:
	bool bLeftLegDamaged = false;
	bool bRightLegDamaged = false;
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

	virtual void Destroyed() override;


public:
	UPROPERTY(VisibleAnywhere, Category = "Bot")
		FString DisplayName = FString();

	UPROPERTY(VisibleAnywhere)
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

	void SetNewEnemyTarget(ABot* TargetBot);

	void UpdateEquipmentTeamColors();

	void SetEquipment();

	void SpawnBloodParticle(FVector ImpactPoint, FVector CauserLocation);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USoundBase* FallDamageSound = nullptr;

	void SpawnReloadingParticle(UNiagaraSystem* Particle, FTransform Transform);

	/** Useful when bot is stuck */
	void RespawnAtRandomPlace();

	bool CanHatBeDetached();

	void TryDetachHat();

	void DetachHatAfterShot(FBulletHitResult BulletHitResult, float ImpulseForce, FVector ShotLocation);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USoundBase* HatDamageSound = nullptr;

	void AttachHat();
private:
	FTransform HatTransform;

	bool bReady = false;

	void MoveToRandomLocation();

	bool bMovingToRandomLocation = false;

	FVector RandomLocationTarget;

	UBotNameWidget* NameWidget = nullptr;

	FString ChatBubbleMessage;

	bool bHatAttached = false;

	bool bCanHatBeDetached = false;

	float SecondsAfterDeath = 0.0f;


	void DeatachWeapon();

	UChattersGameSession* GetGameSession();

	void FindNewEnemyTarget();


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

	FVector GetFirearmBulletTargetWorldPosition(FVector OutBulletWorldPosition, float BulletDistance, FRotator GunRotation = FRotator(0.0f), bool bShouldRecalculateGunLocation = true, bool bBulletOffset = true, float RecoilFactor = 1.0f);

	FRotator GetGunRotation();

	FBulletHitResult LineTraceFromGun(UFirearmWeaponItem* FirearmRef, bool bBulletOffset, bool bDrawDebugLines = false);

	float TimeSinceStartedMovingInCombat = 0.0f;

	FVector GunAnimationRotationPoint = FVector(0.0f, 0.0f, 10.094025);

	FManualTimer SecondsAimingWithoutHitting = FManualTimer(2.5f);

	UPROPERTY(VisibleAnywhere)
		FManualTimer DefenderSecondsWithoutMoving = FManualTimer(6.0f);
	/** Allow defender to move after 20s */
	float DefenderMaxSecondsWithoutMoving = 10.0f;

	UFUNCTION(BlueprintCallable)
		void MeleeCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool TraceToTargetResult(bool bIgnoreBots = false);

	FManualTimer CombatTickTimeout = FManualTimer(0.25f);

	UPROPERTY(VisibleAnywhere)
		FManualTimer SecondsWithoutMoving = FManualTimer(3.0f);

	FVector LastTickLocation;
	
	/** If once checked the possibility to create the floor blood decal */
	bool bCheckedBloodDecalCreation = false;

	void CreateFloorBloodDecal();

	UPROPERTY()
		ABloodDecal* FloorBloodDecalActor = nullptr;

	FManualTimer AimingTime = FManualTimer(0.2f);


	bool bFallingLastTick = false;

	float FallingStartZLocation = 0.0f;;

	int32 StuckCount = 0;
private:
	UPROPERTY()
		FBotTarget Target;

	float SecondsSinceLastBarrelsCheck = 0.0f;
public:
	static ABot* CreateBot(UWorld* World, FString NameToSet, int32 IDToSet, TSubclassOf<ABot> Subclass, UChattersGameSession* GameSessionObject);
public:
	/** Capsule hitbox for melee fight  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UCapsuleComponent* MeleeHitbox;

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

	void OnDead(ABot* Killer = nullptr, EWeaponType WeaponType = EWeaponType::None, FVector ImpulseVector = FVector(0.0f), FVector ImpulseLocation = FVector(0.0f), FName BoneHit = NAME_None, bool bHeadshot = false);

	UFUNCTION(BlueprintCallable)
		float GetSpeed();

	UFUNCTION(BlueprintCallable)
		float GetSpeedForAnimationBlueprint();

	UFUNCTION(BlueprintCallable)
		UWeaponItem* GetWeaponRef();

	UPROPERTY(EditDefaultsOnly)
		UNiagaraSystem* BloodNiagaraParticle = nullptr;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		UNiagaraSystem* GetDefaultBloodParticle();

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

	void SetMeleeCollisionEnabled(bool bEnabled);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<ABloodDecal> FloorBloodDecalSubclass = nullptr;

	void Clear();

	void RemoveBloodDecal();

	UFUNCTION(BlueprintCallable)
		FEyesRotation GetEyesRotation();
private:
	bool bRigidBodiesSleep = false;

	FManualTimer HatDetachedTimer = FManualTimer(3.5f);

	FManualTimer WeaponDetachTimer = FManualTimer(3.5f);

public:
	/** Should use head animation blueprint or simple Master Pose Component */
	bool bUseDetailedHeadAnimation = false;

	/** Based on distance from camera and other states changes bUseDetailedHeadAnimation value */
	void UpdateHeadAnimationType(float CameraDistance = -1.0f, bool bForce = false);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UAnimInstance> HeadAnimationBlueprint = nullptr;

	FManualTimer BotReviveDeathmatchTimer = FManualTimer(3.0f);

	bool bShouldReviveBot = false;

	void ReviveBotDeatchmatch();

	FManualTimer ReviveCollisionTimer = FManualTimer(3.0f);

	bool bReviveCollisionTimerActive = false;

private:
	/** Set visibility for projectile mesh (for example, arrow) */
	void SetWeaponProjectileMeshVisibility(bool bVisible);

	UPROPERTY()
		TArray<UStaticMeshComponent*> ProjectileMeshesAttached;

	FManualTimer RemoveProjectileMeshesTimeout = FManualTimer(10.0f);
public:
	bool bProjectileMeshExists = false;
	bool bProjectileMeshVisibility = false;

	void AttachProjectileMeshToBody(UStaticMesh* StaticMesh, FVector Location, FRotator Rotation, FName BoneName);

	/** Attached projectile mesh counter */
	int32 ProjectileMeshCounter = 0;

	void RemoveAllAttachedProjectileMeshes();

	bool bWinner = false;

	void WinnerTick(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FManualTimer HitBoneRotationTimer = FManualTimer(2.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCurveFloat* HitBoneRotationCurve = nullptr;

	UPROPERTY()
		TArray<ABloodDecal*> WallBloodDecals;

	int32 MaxWallBloodDecals = 3;

	void AddWallBloodDecal(FHitResult HitResult);

	void RemoveWallBloodDecal(ABloodDecal* Decal);

	void TryAddWallBloodDecal(FVector StartPoint, FVector EndPoint);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FManualTimer DropEquipmentAfterDeathTimer = FManualTimer(0.2f);

	UPROPERTY(EditAnywhere)
		FTransform FirstPersonOffset;

	bool IsHatAttached();

	bool bIsHeadHidden = false;

	FManualTimer DestroyAfterDeathTimer = FManualTimer(30.0f);

	bool bDetailsVisibility = true;

	bool bBotVisible = true;

	/**
	* Set visibility of hat, head, beard, and weapon
	*/
	void SetCharacterDetailsVisible(bool bVisible);

	void UpdateDetailsVisibilityByDistance(float CameraDistance);

	FBotTarget GetTargetData();

	bool bCinematicCameraAttached = false;
private:
	FName DeathBoneName = NAME_None;

	bool bCanSpawnBlood = true;

	UPROPERTY()
		FBotDamagedBonesData DamagedBonesData;

	float GetMaxSpeedForBot(float RequiredSpeed);

	void UpdateBonesDamageData(FName BoneName);

	void ClearDamagedBonesData();

	void SetUseControllerRotationYaw(bool bUse);

	FManualTimer FindNewTargetTimer = FManualTimer(0.1f);

	FManualTimer RagdollAfterDeathTimer = FManualTimer(0.1f);

	bool bInstantFallAfterDeath = false;
};