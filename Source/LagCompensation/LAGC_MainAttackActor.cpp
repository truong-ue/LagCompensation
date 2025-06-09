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

void ALAGC_MainAttackActor::SetActive(bool bIsActive, uint8 InSpeed)
{
	Active = bIsActive;
	if (HasAuthority())
	{
		APawn* LocalInstigator = GetInstigator();
		InitLocation = LocalInstigator->GetActorLocation();


		// Make struct to send data via RPC
		FAttackInfo AttackInfo;

		// Set bIsActive (bit 3 in Flags)
		if (bIsActive) AttackInfo.Flags |= (1 << 3);
			else AttackInfo.Flags &= ~(1 << 3);
		AttackInfo.Speed = InSpeed;
		
		// Only send Speed if it changed
		if (MASpeed != InSpeed)
		{
			AttackInfo.Flags |= (1 << 0);
		}

		// @Todo: Only send data if change for yaw, location
		AttackInfo.Flags |= (1 << 1);
		AttackInfo.Flags |= (1 << 2);

		AttackInfo.ActiveServerTime = UGameplayStatics::GetTimeSeconds(GetWorld());
		AttackInfo.MAYaw = LocalInstigator->GetActorRotation().Yaw;
		AttackInfo.InitLocation = FVector2f(InitLocation.X, InitLocation.Y);
		
		NetMulti_ToggleTrigger(AttackInfo);
	}
}

void ALAGC_MainAttackActor::NetMulti_ToggleTrigger_Implementation(FAttackInfo AttackInfo)
{
	// Get bIsActive (bit 3 in Flags)
	Active = AttackInfo.Flags & (1 << 3);
	SetActorHiddenInGame(!Active);
	SetActorEnableCollision(Active);
	if (Active)
	{
		FRotator Rotation;
		FVector Direction;
		float OverTime;
		//Compute RangeOffset
		if (HasAuthority())
		{
			MASpeed = AttackInfo.Speed;
			ActiveServerTime = AttackInfo.ActiveServerTime;
			InitLocation = FVector(AttackInfo.InitLocation.X, AttackInfo.InitLocation.Y, 0.0f);
			MAYaw = AttackInfo.MAYaw;

			Rotation = FRotator(0.0f, MAYaw, 0.0f);
			Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
			OverTime = CalculateSpeed(MASpeed);
			
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
			ActiveServerTime = AttackInfo.ActiveServerTime;
			
			if (AttackInfo.Flags & 1 << 0)
			{
				MASpeed = AttackInfo.Speed; // Speed Changed
			}
			
			if (AttackInfo.Flags & 1 << 1)
			{
				MAYaw = AttackInfo.MAYaw; // Yaw Changed
			}
			
			if (AttackInfo.Flags & 1 << 2)
			{
				InitLocation = FVector(AttackInfo.InitLocation.X, AttackInfo.InitLocation.Y, 0.0f); // Init Location Changed
			}

			Rotation = FRotator(0.0f, MAYaw, 0.0f);
			Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
			OverTime = CalculateSpeed(MASpeed);
			
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

bool ALAGC_MainAttackActor::IsActive() const
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

float ALAGC_MainAttackActor::CalculateSpeed(uint8 Speed)
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