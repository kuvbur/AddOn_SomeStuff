# third_party/qrcodegen — Генерация QR-кодов

> Хеш коммита: 493caf5 (2026-09-22). Сторонний код — пофункциональная документация не ведётся.

## Назначение
Встраиваемая библиотека QR Code generator (Nayuki, MIT). Используется через `CommonFunction::TextToQRCode` (ParamValue.toQRCode). [из лицензии, qrcodegen.hpp:1-22]

## Ключевые типы (namespace qrcodegen) [из комментария заголовка]
`QrCode` — encodeText/encodeBinary, getModule, toSvgString; `QrSegment` — immutable, фабрики makeNumeric/makeAlphanumeric/makeBytes/makeSegments [qrcodegen.hpp:35-40]

## Зависимости
- Только стандартная библиотека; используется из CommonFunction [по include]
