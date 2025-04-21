// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LAGC_MainAttackActor.h"
#include "Components/ActorComponent.h"
#include "LAGC_MainAttackComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LAGCOMPENSATION_API ULAGC_MainAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULAGC_MainAttackComponent();

	//POOL OBJECT 1
	UFUNCTION(BlueprintCallable, Category = "Pool")
	ALAGC_MainAttackActor* SpawnMainAttack(int32 Speed);

	UPROPERTY(EditAnywhere, Category = "Object Pool")
	TSubclassOf<class ALAGC_MainAttackActor> PoolMainAttackSubclass;

	UPROPERTY(EditAnywhere, Category = "Object Pool")
	int PoolMainAttackSize = 20;

	UPROPERTY()
	int IndexMainAttackToSpawn = 0;

	//UFUNCTION()
	//void OnMainAttackDespawn(ALAGC_MainAttackActor* PoolActor);

	UFUNCTION(BlueprintCallable, Category = "Pool")
	void SpawnMainAttackPool(APawn* Creator);

	UFUNCTION(BlueprintCallable, Category = "Pool")
	void SetPoolMainAttackSubClass(TSubclassOf<class ALAGC_MainAttackActor> SetPoolObject);


protected:
	//POOL OBJECT 01
	TArray<ALAGC_MainAttackActor*> AllMainAttack;
	TArray<int> SpawnedMainAttackIndexes;
		
};