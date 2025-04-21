// Fill out your copyright notice in the Description page of Project Settings.

#include "LAGC_MainAttackActor.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALAGC_MainAttackActor::ALAGC_MainAttackActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}
 
void ALAGC_MainAttackActor::Deactivate()
{
	SetActive(false, 0);
	GetWorldTimerManager().ClearAllTimersForObject(this);
	//OnMainAttackDespawn.Broadcast(this);
	// 
	//All MainAttacks are DORMANT so we ForceNetUpdate when needed (here when we deactivate it).
	ForceNetUpdate();
}

void ALAGC_MainAttackActor::SetActive(bool IsActive, int32 Speed)
{
	Active = IsActive;
	SetActorHiddenInGame(!IsActive);
	SetActorEnableCollision(IsActive);
	if (IsActive)
	{
		//LAUNCHING THE MAIN ATTACK :
		//Create a timer to Deactivate the MainAttack once it reaches its final position
		GetWorldTimerManager().SetTimer(LifeSpanTimer, this, &ALAGC_MainAttackActor::Deactivate, CalculateSpeed(Speed), false);
		
		// we only launch on the floor (Z = 0)
		InitialLoc3D = PawnInsti->GetActorLocation();
		InitialLoc3D = { InitialLoc3D.X, InitialLoc3D.Y, 0 };

		//we get the hero yaw rotation, so the MainAttack gets launch in front of the hero
		FRotator HeroRot = PawnInsti->GetActorRotation();
		float HeroRotZ = HeroRot.Yaw;
		FRotator MARot = { 0, 0, HeroRotZ };

		// Set the replicated value for the speed and Yaw
		MASpeed = Speed;
		MAYaw = HeroRotZ;

		//We only replicate the 2D location
		InitialLoc = { InitialLoc3D.X, InitialLoc3D.Y };

		//Set the destination location (max range in front of hero)
		FVector Forward01 = PawnInsti->GetActorForwardVector();
		FVector Forward02 = Forward01 * MaxRange;
		FVector End = InitialLoc3D + Forward02;
		DestinationLocation = End;

		//we teleport the MainAttack to the hero position (starting to launch if from this position)
		TeleportTo(InitialLoc3D, MARot, true, true);

		//we move it (here on server, in BP for clients)
		MoveTo(End, MARot, CalculateSpeed(Speed));

		//we just use this Trigger variable with a RepNotify to notify ALL clients a MainAttack has been launched and we do the move in the BP with the BP event.
		if (Trigger < 255)
		{
			Trigger++;
		}
		else
		{
			Trigger = 1;
		}

		//All MainAttacks are DORMANT so we ForceNetUpdate when needed (here when we launch it).
		ForceNetUpdate();
	}
	//else
	//{
	//	MoveToStop();
	//}
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

void ALAGC_MainAttackActor::OnRep_Trigger()
{
	BPEvent_Start();
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

void ALAGC_MainAttackActor::MoveToStop()
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.UUID = 1;

	UKismetSystemLibrary::MoveComponentTo(RootComponent, FVector().ZeroVector, FRotator().ZeroRotator, false, false, 15.0f, true, EMoveComponentAction::Stop, LatentInfo);
}

void ALAGC_MainAttackActor::SetPawnInsti(APawn* Insti)
{
	PawnInsti = Insti;
}

void ALAGC_MainAttackActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALAGC_MainAttackActor, MAYaw);
	DOREPLIFETIME(ALAGC_MainAttackActor, InitialLoc);
	DOREPLIFETIME(ALAGC_MainAttackActor, MASpeed);
	DOREPLIFETIME(ALAGC_MainAttackActor, Trigger);
}

float ALAGC_MainAttackActor::CalculateSpeed(int32 Speed)
{
	MASpeed = Speed;
	float MARealSpeed = (float)MASpeed / 10 * 2;
	return MARealSpeed;
}