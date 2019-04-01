// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float StiringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};


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

	void UpdateLocationFromVelocity(float DeltaTime);

	void SimulateMove(const FGoKartMove& Move);

	void ClearAcknowledgedMoves(FGoKartMove pLastMove);

	FGoKartMove CreateMove(float DeltaTime);
	
	FString GetTextOfRole(ENetRole Role);

	FVector GetRollingResistance();
	FVector GetAirResistance();

	// Mass of Car in kgs
	UPROPERTY(EditAnywhere)
	float Mass = 1000.f;

	UPROPERTY(EditAnywhere)
	float DragCoeffient = 5.f;

	UPROPERTY(EditDefaultsOnly)
	float RollingFrictionCoeffient = 0.05;

	// in Newtons
	UPROPERTY(EditAnywhere)
	float MaxThrottleForce = 10000.f;

	// in m 
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 15.f;

	float Throttle;

	float StiringThrow;

	FVector Velocity;

//	UPROPERTY(Replicated)
//	FGoKartMove Move;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_ReplicatedServerState();

	TArray<FGoKartMove> UnacknowledgedMoves;
};
