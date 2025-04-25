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
	
	void SetActive(bool bIsActive, int32 InSpeed = 0);

	UFUNCTION(NetMulticast, Reliable, Category = "Attack")
	void NetMulti_ToggleTrigger(bool bIsActive, int32 InSpeed, float InActiveServerTime, FVector2D InLocation, float InYaw);
	
	void SetIndex(int Index);
	bool IsActive();
	int GetIndex();

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	uint8 MASpeed = 125; //correspond au missile speed attribute/2
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attack")
	float MaxRange = 5000;

	UFUNCTION(Blueprintcallable, Category = "Attack")
	void MoveTo(FVector Destination, FRotator Rotation, float Speed);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

protected:

	float CalculateSpeed(int32 Speed);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Attack")
	bool Active = false;

	// Trigger event on Client to fire projectile
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	float ActiveServerTime = 0.0f;

	// Yaw
	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	float MAYaw = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	FVector InitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Attack") 
	class ALagCompensationPlayerController* PlayerController;

	int MainAttackIndex = -1;

	FTimerHandle LifeSpanTimer;

public:
	virtual float ComputeRangeOffset(float& TimeDelay);
};

