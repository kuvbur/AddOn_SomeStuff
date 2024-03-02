//------------ kuvbur 2022 ------------
#if !defined (RULEPK1_HPP)
#define	RULEPK1_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27

void ReservationCHandler(const GS::HashTable<API_Guid, short>& reserved,
	const GS::HashSet<API_Guid>& released,
	const GS::HashSet<API_Guid>& deleted);

void ElementEventHandler(const API_NotifyElementType* elemType, API_ActTranPars& actTranPars);

#endif