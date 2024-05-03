// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EchoActor.generated.h"

UCLASS()
class CORRY_API AEchoActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AEchoActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	

};
