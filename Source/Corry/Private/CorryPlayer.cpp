// Fill out your copyright notice in the Description page of Project Settings.


#include "CorryPlayer.h"
#include <Camera/CameraComponent.h>
#include <MotionControllerComponent.h>
#include <../../../../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h>
#include <../../../../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/CapsuleComponent.h>

// Sets default values
ACorryPlayer::ACorryPlayer()
{
	PrimaryActorTick.bCanEverTick = true;


	// VRī�޶� ������Ʈ�� �����ϰ� ��Ʈ�� ���̰� �ʹ�.
	vrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VR Camera"));
	vrCamera->SetupAttachment(RootComponent);

	// �����Ʈ�ѷ� �޼�, ������ �����ϰ� ��Ʈ�� ���̰� �ʹ�.
	motionLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Motion Left"));
	motionLeft->SetTrackingMotionSource(TEXT("Left"));
	motionLeft->SetupAttachment(RootComponent);

	motionRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Motion Right"));
	motionRight->SetTrackingMotionSource(TEXT("Right"));
	motionRight->SetupAttachment(RootComponent);

	// �޼�, ������ ���̷�Ż�޽�������Ʈ�� ���� �����Ʈ�ѷ��� ���̰� �ʹ�.
	meshLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Left"));
	meshLeft->SetupAttachment(motionLeft);

	meshRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Right"));
	meshRight->SetupAttachment(motionRight);

	// �޼�, ������ ���̷�Ż�޽ø� �ε��ؼ� �����ϰ� �ʹ�. -> ������ ����̸� �θ���.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMeshLeft(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));
	// �ε忡 �����ߴٸ� �����Ѵ�
	if (tempMeshLeft.Succeeded())
	{
		meshLeft->SetSkeletalMesh(tempMeshLeft.Object);
		meshLeft->SetRelativeLocationAndRotation(FVector(-2.98f, -3.5f, 4.56f), FRotator(-25.0f, -179.99f, 89.99f));

		//meshLeft->SetRelativeLocation(FVector(-2.981260, -3.500000, 4.561753));
		//meshLeft->SetRelativeRotation(FRotator(-25.000000, -179.999999, 89.999998));
		// (X=-2.981260,Y=-3.500000,Z=4.561753)
		// (Pitch=-25.000000,Yaw=-179.999999,Roll=89.999998)
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMeshRight(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_right.SKM_MannyXR_right'"));

	if (tempMeshRight.Succeeded())
	{
		meshRight->SetSkeletalMesh(tempMeshRight.Object);
		meshRight->SetRelativeLocationAndRotation(FVector(-2.98f, 3.5f, 4.56f), FRotator(25.0f, 0.0f, 89.99f));

		//meshRight->SetRelativeLocation(FVector(-2.981260, 3.500000, 4.561753));
		//meshRight->SetRelativeRotation(FRotator(25.000000, 0.000000, 89.999999));
		// (X=-2.981260,Y=3.500000,Z=4.561753)
		// (Pitch=25.000000,Yaw=0.000000,Roll=89.999999)
	}

	// ��Ŭ�� �����ϰ� �浹ó���� ���� �ʰ� ó���ϰ� �ʹ�.
	teleportCircle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Teleport Circle"));
	teleportCircle->SetupAttachment(RootComponent);
	teleportCircle->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void ACorryPlayer::BeginPlay()
{
	Super::BeginPlay();
	

	// �÷��̾� ��Ʈ�ѷ��� �������� �ʹ�.
	auto* pc = Cast<APlayerController>(Controller);

	// Enhanced Input Local Player Subsystem�� �����ͼ�
	if (pc)
	{
		auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		if (subsystem)
		{
			// Add Mapping Context�� ȣ���ϰ� �ʹ�.
			subsystem->AddMappingContext(imc_vrPlayer, 0);
		}
	}

	ResetTeleport();
}

void ACorryPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	// ���� ��ư�� �������ٸ�
	if (bTeleporting)
	{
		// ���� ��̸�
		if (bTeleportCurve)
		{
			TickCurve();
			UE_LOG(LogTemp, Warning, TEXT("curve"));
		}
		else // �׷��� ������
		{
			TickLine();
			UE_LOG(LogTemp, Warning, TEXT("line"));
		}
	}
}

void ACorryPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);



	// check(true); 0�� �ƴϸ� true.

	auto* input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// check(input);

	if (input)
	{
		input->BindAction(ia_move, ETriggerEvent::Triggered, this, &ACorryPlayer::OnIAMove);
		input->BindAction(ia_turn, ETriggerEvent::Triggered, this, &ACorryPlayer::OnIATurn);

		// �ڷ���Ʈ �Է��� ����ϰ� �ʹ�.
		// ������ �� OnIATeleportStart
		input->BindAction(ia_teleport, ETriggerEvent::Started, this, &ACorryPlayer::OnIATeleportStart);
		// ���� �� OnIATeleportEnd
		input->BindAction(ia_teleport, ETriggerEvent::Completed, this, &ACorryPlayer::OnIATeleportEnd);
	}
}

void ACorryPlayer::OnIAMove(const FInputActionValue& value)
{
	FVector2D v = value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), v.Y);
	AddMovementInput(GetActorRightVector(), v.X);
}

void ACorryPlayer::OnIATurn(const FInputActionValue& value)
{
	float v = value.Get<float>();
	AddControllerYawInput(v);
}

void ACorryPlayer::OnIATeleportStart(const FInputActionValue& value)
{
	// �ڷ���Ʈ ��ư�� ������ ��Ŭ�� ���̰�
	bTeleporting = true;

}

void ACorryPlayer::OnIATeleportEnd(const FInputActionValue& value)
{
	// �ڷ���Ʈ ��ư�� ���� ��Ŭ�Ⱥ��̰� �ʹ�.
	// ���� ��Ŭ�� Ȱ��ȭ �Ǿ� �ִٸ�, �������� �̵��ϰ� �ʹ�
	if (teleportCircle->GetVisibleFlag())
	{
		DoTeleport();
	}

	ResetTeleport(); // �ʱ�ȭ �Լ� ����	
}

void ACorryPlayer::DrawLine(const FVector& start, const FVector& end)
{
	DrawDebugLine(GetWorld(), start, end, FColor::Red, false, -1, 0, 1);
}

bool ACorryPlayer::HitTest(FVector start, FVector end, FHitResult& OutHitInfo)
{
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	return GetWorld()->LineTraceSingleByChannel(OutHitInfo, start, end, ECC_Visibility, params);
}

void ACorryPlayer::ResetTeleport()
{
	// �ڷ���Ʈ ���� �ƴ�.
	bTeleporting = false;
	// ��Ŭ�� ������ �ʰ�
	teleportCircle->SetVisibility(false);
}

void ACorryPlayer::DoTeleport()
{
	FVector height = FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	// �������� �̵�  (ĸ���� �Ǻ� ��ġ��ŭ z������ �÷��ش�.)
	SetActorLocation(teleportLocation + height);
}

void ACorryPlayer::TickLine()
{
	FVector start = meshRight->GetComponentLocation();
	FVector end = start + meshRight->GetRightVector() * 100000;

	// ����Ʈ���̽� �浹 ����
	CheckHitTeleport(start, end);

	// �� �׸���
	DrawLine(start, end);
}

void ACorryPlayer::TickCurve()
{
	// �������ϰ���
	// 1/2 * �߷� * t * t

	// 1. ������ ����ؼ� ���������� ������� ���� �ְ� �ʹ�.
	MakeCurvePoints();
	// 2. ���� �����鼭 �浹ó���� �ϰ� �ʹ�. ��� ������ CheckHitTeleport�� �̿��ؼ� ó���ϰ� �ʹ�

	FHitResult hitInfo;
	int maxPoints = points.Num();
	for (int i = 0; i < points.Num() - 1; i++)
	{
		if (CheckHitTeleport(points[i], points[i + 1]))
		{
			// ��� �ε�����.
			maxPoints = i + 1;
			break;
		}
	}

	// 3. ���� �׸��� �ʹ�.
	DrawCurve(maxPoints);
}

bool ACorryPlayer::CheckHitTeleport(const FVector& start, FVector& end)
{
	// ����Ʈ���̽��� �ؼ� �ε��� ���� �ִٸ�
	FHitResult hitInfo;
	bool bHit = HitTest(start, end, hitInfo);
	if (bHit && hitInfo.GetActor()->GetName().Contains(TEXT("Floor")))
	{
		end = hitInfo.ImpactPoint;

		// �ڷ���Ʈ ������ ���� ����
		teleportLocation = hitInfo.Location;

		// �� ���� ��Ŭ�� ���̰� �ϰ�, ��ġ�ϰ� 
		teleportCircle->SetWorldLocation(hitInfo.Location);
		teleportCircle->SetVisibility(true);
	}
	// �׷��� �ʴٸ�
	else
	{
		// ��Ŭ�� ������ �ʰ� �ϰ� �ʹ�.
		teleportCircle->SetVisibility(false);
	}
	return bHit;
}

void ACorryPlayer::MakeCurvePoints()
{
	// 1/2 * gravity * t * t ������ �߷��� 9.8m/sec
	// curveStep ������ �� ����
	points.Empty(curveStep);
	FVector gravity(0, 0, -981);
	float simDT = 1.f / 60.f; // �ùķ����� ��Ÿ Ÿ��
	FVector point = meshRight->GetComponentLocation(); // ���� ���� ��
	FVector velocity = meshRight->GetRightVector() * 1000;
	points.Add(point);

	for (int i = 0; i < curveStep; i++)
	{
		point += velocity * simDT + 0.5f * gravity * simDT * simDT;// ���� ����(velocity)�� �߷°��ӵ��� �����־� ��������
		velocity += gravity * simDT;
		points.Add(point); // ���� �迭�� ������ �ִ´�
	}
}

void ACorryPlayer::DrawCurve(int max)
{
	for (int i = 0; i < max - 1; i++)
	{
		DrawLine(points[i], points[i + 1]);
	}
}