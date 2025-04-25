// Fill out your copyright notice in the Description page of Project Settings.

#include "LAGC_MainAttackActor.h"

#include "LagCompensationPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALAGC_MainAttackActor::ALAGC_MainAttackActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Active = false;
}
 
void ALAGC_MainAttackActor::Deactivate()
{
	SetActive(false);
	//OnMainAttackDespawn.Broadcast(this);
	// 
	//All MainAttacks are DORMANT so we ForceNetUpdate when needed (here when we deactivate it).
	//ForceNetUpdate();
}

void ALAGC_MainAttackActor::SetActive(bool bIsActive, int32 InSpeed)
{
	Active = bIsActive;
	if (HasAuthority())
	{
		ActiveServerTime = UGameplayStatics::GetTimeSeconds(GetWorld());
		APawn* LocalInstigator = GetInstigator();
		InitLocation = LocalInstigator->GetActorLocation();
		MAYaw = LocalInstigator->GetActorRotation().Yaw;
		
		NetMulti_ToggleTrigger(bIsActive, InSpeed, ActiveServerTime, FVector2D(InitLocation.X, InitLocation.Y), MAYaw);
	}
}

void ALAGC_MainAttackActor::NetMulti_ToggleTrigger_Implementation(bool bIsActive, int32 InSpeed, float InActiveServerTime, FVector2D InLocation,
	float InYaw)
{
	SetActorHiddenInGame(!bIsActive);
	SetActorEnableCollision(bIsActive);
	if (bIsActive)
	{
		MASpeed = InSpeed;
		ActiveServerTime = InActiveServerTime;
		InitLocation = FVector(InLocation, 0.0f);
		MAYaw = InYaw;

		FRotator Rotation = FRotator(0.0f, MAYaw, 0.0f);
		FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);

		float OverTime = CalculateSpeed(MASpeed);
		//Compute RangeOffset
		if (HasAuthority())
		{
			// Move on Server side
			FVector Destination = InitLocation + Direction * MaxRange;
			
			//we teleport the MainAttack to the offset location
			TeleportTo(InitLocation, Rotation, true, true);

			//we move it
			MoveTo(Destination, Rotation, OverTime);
			
			// Deactivate after overtime
			GetWorldTimerManager().ClearTimer(LifeSpanTimer);
			GetWorldTimerManager().SetTimer(LifeSpanTimer, this, &ThisClass::Deactivate, OverTime, false);
		}
		else
		{
			// Move On Client side (lag compensation)
			float TimeDelay = 0.0f;
			float RangeOffset = ComputeRangeOffset(TimeDelay);
	
			FVector InitLoc = InitLocation + Direction * RangeOffset;
			FVector Destination = InitLoc + Direction * MaxRange;
	
			//we teleport the MainAttack to the offset location
			TeleportTo(InitLoc, Rotation, true, true);
			//we move it (for clients)
			MoveTo(Destination, Rotation, OverTime);

			//Predict deactivate on Client
			GetWorldTimerManager().ClearTimer(LifeSpanTimer);
			GetWorldTimerManager().SetTimer(LifeSpanTimer, this, &ThisClass::Deactivate, OverTime - TimeDelay, false);
		}
	}
	else
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
	}
}

void ALAGC_MainAttackActor::SetIndex(int Index)
{
	MainAttackIndex = Index;
}

bool ALAGC_MainAttackActor::IsActive()
{
	return Active;
}

int ALAGC_MainAttackActor::GetIndex()
{
	return MainAttackIndex;
}

void ALAGC_MainAttackActor::MoveTo(FVector Destination, FRotator Rotation, float Speed)
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.UUID = 1;
	UKismetSystemLibrary::MoveComponentTo(RootComponent, Destination,
		Rotation, false, false, Speed, true,
		EMoveComponentAction::Type::Move, LatentInfo);
}

void ALAGC_MainAttackActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALAGC_MainAttackActor, Active);
}

float ALAGC_MainAttackActor::CalculateSpeed(int32 Speed)
{
	MASpeed = Speed;
	return (float)MASpeed / 10.0f * 2.0f;
}

float ALAGC_MainAttackActor::ComputeRangeOffset(float& TimeDelay)
{
	if (!PlayerController)
	{
		// Try to get player controller
		PlayerController = Cast<ALagCompensationPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (!PlayerController){
			return 0.0f;
		}
	}
	
	float ServerTimeOffset = PlayerController->GetServerTimeOffset();
	float CurrentTime = UGameplayStatics::GetTimeSeconds(GetWorld());
	TimeDelay = CurrentTime + ServerTimeOffset - ActiveServerTime;
	return MaxRange / CalculateSpeed(MASpeed) * TimeDelay;
}