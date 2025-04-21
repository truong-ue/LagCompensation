// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "LagCompensationPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class ALagCompensationPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALagCompensationPlayerController();
	
	/** Time Threshold to know if it was a short press */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	//float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	//UNiagaraSystem* FXCursor;

	/** MappingContext */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	//UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	//UInputAction* SetDestinationClickAction;

	/** Jump Input Action */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	//UInputAction* SetDestinationTouchAction;
	
protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	//uint32 bMoveToMouseCursor : 1;

	//virtual void SetupInputComponent() override;
	
	// To add mapping context
	virtual void BeginPlay();

	/** Input handlers for SetDestination action. */
	//void OnInputStarted();
	//void OnSetDestinationTriggered();
	//void OnSetDestinationReleased();
	//void OnTouchTriggered();
	//void OnTouchReleased();

private:
	//FVector CachedDestination;

	//bool bIsTouch; // Is it a touch device
	//float FollowTime; // For how long it has been pressed

	UPROPERTY()
	float ServerTimeOffset = 0.0f;

	
public:
	FORCEINLINE float GetServerTimeOffset() const  { return ServerTimeOffset; }

private:

	FTimerHandle CalculateServerTimeOffsetTimerHandle;

	void CalculateServerTimeOffset();
	
	UFUNCTION(Server, Reliable, Category = "Lag Compensation")
	void Server_CalculateServerTimeOffset(float TimeAtRequest);

	//Server reply to client request
	UFUNCTION(Client, Reliable, Category = "Lag Compensation")
	void Client_CalculateServerTimeOffset(float TimeAtRequest, float TimeServer);
};


