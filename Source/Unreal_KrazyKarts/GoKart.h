// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

	void UpdateLocationFromVelocity(float DeltaTime);

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

	// in degree/s 
//	UPROPERTY(EditAnywhere)
//	float MaxStiringThrow = 90.f;

	float Throttle;
	float StiringThrow;

	FVector Velocity;
};
