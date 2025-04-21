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

	//Pour calculer la distance entre début et fin de trajectoire
	UPROPERTY()
	FVector_NetQuantize InitialLoc3D;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Pool")
	FVector2D InitialLoc;

	UFUNCTION()
	void OnRep_Trigger();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pool")
	float MaxRange = 5000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pool")
	APawn* PawnInsti;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pool")
	FVector DestinationLocation;

	UFUNCTION(Blueprintcallable, Category = "Pool")
	void MoveTo(FVector Destination, FRotator Rotation, float Speed);

	UFUNCTION(Blueprintcallable, Category = "Pool")
	void MoveToStop();

	UFUNCTION(BlueprintImplementableEvent)
	void BPEvent_Start();

	UFUNCTION()
	void SetPawnInsti(APawn* Insti);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

protected:
	//This variable is needed to retrigger a missile already used on client
	//Need a different var than the InitialLoc because if a hero doesnt move the IniitalLoc doesnt change and thus doesnt trigger
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "OnRep_Trigger", Category = "Pool")
	uint8 Trigger;

	float CalculateSpeed(int32 Speed);

	bool Active;

	int MainAttackIndex;

	FTimerHandle LifeSpanTimer;
	FTimerHandle SearchTimer;

};