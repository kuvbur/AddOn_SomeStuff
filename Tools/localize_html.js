#!/usr/bin/env node
// localize_html.js — генерация и контроль актуальности английского HTML палитры.
//
// Модель: Sources/AddOnResources/RFIX/HTML/Interface_ru.html — единственный
// редактируемый исходник. Переводимые строки собраны в маркированном
// RU-каталоге (JSON-массив внутри inline-скрипта). Этот инструмент читает его,
// сверяет с ручным EN-каталогом и пишет автономный Interface_en.html: тот же
// код, встроенный английский каталог, без внешних загрузок.
//
// Команды:
//   node Tools/localize_html.js status
//   node Tools/localize_html.js generate
//   node Tools/localize_html.js check
//   node Tools/localize_html.js export-pending --out <file.json>
//
// check ничего не пишет: отсутствующий, устаревший или подделанный EN-файл —
// ошибка, а не повод перегенерировать молча.

"use strict";

const fs = require("node:fs");
const path = require("node:path");

const SCHEMA_VERSION = 1;
const LOCALE = "en";
const KEY_TITLE = "app.title";

const CATALOG_BEGIN = "<!-- i18n-catalog-begin -->";
const CATALOG_END = "<!-- i18n-catalog-end -->";
const CATALOG_CONST = "const I18N_CATALOG = ";

const DEFAULT_RU = path.join(
  __dirname, "..", "Sources", "AddOnResources", "RFIX", "HTML", "Interface_ru.html"
);
const DEFAULT_EN = path.join(
  __dirname, "..", "Sources", "AddOnResources", "RFIX", "HTML", "Interface_en.html"
);
const DEFAULT_CATALOG = path.join(
  __dirname, "..", "Sources", "AddOnResources", "RFIX", "HTML", "i18n", "en.json"
);

// ─── ошибки ─────────────────────────────────────────────────────────────

class L10nError extends Error {}
function fail(msg) {
  throw new L10nError(msg);
}

// ─── чтение RU-каталога ─────────────────────────────────────────────────

function countOccurrences(hay, needle) {
  let n = 0;
  let at = hay.indexOf(needle);
  while (at !== -1) {
    n += 1;
    at = hay.indexOf(needle, at + needle.length);
  }
  return n;
}

// Достаёт JSON-массив каталога из маркированного блока. Возвращает
// { json, prefix, suffix } — что нужно для обратной подстановки без
// повторной сериализации остального документа.
function sliceCatalogJson(block) {
  const ci = block.indexOf(CATALOG_CONST);
  if (ci === -1) {
    fail("В блоке каталога нет объявления '" + CATALOG_CONST + "'.");
  }
  const prefix = block.slice(0, ci + CATALOG_CONST.length);
  const after = block.slice(ci + CATALOG_CONST.length);
  const start = after.indexOf("[");
  if (start === -1) fail("После '" + CATALOG_CONST + "' не найдено начало массива JSON.");
  const end = after.lastIndexOf("]");
  if (end < start) fail("Массив каталога не закрыт.");
  return { json: after.slice(start, end + 1), prefix: prefix, suffix: after.slice(end + 1) };
}

function validateRuCatalog(parsed) {
  if (!Array.isArray(parsed)) fail("RU-каталог должен быть JSON-массивом.");
  const seen = new Set();
  parsed.forEach(function (entry, i) {
    if (!entry || typeof entry !== "object") {
      fail("Запись RU-каталога #" + i + " не объект.");
    }
    if (typeof entry.key !== "string" || !entry.key) {
      fail("Запись RU-каталога #" + i + ": пустой или нестроковый key.");
    }
    if (seen.has(entry.key)) {
      fail("В RU-каталоге дублирующийся key: " + entry.key);
    }
    seen.add(entry.key);
    if (typeof entry.text !== "string" || !entry.text.trim()) {
      fail("Запись " + entry.key + ": пустой или нестроковый text.");
    }
    if (entry.context != null && typeof entry.context !== "string") {
      fail("Запись " + entry.key + ": context должен быть строкой.");
    }
  });
}

function readRuSource(html) {
  if (typeof html !== "string" || html.length === 0) {
    fail("Исходный HTML пуст.");
  }
  const beginN = countOccurrences(html, CATALOG_BEGIN);
  const endN = countOccurrences(html, CATALOG_END);
  if (beginN !== 1 || endN !== 1) {
    fail(
      "Маркеры каталога должны встречаться ровно по одному разу (найдено begin=" +
        beginN + ", end=" + endN + "): " + CATALOG_BEGIN + " / " + CATALOG_END
    );
  }
  const b = html.indexOf(CATALOG_BEGIN);
  const e = html.indexOf(CATALOG_END);
  if (e < b) fail("Маркеры каталога переставлены: end раньше begin.");

  const sliced = sliceCatalogJson(html.slice(b, e));
  let parsed;
  try {
    parsed = JSON.parse(sliced.json);
  } catch (err) {
    fail("JSON RU-каталога не разбирается: " + err.message);
  }
  validateRuCatalog(parsed);
  return { catalog: parsed, html: html };
}

// ─── чтение EN-каталога ─────────────────────────────────────────────────

function readEnCatalog(text) {
  let parsed;
  try {
    parsed = JSON.parse(text);
  } catch (err) {
    fail("JSON EN-каталога не разбирается: " + err.message);
  }
  if (!parsed || typeof parsed !== "object" || Array.isArray(parsed)) {
    fail("EN-каталог должен быть JSON-объектом.");
  }
  if (parsed.schemaVersion !== SCHEMA_VERSION) {
    fail(
      "Неподдерживаемый schemaVersion EN-каталога: " +
        parsed.schemaVersion + " (ожидается " + SCHEMA_VERSION + ")."
    );
  }
  if (parsed.locale !== LOCALE) {
    fail('Неподдерживаемый locale EN-каталога: ' + parsed.locale + " (ожидается " + LOCALE + ").");
  }
  if (!Array.isArray(parsed.messages)) {
    fail("В EN-каталоге нет массива messages.");
  }
  const seen = new Set();
  parsed.messages.forEach(function (m, i) {
    if (!m || typeof m !== "object") {
      fail("Запись EN-каталога #" + i + " не объект.");
    }
    if (typeof m.key !== "string" || !m.key) {
      fail("Запись EN-каталога #" + i + ": пустой или нестроковый key.");
    }
    if (seen.has(m.key)) fail("В EN-каталоге дублирующийся key: " + m.key);
    seen.add(m.key);
    if (typeof m.translation !== "string") {
      fail("Запись " + m.key + ": translation должен быть строкой.");
    }
    if (typeof m.source !== "string" || !m.source) {
      fail("Запись " + m.key + ": отсутствует исходный source.");
    }
  });
  return parsed;
}

function readEnCatalogFile(file) {
  let text;
  try {
    text = fs.readFileSync(file, "utf8");
  } catch (err) {
    fail("EN-каталог не читается: " + file + " (" + err.message + ")");
  }
  return readEnCatalog(text);
}

// ─── сверка ─────────────────────────────────────────────────────────────

const PLACEHOLDER_RE = /\{([A-Za-z_][A-Za-z0-9_]*)\}/g;

function placeholders(text) {
  const out = new Set();
  if (typeof text !== "string") return out;
  let m;
  PLACEHOLDER_RE.lastIndex = 0;
  while ((m = PLACEHOLDER_RE.exec(text)) !== null) out.add(m[1]);
  return out;
}

function sameSet(a, b) {
  if (a.size !== b.size) return false;
  const it = a.values();
  for (let v = it.next(); !v.done; v = it.next()) {
    if (!b.has(v.value)) return false;
  }
  return true;
}

function compareCatalogs(ruCatalog, enMessages) {
  const ruByKey = new Map();
  ruCatalog.forEach(function (e) { ruByKey.set(e.key, e); });
  const enList = enMessages || [];

  const missing = [];
  const stale = [];
  const unreviewed = [];
  const empty = [];
  const placeholderMismatch = [];

  enList.forEach(function (m) {
    const ru = ruByKey.get(m.key);
    if (!ru) return; // obsolete — собирается отдельным проходом
    if (ru.text !== m.source || (ru.context || "") !== (m.context || "")) {
      stale.push({
        key: m.key,
        previousSource: m.source,
        currentSource: ru.text,
        previousContext: m.context || "",
        currentContext: ru.context || "",
      });
    }
    if (typeof m.translation === "string" && !m.translation.trim()) {
      empty.push({ key: m.key });
    }
    const rp = placeholders(ru.text);
    const tp = placeholders(m.translation);
    if (!sameSet(rp, tp)) {
      placeholderMismatch.push({
        key: m.key,
        sourcePlaceholders: [...rp].sort(),
        translationPlaceholders: [...tp].sort(),
      });
    }
    if (m.reviewed !== true) {
      unreviewed.push({ key: m.key, translation: m.translation });
    }
  });

  const enKeys = new Set();
  enList.forEach(function (m) { enKeys.add(m.key); });

  ruCatalog.forEach(function (e) {
    if (!enKeys.has(e.key)) {
      missing.push({ key: e.key, source: e.text, context: e.context || "" });
    }
  });

  const obsolete = [];
  enList.forEach(function (m) {
    if (!ruByKey.has(m.key)) {
      obsolete.push({ key: m.key, source: m.source, translation: m.translation });
    }
  });

  return {
    missing: missing,
    stale: stale,
    obsolete: obsolete,
    unreviewed: unreviewed,
    empty: empty,
    placeholderMismatch: placeholderMismatch,
  };
}

function hasAnyProblem(cmp) {
  return (
    cmp.missing.length + cmp.stale.length + cmp.obsolete.length +
    cmp.unreviewed.length + cmp.empty.length + cmp.placeholderMismatch.length > 0
  );
}

function collectReport(ruCatalog, enMessages) {
  const cmp = compareCatalogs(ruCatalog, enMessages);
  const counts = {
    totalRu: ruCatalog.length,
    totalEn: (enMessages || []).length,
    missing: cmp.missing.length,
    stale: cmp.stale.length,
    obsolete: cmp.obsolete.length,
    unreviewed: cmp.unreviewed.length,
    empty: cmp.empty.length,
    placeholderMismatch: cmp.placeholderMismatch.length,
  };
  return { hasProblems: hasAnyProblem(cmp), counts: counts, cmp: cmp };
}

// ─── генерация ──────────────────────────────────────────────────────────

// JSON.stringify не экранирует <, > и U+2028/U+2029 — внутри <script> они
// закрывают тег или рвут строку.
function escapeJsonForScript(value) {
  return JSON.stringify(value)
    .replace(/</g, "\\u003c")
    .replace(/>/g, "\\u003e")
    .replace(/\u2028/g, "\\u2028")
    .replace(/\u2029/g, "\\u2029");
}

function escapeHtmlText(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;");
}

// Исполняемый каталог для времени выполнения. Форма ТА ЖЕ, что у RU-исходника
// (массив записей), чтобы код инициализации в обоих файлах совпадал байт в байт:
// `I18N_CATALOG.forEach(...)` не должен работать в одном файле и падать в другом.
// Поле text несёт уже переведённую строку; служебные поля файлового формата
// (source/context/reviewed) в исполняемый каталог не попадают.
function buildEnCatalogEntries(catalog, messages) {
  const byKey = new Map();
  messages.forEach(function (m) { byKey.set(m.key, m); });

  return catalog.map(function (ru) {
    const m = byKey.get(ru.key);
    if (!m) fail("Нет перевода для ключа: " + ru.key);
    if (typeof m.translation !== "string" || !m.translation.trim()) {
      fail("Пустой перевод для ключа: " + ru.key);
    }
    return { key: ru.key, text: m.translation };
  });
}

function titleTranslation(catalog, messages) {
  if (!catalog.some(function (e) { return e.key === KEY_TITLE; })) {
    fail("В RU-каталоге нет обязательного ключа " + KEY_TITLE + ".");
  }
  const m = messages.find(function (x) { return x.key === KEY_TITLE; });
  if (!m) fail("Нет перевода для обязательного ключа " + KEY_TITLE + ".");
  if (typeof m.translation !== "string" || !m.translation.trim()) {
    fail("Пустой перевод для обязательного ключа " + KEY_TITLE + ".");
  }
  return m.translation;
}

function replaceCatalog(html, entries) {
  const b = html.indexOf(CATALOG_BEGIN);
  const e = html.indexOf(CATALOG_END);
  if (b === -1 || e === -1 || e < b) fail("Маркеры каталога не найдены.");
  const sliced = sliceCatalogJson(html.slice(b, e));
  const newline = html.indexOf("\r\n") !== -1 ? "\r\n" : "\n";
  const indent = "    ";
  const body = entries
    .map(function (entry) {
      return (
        indent + "{ " +
        '"key": ' + escapeJsonForScript(entry.key) + ", " +
        '"text": ' + escapeJsonForScript(entry.text) +
        " }"
      );
    })
    .join("," + newline);
  const rendered = sliced.prefix + "[" + newline + body + newline + "]" + sliced.suffix;
  return html.slice(0, b + CATALOG_BEGIN.length) + newline + rendered + newline + html.slice(e);
}

// Только языковые метаданные: lang у <html> и содержимое <title>.
function replaceLang(html, locale) {
  const re = /(<html\b[^>]*?\blang=")([A-Za-z-]+)(")/g;
  const matches = html.match(re) || [];
  if (matches.length !== 1) {
    fail(
      'В исходном HTML ожидался ровно один атрибут lang="..." (найдено ' +
        matches.length + "): замена неоднозначна."
    );
  }
  return html.replace(re, "$1" + locale + "$3");
}

function replaceTitle(html, title) {
  const re = /<title>([^<]*)<\/title>/g;
  const matches = html.match(re) || [];
  if (matches.length !== 1) {
    fail(
      "В исходном HTML ожидался ровно один <title> (найдено " +
        matches.length + "): замена неоднозначна."
    );
  }
  return html.replace(re, "<title>" + escapeHtmlText(title) + "</title>");
}

function renderEnHtml(args) {
  let out = args.html;
  out = replaceCatalog(out, buildEnCatalogEntries(args.catalog, args.messages));
  out = replaceLang(out, LOCALE);
  out = replaceTitle(out, titleTranslation(args.catalog, args.messages));
  return out;
}

// ─── очередь перевода ───────────────────────────────────────────────────

// Только переводимые сообщения: данные проекта, секреты и произвольный HTML
// сюда не попадают — файл предназначен в том числе для передачи переводчику.
function buildPendingExport(ruCatalog, enMessages) {
  const byKey = new Map();
  (enMessages || []).forEach(function (m) { byKey.set(m.key, m); });

  const messages = [];
  ruCatalog.forEach(function (ru) {
    const m = byKey.get(ru.key);
    const isStale = !!m && (ru.text !== m.source || (ru.context || "") !== (m.context || ""));
    if (m && !isStale) return; // перевод актуален — в очередь не попадает
    messages.push({
      key: ru.key,
      source: ru.text,
      context: ru.context || "",
      previousSource: m ? m.source : "",
      previousTranslation: m ? m.translation : "",
      reason: !m ? "new" : "stale",
    });
  });

  const obsolete = [];
  (enMessages || []).forEach(function (m) {
    if (!ruCatalog.some(function (ru) { return ru.key === m.key; })) {
      obsolete.push({ key: m.key, source: m.source, translation: m.translation });
    }
  });

  return { locale: LOCALE, messages: messages, obsolete: obsolete };
}

// ─── файловый слой ──────────────────────────────────────────────────────

function writeFileAtomic(target, content) {
  const dir = path.dirname(target);
  const tmp = path.join(dir, "." + path.basename(target) + ".tmp-" + process.pid);
  fs.writeFileSync(tmp, content, "utf8");
  try {
    fs.renameSync(tmp, target);
  } catch (err) {
    try { fs.unlinkSync(tmp); } catch (_) { /* временный файл уже удалён */ }
    fail(
      "Не удалось заменить " + target + ": " + err.message +
        ". Возможно, файл занят другим процессом (Archicad)."
    );
  }
}

function readTextOrNull(file) {
  try {
    return fs.readFileSync(file, "utf8");
  } catch (_) {
    return null;
  }
}

function loadOrFail() {
  let html;
  try {
    html = fs.readFileSync(DEFAULT_RU, "utf8");
  } catch (err) {
    fail("RU-исходник не читается: " + DEFAULT_RU + " (" + err.message + ")");
  }
  const ru = readRuSource(html);
  const en = readEnCatalogFile(DEFAULT_CATALOG);
  return { ru: ru, en: en };
}

// ─── CLI ────────────────────────────────────────────────────────────────

function usage() {
  return [
    "Usage: node Tools/localize_html.js <status|generate|check|export-pending>",
    "",
    "  status          отчёт missing/stale/obsolete/unreviewed; ничего не пишет",
    "  generate        сгенерировать Interface_en.html (строгий режим)",
    "  check           как status + побайтовое сравнение с существующим EN",
    "  export-pending  выгрузить очередь перевода в JSON (--out <file>)",
    "",
  ].join("\n");
}

function countsLine(label, counts) {
  return (
    label + ": keys=" + counts.totalRu +
    " missing=" + counts.missing +
    " stale=" + counts.stale +
    " obsolete=" + counts.obsolete +
    " unreviewed=" + counts.unreviewed +
    " empty=" + counts.empty +
    " placeholderMismatch=" + counts.placeholderMismatch + "\n"
  );
}

function printProblems(cmp) {
  function section(title, items, render) {
    process.stdout.write("\n" + title + ":\n");
    if (!items.length) process.stdout.write("  (нет)\n");
    items.forEach(function (e) { process.stdout.write("  " + render(e) + "\n"); });
  }
  section("missing (нужен перевод)", cmp.missing, function (e) {
    return e.key + " — " + e.source;
  });
  section("stale (изменился source/context)", cmp.stale, function (e) {
    return e.key + "\n    было:  " + e.previousSource + "\n    стало: " + e.currentSource;
  });
  section("obsolete (ключ удалён из RU)", cmp.obsolete, function (e) {
    return e.key;
  });
  section("unreviewed (reviewed != true)", cmp.unreviewed, function (e) {
    return e.key + " — " + JSON.stringify(e.translation);
  });
  section("empty (пустой перевод)", cmp.empty, function (e) { return e.key; });
  section("placeholderMismatch", cmp.placeholderMismatch, function (e) {
    return e.key + " — source " + JSON.stringify(e.sourcePlaceholders) +
      " vs translation " + JSON.stringify(e.translationPlaceholders);
  });
}

function runStatus() {
  const data = loadOrFail();
  const report = collectReport(data.ru.catalog, data.en.messages);
  process.stdout.write(countsLine("status", report.counts));
  if (!report.hasProblems) return 0;
  printProblems(report.cmp);
  return 1;
}

function runGenerate() {
  const data = loadOrFail();
  const report = collectReport(data.ru.catalog, data.en.messages);
  if (report.hasProblems) {
    process.stderr.write(
      "generate: каталог неполон или устарел (" + JSON.stringify(report.counts) +
        "). Сначала status, затем переведите перечисленное. Файл EN не изменён.\n"
    );
    printProblems(report.cmp);
    return 1;
  }
  const out = renderEnHtml({
    html: data.ru.html,
    catalog: data.ru.catalog,
    messages: data.en.messages,
  });
  writeFileAtomic(DEFAULT_EN, out);
  process.stdout.write(
    "generate: записан " + DEFAULT_EN + " (" + data.ru.catalog.length + " сообщений).\n"
  );
  return 0;
}

function runCheck() {
  const data = loadOrFail();
  const report = collectReport(data.ru.catalog, data.en.messages);
  const onDisk = readTextOrNull(DEFAULT_EN);

  if (report.hasProblems) {
    process.stderr.write(
      "check: каталог неполон или устарел (" + JSON.stringify(report.counts) +
        "). Файл EN не изменён.\n"
    );
    printProblems(report.cmp);
    return 1;
  }
  if (onDisk === null) {
    process.stderr.write(
      "check: Interface_en.html не найден (" + DEFAULT_EN + "). Создайте его командой generate.\n"
    );
    return 1;
  }
  const expected = renderEnHtml({
    html: data.ru.html,
    catalog: data.ru.catalog,
    messages: data.en.messages,
  });
  if (onDisk !== expected) {
    process.stderr.write(
      "check: Interface_en.html устарел или подделан — отличается от того, что даёт generate.\n"
    );
    return 1;
  }
  process.stdout.write(countsLine("check", report.counts));
  return 0;
}

function runExportPending(outPath) {
  if (!outPath) fail("export-pending требует --out <file.json>.");
  const data = loadOrFail();
  const pending = buildPendingExport(data.ru.catalog, data.en.messages);
  const target = path.isAbsolute(outPath)
    ? outPath
    : path.resolve(__dirname, "..", outPath);
  writeFileAtomic(target, JSON.stringify(pending, null, 2) + "\n");
  process.stdout.write(
    "export-pending: " + pending.messages.length + " записей, " +
      pending.obsolete.length + " obsolete → " + target + "\n"
  );
  return 0;
}

function main(argv) {
  const cmd = argv[0];
  switch (cmd) {
    case "status":
      return runStatus();
    case "generate":
      return runGenerate();
    case "check":
      return runCheck();
    case "export-pending":
      return runExportPending(
        argv.indexOf("--out") !== -1 ? argv[argv.indexOf("--out") + 1] : null
      );
    case "--help":
    case "-h":
      process.stdout.write(usage());
      return 0;
    default:
      process.stderr.write(
        (cmd ? "Неизвестная команда: " + cmd + "\n\n" : "") + usage()
      );
      return cmd ? 1 : 1;
  }
}

if (require.main === module) {
  try {
    process.exit(main(process.argv.slice(2)));
  } catch (err) {
    if (err instanceof L10nError) {
      process.stderr.write("localize_html.js: " + err.message + "\n");
      process.exit(1);
    }
    throw err;
  }
}

module.exports = {
  L10nError: L10nError,
  readRuSource: readRuSource,
  readEnCatalog: readEnCatalog,
  readEnCatalogFile: readEnCatalogFile,
  compareCatalogs: compareCatalogs,
  collectReport: collectReport,
  hasAnyProblem: hasAnyProblem,
  renderEnHtml: renderEnHtml,
  buildPendingExport: buildPendingExport,
  writeFileAtomic: writeFileAtomic,
};