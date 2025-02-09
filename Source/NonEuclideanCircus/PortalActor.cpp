// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APortalActor::APortalActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    bIsActive = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent->Mobility = EComponentMobility::Static;

    PortalRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRootComponent"));
    PortalRootComponent->SetupAttachment(GetRootComponent());
    PortalRootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    PortalRootComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    PortalRootComponent->Mobility = EComponentMobility::Movable;
}

// Called when the game starts or when spawned
void APortalActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool APortalActor::IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
    FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
    float PortalDot = PortalPlane.PlaneDot(Point);

    //If < 0 means we are behind the Plane
    //See : http://api.unrealengine.com/INT/API/Runtime/Core/Math/FPlane/PlaneDot/index.html
    return (PortalDot >= 0);
}

bool APortalActor::IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
    FVector IntersectionPoint;
    FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
    float PortalDot = PortalPlane.PlaneDot(Point);
    bool IsCrossing = false;
    bool IsInFront = PortalDot >= 0;

    bool IsIntersect = FMath::SegmentPlaneIntersection(LastPosition,
        Point,
        PortalPlane,
        IntersectionPoint);

    // Did we intersect the portal since last Location ?
    // If yes, check the direction : crossing forward means we were in front and now at the back
    // If we crossed backward, ignore it (similar to Prey 2006)
    if (IsIntersect && !IsInFront && LastInFront)
    {
        IsCrossing = true;
    }

    // Store values for next check
    LastInFront = IsInFront;
    LastPosition = Point;

    return IsCrossing;
}

void APortalActor::TeleportActor(AActor* ActorToTeleport)
{
    if (ActorToTeleport == nullptr || Target == nullptr)
    {
        return;
    }

    //-------------------------------
    //Retrieve and save Player Velocity
    //(from the Movement Component)
    //-------------------------------
    FVector SavedVelocity = FVector::ZeroVector;
    ACharacter* EC = nullptr;

    if (ActorToTeleport->IsA(ACharacter::StaticClass()))
    {
        EC = Cast<ACharacter>(ActorToTeleport);

        SavedVelocity = EC->GetCharacterMovement()->Velocity;
    }

    //-------------------------------
    //Compute and apply new location
    //-------------------------------
    FHitResult HitResult;
    FVector NewLocation = ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(),
        this,
        Target);

    ActorToTeleport->SetActorLocation(NewLocation,
        false,
        &HitResult,
        ETeleportType::TeleportPhysics);

    //-------------------------------
    //Compute and apply new rotation
    //-------------------------------
    FRotator NewRotation = ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(),
        this,
        Target);

    //Apply new rotation
    ActorToTeleport->SetActorRotation(NewRotation);

    //-------------------------------
    //If we are teleporting a character we need to
    //update its controller as well and reapply its velocity
    //-------------------------------
    if (ActorToTeleport->IsA(ACharacter::StaticClass()))
    {
        //Update Controller
        AController* EPC = EC->GetController();

        if (EPC != nullptr)
        {
            NewRotation = ConvertRotationToActorSpace(EPC->GetControlRotation(),
                this,
                Target);

            EPC->SetControlRotation(NewRotation);
        }

        //Reapply Velocity (Need to reorient direction into local space of Portal)
        {
            FVector Dots;
            Dots.X = FVector::DotProduct(SavedVelocity, GetActorForwardVector());
            Dots.Y = FVector::DotProduct(SavedVelocity, GetActorRightVector());
            Dots.Z = FVector::DotProduct(SavedVelocity, GetActorUpVector());

            FVector NewVelocity = Dots.X * Target->GetActorForwardVector()
                + Dots.Y * Target->GetActorRightVector()
                + Dots.Z * Target->GetActorUpVector();

            EC->GetMovementComponent()->Velocity = NewVelocity;
        }
    }


    //Cleanup Teleport
    LastPosition = NewLocation;
}

FVector APortalActor::ConvertLocationToActorSpace(FVector Location, AActor* Reference, AActor* TargetActor)
{
    if (Reference == nullptr || TargetActor == nullptr)
    {
        return FVector::ZeroVector;
    }

    FVector Direction = Location - Reference->GetActorLocation();
    FVector TargetLocation = TargetActor->GetActorLocation();

    FVector Dots;
    Dots.X = FVector::DotProduct(Direction, Reference->GetActorForwardVector());
    Dots.Y = FVector::DotProduct(Direction, Reference->GetActorRightVector());
    Dots.Z = FVector::DotProduct(Direction, Reference->GetActorUpVector());

    FVector NewDirection = Dots.X * Target->GetActorForwardVector()
        + Dots.Y * TargetActor->GetActorRightVector()
        + Dots.Z * TargetActor->GetActorUpVector();

    return TargetLocation + NewDirection;
}

FRotator APortalActor::ConvertRotationToActorSpace(FRotator Rotation, AActor* Reference, AActor* TargetActor)
{
    if (Reference == nullptr || TargetActor == nullptr)
    {
        return FRotator::ZeroRotator;
    }

    FTransform SourceTransform = Reference->GetActorTransform();
    FTransform TargetTransform = TargetActor->GetActorTransform();
    FQuat QuatRotation = FQuat(Rotation);

    FQuat LocalQuat = SourceTransform.GetRotation().Inverse() * QuatRotation;
    FQuat NewWorldQuat = TargetTransform.GetRotation() * LocalQuat;

    return NewWorldQuat.Rotator();
}