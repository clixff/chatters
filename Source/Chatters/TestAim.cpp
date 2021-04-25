// Fill out your copyright notice in the Description page of Project Settings.


#include "TestAim.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ATestAim::ATestAim()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->CenterPoint = CreateDefaultSubobject<USphereComponent>(TEXT("Center"));
	this->SetRootComponent(this->CenterPoint);

	this->ArmShoulder = CreateDefaultSubobject<USphereComponent>(TEXT("ArmShoulder"));
	this->ArmShoulder->SetupAttachment(this->CenterPoint);
	this->ArmShoulder->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	this->OutBullet = CreateDefaultSubobject<USphereComponent>(TEXT("OutBullet"));
	this->OutBullet->SetupAttachment(this->CenterPoint);
	this->OutBullet->SetRelativeLocation(FVector(150.0f, 0.0f, 200.0f));

	this->TestPoint = CreateDefaultSubobject<USphereComponent>(TEXT("TestPoint"));
	this->TestPoint->SetupAttachment(this->CenterPoint);
	this->TestPoint->SetRelativeLocation(FVector(150.0f, 0.0f, 70.0f));

}

// Called when the game starts or when spawned
void ATestAim::BeginPlay()
{
	Super::BeginPlay();
	this->DefaultArmLocation = this->ArmShoulder->GetRelativeLocation();
	this->DefaultOutLocation = this->OutBullet->GetRelativeLocation();
	this->ArmOutDiff = this->DefaultOutLocation.Z - this->DefaultArmLocation.Z;
}

// Called every frame
void ATestAim::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	this->Aim();
	this->Draw();
}

void ATestAim::Aim()
{
	if (!this->TargetActor)
	{
		return;
	}

	FVector TargetRelativeLocation = this->TargetActor->GetActorLocation() - this->GetActorLocation();

	float HorizontalRotation = UKismetMathLibrary::FindLookAtRotation(FVector(0.0f), TargetRelativeLocation).Yaw;

	FVector TargetRotatedLocation = FRotator(0.0f, HorizontalRotation * -1.0f, 0.0f).RotateVector(TargetRelativeLocation);

	FRotator ArmTargetRotation = UKismetMathLibrary::FindLookAtRotation(FVector(0.0f, 0.0f, this->DefaultOutLocation.Z), TargetRotatedLocation);

	FVector TestVec = FRotator(0.0f, ArmTargetRotation.Pitch, 0.0f).RotateVector(FVector(this->DefaultOutLocation.Z, 0.0f, 0.0f));

	this->Pitch = FMath::Atan2(TargetRotatedLocation.Z - TestVec.X, TargetRotatedLocation.X + TestVec.Y);
	this->Pitch = FMath::RadiansToDegrees(this->Pitch);

	this->SetActorRotation(FRotator(this->Pitch, HorizontalRotation, 0.0f));


	// Trace
	UWorld* World = this->GetWorld();
	if (!World)
	{
		return;
	}

	FVector RotationVector = this->GetActorRotation().Vector();
	RotationVector.Normalize();

	FVector StarTraceLocation = this->OutBullet->GetComponentLocation();
	
	FVector EndTraceLocation = StarTraceLocation + (RotationVector * 1000.0f);

	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult, StarTraceLocation, EndTraceLocation, ECollisionChannel::ECC_Visibility);

	DrawDebugLine(World, StarTraceLocation, EndTraceLocation, FColor(0, 255, 0), false, -1.0f);

	if (HitResult.bBlockingHit)
	{
		DrawDebugSphere(World, HitResult.ImpactPoint, 10.0f, 6, FColor(255, 0, 0), false, -1.0f);
	}

	DrawDebugLine(World, this->TestPoint->GetComponentLocation(), this->TestPoint->GetComponentLocation() + (RotationVector * 1000.0f), FColor(0, 0, 255), false, -1.0f);

}

void ATestAim::Draw()
{
	UWorld* World = this->GetWorld();
	if (!World)
	{
		return;
	}

	DrawDebugSphere(World, this->ArmShoulder->GetComponentLocation(), 16.0f, 12, FColor(255, 0, 0), false, -1.0f);
	DrawDebugSphere(World, this->OutBullet->GetComponentLocation(), 16.0f, 12, FColor(0, 255, 0), false, -1.0f);
	DrawDebugSphere(World, this->TestPoint->GetComponentLocation(), 16.0f, 12, FColor(0, 0, 255), false, -1.0f);
	DrawDebugSphere(World, this->CenterPoint->GetComponentLocation(), 16.0f, 12, FColor(255, 255, 255), false, -1.0f);

	DrawDebugLine(World, this->ArmShoulder->GetComponentLocation(), this->OutBullet->GetComponentLocation(), FColor(255, 0, 0), false, -1.0f);
	DrawDebugLine(World, this->ArmShoulder->GetComponentLocation(), this->TestPoint->GetComponentLocation(), FColor(255, 0, 0), false, -1.0f);
	DrawDebugLine(World, this->OutBullet->GetComponentLocation(), this->TestPoint->GetComponentLocation(), FColor(255, 0, 0), false, -1.0f);
}