// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"

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

	float ChangeInRotation = DeltaTime * MaxStiringThrow * StiringThrow;
	FQuat RotationApplied(GetActorUpVector(), FMath::DegreesToRadians(ChangeInRotation));

	Velocity = RotationApplied.RotateVector(Velocity);
	AddActorWorldRotation(RotationApplied);
	
	FVector Force = GetActorForwardVector() * MaxThrottleForce * Throttle;
	FVector AirResistanceForce = FMath::Square(Velocity.Size()) * DragCoeffient * -Velocity.GetSafeNormal();

	FVector Acceleration = (Force + AirResistanceForce) / Mass;

	Velocity += Acceleration * DeltaTime;

	UpdateLocationFromVelocity(DeltaTime);
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
