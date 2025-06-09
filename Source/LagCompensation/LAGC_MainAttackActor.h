// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LAGC_MainAttackActor.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMainAttackDespawn, ALAGC_MainAttackActor*, LAGC_MainAttackComponent);

USTRUCT()
struct FAttackInfo
{
	GENERATED_BODY()
	
	// Flags to check what data be sent and store bIsActive (bit 3: bIsActive)
	// Bit 0: Speed Changed
	// Bit 1: Yaw Changed (not implement
	// Bit 2: Location Changed
	// Bit 3: bIsActive
	// => LengthBits: 4
	uint8 Flags = 0;                // 1 byte
	uint8 Speed = 0;                // 1 byte
	
	float MAYaw = 0.0f;				// 4 bytes
	float ActiveServerTime = 0.0f;  // 4 bytes
	FVector2f InitLocation = FVector2f::ZeroVector; // 8 bytes
	
	
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar.SerializeBits(&Flags, 4);
		
		if (Ar.IsSaving())
		{
			if (Flags & 1 << 0) Ar << Speed;
			if (Flags & 1 << 1)
			{
				// Compress Yaw
				uint8 CompressedYaw = FMath::RoundToInt(MAYaw / 360.0f * 255.0f);
				Ar << CompressedYaw;
			}
			if (Flags & 1 << 2) Ar << InitLocation;
		}
	
		if (Ar.IsLoading())
		{
			if (Flags & 1 << 0) Ar << Speed;
			if (Flags & 1 << 1)
			{
				uint8 CompressedYaw = 0;
				Ar<<CompressedYaw;
				MAYaw = static_cast<float>(CompressedYaw) / 255.0f * 360.0f;
			}
			if (Flags & 1 << 2) Ar << InitLocation;
		}
	
		// Always send
		Ar << ActiveServerTime;
		
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FAttackInfo> : public TStructOpsTypeTraitsBase2<FAttackInfo>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = false,
	};
};

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
	
	void SetActive(bool bIsActive, uint8 InSpeed = 0);

	UFUNCTION(NetMulticast, Reliable, Category = "Attack")
	void NetMulti_ToggleTrigger(FAttackInfo AttackInfo);
	
	void SetIndex(int Index);
	bool IsActive() const;
	int GetIndex();

	UPROPERTY(BlueprintReadWrite, Category = "Attack")
	uint8 MASpeed = 125; //correspond au missile speed attribute/2
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attack")
	float MaxRange = 5000;

	UFUNCTION(Blueprintcallable, Category = "Attack")
	void MoveTo(FVector Destination, FRotator Rotation, float Speed);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

protected:

	float CalculateSpeed(uint8 Speed);

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

