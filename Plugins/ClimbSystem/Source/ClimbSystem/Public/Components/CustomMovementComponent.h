// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class CLIMBSYSTEM_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCustomMovementComponent();

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:

#pragma region ClimbTrace

	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false);

#pragma endregion

#pragma region ClimbCore

	void TraceClimbableSurfaces();

#pragma endregion


#pragma region ClimbVariable

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterMovement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbSurfaceTraceTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterMovement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterMovement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceHalfHeight;

#pragma endregion
};
