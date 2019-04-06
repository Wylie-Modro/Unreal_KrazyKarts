// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicator.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();
	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}


// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) { return; }

	FGoKartMove LastMove = MovementComponent->GetLastMove();

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	// We are the server and in contol of the pawn
	if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}

void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceLastUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER) { return; }

	float LerpRatio = ClientTimeSinceLastUpdate / ClientTimeBetweenLastUpdate;

	FHermiteCubicSpline Spline = CreateSpline();

	InterpolateLocation(Spline, LerpRatio);
	InterpolateVelocity(Spline, LerpRatio);
	InterpolateRotation(LerpRatio);
}

FHermiteCubicSpline UGoKartMovementReplicator::CreateSpline()
{
	FHermiteCubicSpline Spline;
	Spline.StartDerivative = ClientStartVelocity * ClientTimeBetweenLastUpdate * 100;
	Spline.TargetDerivative = ServerState.Velocity * ClientTimeBetweenLastUpdate * 100;
	Spline.TargetLocation = ServerState.Transform.GetLocation();

	return Spline;
}

void UGoKartMovementReplicator::InterpolateLocation(const FHermiteCubicSpline& Spline, const float& LerpRatio)
{
	FVector NextCurrentLocation = FMath::CubicInterp(StartTransform.GetLocation(), Spline.StartDerivative, Spline.TargetLocation, Spline.TargetDerivative, LerpRatio);

	if (MeshOffsetRoot == nullptr) { return; }
	MeshOffsetRoot->SetWorldLocation(NextCurrentLocation);
}

void UGoKartMovementReplicator::InterpolateVelocity(const FHermiteCubicSpline& Spline, const float& LerpRatio)
{
	FVector NextCurrentDerivative = FMath::CubicInterpDerivative(StartTransform.GetLocation(), Spline.StartDerivative, Spline.TargetLocation, Spline.TargetDerivative, LerpRatio);
	FVector NextCurrentVelocity = NextCurrentDerivative / (ClientTimeBetweenLastUpdate * 100);
	if (MovementComponent == nullptr) { return; }
	MovementComponent->SetVelocity(NextCurrentVelocity);
}

void UGoKartMovementReplicator::InterpolateRotation(const float& LerpRatio)
{
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat NextCurrentRotation = FQuat::Slerp(StartTransform.GetRotation(), TargetRotation, LerpRatio);

	if (MeshOffsetRoot == nullptr) { return; }
	MeshOffsetRoot->SetWorldRotation(NextCurrentRotation);
}


void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoKartMovementReplicator::OnRep_ReplicatedServerState()
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		OnRep_AutoProxy_ReplicatedServerState();
	}  
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		OnRep_SimProxy_ReplicatedServerState();
	}
}

void UGoKartMovementReplicator::OnRep_AutoProxy_ReplicatedServerState()
{
	if (MovementComponent == nullptr) { return; }

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartMovementReplicator::OnRep_SimProxy_ReplicatedServerState()
{
	ClientTimeBetweenLastUpdate = ClientTimeSinceLastUpdate;
	ClientTimeSinceLastUpdate = 0;
	
	if (MeshOffsetRoot != nullptr) 
	{ 
		StartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		StartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}

	if (MovementComponent == nullptr) { return; }
	ClientStartVelocity = MovementComponent->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}


void UGoKartMovementReplicator::ClearAcknowledgedMoves(FGoKartMove pLastMove)
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

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent == nullptr) { return; }

	MovementComponent->SimulateMove(Move);
	 
	ClientTime += Move.DeltaTime;

	UpdateServerState(Move);
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	float ProposedTime = ClientTime + Move.DeltaTime;

	if (ProposedTime > GetWorld()->TimeSeconds)
	{
		UE_LOG(LogTemp, Error, TEXT("Client is cheating: Running too fast"));
		return false;
	}

	if (!Move.IsValid()) 
	{
		UE_LOG(LogTemp, Error, TEXT("Client is cheating: Input Invalid"));
		return false; 
	}

	return true;
}

