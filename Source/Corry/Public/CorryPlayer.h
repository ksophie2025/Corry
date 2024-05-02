// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CorryPlayer.generated.h"

UCLASS()
class CORRY_API ACorryPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACorryPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// VR카메라 컴포넌트를 생성하고 루트에 붙이고 싶다.
	UPROPERTY(EditDefaultsOnly)
	class UCameraComponent* vrCamera;

	// 모션컨트롤러 왼손, 오른손 생성하고 루트에 붙이고 싶다.
	UPROPERTY(EditDefaultsOnly)
	class UMotionControllerComponent* motionLeft;

	UPROPERTY(EditDefaultsOnly)
	class UMotionControllerComponent* motionRight;

	// 왼손, 오른손 스켈레탈메시컴포넌트를 만들어서 모션컨트롤러에 붙이고 싶다.
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMeshComponent* meshLeft;

	UPROPERTY(EditDefaultsOnly)
	class USkeletalMeshComponent* meshRight;
	// 왼손, 오른손 스켈레탈메시를 로드해서 적용하고 싶다.


	UPROPERTY(EditDefaultsOnly, Category= VR)
	class UInputMappingContext* imc_vrPlayer;

	UPROPERTY(EditDefaultsOnly, Category = VR)
	class UInputAction* ia_move;

	void OnIAMove(const FInputActionValue& value);


	UPROPERTY(EditDefaultsOnly, Category = VR)
	class UInputAction* ia_turn;

	void OnIATurn(const FInputActionValue& value);


	// 텔레포트 처리를 위해서 써클을 표현하고 싶다.
	UPROPERTY(EditDefaultsOnly, Category=VR)
	class UStaticMeshComponent* teleportCircle;

	// 텔레포트 중인가? 여부를 기억하고 싶다.
	bool bTeleporting;

	// 입력처리와 그에 해당하는 함수를 구현하고 싶다.
	UPROPERTY(EditDefaultsOnly, Category=VR)
	class UInputAction* ia_teleport;

	void OnIATeleportStart(const FInputActionValue& value);
	void OnIATeleportEnd(const FInputActionValue& value);

	void DrawLine(const FVector& start, const FVector& end);

	bool HitTest(FVector start, FVector end, FHitResult& OutHitInfo);
	void ResetTeleport();

	// 목적지를 기억하고
	FVector teleportLocation;
	// 목적지로 이동하는 기능
	void DoTeleport();

	// 곡선 텔레포트
	UPROPERTY(EditDefaultsOnly, Category = VR)
	bool bTeleportCurve = true;

	void TickLine();
	void TickCurve();
	
	bool CheckHitTeleport(const FVector& start,  FVector& end);

	UPROPERTY(EditDefaultsOnly, Category = VR)
	int32 curveStep = 200;

	TArray<FVector> points;

	void MakeCurvePoints();

	void DrawCurve(int max);

};
