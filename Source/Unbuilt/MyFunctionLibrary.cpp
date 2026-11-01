// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UMyFunctionLibrary::PlayMenuSound(const UObject* WorldContextObject, USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(WorldContextObject, Sound);
	}
}

