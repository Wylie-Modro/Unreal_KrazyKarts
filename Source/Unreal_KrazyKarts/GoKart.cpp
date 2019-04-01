// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));
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
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementComponent == nullptr) { return; }

	if (Role == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);

		UnacknowledgedMoves.Add(Move);
		MovementComponent->SimulateMove(Move);

		Server_SendMove(Move);
	}

	// We are the server and in contol of the pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}

}


void AGoKart::OnRep_ReplicatedServerState()
{
	if (MovementComponent == nullptr) { return; }

	SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}

	UE_LOG(LogTemp, Warning, TEXT("Updated"));
}

void AGoKart::ClearAcknowledgedMoves(FGoKartMove pLastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > pLastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
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
	if (MovementComponent == nullptr) { return; }

	MovementComponent->SetThrottle(val);
}

void AGoKart::MoveRight(float val)
{
	if (MovementComponent == nullptr) { return; }

	MovementComponent->SetStiringThrow(val);
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent == nullptr) { return; }

	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
//	return FMath::Abs(val) <= 1;
	return true;
	//TODO: Better Implementation  of validate
}

