virtual FVector GetAimTargetLocation_Implementation() const override
{
    return GetMesh()->GetSocketLocation(TEXT("head"));
}

virtual bool CanBeAimTarget_Implementation() const override
{
    return IsAlive();
}
