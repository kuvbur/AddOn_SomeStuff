"""Offline-проверки итогового результата Spec comparator; Archicad не вызывается.

Запуск: python -B -m unittest discover -s Tools -p test_spec_baseline.py -v
"""

import contextlib
import copy
import io
import sys
import unittest
from unittest.mock import patch

import spec_baseline
import spec_benchmark


def response_body():
    return {
        "status": "completed",
        "resultCode": 0,
        "elementsToCreate": 0,
        "elementsToModify": 0,
        "elementsToDelete": 0,
        "elapsedSeconds": 1.0,
    }


def deleted_body(*guids):
    body = response_body()
    body["elementsToDelete"] = len(guids)
    body["deleted"] = {"element": [{"guid": g, "favoriteName": "fav"} for g in guids]}
    return body


class CompareResultTests(unittest.TestCase):
    def run_compare(self, expected, body, succeeded=True, drop_response=False):
        raw = {"succeeded": succeeded}
        if not drop_response:
            raw["result"] = {"addOnCommandResponse": body}
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
        return {"summary": response_body(), "rows": {}, "deleted": {}}

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
            "deleted": spec_baseline.collect_deleted(body),
        }
        code, output = self.run_compare(expected, body)
        self.assertEqual(code, 0, output)
        changed = copy.deepcopy(body)
        changed["created"]["element"][0]["gdlParameter"][0]["value"] = "2"
        code, output = self.run_compare(expected, changed)
        self.assert_rejected(code, output)
        self.assertIn("gdlParameter changed", output)

    def test_replaced_deletion_rejects_equal_summary(self):
        expected = {
            "summary": spec_baseline.summarize(deleted_body("A")),
            "rows": {},
            "deleted": {"A": "fav"},
        }
        code, output = self.run_compare(expected, deleted_body("B"))
        self.assert_rejected(code, output)
        self.assertIn("DELETED lost", output)
        self.assertIn("DELETED new", output)

    def test_deletion_favorite_change_rejects(self):
        expected = {
            "summary": spec_baseline.summarize(deleted_body("A")),
            "rows": {},
            "deleted": {"A": "fav"},
        }
        body = deleted_body("A")
        body["deleted"]["element"][0]["favoriteName"] = "other"
        code, output = self.run_compare(expected, body)
        self.assert_rejected(code, output)
        self.assertIn("DELETED changed", output)

    def test_deletions_lost_against_empty_baseline_reject(self):
        code, output = self.run_compare(self.baseline(), deleted_body("A"))
        self.assert_rejected(code, output)
        self.assertIn("DELETED new", output)

    def test_equal_deletions_pass(self):
        expected = {
            "summary": spec_baseline.summarize(deleted_body("A", "B")),
            "rows": {},
            "deleted": {"A": "fav", "B": "fav"},
        }
        code, output = self.run_compare(expected, deleted_body("B", "A"))
        self.assertEqual(code, 0, output)
        self.assertIn("PASS:", output)

    def test_missing_response_schema_rejects(self):
        code, output = self.run_compare(self.baseline(), {}, drop_response=True)
        self.assert_rejected(code, output)
        self.assertIn("missing required field", output)

    def test_missing_summary_field_rejects_before_comparison(self):
        expected = self.baseline()
        body = response_body()
        del body["elementsToCreate"]
        code, output = self.run_compare(expected, body)
        self.assert_rejected(code, output)
        self.assertIn("missing required field", output)


class BenchmarkSeriesTests(unittest.TestCase):
    """Серия замеров отвергает отказ и ненулевую операцию.

    Проверяется настоящий main(): отвергнутая серия обязана вернуть ненулевой
    код и НЕ записать файлы, иначе отвергнутый замер выглядел бы готовой
    baseline-серией.
    """

    def run_main(self, body, runs=2):
        raw = {"succeeded": True, "result": {"addOnCommandResponse": body}}
        output = io.StringIO()
        written = []
        with patch.object(sys, "argv",
                         ["spec_benchmark.py", "--runs", str(runs), "--mode", "false"]), \
                patch.object(spec_benchmark, "find_port", return_value=19723), \
                patch.object(spec_benchmark, "call_spec", return_value=(raw, 1.1)) as call, \
                patch.object(spec_benchmark.os, "makedirs"), \
                patch.object(spec_benchmark, "write_csv",
                             side_effect=lambda path, rec: written.append(path)), \
                patch.object(spec_benchmark.json, "dump", side_effect=lambda *a, **k: written.append("json")), \
                contextlib.redirect_stdout(output):
            code = spec_benchmark.main()
        return code, output.getvalue(), call.call_count, written

    def test_failed_status_rejects_series(self):
        body = response_body()
        body.update({"status": "failed", "resultCode": -1})
        code, output, calls, written = self.run_main(body)
        self.assertNotEqual(code, 0, output)
        self.assertIn("REJECTED", output)
        self.assertEqual(calls, 1, "series must stop at the first rejected run")
        self.assertEqual(written, [], "rejected series must not write a baseline")

    def test_nonzero_create_stops_series(self):
        body = response_body()
        body["elementsToCreate"] = 9
        code, output, calls, written = self.run_main(body)
        self.assertNotEqual(code, 0, output)
        self.assertIn("REJECTED", output)
        self.assertEqual(calls, 1, "a mutating run must not be repeated")
        self.assertEqual(written, [])

    def test_missing_field_rejects_series(self):
        body = response_body()
        del body["resultCode"]
        code, output, calls, written = self.run_main(body)
        self.assertNotEqual(code, 0, output)
        self.assertIn("missing resultCode", output)
        self.assertEqual(written, [])

    def test_invalid_timing_is_counted_not_dropped(self):
        body = response_body()
        body["elapsedSeconds"] = None
        code, output, calls, written = self.run_main(body, runs=1)
        self.assertIn("BAD TIMING", output)
        self.assertNotEqual(code, 0, output)
        self.assertIn("incomplete", output)
        self.assertEqual(written, [], "incomplete series must not be saved as baseline")

    def test_summarize_reports_sample_counts(self):
        records = [
            dict(response_body(), run=1, clientWallSeconds=1.0, elapsedSeconds=1.0),
            dict(response_body(), run=2, clientWallSeconds=1.0, elapsedSeconds=None),
        ]
        summary = spec_benchmark.summarize(records, False)
        self.assertEqual(summary["runs"], 2)
        self.assertEqual(summary["validTimingSamples"], 1)
        self.assertEqual(summary["discardedTimingSamples"], 1)
        self.assertFalse(summary["series_complete"])


if __name__ == "__main__":
    unittest.main()
