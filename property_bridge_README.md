# Property Bridge — Тестовый интерфейс для работы со свойствами Archicad

## Описание

Python процесс с веб-интерфейсом для тестирования JSON API команд аддона SomeStuff.

## Возможности

- Кнопка "Получить свойства" — вызывает команду `SomeStuffCommand.GetPropertyDefinitions` через JSON API
- Выпадающий список со всеми свойствами проекта (из кэша аддона)
- Простой веб-интерфейс (HTML/JS встроен в Python)

## Запуск

### 1. Сборка аддона

```bash
cd D:/SomeStuff_addon
python Tools/BuildAddOn.py -c config.json -v 25
```

### 2. Запуск Python интерфейса

```bash
cd D:/SomeStuff_addon
python property_bridge.py --port 19723 --ui-port 3030
```

Параметры:
- `--port` — порт Archicad (по умолчанию 19723)
- `--ui-port` — порт веб-интерфейса (по умолчанию 8080, у Вас 3030)

### 3. Открытие интерфейса

Откройте в браузере: `http://127.0.0.1:3030`

## Использование

1. Нажмите кнопку "Получить свойства"
2. В выпадающем списке появятся все свойства из кэша аддона
3. В консоли Python сервера будут видны запросы

## Структура ответа API

```json
{
  "count": 150,
  "properties": [
    {
      "rawName": "{@property:name}",
      "displayName": "Имя свойства",
      "description": "Описание",
      "type": "string"
    }
  ]
}
```

## Требования

1. Archicad 25 должен быть запущен
2. Аддон SomeStuff должен быть загружен (скомпилирован и установлен)
3. В консоли Archicad должно быть: "GetPropertyDefinitionsCommand registered successfully!"

## Примечание

Команда `GetPropertyDefinitions` обращается к кэшу свойств аддона (PROPERTYCACHE().property).
Для обновления кэша используйте функции аддона (например, синхронизацию).
До регистрации будет возвращаться ошибка (которую интерфейс покажет).
