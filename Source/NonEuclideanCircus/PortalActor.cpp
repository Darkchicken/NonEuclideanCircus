// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMaterialLibrary.h>
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <Components/SceneCaptureComponent2D.h>
#include <Kismet/GameplayStatics.h>
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
//#TODO_JShucker: Remove this if you want this to be useable in all games
#include "NECPlayerCharacter.h"

// Sets default values
APortalActor::APortalActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    //Necessary so portal ticks after everything else
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent->Mobility = EComponentMobility::Movable;

    PortalPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalPlane"));
    PortalPlane->SetupAttachment(GetRootComponent());
    PortalPlane->Mobility = EComponentMobility::Movable;
    PortalPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PortalCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PortalCamera"));
    PortalCamera->SetupAttachment(GetRootComponent());
    PortalCamera->CompositeMode = ESceneCaptureCompositeMode::SCCM_Composite;
    PortalCamera->bCaptureEveryFrame = false;
    PortalCamera->bCaptureOnMovement = false;
    PortalCamera->bAlwaysPersistRenderingState = true;

    ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
    ForwardDirection->SetupAttachment(GetRootComponent());

    PlayerDetection = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerDetection"));
    PlayerDetection->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void APortalActor::BeginPlay()
{
	Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimerForNextTick(this, &APortalActor::Init);
    }
	
}

// Called every frame
void APortalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!bInitialized)
    {
        return;
    }

    if (bSmoothOrientationAfterTeleport)
    {
        SmoothPlayerOrientation(DeltaTime);
    }

    if (!WasRecentlyRendered())
    {
        return;
    }

    UpdateFOV();

    UpdateSceneCaptureRecursive(FVector::ZeroVector, FRotator::ZeroRotator);

    UpdateViewportSize();

    ShouldTeleport(DeltaTime);
}

void APortalActor::Init()
{
    DynamicPortalMat = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, PortalMaterial);
    if (DynamicPortalMat == nullptr)
    {
        return;
    }

    PortalPlane->SetMaterial(0, DynamicPortalMat);

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController == nullptr)
    {
        return;
    }

    int32 ViewportSizeX, ViewportSizeY; 
    PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
   
    PortalRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, ViewportSizeX, ViewportSizeY);
    if (PortalRenderTarget == nullptr)
    {
        return;
    }

    DynamicPortalMat->SetTextureParameterValue("Texture", PortalRenderTarget);
    LinkedPortal->PortalCamera->TextureTarget = PortalRenderTarget;

    SetClipPlanes();

    FVector OffsetDistance = ForwardDirection->GetForwardVector() * DefaultOffsetAmount;
    DynamicPortalMat->SetVectorParameterValue("OffsetDistance", UKismetMathLibrary::MakeColor(OffsetDistance.X, OffsetDistance.Y, OffsetDistance.Z, 1.f));

    bInitialized = true;
}

void APortalActor::SetClipPlanes()
{
    if (LinkedPortal == nullptr)
    {
        return;
    }

    FVector ForwardVector = ForwardDirection->GetForwardVector();
    PortalCamera->bEnableClipPlane = true;
    PortalCamera->ClipPlaneBase = PortalPlane->GetComponentLocation() + (ForwardVector * 3.f);
    PortalCamera->ClipPlaneNormal = ForwardVector;
}

void APortalActor::UpdateFOV()
{
    if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
    {
        float PlayerFOV = CameraManager->GetFOVAngle();
        if (PortalCamera->FOVAngle != PlayerFOV)
        {
            PortalCamera->FOVAngle = PlayerFOV;
        }
    }
}

void APortalActor::UpdateSceneCaptureRecursive(FVector Location, FRotator Rotation)
{  
    if (LinkedPortal == nullptr || LinkedPortal->PortalCamera == nullptr)
    {
        return;
    }

    if (CurrentRecursionDepth == 0)
    {
        APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
        if (CameraManager == nullptr)
        {
            return;
        }

        FVector TemporaryLocation = UpdateLocation(CameraManager->GetTransformComponent()->GetComponentLocation());
        FRotator TemporaryRotation = UpdateRotation(CameraManager->GetTransformComponent()->GetComponentRotation());
        
        ++CurrentRecursionDepth;
        UpdateSceneCaptureRecursive(TemporaryLocation, TemporaryRotation);

        LinkedPortal->PortalCamera->SetWorldLocationAndRotation(TemporaryLocation, TemporaryRotation);
        LinkedPortal->PortalCamera->CaptureScene();
        CurrentRecursionDepth = 0;
    }
    else if(CurrentRecursionDepth < MaxRecursionDepth)
    {
        FVector TemporaryLocation = UpdateLocation(Location);
        FRotator TemporaryRotation = UpdateRotation(Rotation);
        ++CurrentRecursionDepth;
        UpdateSceneCaptureRecursive(TemporaryLocation, TemporaryRotation);
        LinkedPortal->PortalCamera->SetWorldLocationAndRotation(TemporaryLocation, TemporaryRotation);
        LinkedPortal->PortalCamera->CaptureScene();
    }
    else
    {
        LinkedPortal->PortalCamera->SetWorldLocationAndRotation(UpdateLocation(Location), UpdateRotation(Rotation));
        PortalPlane->SetVisibility(false);
        LinkedPortal->PortalCamera->CaptureScene();
        PortalPlane->SetVisibility(true);

    }
}

void APortalActor::UpdateViewportSize()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController == nullptr)
    {
        return;
    }

    int32 ViewportSizeX, ViewportSizeY;
    PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

    if (ViewportSizeX != PortalRenderTarget->SizeX || ViewportSizeY != PortalRenderTarget->SizeY)
    {
        //Resize needed, update the render target size
        UKismetRenderingLibrary::ResizeRenderTarget2D(PortalRenderTarget, ViewportSizeX, ViewportSizeY);
    }
}

void APortalActor::UpdateOffsetDistance(FVector Point, FVector PortalLocation, FVector PortalNormal, float DeltaTime)
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter == nullptr)
    {
        return;
    }

    //bool bIsInFront = FVector::DotProduct(PortalNormal, Point - PortalLocation) >= 0.f;
    if (bWasInFront)
    {
        if (UPawnMovementComponent* PlayerMovement = PlayerCharacter->GetMovementComponent())
        {
            //Amount of velocity in direction of portal
            float VelocityTowardsPortal = FVector::DotProduct(PortalNormal * -1.f, PlayerMovement->Velocity);
            float DistanceTravelledThisTick = VelocityTowardsPortal * DeltaTime;
            if (VelocityTowardsPortal > 0.f && FVector::Distance(PlayerCharacter->GetActorLocation(), GetActorLocation()) < OffsetTriggerDistance)
            {  
                float OffsetAmount = OffsetMultiplier * DistanceTravelledThisTick;
                
                //Adjust this value if the player is near the portal and moving quickly toward it
                FVector OffsetDistance = ForwardDirection->GetForwardVector() * OffsetAmount;

                DynamicPortalMat->SetVectorParameterValue("OffsetDistance", UKismetMathLibrary::MakeColor(OffsetDistance.X, OffsetDistance.Y, OffsetDistance.Z, 1.f));
                return;
            }
        }
    }

    FVector OffsetDistance = ForwardDirection->GetForwardVector() * DefaultOffsetAmount;
    DynamicPortalMat->SetVectorParameterValue("OffsetDistance", UKismetMathLibrary::MakeColor(OffsetDistance.X, OffsetDistance.Y, OffsetDistance.Z, 1.f));
}

void APortalActor::ShouldTeleport(float DeltaTime)
{
    //A player was detected in the bounds, handle teleport checks
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter == nullptr)
    {
        return;
    }

    //#TODO_JShucker: Does this need to be the camera on the player character? Can we just use the camera manager's camera?
    UCameraComponent* PlayerCamera = PlayerCharacter->GetComponentByClass<UCameraComponent>();
    if (PlayerCamera == nullptr)
    {
        return;
    }

    UpdateOffsetDistance(PlayerCamera->GetComponentLocation(), GetActorLocation(), ForwardDirection->GetForwardVector(), DeltaTime);

    TSet<AActor*> OverlappingPlayers;
    PlayerDetection->GetOverlappingActors(OverlappingPlayers, PlayerCharacterClass);
    if (OverlappingPlayers.IsEmpty())
    {
        return;
    }

    if (IsPointCrossingPortal(PlayerCamera->GetComponentLocation(), GetActorLocation(), ForwardDirection->GetForwardVector()))
    {
        TeleportCharacter();
    }
}

bool APortalActor::IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter == nullptr)
    {
        return false;
    }

    FVector MovementDirection = FVector::ZeroVector;
    if (UPawnMovementComponent* PlayerMovement = PlayerCharacter->GetMovementComponent())
    {
        MovementDirection = PlayerMovement->Velocity.GetSafeNormal();
    }

    bool bIsInFront = FVector::DotProduct(PortalNormal, Point - PortalLocation) >= 0.f;
    bool bMovingTowardPortal = FVector::DotProduct(PortalNormal, MovementDirection) < 0.f;
    
    FPlane MathPortalPlane = UKismetMathLibrary::MakePlaneFromPointAndNormal(PortalLocation, PortalNormal);

    float T;
    FVector Intersection;
    bool bIsIntersect = UKismetMathLibrary::LinePlaneIntersection(LastPosition, Point, MathPortalPlane, T, Intersection);

    bool IsCrossingPortal = bIsIntersect && !bIsInFront && bWasInFront && bMovingTowardPortal;
   
    bWasInFront = bIsInFront;
    LastPosition = Point;

    return IsCrossingPortal;
}

void APortalActor::TeleportCharacter()
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter == nullptr)
    {
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController == nullptr)
    {
        return;
    }

    FTransform PortalTransform = GetActorTransform();
 
    //Update Actor Location
    FVector PortalScale = PortalTransform.GetScale3D();
    FTransform UpdatedTransform = UKismetMathLibrary::MakeTransform(PortalTransform.GetLocation(), PortalTransform.GetRotation().Rotator(), 
        FVector(PortalScale.X * -1.f, PortalScale.Y * -1.f, PortalScale.Z));
    FVector UpdatedLocation = UKismetMathLibrary::InverseTransformLocation(UpdatedTransform, PlayerCharacter->GetActorLocation());
    UpdatedLocation = UKismetMathLibrary::TransformLocation(LinkedPortal->GetActorTransform(), UpdatedLocation);
   
    //Update Actor Rotation
   
    PlayerCharacter->SetActorLocationAndRotation(UpdatedLocation, UpdateRotation(PlayerCharacter->GetActorRotation()));

    //Update Control Rotation
    FRotator UpdatedControlRotation = UpdateRotation(PlayerController->GetControlRotation());
    PlayerController->SetControlRotation(UpdatedControlRotation);

    //Update Velocity
    if (UPawnMovementComponent *  PlayerMovement = PlayerCharacter->GetMovementComponent())
    {
        PlayerMovement->Velocity = UpdateVelocity(PlayerMovement->Velocity);
    }

    //Smooth player orientation
    if (UKismetMathLibrary::Round(UpdatedControlRotation.Roll) != 0.f || UKismetMathLibrary::Round(PlayerCharacter->GetActorRotation().Roll) != 0.f)
    {
        SmoothOrientationTimer = 0.f;
        InitialControlRotation = UpdatedControlRotation.Quaternion();
        InitialPlayerRotation = PlayerCharacter->GetActorRotation().Quaternion();
        FinalControlRotation = FRotator(UpdatedControlRotation.Pitch, UpdatedControlRotation.Yaw, 0.f).Quaternion();
        FinalPlayerRotation = FRotator::ZeroRotator.Quaternion();
        bSmoothOrientationAfterTeleport = true;
       
    }
    //#TODO_JShucker: See if this can be done within the portal
    /*
    if (ANECPlayerCharacter* NECPlayerCharacter = Cast<ANECPlayerCharacter>(PlayerCharacter))
    {
        NECPlayerCharacter->SmoothPlayerOrientation(UpdatedControlRotation);
    }
    */

    //Camera cut
    if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
    {
        CameraManager->SetGameCameraCutThisFrame();
    }
}

FVector APortalActor::UpdateVelocity(FVector OldVelocity)
{
    FVector NewVelocity = OldVelocity.GetSafeNormal();
    NewVelocity = UpdateRotationAxis(NewVelocity);
    return (NewVelocity * OldVelocity.Length());
}

FVector APortalActor::UpdateLocation(FVector OldLocation)
{
    if (LinkedPortal == nullptr)
    {
        return OldLocation;
    }

    FTransform PortalTransform = GetActorTransform();
    FVector ActorScale = PortalTransform.GetScale3D();
    ActorScale.X *= -1.f;
    ActorScale.Y *= -1.f;
    FTransform UpdatedTransform = UKismetMathLibrary::MakeTransform(PortalTransform.GetLocation(), PortalTransform.GetRotation().Rotator(), ActorScale);
    FVector UpdatedLocation = UKismetMathLibrary::InverseTransformLocation(UpdatedTransform, OldLocation);
    return UKismetMathLibrary::TransformLocation(LinkedPortal->GetActorTransform(), UpdatedLocation);  
}

FRotator APortalActor::UpdateRotation(FRotator OldRotation)
{   
    if (LinkedPortal == nullptr)
    {
        return OldRotation;
    }

    //Get rotation to modify as a quaternion
    FQuat OldRotationQuat = FQuat(OldRotation);
    

    //Convert rotation from world space to local space of entrance portal
    FTransform SourceActorTransform = GetActorTransform();
    FQuat LocalQuat = SourceActorTransform.GetRotation().Inverse() * OldRotationQuat;
    //Modify to find equivalent rotation relative to exit portal
    LocalQuat = ModificationRotation * LocalQuat;
    //Convert rotation fromm local space to world space relative to exit portal
    FTransform LinkedPortalTransform = LinkedPortal->GetActorTransform();
    FQuat NewWorldQuat = LinkedPortalTransform.GetRotation() * LocalQuat;

    return NewWorldQuat.Rotator();
}

FVector APortalActor::UpdateRotationAxis(FVector RotationAxis)
{
    if (LinkedPortal == nullptr)
    {
        return RotationAxis;
    }

    FTransform ActorTransform = GetActorTransform();
    //Transform direction from world space to local space of entrance portal
    FVector UpdatedDirection = UKismetMathLibrary::InverseTransformDirection(ActorTransform, RotationAxis);
    // Given a direction vector and a surface normal, returns the vector reflected across the surface normal.
    //Produces a result like shining a laser at a mirror.
    UpdatedDirection = UKismetMathLibrary::MirrorVectorByNormal(UpdatedDirection, FVector(1.f, 0.f, 0.f));
    UpdatedDirection = UKismetMathLibrary::MirrorVectorByNormal(UpdatedDirection, FVector(0.f, 1.f, 0.f));
    //Transfrom new direction from local space of entrance portal to world space of exit portal
    FTransform LinkedPortalTransform = LinkedPortal->GetActorTransform();
    return UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UpdatedDirection);
}

void APortalActor::SmoothPlayerOrientation(float DeltaTime)
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (PlayerCharacter == nullptr)
    {
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController == nullptr)
    {
        return;
    }

    SmoothOrientationTimer += DeltaTime;
    if (SmoothOrientationTimer >= SmoothOrientationTimeMax)
    {
        SmoothOrientationTimer = SmoothOrientationTimeMax;
        bSmoothOrientationAfterTeleport = false;
    }

    FQuat SmoothedControlRotation = FMath::InterpEaseInOut<FQuat>(InitialControlRotation, FinalControlRotation, SmoothOrientationTimer / SmoothOrientationTimeMax, 2.f);
    FQuat SmoothedPlayerRotation = FMath::InterpEaseInOut<FQuat>(InitialPlayerRotation, FinalPlayerRotation, SmoothOrientationTimer / SmoothOrientationTimeMax, 2.f);

    PlayerController->SetControlRotation(SmoothedControlRotation.Rotator());
    PlayerCharacter->SetActorRotation(SmoothedPlayerRotation.Rotator());
}
