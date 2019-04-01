// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameStateBase.h"


// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.StiringThrow = StiringThrow;
	Move.Throttle = Throttle;
	
	Move.Time = GetOwner()->GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	return Move;
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& Move)
{

	float ProjectionOfVelocityOnForwardDir = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector());
	float ChangeInRotation = ((Move.DeltaTime * ProjectionOfVelocityOnForwardDir) / MinTurningRadius) * Move.StiringThrow; //in Radians
	FQuat RotationApplied(GetOwner()->GetActorUpVector(), ChangeInRotation);

	Velocity = RotationApplied.RotateVector(Velocity);
	GetOwner()->AddActorWorldRotation(RotationApplied);
	
	FVector ThrottleForce = GetOwner()->GetActorForwardVector() * MaxThrottleForce * Move.Throttle;

	FVector Acceleration = (ThrottleForce + GetAirResistance() + GetRollingResistance()) / Mass;

	Velocity += Acceleration * Move.DeltaTime;

	UpdateLocationFromVelocity(Move.DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetTextOfRole(GetOwner()->Role), GetOwner(), FColor::White, Move.DeltaTime);
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) { return FVector(0); }
	float g = (-1) * (World->GetGravityZ())/100.f; // make postive and get in m/s^2
	return RollingFrictionCoeffient * Mass * g * -(Velocity.GetSafeNormal());
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	return FMath::Square(Velocity.Size()) * DragCoeffient * -Velocity.GetSafeNormal();
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100.f * DeltaTime;

	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	if (HitResult.IsValidBlockingHit())	
	{
		Velocity = FVector::ZeroVector;
	}
}

FString UGoKartMovementComponent::GetTextOfRole(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
		break;
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
		break;
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
		break;
	case ROLE_Authority:
		return "Authority";
		break;
	default:
		return "Error";
		break;
	}
}
