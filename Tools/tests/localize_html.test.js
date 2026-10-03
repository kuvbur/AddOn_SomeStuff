// Контракт Tools/localize_html.js: чтение RU-каталога, сверка с EN,
// генерация автономного EN-HTML и режим проверки без записи.
//
// Фикстуры — минимальные, намеренно не копия рабочего HTML: контракт
// инструмента не должен зависеть от размера исходника.

const test = require("node:test");
const assert = require("node:assert");
const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");

const {
  readRuSource,
  readEnCatalog,
  compareCatalogs,
  renderEnHtml,
  collectReport,
} = require("../localize_html.js");

let scratchCounter = 0;
function makeScratch() {
  scratchCounter += 1;
  const dir = path.join(
    os.tmpdir(),
    "somestuff_i18n_test_" + process.pid + "_" + scratchCounter
  );
  fs.mkdirSync(dir, { recursive: true });
  return dir;
}

// ─── фикстуры ───────────────────────────────────────────────────────────

function ruHtml(catalogJson, opts) {
  const o = opts || {};
  const lang = o.lang || "ru";
  const title = o.title || "Заголовок RU";
  return [
    "<!DOCTYPE html>",
    '<html lang="' + lang + '" style="--ac-text-primary:#111;">',
    "<head>",
    '<meta charset="UTF-8">',
    "<title>" + title + "</title>",
    "</head>",
    "<body>",
    '<!-- i18n-catalog-begin -->',
    "const I18N_CATALOG = " + catalogJson + ";",
    '<!-- i18n-catalog-end -->',
    "const I18N_INDEX = Object.create(null);",
    "I18N_CATALOG.forEach(function (e) { I18N_INDEX[e.key] = e.source; });",
    "</body>",
    "</html>",
    "",
  ].join(o.newline || "\n");
}

const RU_CATALOG = [
  { key: "app.title", text: "Палитра SomeStuff", context: "Заголовок вкладки" },
  { key: "common.ok", text: "ОК", context: "" },
];

const RU_TITLE = "Палитра SomeStuff";

function enCatalog(messages, overrides) {
  return JSON.stringify(
    Object.assign(
      { schemaVersion: 1, locale: "en", messages: messages || [] },
      overrides || {}
    ),
    null,
    2
  );
}

function enMessages(extra) {
  return [
    { key: "app.title", source: "Палитра SomeStuff", context: "Заголовок вкладки", translation: "SomeStuff Palette", reviewed: true },
    { key: "common.ok", source: "ОК", context: "", translation: "OK", reviewed: true },
  ].concat(extra || []);
}

// ─── чтение RU-каталога ─────────────────────────────────────────────────

test("readRuSource: читает каталог из маркеров и исходный текст файла", () => {
  const html = ruHtml(JSON.stringify(RU_CATALOG, null, 2));
  const res = readRuSource(html);
  assert.strictEqual(res.catalog.length, 2);
  assert.strictEqual(res.catalog[0].key, "app.title");
  assert.strictEqual(res.catalog[0].text, "Палитра SomeStuff");
  assert.strictEqual(res.html, html, "исходный текст возвращается без изменений");
});

test("readRuSource: сообщает об отсутствии маркеров", () => {
  assert.throws(() => readRuSource("<html>нет каталога</html>"), /маркер/i);
});

test("readRuSource: сообщает о повторе маркеров", () => {
  const bad =
    ruHtml(JSON.stringify(RU_CATALOG)) +
    "<!-- i18n-catalog-begin -->\nconst X = [];\n<!-- i18n-catalog-end -->\n";
  assert.throws(() => readRuSource(bad), /маркер/i);
});

test("readRuSource: сообщает о невалидном JSON каталога", () => {
  assert.throws(() => readRuSource(ruHtml("[{key: 'нет кавычек'}]")), /JSON/i);
});

test("readRuSource: сообщает о повторяющемся ключе в RU-каталоге", () => {
  const dup = [
    { key: "a.b", text: "Раз", context: "" },
    { key: "a.b", text: "Два", context: "" },
  ];
  assert.throws(() => readRuSource(ruHtml(JSON.stringify(dup))), /дублирующ/i);
});

test("readRuSource: пустой source в RU-каталоге — ошибка", () => {
  const bad = [{ key: "a.b", text: "", context: "" }];
  assert.throws(() => readRuSource(ruHtml(JSON.stringify(bad))), /text/i);
});

// ─── чтение EN-каталога ─────────────────────────────────────────────────

test("readEnCatalog: принимает корректный каталог", () => {
  const res = readEnCatalog(enCatalog(enMessages()));
  assert.strictEqual(res.schemaVersion, 1);
  assert.strictEqual(res.locale, "en");
  assert.strictEqual(res.messages.length, 2);
});

test("readEnCatalog: отвергает неверную schemaVersion", () => {
  assert.throws(() => readEnCatalog(enCatalog([], { schemaVersion: 2 })), /schemaVersion/i);
});

test("readEnCatalog: отвергает неверный locale", () => {
  assert.throws(() => readEnCatalog(enCatalog([], { locale: "de" })), /locale/i);
});

test("readEnCatalog: отвергает повтор ключа", () => {
  const msgs = enMessages();
  msgs.push({ key: "common.ok", source: "ОК", context: "", translation: "OK", reviewed: true });
  assert.throws(() => readEnCatalog(enCatalog(msgs)), /дублирующ/i);
});

// ─── сверка каталогов ───────────────────────────────────────────────────

test("compareCatalogs: полное совпадение — без проблем", () => {
  const res = compareCatalogs(RU_CATALOG, readEnCatalog(enCatalog(enMessages())).messages);
  assert.deepStrictEqual(res.missing, []);
  assert.deepStrictEqual(res.stale, []);
  assert.deepStrictEqual(res.obsolete, []);
  assert.deepStrictEqual(res.unreviewed, []);
  assert.deepStrictEqual(res.empty, []);
  assert.deepStrictEqual(res.placeholderMismatch, []);
});

test("compareCatalogs: ключ без перевода — missing", () => {
  const ru = RU_CATALOG.concat([{ key: "new.key", text: "Новое", context: "" }]);
  const res = compareCatalogs(ru, enMessages());
  assert.strictEqual(res.missing.length, 1);
  assert.strictEqual(res.missing[0].key, "new.key");
});

test("compareCatalogs: изменённый русский source — stale, даже при том же ключе", () => {
  const ru = [
    { key: "app.title", text: "Палитра SomeStuff 2", context: "Заголовок вкладки" },
    RU_CATALOG[1],
  ];
  const res = compareCatalogs(ru, enMessages());
  assert.strictEqual(res.stale.length, 1);
  assert.strictEqual(res.stale[0].key, "app.title");
  assert.ok(res.stale[0].currentSource, "устаревшая запись несёт новый source");
});

test("compareCatalogs: изменённый context — stale", () => {
  const ru = [
    { key: "app.title", text: "Палитра SomeStuff", context: "Заголовок окна" },
    RU_CATALOG[1],
  ];
  const res = compareCatalogs(ru, enMessages());
  assert.strictEqual(res.stale.length, 1);
  assert.strictEqual(res.stale[0].key, "app.title");
});

test("compareCatalogs: удалённый в RU ключ — obsolete", () => {
  const ru = [RU_CATALOG[0]];
  const res = compareCatalogs(ru, enMessages());
  assert.strictEqual(res.obsolete.length, 1);
  assert.strictEqual(res.obsolete[0].key, "common.ok");
});

test("compareCatalogs: пустой translation — empty", () => {
  const msgs = enMessages([
    { key: "blank.key", source: "Пусто", context: "", translation: "  ", reviewed: true },
  ]);
  const ru = RU_CATALOG.concat([{ key: "blank.key", text: "Пусто", context: "" }]);
  const res = compareCatalogs(ru, msgs);
  assert.strictEqual(res.empty.length, 1);
  assert.strictEqual(res.empty[0].key, "blank.key");
});

test("compareCatalogs: reviewed не true — unreviewed", () => {
  const msgs = enMessages([
    { key: "draft.key", source: "Черновик", context: "", translation: "Draft", reviewed: false },
  ]);
  const ru = RU_CATALOG.concat([{ key: "draft.key", text: "Черновик", context: "" }]);
  const res = compareCatalogs(ru, msgs);
  assert.strictEqual(res.unreviewed.length, 1);
});

test("compareCatalogs: несовпадение плейсхолдеров", () => {
  const msgs = enMessages([
    { key: "count.key", source: "Найдено {count}", context: "", translation: "Found", reviewed: true },
  ]);
  const ru = RU_CATALOG.concat([{ key: "count.key", text: "Найдено {count}", context: "" }]);
  const res = compareCatalogs(ru, msgs);
  assert.strictEqual(res.placeholderMismatch.length, 1);
});

test("compareCatalogs: повтор одного плейсхолдера учитывается по именам, не по счёту", () => {
  const msgs = enMessages([
    { key: "rep.key", source: "{a} и {a}", context: "", translation: "{a} and {a}", reviewed: true },
  ]);
  const ru = RU_CATALOG.concat([{ key: "rep.key", text: "{a} и {a}", context: "" }]);
  const res = compareCatalogs(ru, msgs);
  assert.deepStrictEqual(res.placeholderMismatch, []);
});

test("collectReport: hasProblems true при любой проблеме", () => {
  const ru = RU_CATALOG.concat([{ key: "new.key", text: "Новое", context: "" }]);
  const report = collectReport(ru, enMessages());
  assert.strictEqual(report.hasProblems, true);
  assert.strictEqual(report.counts.missing, 1);
});

test("collectReport: hasProblems false на полном каталоге", () => {
  const report = collectReport(RU_CATALOG, enMessages());
  assert.strictEqual(report.hasProblems, false);
});

// ─── генерация EN ───────────────────────────────────────────────────────

test("renderEnHtml: меняет lang, title и каталог, остальное — байт в байт", () => {
  const html = ruHtml(JSON.stringify(RU_CATALOG, null, 2), { title: "Старый" });
  const out = renderEnHtml({
    html,
    catalog: RU_CATALOG,
    messages: enMessages(),
  });

  assert.ok(out.includes('<html lang="en"'), "lang заменён на en");
  assert.ok(out.includes("<title>SomeStuff Palette</title>"), "title взят из app.title");
  assert.ok(!out.includes("Старый"), "старый заголовок не остался");
  // Исполняемый каталог — компактный объект key → строка (для обращения по
  // ключу в горячем пути), служебные поля файлового формата туда не попадают.
  assert.ok(out.includes('{ "key": "app.title", "text": "SomeStuff Palette" }'),
    "запись каталога несёт ключ и уже переведённую строку");
  assert.ok(out.includes('"translation"') === false, "служебные поля файла в HTML не попадают");
  assert.ok(out.includes('"source"') === false, "исходный source в HTML не попадает");
  assert.ok(out.includes('"reviewed"') === false, "отметка reviewed в HTML не попадает");

  // Всё вне разрешённых областей идентично.
  const strip = (s) =>
    s
      .replace(/<html lang="[a-z]+"/, '<html lang="X"')
      .replace(/<title>[^<]*<\/title>/, "<title>X</title>")
      .replace(
        /<!-- i18n-catalog-begin -->[\s\S]*?<!-- i18n-catalog-end -->/,
        "CATALOG"
      );
  assert.strictEqual(strip(out), strip(html), "код вне каталога не изменён");
});

test("renderEnHtml: детерминирован — два вызова дают одинаковый результат", () => {
  const html = ruHtml(JSON.stringify(RU_CATALOG, null, 2));
  const args = { html, catalog: RU_CATALOG, messages: enMessages() };
  assert.strictEqual(renderEnHtml(args), renderEnHtml(args));
});

test("renderEnHtml: сохраняет CRLF исходника", () => {
  const html = ruHtml(JSON.stringify(RU_CATALOG, null, 2), { newline: "\r\n" });
  const out = renderEnHtml({ html, catalog: RU_CATALOG, messages: enMessages() });
  assert.ok(out.includes("\r\n"), "CRLF сохранены");
  assert.ok(!/[^\r]\n/.test(out), " Lone LF не появились");
});

test("renderEnHtml: сохраняет BOM", () => {
  const html = "﻿" + ruHtml(JSON.stringify(RU_CATALOG, null, 2));
  const out = renderEnHtml({ html, catalog: RU_CATALOG, messages: enMessages() });
  assert.ok(out.startsWith("﻿"), "BOM сохранён");
});

test("renderEnHtml: экранирует последовательность </script> в переводе", () => {
  const msgs = enMessages([
    { key: "evil.key", source: "Опасно", context: "", translation: "</script><b>x", reviewed: true },
  ]);
  const ru = RU_CATALOG.concat([{ key: "evil.key", text: "Опасно", context: "" }]);
  const out = renderEnHtml({ html: ruHtml(JSON.stringify(RU_CATALOG, null, 2)), catalog: ru, messages: msgs });
  assert.ok(!out.includes("</script><b>x"), "сырая последовательность не попала в файл");
  assert.ok(out.includes("\\u003c"), "символ < экранирован в JSON каталога");
});

test("renderEnHtml: экранирует U+2028 и U+2029", () => {
  const msgs = enMessages([
    { key: "sep.key", source: "Разделитель", context: "", translation: "a\u2028b\u2029c", reviewed: true },
  ]);
  const ru = RU_CATALOG.concat([{ key: "sep.key", text: "Разделитель", context: "" }]);
  const out = renderEnHtml({ html: ruHtml(JSON.stringify(RU_CATALOG, null, 2)), catalog: ru, messages: msgs });
  assert.ok(!out.includes("\u2028") && !out.includes("\u2029"), "разделители строк экранированы");
  assert.ok(out.includes("\\u2028"));
});

test("renderEnHtml: экранирует кавычки и амперсанд в заголовке", () => {
  const msgs = enMessages();
  msgs[0] = Object.assign({}, msgs[0], {
    translation: 'A & B "C" <D>',
  });
  const out = renderEnHtml({
    html: ruHtml(JSON.stringify(RU_CATALOG, null, 2)),
    catalog: RU_CATALOG,
    messages: msgs,
  });
  assert.ok(!out.includes('A & B "C" <D>'), "сырой заголовок не попал в <title>");
  assert.ok(/<title>A &amp; B "C" &lt;D&gt;<\/title>/.test(out),
    "в заголовке экранированы & и < >, кавычки в тексте безопасны");
});

test("renderEnHtml: без ключа app.title — ошибка", () => {
  const ru = [{ key: "common.ok", text: "ОК", context: "" }];
  const msgs = [{ key: "common.ok", source: "ОК", context: "", translation: "OK", reviewed: true }];
  assert.throws(
    () => renderEnHtml({ html: ruHtml(JSON.stringify(ru, null, 2)), catalog: ru, messages: msgs }),
    /app\.title/
  );
});

test("renderEnHtml: два маркера lang — ошибка (неоднозначная замена)", () => {
  const html =
    ruHtml(JSON.stringify(RU_CATALOG, null, 2)) +
    "<html lang=\"ru\">второй html-тег</html>\n";
  assert.throws(
    () => renderEnHtml({ html, catalog: RU_CATALOG, messages: enMessages() }),
    /lang/
  );
});

test("renderEnHtml: lang на не-html теге не считается совпадением", () => {
  const html =
    ruHtml(JSON.stringify(RU_CATALOG, null, 2)) +
    "<p lang=\"ru\">не html</p>\n";
  const out = renderEnHtml({ html, catalog: RU_CATALOG, messages: enMessages() });
  assert.ok(out.includes('<p lang="ru">'), "чужой атрибут не тронут");
  assert.ok(out.includes('<html lang="en"'), "целевой lang заменён");
});

test("renderEnHtml: форма исполняемого каталога совпадает с RU (массив, не объект)", () => {
  // Если EN-каталог станет объектом, код инициализации `I18N_CATALOG.forEach`
  // в EN-файле упадёт — а валидаторы синтаксиса этого не видят.
  const out = renderEnHtml({
    html: ruHtml(JSON.stringify(RU_CATALOG, null, 2)),
    catalog: RU_CATALOG,
    messages: enMessages(),
  });
  const m = out.match(/const I18N_CATALOG = (\[[\s\S]*?\]);/);
  assert.ok(m, "массив каталога извлекается");
  const parsed = JSON.parse(m[1]);
  assert.ok(Array.isArray(parsed), "EN-каталог — массив, как у RU-исходника");
  assert.strictEqual(parsed.length, RU_CATALOG.length);
  assert.ok(typeof parsed[0].key === "string", "запись имеет строковый key");
  assert.ok(typeof parsed[0].text === "string", "запись имеет строковый text");
  // Мини-скрипт инициализации реального HTML обязан отработать на EN-файле.
  // Реальный код инициализации из HTML-шаблона: и Object.create, и forEach.
  const body =
    "var I18N_CATALOG = " + m[1] + ";\n" +
    "var I18N_INDEX = Object.create(null);\n" +
    "I18N_CATALOG.forEach(function (e) { I18N_INDEX[e.key] = e.text; });\n" +
    "return I18N_INDEX;";
  const fn = new Function("Object", body);
  const index = fn({ create: () => ({}) });
  assert.strictEqual(Object.keys(index).length, RU_CATALOG.length);
  assert.strictEqual(index["app.title"], "SomeStuff Palette",
    "в runtime-индексе лежит перевод, а не русский source");
});
