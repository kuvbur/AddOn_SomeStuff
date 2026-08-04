#!/usr/bin/env python3
"""
Вспомогательные функции для тестирования JSON-команд SomeStuff Add-On.

Использование как модуль:
    from test_helpers import ArchicadTestClient
    
    client = ArchicadTestClient()
    response = client.call_command("Health")
    print(response)
"""

import json
import urllib.request
import urllib.error

COMMAND_NAMESPACE = "SomeStuffCommand"
DEFAULT_PORT_RANGE = range(19723, 19744)
TIMEOUT = 10


class ArchicadTestClient:
    """
    Клиент для вызова JSON-команд ArchiCAD.
    """

    def __init__(self, port=None):
        """
        Args:
            port: порт ArchiCAD (если None — автоопределение)
        """
        if port:
            self.port = port
        else:
            self.port = self.find_archicad_port()
            if not self.port:
                raise ConnectionError("ArchiCAD не найден. Убедитесь, что ArchiCAD запущен.")

    @staticmethod
    def find_archicad_port(timeout=0.5):
        """Автоопределение порта ArchiCAD."""
        for port in DEFAULT_PORT_RANGE:
            try:
                req = urllib.request.Request(
                    f"http://127.0.0.1:{port}",
                    data=json.dumps({"command": "API.GetProjectInfo"}).encode("utf-8"),
                    headers={"Content-Type": "application/json"},
                    method="POST"
                )
                with urllib.request.urlopen(req, timeout=timeout) as resp:
                    if resp.status == 200:
                        return port
            except Exception:
                continue
        return None

    def call_command(self, command_name, parameters=None):
        """
        Вызов JSON-команды аддона.

        Args:
            command_name: имя команды (например, "Health")
            parameters: словарь параметров (опционально)

        Returns:
            dict: ответ от ArchiCAD
        """
        if parameters is None:
            parameters = {}

        payload = {
            "command": "API.ExecuteAddOnCommand",
            "parameters": {
                "addOnCommandId": {
                    "commandNamespace": COMMAND_NAMESPACE,
                    "commandName": command_name
                },
                "addOnCommandParameters": parameters
            }
        }

        url = f"http://127.0.0.1:{self.port}"
        data = json.dumps(payload).encode("utf-8")

        req = urllib.request.Request(
            url,
            data=data,
            headers={"Content-Type": "application/json"},
            method="POST"
        )

        try:
            with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
                response_body = resp.read().decode("utf-8")
                return json.loads(response_body)
        except urllib.error.HTTPError as e:
            error_body = e.read().decode("utf-8")
            return {"error": f"HTTP {e.code}", "details": error_body}
        except Exception as e:
            return {"error": str(e)}

    def call_tapir_command(self, tapir_command, params=None):
        """
        Вызов стандартной команды Tapir (не аддона).

        Args:
            tapir_command: имя команды Tapir (например, "GetAllElements")
            params: параметры команды

        Returns:
            dict: ответ от ArchiCAD
        """
        if params is None:
            params = {}

        payload = {
            "command": f"API.{tapir_command}",
            "parameters": params
        }

        url = f"http://127.0.0.1:{self.port}"
        data = json.dumps(payload).encode("utf-8")

        req = urllib.request.Request(
            url,
            data=data,
            headers={"Content-Type": "application/json"},
            method="POST"
        )

        try:
            with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
                response_body = resp.read().decode("utf-8")
                return json.loads(response_body)
        except Exception as e:
            return {"error": str(e)}

    def is_command_available(self, command_name):
        """
        Проверка доступности команды (по наличию в ответе).
        """
        response = self.call_command(command_name)
        return "error" not in response


def format_response(response, indent=2):
    """Красивый вывод JSON-ответа."""
    return json.dumps(response, ensure_ascii=False, indent=indent)


def extract_addon_response(response):
    """
    Извлечение ответа аддона из обёртки ArchiCAD.

    Returns:
        dict: содержимое addOnCommandResponse или пустой dict
    """
    result = response.get("result", {})
    return result.get("addOnCommandResponse", {})


if __name__ == "__main__":
    # Быстрая проверка
    try:
        client = ArchicadTestClient()
        print(f"Подключено к ArchiCAD на порту {client.port}")

        response = client.call_command("Health")
        print("Ответ Health:", format_response(response))
    except Exception as e:
        print(f"Ошибка: {e}")
