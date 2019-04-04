// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

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


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREAL_KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(const FGoKartMove& Move);
	
	FGoKartMove CreateMove(float DeltaTime);

	void SetVelocity(FVector vect) { Velocity = vect; }
	FVector GetVelocity() { return Velocity; }

	void SetThrottle(float val) { Throttle = val;  }
	void SetStiringThrow(float val) { StiringThrow = val;  }

	FGoKartMove GetLastMove() { return LastMove; }

private:
	void UpdateLocationFromVelocity(float DeltaTime);

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

	FGoKartMove LastMove;
};
