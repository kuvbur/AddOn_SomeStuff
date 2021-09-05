void	Do_ElementMonitor();
GSErrCode SyncParamAndProp(const API_Guid& elemGuid, API_Property& property, const GS::UniString& paramName, int& syncdirection);
GSErrCode __ACENV_CALL	ElementEventHandlerProc(const API_NotifyElementType* elemType);
static void SyncData(const API_Guid& elemGuid);

GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
GSErrCode WriteParam2Prop(const API_Guid& elemGuid, API_Property& property, const GS::UniString& paramName);

bool CheckElementType(const API_ElemTypeID& elementType);
