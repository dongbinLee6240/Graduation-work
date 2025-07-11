// Fill out your copyright notice in the Description page of Project Settings.


#include "RFTGameInstance.h"

void URFTGameInstance::SetUID(const FString& NewUID)
{
	UID = NewUID;
}

void URFTGameInstance::SetCID(const FString& NewCID)
{
	CID = NewCID;
}

FString URFTGameInstance::GetUID() const
{
	return UID;
}

FString URFTGameInstance::GetCID() const
{
	return CID;
}
