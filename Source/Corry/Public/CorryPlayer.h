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

	// VRī�޶� ������Ʈ�� �����ϰ� ��Ʈ�� ���̰� �ʹ�.
	UPROPERTY(EditDefaultsOnly)
	class UCameraComponent* vrCamera;

	// �����Ʈ�ѷ� �޼�, ������ �����ϰ� ��Ʈ�� ���̰� �ʹ�.
	UPROPERTY(EditDefaultsOnly)
	class UMotionControllerComponent* motionLeft;

	UPROPERTY(EditDefaultsOnly)
	class UMotionControllerComponent* motionRight;

	// �޼�, ������ ���̷�Ż�޽�������Ʈ�� ���� �����Ʈ�ѷ��� ���̰� �ʹ�.
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMeshComponent* meshLeft;

	UPROPERTY(EditDefaultsOnly)
	class USkeletalMeshComponent* meshRight;
	// �޼�, ������ ���̷�Ż�޽ø� �ε��ؼ� �����ϰ� �ʹ�.


	UPROPERTY(EditDefaultsOnly, Category= VR)
	class UInputMappingContext* imc_vrPlayer;

	UPROPERTY(EditDefaultsOnly, Category = VR)
	class UInputAction* ia_move;

	void OnIAMove(const FInputActionValue& value);


	UPROPERTY(EditDefaultsOnly, Category = VR)
	class UInputAction* ia_turn;

	void OnIATurn(const FInputActionValue& value);


	// �ڷ���Ʈ ó���� ���ؼ� ��Ŭ�� ǥ���ϰ� �ʹ�.
	UPROPERTY(EditDefaultsOnly, Category=VR)
	class UStaticMeshComponent* teleportCircle;

	// �ڷ���Ʈ ���ΰ�? ���θ� ����ϰ� �ʹ�.
	bool bTeleporting;

	// �Է�ó���� �׿� �ش��ϴ� �Լ��� �����ϰ� �ʹ�.
	UPROPERTY(EditDefaultsOnly, Category=VR)
	class UInputAction* ia_teleport;

	void OnIATeleportStart(const FInputActionValue& value);
	void OnIATeleportEnd(const FInputActionValue& value);

	void DrawLine(const FVector& start, const FVector& end);

	bool HitTest(FVector start, FVector end, FHitResult& OutHitInfo);
	void ResetTeleport();

	// �������� ����ϰ�
	FVector teleportLocation;
	// �������� �̵��ϴ� ���
	void DoTeleport();

	// � �ڷ���Ʈ
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
