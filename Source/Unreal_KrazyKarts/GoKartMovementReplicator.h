// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"
#include "UnrealNetwork.h"
#include "GoKartMovementReplicator.generated.h"


USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGoKartMove LastMove;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREAL_KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TArray<FGoKartMove> UnacknowledgedMoves;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

private: 

	void ClearAcknowledgedMoves(FGoKartMove pLastMove);

	UFUNCTION()
	void OnRep_ReplicatedServerState();
	void OnRep_AutoProxy_ReplicatedServerState();
	void OnRep_SimProxy_ReplicatedServerState();

	void UpdateServerState(const FGoKartMove& Move);

	void ClientTick(float DeltaTime);

	UPROPERTY()
	UGoKartMovementComponent* MovementComponent;

	float ClientTimeSinceLastUpdate;
	float ClientTimeBetweenLastUpdate;

	FTransform StartTransform;
};
