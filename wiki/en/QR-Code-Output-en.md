# Generating QR Code Based on Property Values

[Video example by Alexander Pushkarev](https://youtu.be/Uo_6s2QSULU?si=g8V7Hm4CZpV_lURu) [Rutube](https://rutube.ru/video/8e3f6bc59758fbac144e7c57c035695e/?r=wd)

The add-on prepares data for rendering a QR code using a separate [macro](https://github.com/kuvbur/gdl_macro/blob/master/gdl/macro_qrcode.gsm). You can call this macro in the 2D or 3D scripts of your library elements, or use the ready-made object and label from [this library](https://github.com/kuvbur/gdl_bibl/tree/master/kuvbur_QRCode).

To convert a property value into a QR code, create a property with the data type String and specify `Sync_from{QRCode:Property:PROPERTY_NAME}` in its description.

When using the label, specify this property as the data source.

To pass data to the object, create an IFC property named QRCode with the type ifcText and set its rule to the property where the add-on outputs the prepared string.
![Example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/qr_ifc-ru.PNG)

Code to insert into the object (example, displays QR code in the interactive catalog if the display of window and door markers is enabled in the Floor Plan):

```
!!!!For rendering in 3D, add a text parameter macro_qr_txt to the object and the code:
!!!---------------------------------------------
!!!-----------Main script (at the beginning)--------
!!!---------------------------------------------
!if strlen(macro_qr_ifc_property_name)<1 then macro_qr_ifc_property_name = "QRCode"
!
!if strlen(macro_qr_ifc_property_name)>0 then
!	DIM parNamesArray[]
!	n = APPLICATION_QUERY ("OwnCustomParameters", "GetParameterNames(IFC.Folder)", parNamesArray)
!	ifc_id = ""
!	for i=1 to vardim1(parNamesArray) step 3
!		if parNamesArray[i+1]=macro_qr_ifc_property_name then
!			ifc_id = parNamesArray[i]
!			i=vardim1(parNamesArray)
!		endif
!	next i
!	if strlen(ifc_id)>0 then
!		ifc_id = "GetParameter("+ifc_id+")"
!		n = APPLICATION_QUERY ("OwnCustomParameters", ifc_id , parValue)
!		macro_qr_txt = parValue
!	endif
!endif
!if strlen(macro_qr_txt)>0 then parameters macro_qr_txt=macro_qr_txt
!!!---------------------------------------------
!!!-----------3D script (at the beginning)--------
!!!---------------------------------------------
!show=0 : n = REQUEST ("window_show_dim", "", show)
!if show and GLOB_PREVIEW_MODE=2 and GLOB_SCRIPT_TYPE=3 then
!	CALL "macro_qrcode" parameters macro_qr_txt = macro_qr_txt, macro_qr_A = 1, is_show3d=1
!	end
!endif

!!!!For rendering in 2D, add the code, additional parameters are not required
!!!---------------------------------------------
!!!-----------2D script (at the beginning)--------
!!!---------------------------------------------
!show=0 : n = REQUEST ("window_show_dim", "", show)
!if show and GLOB_PREVIEW_MODE=2 then
!	CALL "macro_qrcode" parameters all, is_show3d=1
!	end
!endif
```