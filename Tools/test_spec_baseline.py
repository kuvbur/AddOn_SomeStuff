"""Offline-проверки итогового результата Spec comparator; Archicad не вызывается.

Запуск: python -B -m unittest discover -s Tools -p test_spec_baseline.py -v
"""

import contextlib
import copy
import io
import unittest
from unittest.mock import patch

import spec_baseline


def response_body():
    return {
        "status": "completed",
        "resultCode": 0,
        "elementsToCreate": 0,
        "elementsToModify": 0,
        "elementsToDelete": 0,
        "elapsedSeconds": 1.0,
    }


class CompareResultTests(unittest.TestCase):
    def run_compare(self, expected, body, succeeded=True):
        raw = {
            "succeeded": succeeded,
            "result": {"addOnCommandResponse": body},
        }
        output = io.StringIO()
        with patch.object(spec_baseline.sys, "argv", ["spec_baseline.py", "compare", "synthetic"]), \
                patch.object(spec_baseline, "find_port", return_value=19723), \
                patch.object(spec_baseline, "call_spec", return_value=raw), \
                patch.object(spec_baseline, "load", return_value=expected), \
                patch.object(spec_baseline, "save") as save, \
                contextlib.redirect_stdout(output):
            code = spec_baseline.main()
        save.assert_not_called()
        return code, output.getvalue()

    def baseline(self):
        return {"summary": response_body(), "rows": {}}

    def assert_rejected(self, code, output):
        self.assertNotEqual(code, 0, output)
        self.assertNotIn("PASS:", output)

    def test_equal_response_passes(self):
        code, output = self.run_compare(self.baseline(), response_body())
        self.assertEqual(code, 0, output)
        self.assertIn("PASS:", output)
        self.assertNotIn("FAIL", output)

    def test_each_summary_mismatch_rejects_equal_rows(self):
        for key, value in (
            ("status", "failed"),
            ("resultCode", -1),
            ("elementsToCreate", 1),
            ("elementsToModify", 1),
            ("elementsToDelete", 1),
        ):
            with self.subTest(key=key):
                body = response_body()
                body[key] = value
                code, output = self.run_compare(self.baseline(), body)
                self.assert_rejected(code, output)
                self.assertIn("summary " + key, output)

    def test_missing_summary_field_rejects(self):
        body = response_body()
        del body["resultCode"]
        code, output = self.run_compare(self.baseline(), body)
        self.assert_rejected(code, output)

    def test_elapsed_time_is_not_a_behavior_difference(self):
        body = response_body()
        body["elapsedSeconds"] = 99.0
        code, output = self.run_compare(self.baseline(), body)
        self.assertEqual(code, 0, output)
        self.assertIn("PASS:", output)

    def test_row_mismatch_rejects_equal_summary(self):
        expected = self.baseline()
        expected["rows"]["missing"] = {
            "properties": {}, "gdl": {}, "sources": 0,
        }
        code, output = self.run_compare(expected, response_body())
        self.assert_rejected(code, output)
        self.assertIn("MISSING row", output)

    def test_summary_and_row_mismatches_are_both_reported(self):
        expected = self.baseline()
        expected["rows"]["missing"] = {
            "properties": {}, "gdl": {}, "sources": 0,
        }
        body = response_body()
        body["elementsToDelete"] = 1
        code, output = self.run_compare(expected, body)
        self.assert_rejected(code, output)
        self.assertIn("summary elementsToDelete", output)
        self.assertIn("MISSING row", output)
        self.assertIn("2 difference(s)", output)

    def test_command_failure_rejects(self):
        code, output = self.run_compare(self.baseline(), response_body(), succeeded=False)
        self.assert_rejected(code, output)
        self.assertIn("command failed", output)

    def test_missing_baseline_rejects(self):
        code, output = self.run_compare(None, response_body())
        self.assertEqual(code, 4, output)
        self.assertNotIn("PASS:", output)

    def test_existing_nonempty_payload_contract(self):
        body = response_body()
        body["created"] = {"element": [{
            "favoriteName": "synthetic",
            "property": [{"name": "name", "value": "row"}],
            "gdlParameter": [{"name": "quantity", "value": "1"}],
            "sourceElement": [{"guid": "synthetic-source"}],
        }]}
        expected = {
            "summary": spec_baseline.summarize(body),
            "rows": spec_baseline.collect_rows(body, "created"),
        }
        code, output = self.run_compare(expected, body)
        self.assertEqual(code, 0, output)
        changed = copy.deepcopy(body)
        changed["created"]["element"][0]["gdlParameter"][0]["value"] = "2"
        code, output = self.run_compare(expected, changed)
        self.assert_rejected(code, output)
        self.assertIn("gdlParameter changed", output)


if __name__ == "__main__":
    unittest.main()
