// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//float ChangeInRotation = DeltaTime * MaxStiringThrow * StiringThrow;
	float ProjectionOfVelocityOnForwardDir = FVector::DotProduct(Velocity, GetActorForwardVector());
	float ChangeInRotation = ((DeltaTime * ProjectionOfVelocityOnForwardDir) / MinTurningRadius) * StiringThrow; //in Radians
	FQuat RotationApplied(GetActorUpVector(), ChangeInRotation);

	Velocity = RotationApplied.RotateVector(Velocity);
	AddActorWorldRotation(RotationApplied);
	
	FVector ThrottleForce = GetActorForwardVector() * MaxThrottleForce * Throttle;

	FVector Acceleration = (ThrottleForce + GetAirResistance() + GetRollingResistance()) / Mass;

	Velocity += Acceleration * DeltaTime;

	UpdateLocationFromVelocity(DeltaTime);
}

FVector AGoKart::GetRollingResistance()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) { return FVector(0); }
	float g = (-1) * (World->GetGravityZ())/100.f; // make postive and get in m/s^2
	return RollingFrictionCoeffient * Mass * g * -(Velocity.GetSafeNormal());
}

FVector AGoKart::GetAirResistance()
{
	return FMath::Square(Velocity.Size()) * DragCoeffient * -Velocity.GetSafeNormal();
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100.f * DeltaTime;

	FHitResult HitResult;
	AddActorWorldOffset(Translation, true, &HitResult);

	if (HitResult.IsValidBlockingHit())	
	{
		Velocity = FVector::ZeroVector;
	}
}


// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float val)
{
	Throttle = val;
}

void AGoKart::MoveRight(float val)
{
	StiringThrow = val;
}
