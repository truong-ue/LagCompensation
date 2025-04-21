// Fill out your copyright notice in the Description page of Project Settings.


#include "LAGC_MainAttackComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
ULAGC_MainAttackComponent::ULAGC_MainAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

ALAGC_MainAttackActor* ULAGC_MainAttackComponent::SpawnMainAttack(int32 Speed)
{
	ALAGC_MainAttackActor* PoolableActor;
	IndexMainAttackToSpawn = IndexMainAttackToSpawn + 1;
	if (IndexMainAttackToSpawn <= PoolMainAttackSize - 1)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MainAttack: IndexMainAttackToSpawn : %i"), IndexMainAttackToSpawn);
		PoolableActor = AllMainAttack[IndexMainAttackToSpawn];
		PoolableActor->SetActive(true, Speed);
		return PoolableActor;
	}
	else
	{
		IndexMainAttackToSpawn = 0;
		PoolableActor = AllMainAttack[0];
		PoolableActor->SetActive(true, Speed);
		return PoolableActor;
	}
}

//void ULAGC_MainAttackComponent::OnMainAttackDespawn(ALAGC_MainAttackActor* PoolActor)
//{
	//SpawnedMainAttackIndexes.Remove(PoolActor->GetIndex());
//}

void ULAGC_MainAttackComponent::SpawnMainAttackPool(APawn* Creator)
{
	if (PoolMainAttackSubclass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			for (int i = 0; i < PoolMainAttackSize; i++)
			{
				const FVector Loc = FVector().ZeroVector;
				const FRotator Rot = FRotator().ZeroRotator;
				FTransform SpawnTransform(Rot, Loc);
				auto PoolableActor = Cast<ALAGC_MainAttackActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, PoolMainAttackSubclass, SpawnTransform));

				if (PoolableActor != nullptr)
				{
					UGameplayStatics::FinishSpawningActor(PoolableActor, SpawnTransform);
					
					PoolableActor->SetPawnInsti(Creator);
					
					PoolableActor->SetActive(false, 0);
					PoolableActor->SetIndex(i);
					//PoolableActor->OnMainAttackDespawn.AddDynamic(this, &ULAGC_MainAttackComponent::OnMainAttackDespawn);
					AllMainAttack.Add(PoolableActor);
					UE_LOG(LogTemp, Warning, TEXT("MainAttack added to AllMainAttack, index : %i"), i);
				}
			}
		}
	}
}

void ULAGC_MainAttackComponent::SetPoolMainAttackSubClass(TSubclassOf<class ALAGC_MainAttackActor> SetPoolObject)
{
	PoolMainAttackSubclass = SetPoolObject;
}

