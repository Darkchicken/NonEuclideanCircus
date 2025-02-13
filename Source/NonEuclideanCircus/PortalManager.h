// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalManager.generated.h"

class APortalActor;
class USceneCaptureComponent2D;

UCLASS()
class NONEUCLIDEANCIRCUS_API APortalManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalManager(const FObjectInitializer& ObjectInitializer);

    // Called by a Portal actor when wanting to teleport something
    UFUNCTION(BlueprintCallable, Category = "Portal")
    void RequestTeleportByPortal(APortalActor* Portal, AActor* TargetToTeleport);

    // Save a reference to the PlayerControler
    void SetControllerOwner(APlayerController* NewOwner);

    // Various setup that happens during spawn
    void Init();

    // Manual Tick
    void Update(float DeltaTime);

    // Find all the portals in world and update them
    // returns the most valid/usable one for the Player
    APortalActor* UpdatePortalsInWorld();

    // Update SceneCapture
    void UpdateCapture(APortalActor* Portal);

    FMatrix GetCameraProjectionMatrix();

private:
    //Function to create the Portal render target
    void GeneratePortalTexture();

    UPROPERTY()
    USceneCaptureComponent2D* SceneCapture;

    UPROPERTY(transient)
    UTextureRenderTarget2D* PortalTexture;

    UPROPERTY()
    APlayerController* ControllerOwner;

    UPROPERTY()
    ACharacter* PlayerCharacter;

    int32 PreviousScreenSizeX;
    int32 PreviousScreenSizeY;

    float UpdateDelay;

};
