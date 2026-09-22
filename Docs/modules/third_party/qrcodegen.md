# third_party/qrcodegen — Генерация QR-кодов

> Хеш коммита: f8f599c (2026-09-22)

## Назначение модуля
Встраиваемая сторонняя библиотека QR Code generator (C++), Project Nayuki, MIT License. Используется add-on для вывода значений свойств в QR (`ParamValue.toQRCode`, `CommonFunction::TextToQRCode`). [из комментария лицензии, qrcodegen.hpp:1-22]

## Файлы модуля
- `Sources/AddOn/third_party/qrcodegen.cpp/hpp` (hpp 549 строк)
- Также `third_party/alphanum.h`, `third_party/exprtk.h` (включаются CommonFunction)

## Ключевые типы (namespace qrcodegen)
- `QrCode` — код: версии ECC, `encodeText/encodeBinary`, `getModule`, `toSvgString` [по коду заголовка]
- `QrSegment` — сегмент данных (immutable), фабрики `makeNumeric/makeAlphanumeric/makeBytes/makeSegments` [из комментария, qrcodegen.hpp:35-40]

## Зависимости
- Только стандартная библиотека (`<array>`, `<cstdint>`, `<vector>` и др.)
- Используется из: `CommonFunction` (`TextToQRCode`)

## Примечания
- Сторонний код: не документируется пофункционально, исходная документация в лицензии/заголовке Nayuki. Полный перечень символов — `Docs/_generated/symbols.json`, модуль third_party (197 символов)