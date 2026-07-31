#!/usr/bin/env python3
# /// script
# requires-python = ">=3.9"
# dependencies = []
# ///

"""
Property Bridge — тестовый Python интерфейс для получения свойств из Archicad.
Запуск: python property_bridge.py (автоопределение порта) или python property_bridge.py --port 19723
"""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import argparse
import json
import os
import sys
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
import socket

BRIDGE_VERSION = "0.1.0"
AC_PORT_FIRST = 19723
AC_PORT_LAST = 19743

def find_archicad_port(timeout=0.5):
    """Автоопределение порта Archicad (сканирует диапазон 19723-19743)."""
    for port in range(AC_PORT_FIRST, AC_PORT_LAST + 1):
        try:
            request = urllib.request.Request(
                "http://127.0.0.1:{}".format(port),
                data=json.dumps({"command": "API.GetProjectInfo", "parameters": {}}).encode("utf-8"),
                headers={"Content-Type": "application/json"}
            )
            with urllib.request.urlopen(request, timeout=timeout) as response:
                if response.status == 200:
                    print("Найден Archicad на порту: {}".format(port))
                    return port
        except (urllib.error.URLError, socket.timeout, ConnectionRefusedError):
            continue
        except Exception as e:
            continue
    return None

# HTML интерфейс (встроен в Python)
HTML_TEMPLATE = """<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Property Bridge</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        button { padding: 10px 20px; font-size: 16px; cursor: pointer; }
        select { padding: 5px; font-size: 14px; min-width: 300px; }
        #status { margin-top: 10px; padding: 10px; border-radius: 5px; }
        .success { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <h2>Property Bridge</h2>
    <button onclick="loadProperties()">Получить свойства</button>
    <div id="status"></div>
    <h3>Список свойств:</h3>
    <select id="propertyList" size="20"></select>

    <script>
    function loadProperties() {
        const status = document.getElementById('status');
        status.className = '';
        status.textContent = 'Загрузка...';
        
        fetch('/api/properties')
            .then(response => {
                if (!response.ok) {
                    throw new Error('HTTP ' + response.status);
                }
                return response.json();
            })
            .then(data => {
                // Логируем ответ для отладки
                console.log('Response:', data);
                
                if (data.error) {
                    status.className = 'error';
                    status.textContent = 'Ошибка: ' + data.error;
                    if (data.details) {
                        console.error('Details:', data.details);
                    }
                    return;
                }
                
                if (!data.properties || !Array.isArray(data.properties)) {
                    status.className = 'error';
                    status.textContent = 'Ошибка: Неверный формат ответа';
                    console.error('Invalid response format:', data);
                    return;
                }
                
                status.className = 'success';
                status.textContent = 'Загружено свойств: ' + data.count;
                
                const select = document.getElementById('propertyList');
                select.innerHTML = '';
                data.properties.forEach(prop => {
                    const option = document.createElement('option');
                    option.value = JSON.stringify(prop);
                    option.textContent = prop.displayName || prop.rawName || 'Unknown';
                    select.appendChild(option);
                });
            })
            .catch(error => {
                status.className = 'error';
                status.textContent = 'Ошибка: ' + error.message;
                console.error('Fetch error:', error);
            });
    }
    </script>
</body>
</html>"""

# ---------------------------------------------------------------------------
# Archicad transport
# ---------------------------------------------------------------------------

def ac_post(port, payload, timeout=30.0):
    """POST запрос к Archicad JSON серверу."""
    request = urllib.request.Request(
        "http://127.0.0.1:{}".format(port),
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
    )
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            return True, json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        return False, {"error": "HTTP {}".format(exc.code), "port": port}
    except urllib.error.URLError as exc:
        return False, {"error": "Cannot reach Archicad: {}".format(exc.reason), "port": port}
    except socket.timeout:
        return False, {"error": "Timeout on port {}".format(port), "port": port}

# ---------------------------------------------------------------------------
# HTTP Handler
# ---------------------------------------------------------------------------

class PropertyBridgeHandler(BaseHTTPRequestHandler):
    server_version = "property-bridge/" + BRIDGE_VERSION
    
    def do_GET(self):
        if self.path == "/" or self.path == "/index.html":
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.send_header("Content-Length", str(len(HTML_TEMPLATE)))
            self.end_headers()
            self.wfile.write(HTML_TEMPLATE.encode("utf-8"))
        elif self.path == "/api/properties":
            # Вызываем нашу команду через API.ExecuteAddOnCommand (правильный формат для ArchiCAD 25)
            ok, body = ac_post(self.server.ac_port, {
                "command": "API.ExecuteAddOnCommand",
                "parameters": {
                    "addOnCommandId": {
                        "commandNamespace": "SomeStuffCommand",
                        "commandName": "GetPropertyDefinitions"
                    },
                    "addOnCommandParameters": {}
                }
            })
            
            # Подробное логирование
            print(f"[API] command=SomeStuffCommand.GetPropertyDefinitions")
            print(f"[API] ok={ok}, body_type={type(body).__name__}")
            if body:
                print(f"[API] body_keys={list(body.keys()) if isinstance(body, dict) else 'not dict'}")
            
            if ok and body and isinstance(body, dict):
                # Правильная структура ответа JSON API ArchiCAD
                result = body.get("result", {})
                response_data = result.get("addOnCommandResponse", {})
                
                if "properties" in response_data:
                    print(f"[API] properties count={len(response_data.get('properties', []))}")
                    self.send_response(200)
                    self.send_header("Content-Type", "application/json")
                    self.send_header("Content-Length", str(len(json.dumps(response_data).encode("utf-8"))))
                    self.end_headers()
                    self.wfile.write(json.dumps(response_data).encode("utf-8"))
                else:
                    # Возможно, ответ в другом формате
                    print(f"[API] WARNING: 'properties' not in response. Full body: {json.dumps(body)[:200]}")
                    self.send_response(500)
                    self.send_header("Content-Type", "application/json")
                    self.end_headers()
                    error_response = {"error": "Invalid response format", "body": str(body)[:500]}
                    self.wfile.write(json.dumps(error_response).encode("utf-8"))
            else:
                self.send_response(500)
                self.send_header("Content-Type", "application/json")
                self.end_headers()
                error_response = {"error": "Failed to call GetPropertyDefinitions", "details": str(body)[:500]}
                self.wfile.write(json.dumps(error_response).encode("utf-8"))
        else:
            self.send_response(404)
            self.end_headers()
    
    def log_message(self, format, *args):
        # Отключаем лишний вывод
        pass

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=None, help="Archicad instance port (default: auto-detect)")
    parser.add_argument("--ui-port", type=int, default=3030, help="UI server port")
    args = parser.parse_args()
    
    # Автоопределение порта Archicad, если не указан явно
    if args.port is None:
        print("Определение порта Archicad...")
        args.port = find_archicad_port()
        if args.port is None:
            print("ОШИБКА: Archicad не найден. Убедитесь, что Archicad запущен.")
            print("Можно указать порт вручную: python property_bridge.py --port <номер_порта>")
            sys.exit(1)
    
    server = ThreadingHTTPServer(("127.0.0.1", args.ui_port), PropertyBridgeHandler)
    server.ac_port = args.port  # Передаем порт Archicad в сервер
    
    print("Property Bridge запущен")
    print("  UI: http://127.0.0.1:{}".format(args.ui_port))
    print("  Archicad port: {}".format(args.port))
    
    server.serve_forever()

if __name__ == "__main__":
    main()
