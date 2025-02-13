// Fill out your copyright notice in the Description page of Project Settings.


#include "NECPlayerCharacter.h"
#include "NECPlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANECPlayerCharacter::ANECPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANECPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
/*void ANECPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ANECPlayerController* PC = Cast<ANECPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->UpdatePortalManager(DeltaTime);
	}
}
*/

void ANECPlayerCharacter::TickActor(float DeltaSeconds, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaSeconds, TickType, ThisTickFunction);

	if (ANECPlayerController* PC = Cast<ANECPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->UpdatePortalManager(DeltaSeconds);
	}
}

// Called to bind functionality to input
void ANECPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

