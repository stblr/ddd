#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

void REPLACED(GDSetArray)(u32 attr, const void *base, u8 stride);
REPLACE void GDSetArray(u32 attr, const void *base, u8 stride);
void REPLACED(GDSetArrayRaw)(u32 attr, u32 base, u8 stride);
REPLACE void GDSetArrayRaw(u32 attr, u32 base, u8 stride);
