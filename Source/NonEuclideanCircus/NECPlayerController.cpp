// Fill out your copyright notice in the Description page of Project Settings.


#include "NECPlayerController.h"
#include "PortalManager.h"

void ANECPlayerController::BeginPlay()
{
    FActorSpawnParameters SpawnParams;

    PortalManager = nullptr;
    PortalManager = GetWorld()->SpawnActor<APortalManager>(APortalManager::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);
    PortalManager->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    PortalManager->SetControllerOwner(this);
    PortalManager->Init();
}

void ANECPlayerController::UpdatePortalManager(float DeltaTime)
{
    if (PortalManager)
    {
        PortalManager->Update(DeltaTime);
    }
}
