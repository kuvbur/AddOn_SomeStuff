# Создание QR кода на основе значения свойств

[Видео с примером от Александра Пушкарёва](https://youtu.be/Uo_6s2QSULU?si=g8V7Hm4CZpV_lURu) [Rutube](https://rutube.ru/video/8e3f6bc59758fbac144e7c57c035695e/?r=wd)

Аддон осуществляет подготовку данных для отрисовки QR кода отдельным [макросом](https://github.com/kuvbur/gdl_macro/blob/master/gdl/macro_qrcode.gsm). Вызов данного макроса можно добавить в 2д или 3д скрипты своих библиотечных элементов, либо воспользоваться готовыми объектом и выносной надписью [из этой библиотеки](https://github.com/kuvbur/gdl_bibl/tree/master/kuvbur_QRCode).

Для перевода значения свойства в QR код необходимо создать свойство с типом данных Строка, в описании указать `Sync_from{QRCode:Property:ИМЯ_СВОЙСТВА}`.

При использовании выносной надписи указать это свойство в качестве источника данных.

Для передачи данных в объект необходимо создать IFC свойство с именем QRCode и типом ifcText и задать ему в качестве правила свойство, куда аддон вывел подготовленную строку.
![Пример](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/qr_ifc-ru.PNG)

Код для вставки в объект (пример, выводит QR код в интерактивный каталог в случае, если в ПМВ включен показ маркеров окон и дверей):

```
!!!!Для отрисовки в 3д добавить в объект текстовый параметр macro_qr_txt и код:
!!!---------------------------------------------
!!!-----------Основной скрипт (в начало)--------
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
!!!-----------3д скрипт (в начало)--------
!!!---------------------------------------------
!show=0 : n = REQUEST ("window_show_dim", "", show)
!if show and GLOB_PREVIEW_MODE=2 and GLOB_SCRIPT_TYPE=3 then
!	CALL "macro_qrcode" parameters macro_qr_txt = macro_qr_txt, macro_qr_A = 1, is_show3d=1
!	end
!endif

!!!!Для отрисовки в 2д добавить код, дополнительные параметры можно не создавать
!!!---------------------------------------------
!!!-----------2д скрипт (в начало)--------
!!!---------------------------------------------
!show=0 : n = REQUEST ("window_show_dim", "", show)
!if show and GLOB_PREVIEW_MODE=2 then
!	CALL "macro_qrcode" parameters all, is_show3d=1
!	end
!endif
```
