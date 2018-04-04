#pragma once

#define FIELD_GET(reg, field) ((reg & field) >> (field##_Pos))
#define FIELD_IS_SET(reg, field) ((reg & field) != 0)
