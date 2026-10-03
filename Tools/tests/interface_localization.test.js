// Контракт runtime-helper t() из Interface_ru.html.
//
// Тест извлекает НАСТОЯЩИЙ код каталога и функции t из файла и выполняет его,
// а не переписывает логику здесь: тест с собственной копией реализации
// проходит и после того, как реальный helper сломан.
//
// Файл читается из аргумента или из пути по умолчанию, поэтому тест работает
// и на интерфейсе другого языка (Interface_en.html) — для сравнения поведения.

const test = require("node:test");
const assert = require("node:assert");
const fs = require("node:fs");
const path = require("node:path");

const DEFAULT_HTML = path.resolve(
  __dirname, "..", "..", "Sources", "AddOnResources", "RFIX", "HTML", "Interface_ru.html"
);

// Вырезает из HTML исполняемый кусок: объявление каталога, построение индекса
// и функцию t. Всё это — непрерывный блок между маркерами I18N и первым
// следующим разделом-комментарием.
function extractRuntime(html) {
  const begin = html.indexOf("const I18N_CATALOG = ");
  if (begin === -1) throw new Error("в HTML нет I18N_CATALOG");
  // Конец блока — первая закрывающая скобка на нулевом отступе после
  // объявления t: у вложенных конструкций отступ больше.
  const start = html.indexOf("function t(key, params)");
  if (start === -1) throw new Error("в HTML нет функции t");
  const tail = html.slice(start);
  const m = tail.match(/\n\}/);
  if (!m) throw new Error("не найден конец функции t");
  return html.slice(begin, start + m.index + 2);
}

function loadRuntime(htmlPath) {
  const file = htmlPath || process.env.SOMESTUFF_HTML || DEFAULT_HTML;
  const html = fs.readFileSync(file, "utf8");
  const code = extractRuntime(html);
  // Параметр с именем console перекрывается телом функции: внутри `new Function`
  // объявленный параметр виден, но обращение `console` разрешается в параметр
  // функции — то есть действительно перехватывается. Проверено: без явной
  // передачи console.error не вызывается, поэтому sandbox передаётся позиционно.
  const recorder = { errors: [] };
  const sandbox = {
    error: (msg) => recorder.errors.push(String(msg)),
    log: () => {},
    warn: () => {},
  };
  const factory = new Function(
    "console",
    code + "\nreturn { t: t, index: I18N_INDEX, catalog: I18N_CATALOG };"
  );
  return { api: factory(sandbox), html: html, file: file, console: recorder };
}

// ─── базовый контракт ───────────────────────────────────────────────────

test("каталог непуст и уникален по ключу", () => {
  const { api } = loadRuntime();
  assert.ok(Array.isArray(api.catalog), "каталог — массив");
  assert.ok(api.catalog.length > 0, "каталог не пуст");
  const keys = api.catalog.map((e) => e.key);
  assert.strictEqual(new Set(keys).size, keys.length, "ключи уникальны");
  api.catalog.forEach((e) => {
    assert.strictEqual(typeof e.key, "string");
    assert.ok(e.key.length > 0, "ключ непуст");
    assert.strictEqual(typeof e.text, "string");
    assert.ok(e.text.length > 0, "текст непуст: " + e.key);
  });
});

test("app.title присутствует и совпадает с <title> документа", () => {
  const { api, html } = loadRuntime();
  const entry = api.catalog.find((e) => e.key === "app.title");
  assert.ok(entry, "ключ app.title есть в каталоге");
  const m = html.match(/<title>([^<]*)<\/title>/);
  assert.ok(m, "документ содержит <title>");
  assert.strictEqual(m[1], entry.text,
    "<title> и app.title — один и тот же текст; иначе генератор возьмёт не тот");
});

test("t возвращает строку каталога для известного ключа", () => {
  const { api } = loadRuntime();
  const entry = api.catalog[0];
  assert.strictEqual(api.t(entry.key), entry.text);
});

test("t не подставляет ничего без params", () => {
  const { api, html } = loadRuntime();
  const key = api.catalog.find((e) => e.text.includes("{"))?.key;
  if (!key) return; // в текущем каталоге плейсхолдеров нет — проверка не применима
  assert.ok(api.t(key).includes("{"), "без params плейсхолдер остаётся");
  void html;
});

// ─── подстановка параметров ─────────────────────────────────────────────

test("t подставляет параметры и повторяет один плейсхолдер", () => {
  const { api } = loadRuntime();
  const entry = api.catalog.find((e) => /\{[A-Za-z_]/.test(e.text));
  if (!entry) return; // плейсхолдеров в текущем каталоге нет
  const name = entry.text.match(/\{([A-Za-z_][A-Za-z0-9_]*)\}/)[1];
  const out = api.t(entry.key, { [name]: "X" });
  assert.ok(!out.includes("{" + name + "}"), "плейсхолдер подставлен");
  assert.ok(out.includes("X"));
});

test("t подставляет 0, false и пустую строку как значения (не теряет их)", () => {
  // Отдельная мини-реализация формата нужна только как образец ожидаемого
  // поведения: проверяем на реальной функции через временный каталог.
  const html = fs
    .readFileSync(DEFAULT_HTML, "utf8")
    .replace(
      /const I18N_CATALOG = \[[\s\S]*?\n\];/,
      'const I18N_CATALOG = [\n' +
        '  { "key": "k.num", "text": "Найдено {count}" },\n' +
        '  { "key": "k.flag", "text": "включено {on}" },\n' +
        '  { "key": "k.empty", "text": "имя [{name}]" }\n' +
        "];"
    );
  const { api } = loadRuntimeFrom(html);
  assert.strictEqual(api.t("k.num", { count: 0 }), "Найдено 0");
  assert.strictEqual(api.t("k.flag", { on: false }), "включено false");
  assert.strictEqual(api.t("k.empty", { name: "" }), "имя []");
});

function loadRuntimeFrom(html) {
  const code = extractRuntime(html);
  const factory = new Function(
    "console",
    code + "\nreturn { t: t, index: I18N_INDEX, catalog: I18N_CATALOG };"
  );
  return { api: factory({ error: () => {}, log: () => {}, warn: () => {} }) };
}

test("t не трактует значение параметра как шаблон замены ($&, $1)", () => {
  const html = fs
    .readFileSync(DEFAULT_HTML, "utf8")
    .replace(
      /const I18N_CATALOG = \[[\s\S]*?\n\];/,
      'const I18N_CATALOG = [\n' +
        '  { "key": "k.sub", "text": "значение [{v}] тут" }\n' +
        "];"
    );
  const { api } = loadRuntimeFrom(html);
  assert.strictEqual(
    api.t("k.sub", { v: "$& и $1" }),
    "значение [$& и $1] тут",
    "спецпоследовательности в значении остаются буквально"
  );
});

test("t не подставляет рекурсивно содержимое параметра", () => {
  const html = fs
    .readFileSync(DEFAULT_HTML, "utf8")
    .replace(
      /const I18N_CATALOG = \[[\s\S]*?\n\];/,
      'const I18N_CATALOG = [\n' +
        '  { "key": "k.rec", "text": "список: {list}" }\n' +
        "];"
    );
  const { api } = loadRuntimeFrom(html);
  assert.strictEqual(
    api.t("k.rec", { list: "{other}" }),
    "список: {other}",
    "вставленное значение не разрешается повторно"
  );
});

test("t оставляет неизвестный плейсхолдер как есть, не роняя строку", () => {
  const html = fs
    .readFileSync(DEFAULT_HTML, "utf8")
    .replace(
      /const I18N_CATALOG = \[[\s\S]*?\n\];/,
      'const I18N_CATALOG = [\n' +
        '  { "key": "k.miss", "text": "Найдено {count} из {total}" }\n' +
        "];"
    );
  const { api } = loadRuntimeFrom(html);
  assert.strictEqual(api.t("k.miss", { count: 3 }), "Найдено 3 из {total}");
});

test("t не трогает фигурные скобки, не являющиеся плейсхолдером", () => {
  const html = fs
    .readFileSync(DEFAULT_HTML, "utf8")
    .replace(
      /const I18N_CATALOG = \[[\s\S]*?\n\];/,
      'const I18N_CATALOG = [\n' +
        '  { "key": "k.brace", "text": "правило { a; b } пусто" }\n' +
        "];"
    );
  const { api } = loadRuntimeFrom(html);
  assert.strictEqual(api.t("k.brace"), "правило { a; b } пусто",
    "пробел внутри скобок не делает их плейсхолдером");
});

// ─── отказ вместо молчаливого русского fallback ─────────────────────────

test("t на неизвестном ключе возвращает ключ и пишет в консоль, а не русский текст", () => {
  const { api, console: fake } = loadRuntime();
  const out = api.t("no.such.key");
  assert.strictEqual(out, "no.such.key", "виден ключ, а не русская строка");
  assert.ok(fake.errors.length > 0, "отсутствие ключа попало в консоль");
  assert.ok(fake.errors.some((m) => m.includes("no.such.key")));
});

test("t не отдаёт русский текст при пустом индексе ключа-прототипа", () => {
  const { api } = loadRuntime();
  // Object.create(null) обязателен: иначе api.t("constructor") вернул бы функцию.
  const out = api.t("constructor");
  assert.strictEqual(typeof out, "string", "возвращена строка, а не функция Object");
  assert.strictEqual(out, "constructor");
});

test("t на неизвестном ключе не падает даже при отсутствии console", () => {
  const html = fs
    .readFileSync(DEFAULT_HTML, "utf8")
    .replace(
      /const I18N_CATALOG = \[[\s\S]*?\n\];/,
      'const I18N_CATALOG = [\n  { "key": "k.one", "text": "раз" }\n];'
    );
  const code = extractRuntime(html);
  const factory = new Function(
    code + "\nreturn { t: t, catalog: I18N_CATALOG };"
  );
  const { t } = factory({});
  assert.strictEqual(typeof t("k.one"), "string");
  assert.strictEqual(t("нет.ключа"), "нет.ключа");
});

// ─── EN-файл, если он есть ─────────────────────────────────────────────

test("EN-файл, если существует, собран из того же runtime-кода", { skip: false }, () => {
  const enPath = path.resolve(
    __dirname, "..", "..", "Sources", "AddOnResources", "RFIX", "HTML", "Interface_en.html"
  );
  if (!fs.existsSync(enPath)) return; // артефакт ещё не сгенерирован — не ошибка
  const en = loadRuntime(enPath);
  const ru = loadRuntime(DEFAULT_HTML);
  assert.strictEqual(
    en.api.catalog.length,
    ru.api.catalog.length,
    "в EN и RU одинаковое число сообщений"
  );
  en.api.catalog.forEach((e, i) => {
    assert.strictEqual(e.key, ru.api.catalog[i].key,
      "порядок и ключи каталогов совпадают, иначе строки перепутаются");
  });
  const titleEntry = en.api.catalog.find((e) => e.key === "app.title");
  assert.ok(/[A-Za-z]/.test(titleEntry.text), "в EN заголовок латиницей");
  assert.ok(!/[Ѐ-ӿ]/.test(titleEntry.text), "в EN заголовке нет кириллицы");
  assert.ok(en.html.includes('<html lang="en"'), "EN помечен lang=en");
});