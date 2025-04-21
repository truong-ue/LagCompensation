// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LAGC_MainAttackActor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMainAttackDespawn, ALAGC_MainAttackActor*, LAGC_MainAttackComponent);

UCLASS()
class LAGCOMPENSATION_API ALAGC_MainAttackActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALAGC_MainAttackActor();

	//FOnMainAttackDespawn OnMainAttackDespawn;

	UFUNCTION(BlueprintCallable, Category = "Pool")
	void Deactivate();

	void SetActive(bool IsActive, int32 Speed);
	void SetIndex(int Index);
	bool IsActive();
	int GetIndex();

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Pool")
	float MAYaw = 0;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Pool")
	uint8 MASpeed = 125; //correspond au missile speed attribute/2

	//Pour calculer la distance entre d�but et fin de trajectoire
	UPROPERTY()
	FVector_NetQuantize InitialLoc3D = FVector_NetQuantize::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Pool")
	FVector2D InitialLoc = FVector2D::ZeroVector;

	UFUNCTION()
	void OnRep_Trigger();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pool")
	float MaxRange = 5000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pool")
	FVector DestinationLocation = FVector::ZeroVector;

	UFUNCTION(Blueprintcallable, Category = "Pool")
	void MoveTo(FVector Destination, FRotator Rotation, float Speed);

	UFUNCTION(Blueprintcallable, Category = "Pool")
	void MoveToStop();

	UFUNCTION(BlueprintImplementableEvent)
	void BPEvent_Start();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

protected:

	float CalculateSpeed(int32 Speed);

	bool Active;

	// Trigger event on Client to fire projectile
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "OnRep_Trigger", Category = "Pool")
	float ActiveServerTime = 0.0f;

	int MainAttackIndex = -1;

	FTimerHandle LifeSpanTimer;
	FTimerHandle SearchTimer;

public:
	virtual float ComputeRangeOffset(float& TimeDelay);
};

