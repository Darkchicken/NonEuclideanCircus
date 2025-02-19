// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

class USceneCaptureComponent2D;
class UArrowComponent;
class UBoxComponent;

UCLASS()
class NONEUCLIDEANCIRCUS_API APortalActor : public AActor
{
    GENERATED_UCLASS_BODY()

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
   void Init();

   void SetClipPlanes();

   //Update the portal camera's FOV to match the player's FOV
   void UpdateFOV();

   void UpdateSceneCaptureRecursive(FVector Location, FRotator Rotation);

   //Update viewport size if it has been changed
   void UpdateViewportSize();

   //Update offset distance based on player's location and velocity
   void UpdateOffsetDistance(FVector Point, FVector PortalLocation, FVector PortalNormal, float DeltaTime);

   void ShouldTeleport(float DeltaTime);

   bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

   void TeleportCharacter();

   FVector UpdateVelocity(FVector OldVelocity);
   FVector UpdateLocation(FVector OldLocation);
   FRotator UpdateRotation(FRotator OldRotation);

   FVector UpdateRotationAxis(FVector RotationAxis);


   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   UMaterialInterface* PortalMaterial = nullptr;

   //Plane that displays the render texture
   UPROPERTY(EditAnywhere, BlueprintReadOnly)
   UStaticMeshComponent* PortalPlane = nullptr;

   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   USceneCaptureComponent2D* PortalCamera = nullptr;

   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   UArrowComponent* ForwardDirection = nullptr;

   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   UBoxComponent* PlayerDetection = nullptr;

   //The portal that an actor will exit from when they enter this portal
   UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
   APortalActor* LinkedPortal = nullptr;

   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   TSubclassOf<AActor> PlayerCharacterClass;

   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   float DefaultOffsetAmount = -4.f;

   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   float OffsetMultiplier = -4.f;

   //Plane material offset will only occur if player is closer than OffsetTriggerDistance to the portal
   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   float OffsetTriggerDistance = 200.f;

   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
   int32 MaxRecursionDepth = 3;

protected:
    UMaterialInstanceDynamic* DynamicPortalMat = nullptr;
    UTextureRenderTarget2D* PortalRenderTarget = nullptr;

    bool bInitialized = false;

    FVector LastPosition;
    bool bWasInFront = false;
    int32 CurrentRecursionDepth = 0;
};
