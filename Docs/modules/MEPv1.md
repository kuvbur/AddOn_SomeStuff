# MEPv1 — Обработка данных MEP

## Назначение
Подсистема чтения и обработки данных MEP (механика, электрика, сантехника) для ArchiCAD 27+. Работает с канальными, трубопроводными системами, вентиляцией, кабельными трассами.

## Файлы
- `MEPv1.cpp/hpp` — основной модуль

## Зависимости (include)
- `ACAPI/MEPAdapter.hpp`, `MEPElement.hpp`, `MEPModifiableElement.hpp`
- `ACAPI/MEPPipingPort.hpp`, `MEPPort.hpp`, `MEPRigidSegment.hpp`
- `ACAPI/MEPRoutingElement.hpp`, `MEPRoutingNode.hpp`, `MEPRoutingSegment.hpp`
- `ACAPI/MEPVentilationPort.hpp`, `MEPBranch.hpp`, `MEPFitting.hpp`
- `ACAPI/MEPEnums.hpp` (AC28+)
- Сотни строк кода с условными компиляциями по ServerMainVers_2700/2800/2900

## Публичный API

| Функция | Назначение |
|---------|------------|
| `GetSubElementOfRouting` | Сбор подэлементов маршрутизации для MEP-элемента |
| `GetSubElement` | Сбор всех дочерних элементов MEP-объекта |
| `ReadMEP` | Чтение MEP-свойств из заголовка в ParamDictValue |
| `ClearRoutingSubelemCache` | Очистка кэша подэлементов |

## AC28+ функции
`GetMEPData`, `ReadTransitionData`, `ReadRoutingElementData`, `ReadBendData`, `ReadRigidSegmentData`, `ReadRoutingSegmentData`, `ReadDuctSegmentPreferenceTable`, `ReadDuctBendPreferenceTable`, `ReadPipeSegmentPreferenceTable`, `ReadPipeBendPreferenceTable`, `ReadTransitionPreferenceTable` — все возвращают `(bool flag, ParamDictValue&, ACAPI::MEP::ConnectorShape&, ACAPI::MEP::UniqueID&, ...)`

## Ключевые константы (raw names)
`{@mep:physical system name}`, `{@mep:length}`, `{@mep:routing length}`, `{@mep:bend radius}`, `{@mep:width}`, `{@mep:description}` и 10+ других MEP-специфичных raw names.

## Зависимости
- `CommonFunction.hpp`
- `Helpers.hpp`
- `Propertycache.hpp`
- `Definitions.hpp`

## Зависимости (используется в)
- `Propertycache` (ReadMEP для AC2900)
- `Helpers` (MEP-данные при чтении свойств)

## Инварианты
- Множественные условные компиляции по ServerMainVers_2700/2800/2900 — API MEP существенно меняется между версиями
- `ReadMEP` возвращает bool — false означает, что элемент не содержит MEP-данных
