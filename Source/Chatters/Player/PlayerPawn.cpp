// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "../Core/ChattersGameInstance.h"
#include "../Misc/MathHelper.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerPawnController.h"

APlayerPawn* APlayerPawn::Singleton = nullptr;

// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	this->SetRootComponent(this->SphereCollision);

	this->CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	this->CameraBoom->SetupAttachment(this->SphereCollision);
	
	this->Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	this->Camera->SetupAttachment(this->CameraBoom);

	this->MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>(TEXT("MovementComponent"));
	this->MovementComponent->UpdatedComponent = this->SphereCollision;

	this->AIControllerClass = APlayerPawnController::StaticClass();

	BlockControlsOnCinematicCameraTimer.Current = BlockControlsOnCinematicCameraTimer.Max;
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	this->LastZoomValue = this->DefaultAttachedZoom;
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	this->CachedCameraLocation = this->Camera->GetComponentLocation();

	if (this->bReady)
	{
		this->UpdateBotNicknameWidgets();
	}

	if (this->bAttachedToBot && !bFirstPersonCamera)
	{
		this->SetActorRotation(FRotator(0.0f));
	}
	else if (bAttachedToBot && bFirstPersonCamera)
	{
		FRotator Rotation = BotToAttach->FirstPersonOffset.GetRotation().Rotator();

		Rotation = BotToAttach->GetMesh()->GetSocketTransform(TEXT("_head"), ERelativeTransformSpace::RTS_World).GetRotation().Rotator();

		Rotation += BotToAttach->FirstPersonOffset.GetRotation().Rotator();

		Rotation.Pitch += BotToAttach->GetGunPitchRotation();

		SetActorRotation(Rotation);
		SetActorRelativeLocation(BotToAttach->FirstPersonOffset.GetLocation());
	}

	if (CinematicCameraData.bActivated)
	{
		CinematicCameraTick(DeltaTime);
	}

}

void APlayerPawn::AttachToBot(ABot* Bot, bool bNoFirstPerson)
{
	if (Bot)
	{
		if (this->BotToAttach)
		{
			this->BotToAttach->bPlayerAttached = false;
		}

		if (BotToAttach && bFirstPersonCamera)
		{
			ResetAttachedBotHeadVisibility();
		}

		this->BotToAttach = Bot;
		this->BotToAttach->bPlayerAttached = true;

		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, true);
		this->AttachToActor(this->BotToAttach, TransformRules);

		if (!this->bAttachedToBot || (bNoFirstPerson && bFirstPersonCamera))
		{
			if (this->Controller)
			{
				this->Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
			}

			if (this->CameraBoom)
			{
				this->CameraBoom->TargetArmLength = this->LastZoomValue;

				FRotator BotRotation = Bot->GetActorRotation();

				this->CameraBoom->SetRelativeRotation(FRotator(-30.0f, BotRotation.Yaw, 0.0f));
			}
		}
		else
		{
			if (this->CameraBoom)
			{
				FRotator CameraBoomRotation = this->CameraBoom->GetRelativeRotation();
				CameraBoomRotation.Roll = 0.0f;
				this->CameraBoom->SetRelativeRotation(CameraBoomRotation);
			}

			if (!bNoFirstPerson && bFirstPersonCamera)
			{
				SetFirstPersonCamera();
			}
		}

		/** Check is bot visible from camera */

		FRotator RotationToSet = this->FindNewAcceptableCameraRotation(this->CameraBoom->GetRelativeRotation());
		this->CameraBoom->SetRelativeRotation(RotationToSet);

		this->bAttachedToBot = true;
		this->SetSpectatorMenuVisibiliy(true);


		auto* GameSessionRef = GetGameSession();

		if (GameSessionRef)
		{
			GameSessionRef->UpdateBotsByDistance(false, true, Bot->GetActorLocation());
		}
	}
}

void APlayerPawn::DetachFromBot()
{
	if (this->bAttachedToBot)
	{
		this->bAttachedToBot = false;

		if (this->BotToAttach)
		{
			if (bFirstPersonCamera)
			{
				ResetAttachedBotHeadVisibility();
			}

			FVector CameraLocation = this->BotToAttach->GetActorLocation();
			FRotator CameraRotation = this->BotToAttach->GetActorRotation();

			this->BotToAttach->bPlayerAttached = false;

			this->BotToAttach = nullptr;

			if (this->Camera)
			{
				CameraLocation = this->Camera->GetComponentLocation();;
				CameraRotation = this->Camera->GetComponentRotation();
			}

			CameraRotation.Roll = 0.0f;

			if (this->CameraBoom)
			{
				this->CameraBoom->TargetArmLength = 0.0f;
				this->CameraBoom->SetRelativeRotation(FRotator(0.0f));
			}

			this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			this->TeleportTo(CameraLocation, CameraRotation);

			if (this->Controller)
			{
				this->Controller->SetControlRotation(CameraRotation);

			}
		}
		this->SetSpectatorMenuVisibiliy(false);
		
		bFirstPersonCamera = false;
		Camera->FieldOfView = ThirdPersonFOV;
	}
}

float APlayerPawn::GetDistanceFromCamera(FVector Location)
{
	FVector const CameraLocation = this->GetCameraLocation();
	return FVector::Dist(CameraLocation, Location);
}

void APlayerPawn::UpdateBotNicknameWidgets()
{
	auto* GameSessionObject = this->GetGameSession();

	TArray<ABot*> Bots = GameSessionObject->Bots;

	if (!Bots.Num())
	{
		return;
	}

	if (!this->Camera)
	{
		return;
	}

	for (int32 i = 0; i < Bots.Num(); i++)
	{
		ABot* Bot = Bots[i];

		if (!Bot)
		{
			continue;
		}

		FVector const CameraLocation = this->GetCameraLocation();
		FVector const BotLocation = Bot->GetActorLocation();
		float const DistanceFromBot = FVector::Dist(CameraLocation, BotLocation);

		auto* BotNameWidgetComponent = Bot->NameWidgetComponent;

		if (!BotNameWidgetComponent)
		{
			continue;
		}

		auto* BotNameWidget = Cast<UBotNameWidget>(BotNameWidgetComponent->GetWidget());

		if (BotNameWidget)
		{
			float const ClampedDistance = FMath::Clamp(DistanceFromBot, BotNameWidget->MinDistanceToScale, BotNameWidget->MaxDistanceToScale);
			float const DistanceAlpha = (ClampedDistance - BotNameWidget->MinDistanceToScale) / (BotNameWidget->MaxDistanceToScale - BotNameWidget->MinDistanceToScale);

			float NewWidgetSize = FMath::Lerp(BotNameWidget->MaxWrapperScale, BotNameWidget->MinWrapperScale, DistanceAlpha);

			BotNameWidget->UpdateSize(NewWidgetSize);

			float const MinOpacityDistance = BotNameWidget->MaxOpacityDistance.GetLowerBoundValue();
			float const MaxOpacityDistance = BotNameWidget->MaxOpacityDistance.GetUpperBoundValue();

			float const ClampedOpacityDistnace = FMath::Clamp(DistanceFromBot, MinOpacityDistance, MaxOpacityDistance);
			float const OpacityValue = 1.0f - UKismetMathLibrary::NormalizeToRange(ClampedOpacityDistnace, MinOpacityDistance, MaxOpacityDistance);

			BotNameWidget->UpdateOpacity(OpacityValue);
		}

	}

}

void APlayerPawn::SetSpectatorMenuVisibiliy(bool bVisible)
{
	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject)
	{
		auto* SessionWidget = GameSessionObject->GetSessionWidget();

		if (SessionWidget)
		{
			if (bVisible && this->bAttachedToBot && this->BotToAttach)
			{
				SessionWidget->UpdateSpectatorBotName(this->BotToAttach->DisplayName);
				SessionWidget->UpdateSpectatorBotHealth(this->BotToAttach->HealthPoints);
				SessionWidget->UpdateSpectatorBotKills(this->BotToAttach->Kills);
				SessionWidget->SpectatorNicknameColor = this->BotToAttach->GetTeamColor();
			}

			SessionWidget->SetSpectatorWidgetVisibility(bVisible);
		}
	}
}

UChattersGameSession* APlayerPawn::GetGameSession()
{
	if (!this->GameSession)
	{
		auto* GameInstance = UChattersGameInstance::Get();

		if (!GameInstance || GameInstance->GetIsInMainMenu())
		{
			return nullptr;
		}

		this->GameSession = GameInstance->GetGameSession();
	}

	return this->GameSession;
}

FVector APlayerPawn::GetAttachedCameraWorldLocation(float Distance, FRotator CameraRotation)
{
	CameraRotation.Pitch *= -1.0f;
	CameraRotation.Yaw += 180.0f;
	FVector CameraVector = CameraRotation.Vector() * Distance;
	
	FVector BotLocation = FVector(0.0f);

	if (this->BotToAttach)
	{
		BotLocation = this->BotToAttach->GetActorLocation();
	}

	return BotLocation + CameraVector;
}

bool APlayerPawn::IsBotVisibleFromCamera(float Distance, FRotator CameraRotation)
{
	if (!this->BotToAttach)
	{
		return false;
	}

	FVector CameraLocation = this->GetAttachedCameraWorldLocation(Distance, CameraRotation);

	FVector BotLocation = this->BotToAttach->GetActorLocation();
	 
	FHitResult HitResult;
	this->GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, BotLocation, ECollisionChannel::ECC_Camera);

	//FVector TraceEndLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;

	//DrawDebugLine(GetWorld(), CameraLocation, TraceEndLocation, FColor::Red, false, 7.0f);

	auto* HitActor = HitResult.GetActor();

	if (!(HitResult.bBlockingHit && HitActor == this->BotToAttach))
	{
		return false;
	}

	/** Trace from bot to camera */

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this->BotToAttach);
	this->GetWorld()->LineTraceSingleByChannel(HitResult, BotLocation, CameraLocation, ECollisionChannel::ECC_Camera, Params);

	/** Return as visible if there's no collision between bot and camera */
	return !HitResult.bBlockingHit;
}

FRotator APlayerPawn::FindNewAcceptableCameraRotation(FRotator StartRotation)
{
	FRotator RotationToSet = StartRotation;
	float StartYawRotation = FMath::Fmod(RotationToSet.Yaw + 360.0f, 360.0f);

	const float CheckVisibilityAngleAddition = 360.0f / 20.0f;
	const int32 MaxChecks = int32(360.0f / CheckVisibilityAngleAddition);

	float Distance = this->CameraBoom->TargetArmLength;

	for (int32 i = 0; i < MaxChecks; i++)
	{
		float YawRotation = StartYawRotation + (CheckVisibilityAngleAddition * i);
		YawRotation = FMath::Fmod(YawRotation + 360.0f, 360.0f);

		FRotator RotationToCheck = StartRotation;
		RotationToCheck.Yaw = YawRotation;

		/** Check is bot visible from camera */
		bool bVisible = this->IsBotVisibleFromCamera(Distance, RotationToCheck);

		if (bVisible)
		{
			RotationToSet = RotationToCheck;
			break;
		}
	}

	return RotationToSet;
}

void APlayerPawn::CinematicCameraTick(float DeltaTime)
{
	float TimeFactor = (CinematicCameraData.Timer.Current / CinematicCameraData.Timer.Max);
	TimeFactor = FMath::Clamp(TimeFactor, 0.0f, 1.0f);

	FVector StartLocation = CinematicCameraData.StartPoint;
	FVector EndLocation = CinematicCameraData.EndPoint;

	if (!CinematicCameraData.Target)
	{
		ActivateCinematicCamera();
		return;
	}

	FVector BotLocation = CinematicCameraData.Target->GetActorLocation();

	StartLocation += BotLocation;
	EndLocation += BotLocation;

	FVector CurrentLocation = FMath::Lerp(StartLocation, EndLocation, TimeFactor);

	SetActorLocation(CurrentLocation);

	FVector LocationForLookAt = BotLocation;
	auto* Bot = Cast<ABot>(CinematicCameraData.Target);

	if (Bot)
	{
		if (CinematicCameraData.ProjectileActor)
		{
			LocationForLookAt = CinematicCameraData.ProjectileActor->GetActorLocation();
		}
		else
		{
			auto TargetData = Bot->GetTargetData();
			if (TargetData.Actor)
			{
				FVector TargetLocation = TargetData.Actor->GetActorLocation();
				float Dist = FVector::Dist(TargetLocation, BotLocation);

				if (Dist <= 3000.0f)
				{
					LocationForLookAt = TargetData.Actor->GetActorLocation();
				}
			}
		}
	}

	FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, LocationForLookAt);
	Rotation.Roll = 0.0f;

	GetController()->SetControlRotation(Rotation);

	if (!BlockControlsOnCinematicCameraTimer.IsEnded())
	{
		BlockControlsOnCinematicCameraTimer.Add(DeltaTime);
	}
	

	CinematicCameraData.Timer.Add(DeltaTime);
	if (CinematicCameraData.Timer.IsEnded())
	{
		AActor* ActorToAttachCamera = nullptr;
		if (Bot && Bot->CombatAction == ECombatAction::Shooting && FMath::RandRange(0, 1) != 0)
		{
			ActorToAttachCamera = Bot;
		}
		ActivateCinematicCamera(ActorToAttachCamera, false);
		return;
	}
}

void APlayerPawn::ActivateCinematicCamera(AActor* ActorToAttach, bool bBlockCameraControls)
{
	auto* GameSessionRef = GetGameSession();

	if (!GameSessionRef)
	{
		return;
	}

	auto* OldTarget = CinematicCameraData.Target;

	if (OldTarget)
	{
		auto* OldBotTarget = Cast<ABot>(OldTarget);

		if (OldBotTarget)
		{
			OldBotTarget->bCinematicCameraAttached = false;
		}
	}

	if (CinematicCameraData.ProjectileActor)
	{
		GameSessionRef->SetSlomoEnabled(false);
	}

	if (!ActorToAttach)
	{
		auto& AliveBots = GameSessionRef->AliveBots;

		if (!AliveBots.Num())
		{
			return;
		}

		auto* Bot = AliveBots[FMath::RandRange(0, AliveBots.Num() - 1)];
		ActorToAttach = Bot;
	}


	if (!ActorToAttach)
	{
		return;
	}

	auto* Bot = Cast<ABot>(ActorToAttach);

	if (Bot)
	{
		Bot->bCinematicCameraAttached = true;
	}

	FVector Location = ActorToAttach->GetActorLocation();
	FRotator Rotation = ActorToAttach->GetActorRotation();
	Rotation.Pitch = Rotation.Roll = 0.0f;

	FVector StartLocation;
	StartLocation.X = FMath::RandRange(200.0f, 800.0f);
	StartLocation.Y = FMath::RandRange(250.0f, 500.0f);
	StartLocation.Z = FMath::RandRange(-20.0f, 150.0f);

	FVector EndLocation;
	EndLocation.X = FMath::RandRange(200.0f, 800.0f);
	EndLocation.Y = FMath::RandRange(-250.0f, -500.0f);
	EndLocation.Z = FMath::RandRange(-20.0f, 150.0f);

	if (FMath::RandRange(0, 1))
	{
		StartLocation.Y *= -1.0f;
		EndLocation.Y *= -1.0f;
	}

	if (GameSessionRef->bStarted)
	{
		if (FMath::RandRange(0, 1))
		{
			StartLocation.X *= -1.0f;
			StartLocation.Y *= -1.0f;
		}
		else
		{
			EndLocation.X *= -1.0f;
			EndLocation.Y *= -1.0f;
		}

		CinematicCameraData.Timer.Max = FMath::RandRange(5.0f, 7.5f);
	}
	else
	{
		CinematicCameraData.Timer.Max = 4.25f;
		StartLocation.Y /= 3.0f;
		EndLocation.Y /= 3.0f;
	}

	StartLocation = Rotation.RotateVector(StartLocation);
	EndLocation = Rotation.RotateVector(EndLocation);

	CinematicCameraData.bActivated = true;
	CinematicCameraData.StartPoint = StartLocation;
	CinematicCameraData.EndPoint = EndLocation;
	CinematicCameraData.Target = ActorToAttach;
	CinematicCameraData.ProjectileActor = nullptr;

	CinematicCameraData.Timer.Reset();

	SetThirdPersonCamera();
	DetachFromBot();

	GameSessionRef->UpdateBotsByDistance(false, true, Location);

	if (bBlockCameraControls)
	{
		BlockControlsOnCinematicCameraTimer.Current = 0.0f;
	}
}

inline bool APlayerPawn::IsCinematicCameraEnabled()
{
	return CinematicCameraData.bActivated;
}

void APlayerPawn::DeactivateCinematicCamera(bool bAttachToPlayer)
{
	if (CinematicCameraData.Target)
	{
		auto* Bot = Cast<ABot>(CinematicCameraData.Target);

		if (Bot)
		{
			Bot->bCinematicCameraAttached = false;
		}
	}

	if (CinematicCameraData.ProjectileActor)
	{
		auto* GameSessionRef = GetGameSession();
		if (GameSessionRef)
		{
			GameSessionRef->SetSlomoEnabled(false);
		}
	}
	
	auto* OldTarget = CinematicCameraData.Target;
	CinematicCameraData.Target = nullptr;
	CinematicCameraData.ProjectileActor = nullptr;
	if (!CinematicCameraData.bActivated)
	{
		return;
	}

	CinematicCameraData.bActivated = false;
	BlockControlsOnCinematicCameraTimer.Current = BlockControlsOnCinematicCameraTimer.Max;

	if (bAttachToPlayer && OldTarget)
	{
		auto* Bot = Cast<ABot>(OldTarget);
		
		if (Bot && Bot->GetIsAlive())
		{
			AttachToBot(Bot);
		}
	}
}

APlayerPawn* APlayerPawn::Get()
{
	return APlayerPawn::Singleton;
}

void APlayerPawn::Init()
{
	this->bReady = true;
	APlayerPawn::Singleton = this;
}


FVector APlayerPawn::GetCameraLocation()
{
	return this->CachedCameraLocation;
}

void APlayerPawn::RespawnAttachedBot()
{
	if (this->bAttachedToBot && this->BotToAttach)
	{
		this->BotToAttach->RespawnAtRandomPlace();
	}
}

void APlayerPawn::SetThirdPersonCamera()
{
	if (BotToAttach == nullptr)
	{
		return;
	}

	AttachToBot(BotToAttach, true);
	CameraBoom->TargetArmLength = this->LastZoomValue;
	if (Controller)
	{
		Controller->SetControlRotation(FRotator::ZeroRotator);
	}

	bFirstPersonCamera = false;

	ResetAttachedBotHeadVisibility();

	Camera->FieldOfView = ThirdPersonFOV;
}

void APlayerPawn::SetFirstPersonCamera()
{
	if (BotToAttach == nullptr)
	{
		return;
	}

	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
	AttachToComponent(BotToAttach->HeadMesh, TransformRules, TEXT("head_"));
	//Controller->SetControlRotation(FRotator::ZeroRotator);

	Controller->SetControlRotation(BotToAttach->FirstPersonOffset.GetRotation().Rotator());
	SetActorRelativeLocation(BotToAttach->FirstPersonOffset.GetLocation());

	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);

	if (!bFirstPersonCamera)
	{
		ThirdPersonFOV = Camera->FieldOfView;
	}

	Camera->FieldOfView = FirstPersonFOV;

	bFirstPersonCamera = true;

	BotToAttach->HeadMesh->SetHiddenInGame(true);

	if (!BotToAttach->bIsHeadHidden)
	{
		BotToAttach->HeadMesh->bCastHiddenShadow = true;
	}

	if (BotToAttach->HatMesh && BotToAttach->IsHatAttached())
	{
		BotToAttach->HatMesh->SetHiddenInGame(true);
		BotToAttach->HatMesh->bCastHiddenShadow = true;
	}

	if (BotToAttach->BeardMesh)
	{
		BotToAttach->BeardMesh->SetHiddenInGame(true);
		BotToAttach->BeardMesh->bCastHiddenShadow = true;
	}

}

void APlayerPawn::ResetAttachedBotHeadVisibility()
{
	if (BotToAttach)
	{
		if (!BotToAttach->bIsHeadHidden)
		{
			BotToAttach->HeadMesh->SetHiddenInGame(false);
		}
		BotToAttach->HeadMesh->bCastHiddenShadow = false;
		BotToAttach->HatMesh->SetHiddenInGame(false);
		BotToAttach->HatMesh->bCastHiddenShadow = false;
		BotToAttach->BeardMesh->SetHiddenInGame(false);
		BotToAttach->BeardMesh->bCastHiddenShadow = false;
	}
}
