// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartMovementComponent.h"
#include "GoKart.generated.h"

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

UCLASS()
class UNREAL_KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	void MoveForward(float val);
	void MoveRight(float val);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	void ClearAcknowledgedMoves(FGoKartMove pLastMove);

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_ReplicatedServerState();

	TArray<FGoKartMove> UnacknowledgedMoves;

	UPROPERTY(EditAnywhere)
	UGoKartMovementComponent* MovementComponent;
};
