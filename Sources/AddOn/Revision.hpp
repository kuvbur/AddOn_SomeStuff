//------------ kuvbur 2022 ------------
#pragma once
#if !defined(REVISION_HPP)
#define REVISION_HPP
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include "APICommon27.h"
#endif // AC_27
#include "DG.h"
#include "Helpers.hpp"

void SetRevision (void);

#endif
