// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UCustomMovementComponent::UCustomMovementComponent()
{
	ClimbCapsuleTraceRadius = 50.0f;		// 캡슐 트레이스 반지름
	ClimbCapsuleTraceHalfHeight = 72.0f;	// 캡슐 트레이스 반높이
}

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// 이동 모드가 변경될 때 호출되는 함수입니다.
void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		// 클라이밍 모드 진입 시 설정
		// * 이동 방향으로 회전하지 않도록 설정
		bOrientRotationToMovement = false;
		// * 캡슐 크기 줄이기
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.0f);
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		// 클라이밍 모드에서 벗어날 때 설정 복원
		// * 다시 이동 방향으로 회전
		bOrientRotationToMovement = true;
		// * 캡슐 크기 복원
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
		
		// 움직임 즉시 중단
		StopMovementImmediately();
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

#pragma region ClimbTrace

// 캡슐 형태로 여러 개의 오브젝트 트레이스를 수행합니다.
TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		if (bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
		else
		{
			DebugTraceType = EDrawDebugTrace::ForOneFrame;
		}
	}

	// 여러 오브젝트 타입에 대해 캡슐 트레이스를 수행
	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,                                      // 월드 컨텍스트
		Start,                                     // 시작 위치
		End,                                       // 끝 위치
		ClimbCapsuleTraceRadius,                   // 캡슐 반지름
		ClimbCapsuleTraceHalfHeight,               // 캡슐 반높이
		ClimbSurfaceTraceTypes,                    // 탐지할 오브젝트 타입
		false,                                     // 복잡한 콜리전을 사용할지 여부
		TArray<AActor*>(),                         // 무시할 액터 리스트
		DebugTraceType,                            // 디버그 출력 여부
		OutCapsuleTraceHitResults,                 // 결과 배열
		false                                      // 자기 자신(이 트레이스를 실행한 액터)을 무시할지 여부
	);

	return OutCapsuleTraceHitResults;
}

FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	FHitResult OutHit;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		if (bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
		else
		{
			DebugTraceType = EDrawDebugTrace::ForOneFrame;
		}
	}

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,                                      // 월드 컨텍스트
		Start,                                     // 시작 위치
		End,                                       // 끝 위치
		ClimbSurfaceTraceTypes,                    // 탐지할 오브젝트 타입
		false,                                     // 복잡한 콜리전을 사용할지 여부
		TArray<AActor*>(),                         // 무시할 액터 리스트
		DebugTraceType,                            // 디버그 출력 여부
		OutHit,                                    // 결과
		false                                      // 자기 자신(이 트레이스를 실행한 액터)을 무시할지 여부
	);

	return OutHit;
}

#pragma endregion

#pragma region ClimbCore

// 클라이밍 상태를 토글합니다.
void UCustomMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if (bEnableClimb)
	{
		if (CanStartClimbing())
		{
			// Enter the Climb State
			UE_LOG(LogTemp, Warning, TEXT("Start Climb"));
			StartClimbing();
		}
	}
	else
	{
		// Stop Climbing
		UE_LOG(LogTemp, Warning, TEXT("Stop Climb"));
		StopClimbing();
	}
}

// 현재 클라이밍 상태인지 여부를 반환합니다.
bool UCustomMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climb;
}

// 클라이밍을 시작할 수 있는 조건인지 확인합니다.
bool UCustomMovementComponent::CanStartClimbing()
{
	if (IsFalling()) return false;
	if (!TraceClimbableSurfaces()) return false;
	if (!TraceFromEyeHeight(100.0f).bBlockingHit) return false;

	return true;
}

// 클라이밍 상태로 전환합니다.
void UCustomMovementComponent::StartClimbing()
{
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climb);
}

// 클라이밍 상태를 종료하고 낙하 모드로 전환합니다.
void UCustomMovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Falling);
}

// 클라이밍 가능한 표면을 찾기 위해 트레이스를 수행하는 함수입니다.
// * 클라이밍이 가능한 상태인지를 반환합니다.
bool UCustomMovementComponent::TraceClimbableSurfaces()
{
	// 시작 지점을 약간 전방으로 이동시킵니다.
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.0f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;

	// 트레이스의 끝 지점은 전방 방향으로 설정합니다.
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	// 캡슐 트레이스를 실행하고 디버그 모양을 보여줍니다.
	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiByObject(Start, End, true, true);

	return !ClimbableSurfacesTracedResults.IsEmpty();
}

// 눈 높이에서 라인 트레이스를 수행하여 전방 표면을 감지합니다.
FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;

	return DoLineTraceSingleByObject(Start, End, true, true);
}

#pragma endregion