#!/usr/bin/env python3
"""Read-only exact lookup of Archicad zones through its local JSON interface."""

import argparse
import json
import sys
import urllib.error
import urllib.request


def request(port, command, parameters):
    payload = json.dumps({"command": command, "parameters": parameters}).encode("utf-8")
    req = urllib.request.Request(
        "http://127.0.0.1:%d" % port,
        data=payload,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
    with opener.open(req, timeout=20) as response:
        data = json.load(response)
    if data.get("succeeded") is not True or not isinstance(data.get("result"), dict):
        raise ValueError("%s failed: %s" % (command, json.dumps(data, ensure_ascii=False)))
    return data["result"]


def lookup(port, key, query):
    elements = request(port, "API.GetElementsByType", {"elementType": "Zone"}).get("elements")
    if not isinstance(elements, list):
        raise ValueError("Invalid or missing elements array")
    properties = request(
        port,
        "API.GetPropertyIds",
        {"properties": [
            {"type": "BuiltIn", "nonLocalizedName": "Zone_ZoneNumber"},
            {"type": "BuiltIn", "nonLocalizedName": "Zone_ZoneName"},
        ]},
    ).get("properties")
    if not isinstance(properties, list) or len(properties) != 2 or any(
        not isinstance(p, dict) or not isinstance(p.get("propertyId"), dict) or not p["propertyId"].get("guid")
        for p in properties
    ):
        raise ValueError("Could not resolve both zone property IDs")
    matches = []
    for index in range(0, len(elements), 150):
        batch = elements[index:index + 150]
        rows = request(port, "API.GetPropertyValuesOfElements", {"elements": batch, "properties": properties}).get(
            "propertyValuesForElements"
        )
        if not isinstance(rows, list) or len(rows) != len(batch):
            raise ValueError("Incomplete zone property response")
        for element, row in zip(batch, rows):
            if not isinstance(element, dict) or not isinstance(element.get("elementId"), dict):
                raise ValueError("Malformed element ID")
            guid = element["elementId"].get("guid")
            values = row.get("propertyValues") if isinstance(row, dict) else None
            if not guid or not isinstance(values, list) or len(values) != 2:
                raise ValueError("Incomplete zone data")
            result = []
            for item in values:
                prop = item.get("propertyValue") if isinstance(item, dict) else None
                if not isinstance(prop, dict) or prop.get("status") != "normal" or not isinstance(prop.get("value"), str):
                    raise ValueError("Unreadable number or name for zone %s" % guid)
                result.append(prop["value"])
            zone = {"guid": guid, "number": result[0], "name": result[1]}
            if zone[key] == query:
                matches.append(zone)
    return {"zone_count": len(elements), "match_count": len(matches), "matches": matches}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, type=int, help="Port of the explicitly selected Archicad instance")
    selector = parser.add_mutually_exclusive_group(required=True)
    selector.add_argument("--number", help="Exact zone number (string, including leading zeroes)")
    selector.add_argument("--name", help="Exact zone name")
    args = parser.parse_args()
    if not 1 <= args.port <= 65535:
        parser.error("port must be in 1..65535")
    key, query = ("number", args.number) if args.number is not None else ("name", args.name)
    if not query:
        parser.error("the search value must not be empty")
    try:
        result = lookup(args.port, key, query)
    except (OSError, ValueError, KeyError, TypeError, urllib.error.URLError) as exc:
        print(json.dumps({"error": str(exc)}, ensure_ascii=False), file=sys.stderr)
        return 2
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result["match_count"] == 1 else 1


if __name__ == "__main__":
    sys.exit(main())
