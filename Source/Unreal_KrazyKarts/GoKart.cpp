// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ServerState);
//	DOREPLIFETIME(AGoKart, Move);
	DOREPLIFETIME(AGoKart, Throttle);
	DOREPLIFETIME(AGoKart, StiringThrow);
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		FGoKartMove Move;
		Move.DeltaTime = DeltaTime;
		Move.StiringThrow = StiringThrow;
		Move.Throttle = Throttle;
		// TODO: Move.Time = idk bro

		Server_SendMove(Move);
	}

	float ProjectionOfVelocityOnForwardDir = FVector::DotProduct(Velocity, GetActorForwardVector());
	float ChangeInRotation = ((DeltaTime * ProjectionOfVelocityOnForwardDir) / MinTurningRadius) * StiringThrow; //in Radians
	FQuat RotationApplied(GetActorUpVector(), ChangeInRotation);

	Velocity = RotationApplied.RotateVector(Velocity);
	AddActorWorldRotation(RotationApplied);
	
	FVector ThrottleForce = GetActorForwardVector() * MaxThrottleForce * Throttle;

	FVector Acceleration = (ThrottleForce + GetAirResistance() + GetRollingResistance()) / Mass;

	Velocity += Acceleration * DeltaTime;

	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		ServerState.Transform = GetActorTransform();
		ServerState.Velocity = Velocity;
		// TODO: GetLastMove
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetTextOfRole(Role), this, FColor::White, DeltaTime);
}

void AGoKart::OnRep_ReplicatedServerState()
{
	SetActorTransform(ServerState.Transform);
	UE_LOG(LogTemp, Warning, TEXT("Updated"));
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

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	Throttle = Move.Throttle;
	StiringThrow = Move.StiringThrow;
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
//	return FMath::Abs(val) <= 1;
	return true;
	//TODO: Better Implementation  of validate
}

FString AGoKart::GetTextOfRole(ENetRole Role)
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
