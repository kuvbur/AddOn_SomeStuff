# Roombook

## Native3D-покрытия профильных стен (#333)

Адресный контракт AC25 Windows/C++14, 2026-11-15; рабочий `RoombookData.cpp` SHA256 `4664a9a92f90ce5105ebc4e9cf6dc57e1151a47ae97dbd98294b2fea3786f6f3`. Остальные разделы сохраняют исторические привязки. [по коду]

- `SurfaceMaterialName_Read` (`RoombookData.cpp:375`) читает `API_MaterialID` по `API_AttributeIndex`, передаёт адрес собственной `SurfaceMaterialNameRead.name` в `API_Attr_Head.uniStringNamePtr`, сохраняет имя и `GSErrCode` через `Add` в `RoomSurfaceCache.surfaceMaterialNames`. Успех и ошибка лениво кэшируются до первой записи модели; никаких фиксированных ANSI-буферов для сравнения. SDK25: `APIdefs_Attributes.h:71–85`, `ACAPinc.h:414`; существующий Unicode-call-site в этом файле проверен. [по коду; установленные SDK25 headers]
- `RoomSurfaces_Read` (`:388`) после успешного Get3DInfo только для Wall читает исходный элемент через прежний `Element_Read` и определяет `wall.modelElemStructureType == API_ProfileStructure` один раз на snapshot. Перед Push исключает только Native3D-грани такой стены, чьё имя покрытия содержит `PARTIGNOREMATERIAL` (`!NOFIN!`, case-sensitive). Обычные/Composite Wall в native fallback, в том числе маршрут generic Opening, не получают material-фильтр; для не-Wall новых element/material-чтений нет. GUID snapshot cache остаётся независимым от caller. При `cache == nullptr` локальный `RoomSurfaceCache` разделяет имена между гранями/телами одного чтения, но не разными вызовами. SDK body/polygon indices и пустые тела сохраняются; фильтр высоты #327 не меняется. [по коду]
- Намеренный пропуск маркированной грани не добавляет ошибку. Неизвестный тип/ошибка Element_Read даёт stage `profileWall` и не публикует стеновые грани; ошибка покрытия — `surfaceMaterialName` с исходным кодом SDK, эта грань не публикуется. Snapshot получает ReadError. **Существующий `RoomSurfaces_Partition` (`:1039`) отвергает весь source с `!IsComplete()`**, даже если другие грани прочитаны: отказ не ограничен одной гранью в конечном partition. Штатный cleanup не менялся и может удалить прежнюю отделку исключённого источника. [по коду]
- Изолированный стенд исполняет дословные production `RoomSurfaces_Read`, `SurfaceMaterialName_Read`, `Element_Read`, minimum-height predicate и error-recording на реальных SDK25/GS/project types. 3D info/component/polygon и API reads — controlled seams. Fixed expectations одинаковы: baseline105checks/24FAIL → current155checks/0FAIL; количество включает зависящие от выполненных API/ошибок проверки. Покрыты repeated-index across bodies/GUIDs, snapshot/new/null cache, failed reads и новый запуск после ошибки, длинное Unicode-имя с маркером за ANSI-пределом, empty/case-sensitive names, partial failure и обычные/Composite стены/другие типы. Сам generic Opening/геометрический polygon reader, partition и writers в этом стенде не исполняются. [выполненный standalone; native не проверено]
- Полный Data TU SDK25/MSVC14.44 C++14 `/WX /c`: baseline/current exit0, OBJ только в scratch. Fresh isolated clangd version1: прежние8diagnostics, diff-mapped ranges совпали, новых0;2146headers неизменны в каждом прогоне. clang-format/diff-check пройдены. Ни VS/host/PLN, ни APX build/load, commit/push не выполнялись. Source tests и generated/progress не менялись в этом этапе по ограничению scope; generated-координаты ниже могут быть историческими. Evidence: Hermes scratch/profile-ignore-333/{red-final,green-final}/{report.json,results.log}, tu/{baseline,current}/{result.json,publishDiagnostics.json,documentSymbol.json}, tu/current/comparison.json. Повтор: `python C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/profile-ignore-333/run-regression.py green-repeat`. [выполненные standalone/TU/LSP; native не проверено]

## Размещение сечения профильной стены и разрыв пола102 (#294 / #329)

Адресный срез AC25 Windows/C++14, 2026-11-15; рабочий `RoombookData.cpp` SHA256 `3bf67b403dc1eaf3491e3a940ebdd854b32e19df52f8eff9f8bf39b0026a8911`. Прочие разделы сохраняют свои исторические привязки. [по коду]

- `ProfileWall_BuildFloorExclusion` (`RoombookData.cpp:4200`) размещает знаковое сечение instance `stretchedProfile` по левой нормали `(-dy,dx)/length` от `begC`. Повторное отражение по `wall.flipped` удалено: на двух незеркальных native-экземплярах прежняя формула переносила полосу на неверную сторону/смещала её. Штриховки не ребазируются и не растягиваются повторно; дополнительные `offset`/`offsetFromOutside` не вводятся. Документация SDK25 сама по себе абсолютное направление profile X не установила; основание коррекции — независимые native-сечения четырёх экземпляров и SDK replay. Универсальная native-проверка всех modifiers/SEO и других версий этим не заявляется. [по коду; native-захваты; выполненный standalone]
- Reader `ProfileWalls_ReadFloorExclusion` (`:4301`) по-прежнему читает instance memo и сечёт на исходном низе зоны. `FloorContours_SubtractProfileWalls` (`:4269`) сохраняет реальные вычитания, отдельные интервалы/дверные проходы и прежнюю обработку Slab/Mesh/потолков/уклонов/повторного GUID. Допуски, Unify-политика и API чтения/записи не изменены. [по коду]
- Exact SDK25 replay всех сохранённых штриховок/дуг четырёх стен и production subtraction: baseline15FAIL → current0FAIL. Прежние2части пола102 площадью8.8561749664м² заменены одним компонентом9.8792251554м²; обе разности покрытия с исходным контуром равны0. На трёх внутренних срезах каждой стены проверены вхождение profile-полосы в независимое native-сечение и перекрытие его середины, не полное совпадение границ/материалов. Дополнительный независимый native-ray/аналитический аудит подтвердил знак; parent повторил Python и SDK probe (compile/link/run0/0/0). На Z0 SDK Intersect пропускает крайние20-мм штриховки около нижней границы, тогда как полная аналитика raw hatches совпадает с native; на Z=.01 эти штриховки возвращаются. Это наблюдаемый пограничный эффект сечения, не доказанная политика исключения материалов. Для дуговых сечений стен1–3 на Z=.35/.4/.425 подтверждены6/4/8границ с невязкой<4.1e-9м. Сложные высокие сечения wall4 с нечётными crossings не приняты как oracle и не объявлены product FAIL. В sign-only правке пограничное сечение/допуски не менялись. Полнота 3D-материального покрытия и торцовые соединения не объявляются проверенными. Evidence: scratch/profile-native-critical-audit-20261115/audit.md и parent-*-replay.log. [выполненные standalone/independent audit/parent replay]
- Постоянный профильный блок `TestRoomSurfacePartition` на реальном SDK25: одинаковые35assertions дают baseline9FAIL → current0FAIL; прежние projection/Mesh/mixed646assertions —0FAIL. Три полные TU Data/TestMeshFloor/TestRoombookGeometry с `TESTING`, MSVC C++14 `/WX /c` —exit0; fresh isolated clangd version1 имеет те же8/1/0 сообщений, что baseline. Тёплый MCP symbol snapshot был устаревшим; координаты этих трёх функций сверены по fresh isolated documentSymbol и текущему source, только они актуализированы в symbols.json; прежние имена/остальной каталог/callgraph не исправлялись. [выполненные standalone/TU/LSP]
- Canonical `BuildAddOn.py -c config.json -v 25 -r -l INT` —exit0, создан `Build/SomeStuff/25/INT/RelWithDebInfo/SomeStuff.apx` SHA256 `fdd1ba04b2ff9bbaa3e612b6a245e778e1987a160f04f31f50e2712a45d0c289`. SDK linker предупредил LNK4099 об отсутствующем `API_c.pdb`. Оба PLN не изменились; APX в Archicad не загружалась и новая native-приёмка102 не выполнена. Независимое ревью только точной production/test-дельты: No issues found in inspected scope; parent повторно сверил live hashes с ревью и build evidence. Прежние visibility/floor-изменения этим ревью не покрыты. Артефакты: Hermes scratch/profile-transform-fix-20261115/{verification.json,independent-exact-delta-review.md,harness,persistent-red,persistent-current,canonical-release-build.log}; fresh TU/LSP — zone102-unified-20261115/tu-profile-transform. [выполненная сборка; статическое ревью; native не проверено]

### Native-приёмка пола102 и границы общего снимка

AC25 RUS7000, 2026-11-15: после реальной VS Debug|x64 INT компиляции Data/TestRoombookGeometry и relink загружена Debug APX `fd4bdae6cc3c19df8c311bbae0718c02144b8236efbdf1ceecf86ed43754354f`, source `3bf67b40`. Адресный запуск102 создал22элемента; независимые inventory/ownership подтвердили все22. Пол `0F521F68-D296-4F4F-A7A2-E153524A9B93` — один Slab/контур,5координат,0дуг/отверстий,9.8792251554м²,−0.05…+0.05м. Координаты до/после профильного вычитания и writer совпали с native Get/GetMemo на повторе. Повтор не добавил/не удалил GUID, пол и ownership сохранились. У17стеновых GUID изменились bbox; сходство unordered bbox до1.3e-8м не доказывает равенство тел. Unsupported finish pieces сохраняется; разница Zone_MeasuredArea−floor0.0917037476м² не устранена. Полная стеновая приёмка, другие зоны и полный TestKit не заявляются. Оба PLN не сохранены/неизменны, точки восстановлены, host оставлен Run. Evidence: scratch/profile-native-acceptance-20261115/acceptance.json. Указанные выше заявления о невыполненной native-проверке относятся к предыдущему этапу. [native-проверка]

В общий commit по команде «коммить всё дерево» также входит последующая правка #333: кэш собственных Unicode-имён покрытий `surfaceMaterialNames`, фильтр PARTIGNOREMATERIAL только для native-граней профильных стен, сохранение ошибки чтения имени. Этот более поздний код не входил в загруженную native APX. Текущий общий снимок проверен отдельно: три полные TU Data/TestMeshFloor/TestRoombookGeometry, SDK25/MSVC C++14 `/WX /c`, exit0; fresh LSP без новых диагностик. Это не новая APX build/load и не native-проверка #333. Evidence: scratch/zone102-unified-20261115/tu-precommit-all-20261115/result.json. Координаты в предыдущих разделах относятся к их указанным source snapshot и не переносятся на добавленные #333 строки автоматически. [по коду; изолированная компиляция/LSP; native #333 не проверено]

## Видимость оснований

- При обработке оснований зоны проверка видимости слоя и реконструкции выполняется перед проверкой класса отделки. Она применяется к кандидатам поверхностей, обработке связанных элементов, спискам Slab/Mesh и участкам балок. Участки балок дополнительно проверяются по отображению конструкций. Класс сегмента проверяется отдельно. [по коду]
- Предыдущая политика ошибок чтения классификации сохранена: в сборе кандидатов ошибка делает набор неполным, в legacy-проверке ошибка трактуется как отсутствие класса отделки. Путь generic Opening отдельно не изменён; полная видимость по нему и фактическое поведение в Archicad не проверены. [по коду]
- При пересчёте штатная очистка может удалить прежнюю отделку исключённого скрытого основания; владелец подтвердил такую политику. [решение владельца; по коду]

## Отбор оснований и единый Slab-пол (#329 / #331)

Статус: текущая правка проверена офлайн; независимое production-ревью принято с совпадающими live-хешами, поздние дополнения тестов просмотрены родителем и выполнены на текущем snapshot. Реальная плита при равной отметке имеет приоритет; отдельный mixed Mesh/Slab-проход без nominal сохранён. Native-приёмка одного связного пола102 на новой APX ещё не выполнена. Legacy PopLargest не восстанавливает ранее отброшенные компоненты. [решение владельца; по коду, review-acceptance.md и выполненной регрессии]

Адресный контракт AC25 Windows/C++14; рабочий `RoombookData.cpp` SHA256 `754a7ac73fa88ae9561b8f8513852590d5ca5fbf655bb440042ea87fcac11e3c`. Остальные разделы и callgraph сохраняют свои исторические привязки. [по коду]

- `AnalyticGeometry::SlabMeetsBoundary` проверяет исходный интервал плиты: пол — `bottom <= zoneBottom && top >= zoneBottom - 0.5`, потолок — `top >= zoneTop && bottom <= zoneTop + 0.5`. Ровно 500 мм включается; пересечение границы допускается независимо от толщины. Интервалы целиком внутри зоны, не пересекающие нужную границу, не подходят. Невалидные/нечисловые значения отклоняются; равные bottom/top допустимы только в helper для одногранного snapshot. `SlabRole` требует положительные интервалы плиты/зоны и допускает `FloorAndCeiling`. [по коду: `RoomAnalyticGeometry.hpp:117–139`]
- `OtdSlab_FindInRoom` (`RoombookData.cpp:6918`) независимо заполняет оба списка ролей, не дублируя GUID/source/prepared geometry. `OtdSlab_CollectProjectedInputsImpl` (`:3349`) повторяет отбор для prepared/native до записи candidates/provenance: роль, добавленная контактом partition, не обходит высотный фильтр. [по коду]
- Production передаёт исходные `room.zBottom` и `room.zBottom + room.height`; `height_ceil` остаётся отметкой отделки, не границей поиска. Mixed использует исходный `roomFloor`; без `zoneBoundary` standalone сохраняет экстремум соответствующих горизонтальных граней snapshot. [по коду: `OtdSlab_BuildFiniteMixedGeometryImpl:3859`, `OtdRoom_BuildHorizontalFinishes:5603`]
- `OtdSlab_BuildProjectedFinishes` (`:3625`) сортирует конечный `nominal.poly` наравне с основаниями: пол по убыванию, потолок по возрастанию отметки. Реальная плита при точном равенстве идёт раньше nominal; неоднозначность между реальными плитами сохранена. `onlyOnSlab` отключает только nominal candidate; повторного номинального остатка и перепада от nominal нет. [по коду]
- `zoneFootprint` — отдельный необязательный XY-контур комнаты, не синоним `nominal.poly`. Production передаёт `&room.poly.poly` в Slab-only и mixed пол; оба прохода и Slab provenance получают одинаковые границы. Без аргумента сохраняется проекция snapshot и прежняя семантика частичного nominal. Контур с отверстиями не расширяется из-за 3D-проекции; `onlyOnSlab` не отменяет границы комнаты. Потолочный production-маршрут этим изменением не ограничивается. [по коду: `:3349`, `:3554`, `:5603`]
- `OtdSlab_UnifyProjectedFloorParts` (`:3579`) объединяет горизонтальные Floor-Slab до назначения GUID/избранного: совпадают основание, точная отметка, этаж, высота, reference Z, валидность, роль и поля материала. Mesh не объединяется; реальные разрывы и отверстия сохраняются. Helper применяется к результату Slab-only и после `OtdSlab_MergeMixedDomains` (`:3826`); Mesh contribution indices переносятся до добавления префикса, Slab contributions вычисляются заново. Это не слияние разных отметок ради одного элемента. [по коду]
- Mesh gate исключает целую верхнюю грань при `max(world.z) < roomFloor - 0.5` до provenance/domain. Ровно 500 мм допустимо; максимум берётся до XY-подрезки, поэтому поточечной гарантии нижнего уровня нет. Local-bottom/top clipping и отдельные Mesh-грани сохраняются. [по коду: `MeshFloorFinishes_BuildProjectedImpl:3078`]
- Parent-run дословных production projection/Mesh/mixed на SDK25 Geometry/GSRoot: одинаковые 646 assertions дают baseline61FAIL → current0FAIL; baseline имеет только адаптеры трёх сигнатур, игнорирующие новый контур. Покрыты native/prepared, by_slab, расширенный/подрезанный/разделённый/дырявый snapshot, обе mixed-обёртки, один компонент, обе разности покрытия, точные отметки/владение, отверстия и Mesh index/prefix после слияния. Узкий Mesh gate — 33/0. Это не host suite и не выполнение writers. [выполненный standalone]
- Три полные TU Data/TestMeshFloor/TestRoombookGeometry и текущие headers: SDK25/MSVC C++14 `/WX /c`, exit0; fresh isolated publishDiagnostics version1 совпал с baseline — 8/1/0, новых сообщений нет. Live-хеши совпали с frozen inputs. Новая canonical APX build/load, native102 и другие SDK/macOS — [не проверено]. Evidence: Hermes scratch/zone102-unified-20261115/{red-final,green-final,tu-baseline,tu-final}; issues остаются открытыми.

`symbols.json`: по fresh isolated clangd обновлены106координат и добавлен один helper. 27унаследованных неоднозначных/неразрешённых записей Data не объявляются актуальными; 867внешних записей и их порядок сохранены. Полная регенерация, callgraph и защищённый `_progress.md` вне среза. Evidence: `zone102-unified-20261115/symbols-audit.json`. [проверка артефакта]

## Профильные контакты и ранний фильтр стеновых граней (#252 / #327)

Адресный контракт AC25 Windows/C++14, 2026-11-13; рабочий `RoombookData.cpp` SHA256 `c35e79dd78de53e21125c6fd8686a3049a997b9970a916fc9c7fc4edfac99a06`. Остальные разделы сохраняют собственную историческую привязку; вся карточка и callgraph этим обновлением не объявляются актуальными. [по коду]

- `SurfaceWall_ClampToParts` (`RoombookData.cpp:2154`) имеет `profileWall=false` по умолчанию в объявлении. Единственный production-вызов в `OtdRoom_BuildPartitionWallFinishes` (`:5569`, site `:5621`) включает его только для `API_ProfileStructure` после прежних guards Wall/surface/slab_step, успешного чтения и исключения дуги/`APIZRel_None`. [по коду]
- Строгие интервалы на линии контакта приоритетны. Только при их отсутствии профиль допускает одну однозначную параллельную смещённую несущую линию с положительным пересечением контакта; несколько линий с разными смещениями дают отказ, без выбора ближайшей/первой и без вывода стороны по winding. Конечные границы `wallPart` и `roomEdge`, sentinel-правила, внутренние разрывы, peer `onLine`, проёмы и численные допуски сохранены; ordinary walls не получают этот fallback. [по коду]
- `RoomSurface_IsBelowMinimumWallHeight` (`RoombookData.cpp:350`) исключает только корректную негоризонтальную `Native3D`-грань `API_WallID` с мировой высотой `maxZ-minZ > 0 && < min_wall_surface_height` (`0.005` м). Равенство 5 мм, горизонтальные, нулевые/невалидные, analytic, Zone/Mesh/Column не объявляются малой стеновой гранью. Порог не заменяет `min_area` и общие допуски. [по коду]
- `RoomSurfaces_Read` (`RoombookData.cpp:375`, sites `:406–411`) применяет фильтр после `RoomSurface_ReadPolygon`/world transform и до Push, контактов и partition. Намеренный пропуск не добавляет read-error; body сохраняется даже при полном исключении его граней. SDK body/polygon IDs не перенумеровываются; политика зависит от типа элемента, а не caller, и общий GUID-кэш сохраняется. [по коду]
- Материалы профиля, `PartitionInvalidSourcePlane`, cleanup и native-writers не исправлялись. Canonical APX build/load и native-приёмка PF.N не выполнены; #327 остаётся OPEN, #252 — CLOSED. Постоянные адресные блоки исполнил parent: 54 checks/0 failures (совместный scratch-прогон 88/0); SDK25 типы реальные, внешние seams controlled, это не native-приёмка. [по коду; по отчёту parent о выполненной офлайн-проверке]

## Сечение существующих плинтусов-балок (#326)

Контракт AC25 Windows/C++14, рабочая дельта к `c51ff77`. [по коду]

- `Beam_Draw` (`RoombookDraw.cpp:772`) допускает только односегментные настройки избранного с одной схемой, двумя срезами и непустым `beamSegments`. Существующий GUID и memo читаются прежним reader; новый элемент получает memo избранного через прежний swap. [по коду]
- При обновлении переносит управляемые scalar-настройки сечения из избранного: тип структуры, форму, тип профиля, однородность, номинальные размеры и связь размеров. `profileAttr` применяется для целевой Profile, `buildingMaterial` — для целевой Basic; конечные размеры/их связь — только для неоднородного сечения. Неактивные значения сохраняются, `relativeHomeStoryLevel` (SDK25 output-only) не копируется из defaults. [по коду и установленному SDK25]
- Ненулевые допустимые размеры подготовленной геометрии имеют приоритет над избранным. Сравниваются итоговые значения; отличие выставляет `needupdate`, запись остаётся через прежний `APIMemoMask_BeamSegment`. Отдельная замена всего memo, поверхностей, segment header/owner, срезов и профильных изображений не выполняется. Оба вида плинтуса-балки используют этот writer. [по коду]
- Native-сохранение Basic↔Profile/A→B и пересчёт индивидуальных custom/stretched profile payload при смене атрибута — [не проверено]. `assemblySegmentProfiles` и `nProfiles` этой правкой не заменяются; installed SDK25 не определяет их автоматическую регенерацию в данном переходе.

## Неблокирующее разбиение поверхностей (#322)

Контракт AC25 Windows/C++14, рабочая дельта к `a3dc151`, 2026-11-11. [по коду]

- `RoomSurfaceFace_Partition` (`RoombookData.cpp:913`) при отказе проекции отдельного источника сохраняет ошибку и продолжает остальные контакты. Исключение GS при проекции/Boolean не публикует частичный `next`: остаётся прежнее покрытие `cells`. Отказ исходной грани зоны или проверки покрытия по-прежнему исключает эту грань. Допуски не изменены. [по коду]
- `RoomSurfaces_Partition` (`RoombookData.cpp:982`) сохраняет пригодные участки при ошибках других источников/граней. Конфликтующая группа одного face-key исключается целиком; одинаковые дубликаты остаются одним claim. Каждая грань зоны имеет отдельный временный результат и catch: исключение не обрывает соседние грани и не публикует половину грани. `complete=true` только при непустом результате без ошибок; `complete=false` не очищает пригодные `pieces`. [по коду]
- `RoomProcessingContext_PreparePartitions` (`RoombookData.cpp:5059`) выводит каждую `errors` через `msg_rep` с этапом, кодом и GUID зоны. Если результат неполон без конкретной ошибки, выводит общее предупреждение. Неполнота partition больше не возвращает false для всего RoomBook; отдельный отказ `Element_Read` оставлен прежним. API записи и другие gates подготовки/геометрии не меняются. [по коду]
- Ограничение: штатный cleanup существующей отделки не изменён. Прежняя отделка пропущенного участка может оказаться неиспользованной и удалиться; сохранность таких GUID этой правкой не гарантируется. Сводка остаётся расчётной по доступному набору, а не подтверждением полноты или успеха native-записи. [по коду: `RoombookDraw.cpp:3008–3037`]
- Регрессия с реальными SDK25 Geometry/GSRoot и дословными helpers/partition/PreparePartitions: RED 71/24 → GREEN 71/0; preparation/report и исключение отдельной грани — controlled seams. Семь новых постоянных assertions из TestRoombookGeometry отдельно извлечены и исполнены на том же SDK: RED 7/7 → GREEN 7/0. Полный test TU с `TESTING`, C++14, `/WX` скомпилирован; canonical AC25 Windows RelWithDebInfo INT build успешен, Data перекомпилирован и APX перелинкован. Исходная зона с детализированным профилем и native-запись не запускались. [выполненный standalone/build; native не проверено]
- Свежие isolated LSP `publishDiagnostics(version=1)`: Data имеет те же три unused-variable и пять unused-includes, что сохранённый baseline; тест без диагностик, заголовок проверен в контексте обеих TU. Независимое ревью: No issues found in inspected scope. В symbols.json обновлены лишь три изменённые production-функции по свежим documentSymbol; две test-функции сверены без изменения координат. Остальные исторические координаты и callgraph не актуализировались. [LSP/source/hash checks, read-only review]

## Свойства плинтусов на отделочных полах и потолках (#308)

Контракт AC25 Windows, рабочая дельта к `00c6dcc`. [по коду]

- `OtdSlab_WriteSkirt` (`RoombookDraw.cpp:2931`) добавляет значения в общий `paramToWrite` только для валидного OtdSlab с ненулевым GUID и `draw.write_succeeded=true`; успешный no-op допускается. Успех native-writers самих плинтусов не участвует в подсчёте: `length_skirting` остаётся длиной подготовленных отрезков в метрах. [по коду]
- Определения выбираются из PROPERTYCACHE по описаниям `some_stuff_element_tip_skirting` и `some_stuff_element_length_skirting`. Два локальных ParamValue кэшируют поиск в рамках OtdRooms_Draw; `rawName=" "` обозначает отсутствие определения. Найденные свойства обрабатываются независимо, фиктивный ключ в очередь не добавляется. [по коду]
- Длина записывается и при нулевом значении; при `is_equal(length_skirting, 0)` тип очищается. Пустой тип сам по себе не запрещает запись ненулевой длины. Повторные значения заменяются штатным AddParamValue2ParamDictElement, определения в PROPERTYCACHE не изменяются. [по коду]
- Преобразование — ConvertDoubleToParamValue / ConvertStringToParamValue; фактическая запись остаётся в OtdElements_SetSync → ElementsWrite → WriteProperty после успешной draw-транзакции. Прямой записи свойств или GDL в новом helper нет. Фактическое сохранение и повтор в Archicad — [не проверено].

## Безопасный отказ Wall writer (#317)

Адресный контракт AC25 Windows, дельта к `e0dfc7d`, 2026-11-08. Остальные исторические координаты ниже не объявляются актуальными. [по коду]

- `OtdDrawData.require_rollback` — внутренний default-false маркер текущей попытки, не JSON-поле. `Wall_Draw` (`RoombookDraw.cpp:1200`), `OtdWall_Draw` (`:2703`) и `OtdBeam_Draw` (`:2673`) сбрасывают его и флаги успешной записи на входе. [по коду]
- Для существующей стены `Wall_Draw` ставит маркер до Delete старых managed-проёмов, Change стены и попытки создать каждый новый hosted-проём. При последующем отказе маркер сохраняется; существующая родительская стена не удаляется для cleanup. Полный успешный выход, включая no-op, явно выставляет `NoError`, `write_succeeded=true`, `was_created=false` и очищает маркер. [по коду]
- Для новой стены отказ проёма сохраняет локальный cleanup: успешное удаление очищает GUID стены/проёмов и позволяет продолжение; отказ удаления передаёт `deleteErr` и требует общего отката. [по коду]
- `OtdRoom_Draw` (`:2745`) возвращает ненулевой `draw_error` при установленном маркере в циклах стен, потолочных и напольных плинтусов. Остальные writer-ошибки сохраняют прежний continue. `OtdRooms_Draw` (`:2905`) передаёт ошибку через существующий undo callback и не публикует pendingResults/Sync при отказе команды. [по коду]
- Защитные выходы после успешного Get (несовместимый тип/Profile/Poly, чужие hosted-элементы) остаются до разрушительной записи, но теперь явно имеют `APIERR_BADPARS`, не ложный успех. Геометрия, отбор чужих проёмов, R1/#316 и неблокирующие ошибки Roof/#321 не меняются. [по коду]
- SDK25: сигнатуры и undo-scope проверены по установленным ACAPinc.h и HTML; пример `Element_Test/Src/Element_Hotlink.cpp::Do_ChangeHotlink` возвращает ошибку callback для отмены предыдущих изменений. Восстановление конкретной стены/проёмов в Archicad и загрузка нового APX — [не проверено]. In-memory подготовленные GUID при откате не восстанавливаются; прерванный пакет не публикуется как успешный.

## Локальный отказ Mesh-пола / Roof (#321)

- `RoofBatch_Validate` проверяет каждый включённый Mesh-пол отдельно, сбрасывает флаги записи и сохраняет ошибку в `draw.draw_error`; отказ одного участка не завершает общий draw. Неудачный preflight пропускается в matcher и writer, но остаётся в полном JSON-наборе с `write_succeeded=false`. Геометрическая `isValid` не подменяется результатом записи. [по коду]
- `Roof_Prepare` не меняет применимость выбранной многослойки. Если отсутствует `APICWall_ForRoof`, сообщение называет избранное и индекс многослойки и предлагает включить её применение для крыш. Состав не заменяется на Basic, пользовательский атрибут автоматически не редактируется. В SDK25 `APICWall_ForSlab=2`, `APICWall_ForRoof=4` — независимые флаги. [по коду и установленному SDK25]
- `OtdRoom_Draw` после ошибки Roof продолжает остальные плиты, плинтусы и комнаты; не добавляет неудачному результату связи, классификацию или группу. Ошибка чтения старого Roof в matcher также остаётся локальной для Mesh-записи. Общие ошибки удаления, подготовки/отмены и группировки не превращены в успех. [по коду]
- `RoofBatch_CollectUnused` собирает остаток существующей отделки по `(zone, base, role)`, включая прежний `NoSet` fallback. Его удаление отложено до завершения writers: при любом отказе Mesh-участка того же набора остаток сохраняется; при полном успехе удаляется штатно. Переиспользованные GUID уже исключены matcher из всех пулов и не попадают в этот остаток. Обычная отделка сохраняет раннее удаление. [по коду]
- Сводка материалов по-прежнему расчётная и подготовлена до writers; локальная ошибка не превращает её в обмер успешно созданных элементов. `write_succeeded/draw_error` описывают результат записи отдельно. [по коду]
- Native-диагноз AC25 Windows: после изменения владельцем помещения 113, `Roof_Prepare` остановлен перед возвратом на проверке composite153, `flags=2`, Roof-бит отсутствует; API Create ещё не вызван. Прежний глобальный preflight возвращал пустой `rooms`. Это настройка входных данных и отдельная ошибка блокирующей оркестрации, не доказанный дефект геометрии/SDK. [живой отладчик, 2026-11-07]
- AC25 Windows Debug: canonical `BuildAddOn.py -c config.json -v 25` успешен, тот же APX загружен через F5; SHA256 source/APX/обоих сохранённых PLN сверены до/после запуска. Повтор зоны 113 вместо прежнего `rooms=[]` вернул 26 результатов: 22 успешных и 4 неудачных Roof; все четыре прежних Roof GUID сохранились. Полный прогон завершил 25 зон: 1172 результата, 1168 успешных и те же 4 отказа только в 113; successful GUID прочитаны из native inventory, обработаны остальные 24 зоны. Это проверка неблокирующего отказа подготовки, не инъекция ошибок native Create/Change и не приёмка всей геометрии. Модель изменена только в памяти, Save не выполнялся. [native AC25 Windows, 2026-11-07]
- Fresh LSP version1: те же пять unused-variable и одно unused-include, что на точном baseline; новых диагностик нет. Независимое ревью дельты: No issues found in inspected scope. Parent сверил manifest, hashes и все 49 selection/range/name joins. В symbols.json обновлены только 48 Draw-записей (42→48, включая прежние пропущенные helpers/forward declaration); 1026 внешних записей и их порядок сохранены. Остальной generated и callgraph не пересобирались. Дополнительный controlled SDK25/C++14 стенд не исполнялся: после двух ошибок подготовки последняя линковка завершилась LNK2019 ACAPI_Element_Tool; RED/GREEN не заявляются. Другие SDK/macOS и финальный runner перед закрытием issue не проверены.

- Перед коммитом владелец оставил только английский текст диагностики. Единственная дельта к native-снимку a24c88e8 — это сообщение; текущий Draw11bfe814 проверен повторным canonical AC25 Debug build exit0, APXaa2e7e14. Новый native-прогон не выполнялся, host закрыт; оба PLN восстановлены побайтово. 48 Draw-координат перенесены по точному diff и проверены source/name join, внешний generated-срез и порядок неизменны; fresh LSP выше относится к прежнему native-снимку.

## Минимальная площадь отделки (#320)

- `OtdRoom_FilterSmallFinishes` (`RoombookData.cpp:5839`) удаляет из окончательных массивов стен/колонн/откосов и полов/потолков записи с площадью строго меньше `min_area`; равенство сохраняется. Использует `OtdWall_GetArea` с отверстиями и наклоном; для Mesh-пола — площадь поверхности, как в quantities/JSON. Отрицательные и nonfinite значения не объявляются малой площадью. [по коду]
- Вызовы в `OtdRooms_ProcessFinishes` охватывают и раннюю ветку только с плитами, и полный маршрут после проверки provenance, вычитания профильных стен и финального устранения перекрытий, до построения индексов плинтусов. Самостоятельного площадного фильтра линейных плинтусов нет. Малые записи не передаются writers, quantities и JSON. [по коду]
- GUID существующих плит сохраняются после назначения, до изменения контуров; ссылки без сохранившегося элемента возвращаются в штатный unused-список с прежним fallback `NoSet`. Сохранившийся дубликат GUID не возвращается. [по коду]
- Малые grouped-контакты и грани балок пропускаются до отказа адаптера/флага Ambiguous; projected slab ambiguity учитывается по каждому пересечению, а не по сумме малых пересечений. Геометрическое вычитание и исходный partition не меняются. Неоднозначные контакты площадью не меньше порога сохраняют диагностику. [по коду]
- Проверка: AC25 Windows, standalone RED/GREEN и полный TU SDK25/MSVC C++14; это не сборка/загрузка аддона и не native-приёмка. Generated обновлён только для семи затронутых функций по свежему isolated clangd; остальные исторические координаты и callgraph не объявляются актуальными.

## Пустой тип плинтуса (#319)

- `FloorSkirtingSegments_Build` очищает подготовленный напольный массив и прекращает подготовку при пустом `room.tip_floor_skirting`. [по коду]
- `SkirtingSegments_Build` очищает оба массива; пустой `tip_floor_skirting` отключает только Floor, пустой `tip_ceil_cornice` — только Ceil. При обоих пустых типах геометрия не обрабатывается. `beam.tip` берётся из соответствующего поля зоны, включая потолочный тип для Ceil. [по коду]
- `OtdBeam_ProcessFinishes` исключает faces отключённого вида до расчёта толщины/отметки; исходные OtdSlab не меняются. Проверки относятся к пустой строке (`IsEmpty`), не к строке из пробелов. Штатная политика удаления ранее созданной отделки не изменена. [по коду]
- Локальная цель проверки — AC25 Windows; native-создание/удаление и другие версии [не проверено]. Исторические generated-координаты этой правкой не объявляются актуальными: MCP вернул устаревший конец namespace 9586 при текущем файле более 9748 строк.

## Тип существующей линейной отделки (#316)

Адресный контракт AC25 Windows, рабочий `RoombookDraw.cpp` SHA256 `13b75f605590c1771deed948296339cedada8bc3cbe2a74f922b9b9c31f7a5c6`; прочие исторические координаты ниже не актуализированы этим срезом. [по коду]

- `OtdRooms_AssignExistingWallFinishes` (`RoombookDraw.cpp:446`) выбирает reader целевого типа по `wall.draw.favorite.type`, как `OtdWall_Draw`: Wall, Object или Beam. `surface_geometry` нормализует `target.wall.type` только в ветке Wall и не меняет выбранный native-тип. Подбор по библиотечной части Object, роль/основание, одноразовое потребление GUID и штатное удаление остатка сохраняются. [по коду]
- `Object_Draw_Linear` (`RoombookDraw.cpp:1006`) начинает с `draw_error=APIERR_BADPARS`; ранний отказ не считается успешной записью. Если прочитанный GUID не Object, GUID результата очищается и writer не вызывает Create/Change. Успешные Create/Change/no-op записывают фактический код результата в конце функции. Геометрия и политика транзакции не меняются. [по коду]
- Native-переход Wall→Object и повторный запуск в Archicad — [не проверено].

## Полный JSON-набор отделки (#314, 2026-11-06)

Адресный контракт рабочей дельты к `04e4dd9`, AC25 Windows; исторические координаты ниже не пересобраны целиком. [по коду]

- `AppendRoomFinish`/`CollectRoomFinishes` (`RoombookDraw.cpp`) собирают все записи `otdwall`, `otdslab`, `otdbeam_ceil` и `otdbeam_floor` обработанной валидной комнаты, независимо от нового создания, успешной записи, наличия GUID и допуска к writer. Это подготовленный набор текущего запуска, не полный native-инвентарь зоны. Подготовку, ранние отказы и построение не меняют. [по коду]
- `RoomBookFinishResult.is_element` отличает настоящую расчётную запись с null GUID от служебной записи пустой комнаты; в JSON этот внутренний признак не выводится. `is_valid` передаёт валидность wall/slab; у подготовленных beams отдельного флага валидности нет, поэтому он true. [по коду]
- Дополнительные поля элемента JSON: `is_valid`, `drawing_enabled`, `draw_error`. `drawing_enabled` — существующий `OtdWall_ShouldDrawFinish`/`OtdSlab_ShouldDrawFinish` (валидность и настройки комнаты); для beams — валидность комнаты, поскольку отдельного writer-флага для их массивов нет. Это не гарантия пригодности избранного или успешной записи. [по коду]
- `write_succeeded=true, was_created=false` включает и обновление, и no-op; различие между ними не определяется этим ответом. Отключённый/пропущенный writer не становится созданным результатом. Нулевой `otd_guid` допустим; ненулевой GUID без успешной записи сам по себе не доказывает существование в модели. [по коду]
- Площади/длины по-прежнему подготовленные, не native read-back. NaN/Inf заменяются числом 0 с соответствующим `*_available=false`, а не объявляются измеренным нулём. Невалидные записи явно помечены `is_valid=false`. [по коду]
- Публикация остаётся после успешной draw-транзакции; при rollback `pendingResults` не публикуется. Ранний отказ до этой стадии по-прежнему может дать пустой ответ. Запись свойств/Sync идёт отдельно после публикации — `status:returned` не доказывает её успех. Полнота по составам `base_composite` и привязкам result-свойств этой правкой не добавлена. [по коду]

Модуль расчёта отделки: по зонам строит/обновляет отделочные элементы (стены, плиты, балки-плинтусы, объекты, кровли, проёмы), собирает сводку площадей и материалов, синхронизирует связи и свойства.

Исторический раздел: строки ниже — 1-based, сверены с диском на момент коммита `b199125` (`[C3-WIP]` roombook-рефакторинг). Машинные данные: `Docs/_generated/symbols.json` (определения, 1-based) и `Docs/_generated/callgraph.json` (рёбра, метка `roombook_refresh` 2026-10-30).

## Файлы модуля

| Файл | Ответственность |
|---|---|
| `Sources/AddOn/roombook/Roombook.cpp` (201 стр.) | Вход `RoomBook_Run`, сбор зон, оркестрация конвейера |
| `Sources/AddOn/roombook/RoombookData.cpp` (9581 стр.) | Чтение зон/конструкций/параметров, `RoomProcessingContext`, словарь избранного, подготовка геометрии отделки |
| `Sources/AddOn/roombook/RoombookDraw.cpp` (2745 стр.) | Писатели native-элементов: Create/Change/Delete, классификация, синхронизация |
| `Sources/AddOn/roombook/RoombookQuantities.cpp` (584 стр.) | Сводка площадей/материалов, запись данных в комнаты |
| `Sources/AddOn/roombook/Roombook.hpp` (+5 .hpp) | Типы `OtdRoom/OtdWall/OtdSlab/OtdBeam`, `RoomBookFinishResult`, объявления API модуля |

Зависимости: `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp`; внутримодульные заголовки `roombook/` (`CoordinateSearchCache.hpp`, `FloorCoreGeometry.hpp`, `RoomAnalyticGeometry.hpp`, `RoomSurfaces.hpp`, `RoomSurfaceCandidates.hpp`, `RoomSurfacePartition.hpp`, `RoomSurfaceFinish.hpp`). [по include]

## Выбор целевых зон

- `RoomBook_Run(results)` сохраняет прежний выбор: доступные выделенные зоны, иначе все доступные зоны. [по коду]
- `RoomBook_Run(results, const GS::Array<API_Guid>& zones)` использует переданный непустой список без чтения выделения. Пустой список на C++-уровне сохраняет автоматический выбор. Внутри создаётся рабочая копия: последующие Clear/Push не изменяют массив вызывающего. [по коду]
- JSON `SomeStuffCommand.RoomBook` принимает необязательный `zones: string[]`. Явный список должен быть непустым; до запуска проверяются форма GUID (с парными фигурными скобками или без), ненулевое значение, чтение заголовка, тип Zone и прежние фильтры editable/visible/access/workspace/renovation. Дубли удаляются с сохранением порядка первого вхождения. Ошибка любого участника отклоняет весь запрос до запуска RoomBook; к выделению/всем зонам такой запрос не переходит. Проверка доступности выполняется в текущем виде до переключения сочетания слоёв конвейером. [по коду]
- Этот вход использует прежние подготовку, поиск существующей отделки по Sync_GUID zone и writers; формат ответа не изменён. `status: returned` по-прежнему означает возврат из void-функции, а не успешное построение всех элементов. Native-приёмка нового параметра — [не проверено].

## Входные точки

| Вход | Цепочка |
|---|---|
| Меню | `MenuCommandHandler` (`SomeStuff_Main.cpp:375`) → `Roombook::RoomBook_Run` (`SomeStuff_Main.cpp:458`) |
| JSON-команда `RoomBook` | `RoomBookCommand::Execute` (`json_commands/RoomBookCommand.cpp:65`) → `Roombook::RoomBook_Run(results, zones)` (`:97`, определение `Roombook.cpp:107`); результат `RoomBookFinishResult` сериализуется в прежний ответ |
| JSON-команда `FavoritesList` | `RoomBookGetFavoritesCommand::Execute` (`RoomBookCommand.cpp:105`) → `Roombook::Favorite_GetDict` (`:108`, def `RoombookData.cpp:8933`) |
| AC22/AC23 | Заглушка `RoomBook_Run` (`Roombook.cpp:21`) — отчёт «Function not work in AC22 and AC23» |

## Конвейер `RoomBook_Run` (Roombook.cpp:102) — исторические координаты `b199125`

| Шаг | Вызов (site) | Определение |
|---|---|---|
| 1. Применить сочетание слоёв отделки, если настроено | `Roombook.cpp:110` | `LayerCombination_ApplyFinishIfExists` (`Roombook.cpp:68`) |
| 2. Собрать целевые зоны (выделение или весь проект) | `Roombook.cpp:113` | `Zone_CollectTargets` (`Roombook.cpp:25`) |
| 3. Контекст обработки | `Roombook.cpp:120` | `RoomProcessingContext_Prepare` (`RoombookData.cpp:4810`) |
| 3а. Индекс прочитанных элементов | `Roombook.cpp:123` | `RoomProcessingContext_BuildElementReadIndex` (`RoombookData.cpp:4835`) |
| 3б. Обработать элементы для данных комнат | `Roombook.cpp:125` | `RoomProcessingContext_ProcessElementsForRoomData` (`RoombookData.cpp:4907`) |
| 3в. Подготовка перегородок | `Roombook.cpp:126` | `RoomProcessingContext_PreparePartitions` (`RoombookData.cpp:4973`) |
| 3г. Чтение параметров | `Roombook.cpp:130` | `RoomProcessingContext_ReadElementParameters` (`RoombookData.cpp:5028`) |
| 4. Прогресс/отмена | `Roombook.cpp:132,161` | `AdvanceProcessPhase` (`CommonFunction.cpp:2651`) |
| 5. Чтение всех элементов (мемо) | `Roombook.cpp:137` | `ElementsRead` (`Helpers.cpp:5583`) |
| 6. Словарь избранного | `Roombook.cpp:139` | `Favorite_GetDict` (`RoombookData.cpp:8933`) |
| 7. Отделка по зонам | `Roombook.cpp:144` | `Otd_GetOtd_ByZone` (`RoombookData.cpp:5888`) |
| 8. Расчёт полов/потолков/стен/откосов | `Roombook.cpp:147` | `OtdRooms_ProcessFinishes` (`RoombookData.cpp:5711`) |
| 9. Сводка материалов в комнаты | `Roombook.cpp:167,169` | `OtdData_AddRoomToMaterialSummary` (`RoombookQuantities.cpp:61`), `OtdData_WriteRoomMaterialData` (`RoombookQuantities.cpp:137`) |
| 10. Проверка конфигурации отделки | `Roombook.cpp:174` | `Roombook_CheckFinishConfiguration` (`RoombookData.cpp:9448`) |
| 11. Рождение/изменение/удаление элементов | `Roombook.cpp:178` | `OtdRooms_Draw` (`RoombookDraw.cpp:2670`) |

## Карта вызовов: от входа до писателей

```
RoomBook_Run (Roombook.cpp:102)
├─ подготовка данных — см. конвейер выше
└─ OtdRooms_Draw (RoombookDraw.cpp:2670)                        [site Roombook.cpp:178]
   ├─ RoofBatch_Validate (RoombookDraw.cpp:2007)                 [site 2683]
   │  └─ Roof_Prepare (RoombookDraw.cpp:1625) → OtdSlab_SetRoofEdgeTrims (1463)
   ├─ OtdRooms_AssignExistingWallFinishes (RoombookDraw.cpp:444) [site 2689]  ← повторное использование
   │  └─ OtdWall_TakeExistingFinish (RoombookDraw.cpp:397)
   ├─ PrepareElementsForUpdate (CommonFunction.cpp:2511)         [site 2705]
   ├─ ACAPI_Element_Delete (deletelist)                          [site 2709]
   ├─ OtdRoom_Draw (RoombookDraw.cpp:2535)                       [site 2723]  ← на каждую комнату
   │  ├─ OtdWall_ShouldDrawFinish (373) → OtdWall_Draw (2499)    [site 2548]
   │  │  └─ Wall_Draw (1183) | Beam_Draw (764) | Object_Draw_Linear (998)   [sites 2509/2512/2515]
   │  ├─ OtdSlab_ShouldDrawFinish (355) → OtdSlab_Draw (2425)    [site 2576]
   │  │  └─ Roof_Draw (1950) | Slab_Draw (2038) | Object_Draw_Poly (2261)   [sites 2442/2446/2454]
   │  ├─ OtdBeam_Draw (2471)                                     [sites 2592/2603]
   │  │  └─ Beam_Draw (764) | Wall_Draw (1183) | Object_Draw_Linear (998)   [sites 2478/2481/2484]
   │  ├─ Class_Set (80)                                          [sites 2566/2587/2598/2609]
   │  └─ Class_Set_OtdOpening (95)                               [site 2568]
   ├─ CollectRoomFinishes (2654)                                 [site 2730]
   └─ OtdElements_SetSync (645)                                  [site 2742]
```

## Рождение, изменение, удаление отделочных элементов

Все записи SDK сосредоточены в `RoombookDraw.cpp`. Шаблон каждого писателя:

1. Прочитать существующий элемент по GUID, если отделка уже существует;
2. посчитать гвард `needupdate` — `Elems_Check_Update` (`RoombookDraw.cpp:27`: сравнение `floorInd`, `layer`, `renovationStatus` по маске) плюс собственные сравнения полей и memo (геометрия, размеры, материалы, сегменты);
3. развилка: `is_new` → `ACAPI_Element_Create`; `needupdate` → `ACAPI_Element_Change` с маской; иначе — no-op: GUID и успешный результат сохраняются, Change не вызывается.

| Тип отделки | Писатель (def) | `ACAPI_Element_Create` | Гвард / `ACAPI_Element_Change` |
|---|---|---|---|
| Отделочная стена | `Wall_Draw` (`RoombookDraw.cpp:1183`) | `:1244` | гвард `:1288`, сравнения и маски до `:1413`, Change `:1415` |
| Отделка-объект по линейному основанию | `Object_Draw_Linear` (`:998`) | `:1153` | гвард `:1133`, Change `:1159` |
| Линейная балка/плинтус | `Beam_Draw` (`:764`) | `:902` | гвард `:799` (вкл. `begC/endC/level`, материалы, memo-сегменты `:801-897`), Change `:904` |
| Отделочная плита | `Slab_Draw` (`:2038`) | `:2186` | гвард `:2066`, Change `:2229` |
| Отделка-объект по полигону | `Object_Draw_Poly` (`:2261`) | `:2403` | гвард `:2394`, Change `:2407` |
| Отделочная кровля | `Roof_WritePrepared` (`:1731`, подготовка `Roof_Prepare` `:1625`) | `:1733` | гвард `:1779`, Change `:1942` |
| Проём в отделочной стене | `OtdOpening_Draw` (`:264`) | `:309` | изменение не выполняется — проём создаётся внутри новой стены; при неудаче прорезания стена удаляется целиком (`:1434`) |

Выбор варианта построения внутри `Otd*`-маршрутизаторов определяется типом шаблона избранного (`draw.favorite.type`): например, `OtdWall_Draw` → `Wall_Draw` / `Beam_Draw` / `Object_Draw_Linear` (sites 2509/2512/2515), `OtdSlab_Draw` → `Roof_Draw` / `Slab_Draw` / `Object_Draw_Poly` (sites 2442/2446/2454).

Удаления:

| Сайт | Что удаляется |
|---|---|
| `Wall_Draw` (`RoombookDraw.cpp:1280`) | старые проёмы (`oldOpenings`) при обновлении отделочной стены |
| `Wall_Draw` (`:1434`) | только что созданная стена при неудачном прорезании проёма («Непрорезанная оболочка даёт лишнюю отделку; удаляем только новый результат») |
| `OtdRooms_Draw` (`:2709`) | `deletelist` — неиспользованная существующая отделка, собранная `OtdRooms_CollectUnusedFinishingElements` (`:319`; сбор без удаления, удаление одним пакетом) |

## Повторное использование существующей отделки

- `OtdRooms_Draw` → `OtdRooms_AssignExistingWallFinishes` (`RoombookDraw.cpp:444`): соответствия назначаются только окончательным участкам; один GUID нельзя использовать в двух зонах;
- → `OtdWall_TakeExistingFinish` (`:397`): существующий отделочный элемент переводится в состояние повторного использования; применимость проверяет `OtdLinearElement_CanDraw` (`:722`);
- после назначения писатель идёт по ветке `ACAPI_Element_Change` (гварды выше), а не создаёт дубликат.

## Классификация

| Функция | SDK-вызов | Вызывается из |
|---|---|---|
| `Class_Set` (`RoombookDraw.cpp:80`) | `ACAPI_Element_AddClassificationItem` `:87` | `OtdRoom_Draw` (sites 2566/2587/2598/2609) |
| `Class_Set_OtdOpening` (`:95`) | `:102` | `OtdRoom_Draw` (site 2568) |

GUID класса выбирается словарём `Class_GetClassGuid` (`RoombookData.cpp:8647`); для проёмов — класс NoSet, при его отсутствии Wall_Main.

## Финализация

- `CollectRoomFinishes` (`RoombookDraw.cpp:2654`) → `AppendRoomFinish` — наполняет `GS::Array<RoomBookFinishResult>` по комнатам (стены, плиты, кровли, плинтусы; площади через `OtdWall_GetArea`, `RoombookQuantities.cpp:545`);
- `OtdElements_SetSync` (`RoombookDraw.cpp:645`) — связи отделки с основаниями и зонами, общая запись накопленных свойств, включая сводку площадей;
- `RoomBookCommand::Execute` сериализует `RoomBookFinishResult` в JSON-ответ.

## Различия версий (текущее поведение)

- AC27+: переопределение материалов сегментов балки — `APIOptional<API_AttributeIndex>` (`hasValue`/`value`, индекс через `ACAPI_CreateAttributeIndex`); в старших версиях — `API_OverriddenAttribute`. [по коду, `RoombookDraw.cpp:890-898`]
- AC27: `ACAPI_ElementGroup_Create` → `ACAPI_ElementSet_Create` (параметра родительской группы нет — передаётся только список и выходной GUID). [по коду DevKit]
- AC26–30: `GetElemTypeID`/`SetElemTypeID` вместо прямого `typeID`; variation через `type` (AC26+). [по коду DevKit]

## Актуальность

- `Docs/_generated/symbols.json`: roombook-файлы пересобраны 2026-10-30 (дисковые строки определений), остальные модули — прогоны 2026-10-15/30.
- `Docs/_generated/callgraph.json`: roombook-срез (807 рёбер) пересобран grep-сканом определений и call-сайтов после `b199125`; входные рёбра из `MenuCommandHandler` и JSON-команд добавлены явно. Остальные срезы — предыдущего прогона (см. `meta`).
