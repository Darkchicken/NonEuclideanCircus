// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NECPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NONEUCLIDEANCIRCUS_API ANECPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	
};
