// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensationCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

ALagCompensationCharacter::ALagCompensationCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ALagCompensationCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ALagCompensationCharacter::FaceMouseCursor()
{
	APlayerController* PC = this->GetLocalViewingPlayerController();
	if (PC)
	{
		ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2);
		FHitResult HitResult;
		FVector ActorLocation = this->GetActorLocation();
		FRotator ActorRotator = this->GetActorRotation();
		FVector HitLocation;
		PC->GetHitResultUnderCursorByChannel(TraceChannel, false, HitResult);
		HitLocation = { HitResult.Location.X, HitResult.Location.Y, 0 };
		auto Rot = UKismetMathLibrary::FindLookAtRotation(
			ActorLocation,
			HitLocation
		);
		FRotator NewRot = { 0, Rot.Yaw, 0 };
		AController* Controller2 = this->GetController();
		Controller2->SetControlRotation(NewRot);
	}
}
