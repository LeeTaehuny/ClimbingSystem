// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"

#include "Kismet/KismetSystemLibrary.h"

UCustomMovementComponent::UCustomMovementComponent()
{
	ClimbCapsuleTraceRadius = 50.0f;		// 캡슐 트레이스 반지름
	ClimbCapsuleTraceHalfHeight = 72.0f;	// 캡슐 트레이스 반높이
}

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 매 프레임마다 클라이밍 가능한 표면을 탐지합니다.
	TraceClimbableSurfaces();
}

#pragma region ClimbTrace

// 캡슐 형태로 여러 개의 오브젝트 트레이스를 수행합니다.
TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;

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
		bShowDebugShape ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, // 디버그 출력 여부
		OutCapsuleTraceHitResults,                 // 결과 배열
		false                                      // 자기 자신(이 트레이스를 실행한 액터)을 무시할지 여부
	);

	return OutCapsuleTraceHitResults;
}

#pragma endregion

#pragma region ClimbCore

// 클라이밍 가능한 표면을 찾기 위해 트레이스를 수행하는 함수입니다.
void UCustomMovementComponent::TraceClimbableSurfaces()
{
	// 시작 지점을 약간 전방으로 이동시킵니다.
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.0f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;

	// 트레이스의 끝 지점은 전방 방향으로 설정합니다.
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	// 캡슐 트레이스를 실행하고 디버그 모양을 보여줍니다.
	DoCapsuleTraceMultiByObject(Start, End, true);
}

#pragma endregion