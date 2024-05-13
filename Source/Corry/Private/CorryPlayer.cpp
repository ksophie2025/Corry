// Fill out your copyright notice in the Description page of Project Settings.


#include "CorryPlayer.h"
//----------------------------------
#include "EchoActor.h"
#include "Kismet/GameplayStatics.h"
//----------------------------------
#include <Camera/CameraComponent.h>
#include <MotionControllerComponent.h>
#include <../../../../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h>
#include <../../../../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h>
#include <../../../../../../../Source/Runtime/Engine/Classes/Components/CapsuleComponent.h>
#include <../../../../../../../Plugins/FX/Niagara/Source/Niagara/Public/NiagaraComponent.h>
#include <../../../../../../../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraDataInterfaceArrayFunctionLibrary.h>
#include <../../../../../../../Source/Runtime/Engine/Public/EngineUtils.h>



// Sets default values
ACorryPlayer::ACorryPlayer()
{
	PrimaryActorTick.bCanEverTick = true;


	// VR카메라 컴포넌트를 생성하고 루트에 붙이고 싶다.
// 	vrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VR Camera"));
// 	vrCamera->SetupAttachment(RootComponent);

	// 모션컨트롤러 왼손, 오른손 생성하고 루트에 붙이고 싶다.
	motionLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Motion Left"));
	motionLeft->SetTrackingMotionSource(TEXT("Left"));
	motionLeft->SetupAttachment(RootComponent);

	motionRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Motion Right"));
	motionRight->SetTrackingMotionSource(TEXT("Right"));
	motionRight->SetupAttachment(RootComponent);

	// 왼손, 오른손 스켈레탈메시컴포넌트를 만들어서 모션컨트롤러에 붙이고 싶다.
	meshLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Left"));
	meshLeft->SetupAttachment(motionLeft);

	meshRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Right"));
	meshRight->SetupAttachment(motionRight);

	// 왼손, 오른손 스켈레탈메시를 로드해서 적용하고 싶다. -> 생성자 도우미를 부른다.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMeshLeft(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));
	// 로드에 성공했다면 적용한다
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

	// 써클을 생성하고 충돌처리가 되지 않게 처리하고 싶다.
	teleportCircleVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Teleport Circle VFX"));
	teleportCircleVFX->SetupAttachment(RootComponent);
	teleportCircleVFX->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	teleportTraceVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Teleport Trace VFX"));
	teleportTraceVFX->SetupAttachment(RootComponent);

	// -------------------------------------------------------
// 	echoRoot =  CreateDefaultSubobject<USceneComponent>(TEXT("Echo Root"));
// 	echoRoot->SetupAttachment(vrCamera);
	//echoRoot->SetRelativeLocation(FVector(-100,0,0));
	
}


void ACorryPlayer::BeginPlay()
{
	Super::BeginPlay();
	

	// 플레이어 컨트롤러를 가져오고 싶다.
	auto* pc = Cast<APlayerController>(Controller);

	// Enhanced Input Local Player Subsystem을 가져와서
	if (pc)
	{
		auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		if (subsystem)
		{
			// Add Mapping Context를 호출하고 싶다.
			subsystem->AddMappingContext(imc_vrPlayer, 0);
		}
	}

	ResetTeleport();

	// ------------------------------------------------------
	//auto list = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass());
		
	TArray<AActor*> echoes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEchoActor::StaticClass(), echoes);
	
// 	echo = Cast<AEchoActor>(echoes[0]);
// 	
// 	echo->AttachToComponent(echoRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ACorryPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	// 만약 버튼이 눌러졌다면
	if (bTeleporting)
	{
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(teleportTraceVFX, FName("User.PointArray"), points);
		// 만약 곡선이면
		if (bTeleportCurve)
		{
			TickCurve();
			UE_LOG(LogTemp, Warning, TEXT("curve"));
		}
		else // 그렇지 않으면
		{
			TickLine();
			UE_LOG(LogTemp, Warning, TEXT("line"));
		}
	}
}

void ACorryPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);



	// check(true); 0이 아니면 true.

	auto* input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// check(input);

	if (input)
	{
		input->BindAction(ia_move, ETriggerEvent::Triggered, this, &ACorryPlayer::OnIAMove);
		input->BindAction(ia_turn, ETriggerEvent::Triggered, this, &ACorryPlayer::OnIATurn);

		// 텔레포트 입력을 등록하고 싶다.
		// 눌렀을 때 OnIATeleportStart
		input->BindAction(ia_teleport, ETriggerEvent::Started, this, &ACorryPlayer::OnIATeleportStart);
		// 뗐을 때 OnIATeleportEnd
		input->BindAction(ia_teleport, ETriggerEvent::Completed, this, &ACorryPlayer::OnIATeleportEnd);

		// input->BindAction(ia_j, ETriggerEvent::Started, echo, &AEcho)
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
	float v = value.Get<float>()*1.5f;
	AddControllerYawInput(v);
}

void ACorryPlayer::OnIATeleportStart(const FInputActionValue& value)
{
	// 텔레포트 버튼을 누르면 써클이 보이고
	bTeleporting = true;

}

void ACorryPlayer::OnIATeleportEnd(const FInputActionValue& value)
{
	// 텔레포트 버튼을 떼면 써클안보이고 싶다.
	// 만약 써클이 활성화 되어 있다면, 목적지로 이동하고 싶다
	if (teleportCircleVFX->GetVisibleFlag())
	{
		if (bWarp)
		{
			DoWarp();
		}
		else
		{
			DoTeleport();
		}
	}

	ResetTeleport(); // 초기화 함수 실행	
}

void ACorryPlayer::DrawLine(const FVector& start, const FVector& end)
{
	//DrawDebugLine(GetWorld(), start, end, FColor::Red, false, -1, 0, 1);
}

bool ACorryPlayer::HitTest(FVector start, FVector end, FHitResult& OutHitInfo)
{
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	return GetWorld()->LineTraceSingleByChannel(OutHitInfo, start, end, ECC_Visibility, params);
}

void ACorryPlayer::ResetTeleport()
{
	// 텔레보트 중이 아님.
	bTeleporting = false;
	// 써클을 보이지 않게
	teleportCircleVFX->SetVisibility(false);
}

void ACorryPlayer::DoTeleport()
{
	FVector height = FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	// 목적지로 이동  (캡슐의 피봇 위치만큼 z축으로 올려준다.)
	SetActorLocation(teleportLocation + height);
}

void ACorryPlayer::TickLine()
{
	FVector start = meshRight->GetComponentLocation();
	FVector end = start + meshRight->GetRightVector() * 100000;

	// 라인트레이스 충돌 지점
	CheckHitTeleport(start, end);

	// 선 그리기
	DrawLine(start, end);
}

void ACorryPlayer::TickCurve()
{
	// 자유낙하공식
	// 1/2 * 중력 * t * t

	// 1. 궤적을 계산해서 정점정보를 목록으로 갖고 있고 싶다.
	MakeCurvePoints();
	// 2. 점을 이으면서 충돌처리를 하고 싶다. 모든 점들을 CheckHitTeleport를 이용해서 처리하고 싶다

	FHitResult hitInfo;
	int maxPoints = points.Num();
	for (int i = 0; i < points.Num() - 1; i++)
	{
		if (CheckHitTeleport(points[i], points[i + 1]))
		{
			// 어딘가 부딪혔다.
			maxPoints = i + 1;
			break;
		}
	}

	// 3. 선을 그리고 싶다.
	DrawCurve(maxPoints);
}

bool ACorryPlayer::CheckHitTeleport(const FVector& start, FVector& end)
{
	// 라인트레이스를 해서 부딪힌 곳이 있다면
	FHitResult hitInfo;
	bool bHit = HitTest(start, end, hitInfo);
	if (bHit && hitInfo.GetActor()->GetName().Contains(TEXT("Floor")))
	{
		end = hitInfo.ImpactPoint;

		// 텔레포트 목적지 정보 저장
		teleportLocation = hitInfo.Location;

		// 그 곳에 써클을 보이게 하고, 배치하고 
		teleportCircleVFX->SetWorldLocation(hitInfo.Location);
		teleportCircleVFX->SetVisibility(true);
	}
	// 그렇지 않다면
	else
	{
		// 써클을 보이지 않게 하고 싶다.
		teleportCircleVFX->SetVisibility(false);
	}
	return bHit;
}

void ACorryPlayer::MakeCurvePoints()
{
	// 1/2 * gravity * t * t 지구의 중력은 9.8m/sec
	// curveStep 궤적의 점 개수
	points.Empty(curveStep);
	FVector gravity(0, 0, -981);
	float simDT = 1.f / 60.f; // 시뮬레이팅 델타 타임
	FVector point = meshRight->GetComponentLocation(); // 궤적 시작 점
	FVector velocity = meshRight->GetRightVector() * 1000;
	points.Add(point);

	for (int i = 0; i < curveStep; i++)
	{
		point += velocity * simDT + 0.5f * gravity * simDT * simDT;// 원래 벡터(velocity)에 중력가속도를 더해주어 궤적형성
		velocity += gravity * simDT;
		points.Add(point); // 궤적 배열에 점들을 넣는다
	}
}

void ACorryPlayer::DrawCurve(int max)
{
	for (int i = 0; i < max - 1; i++)
	{
		DrawLine(points[i], points[i + 1]);
	}
}

void ACorryPlayer::DoWarp()
{
	if (!bWarp)
		return;

	// 시간이 흐르다가 워프 이동시간이 끝나면 워프 종료
	currentTime = 0;

	FVector height = FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FVector tarLoc = teleportLocation + height;
	FVector originLoc = GetActorLocation();

	// 충돌체를 끄고 싶다 (이동 중에는 끄기)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorld()->GetTimerManager().SetTimer(warpTimerHandle, FTimerDelegate::CreateLambda([&, originLoc, tarLoc]()->void {

		// 이동처리
		currentTime += GetWorld()->GetDeltaSeconds();

		// 현재위치 , 목적지	
		float alpha = currentTime *2 / warpTime;
		FVector curLoc = FMath::Lerp(curLoc, tarLoc, alpha);
		SetActorLocation(curLoc);

		// 만약 도착했다면 
		if (alpha >= 1)
		{
			// 타이머를 멈추고 싶다.
			GetWorld()->GetTimerManager().ClearTimer(warpTimerHandle);
			// 내위치를 tarLoc으로 하고 싶다.
			SetActorLocation(tarLoc);
			// 충돌체를 켜고 싶다.
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		}), 0.033333f, true);
}