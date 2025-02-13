// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

UCLASS()
class NONEUCLIDEANCIRCUS_API APortalActor : public AActor
{
    GENERATED_UCLASS_BODY()

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    //Status of the Portal (being visualized by the player or not)
    UFUNCTION(BlueprintPure, Category = "Portal")
    const bool IsActive() const { return bIsActive; }

    UFUNCTION(BlueprintCallable, Category = "Portal")
    void SetActive(bool NewActive) { bIsActive = NewActive; }

    //Render target to use to display the portal
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Portal")
    void ClearRTT();

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Portal")
    void SetRTT(UTexture* RenderTexture);

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal")
    void ForceTick();

    //Target of where the portal is looking
    UFUNCTION(BlueprintPure, Category = "Portal")
    AActor* GetTarget() const { return Target; }

    UFUNCTION(BlueprintCallable, Category = "Portal")
    void SetTarget(AActor* NewTarget) { Target = NewTarget; }

    //Helpers
    UFUNCTION(BlueprintCallable, Category = "Portal")
    bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

    UFUNCTION(BlueprintCallable, Category = "Portal")
    bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

    UFUNCTION(BlueprintCallable, Category = "Portal")
    void TeleportActor(AActor* ActorToTeleport);

    static FVector ConvertLocationToActorSpace(FVector Location, AActor* Reference, AActor* TargetActor);
    FRotator ConvertRotationToActorSpace(FRotator Rotation, AActor* Reference, AActor* TargetActor);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    USceneComponent* PortalRootComponent;

private:
    bool bIsActive;

    AActor* Target;

    //Used for Tracking movement of a point
    FVector LastPosition;
    bool    LastInFront;
};
