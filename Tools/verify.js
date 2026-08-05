// verify.js — кастомная проверка HTML по ТЗ проекта SomeStuff
// Запуск: node verify.js <путь-к-html>

const fs = require("fs");
const path = require("path");

const file = process.argv[2];
if (!file) { console.error("Usage: node verify.js <file>"); process.exit(1); }

if (!fs.existsSync(file)) {
    console.error(`File not found: ${file}`);
    process.exit(1);
}

const src = fs.readFileSync(file, "utf8");
const errors = [];
const warn = [];

function E(msg) { errors.push(msg); }
function W(msg) { warn.push(msg); }

console.log(`\n=== HTML Validation: ${path.basename(file)} ===`);

// ════════════════════════════════════════════════════════════════════════
// ЗАПРЕЩЕНО
// ════════════════════════════════════════════════════════════════════════

// 1. Нет тега <style> (не в комментариях, не в строках)
function hasStyleTag(html) {
  // Убираем комментарии <!-- ... -->
  const noComments = html.replace(/<!--[\s\S]*?-->/g, '');
  // Убираем строковые литералы "..." и '...' (упрощённо)
  const noStrings = noComments.replace(/"(?:\\.|[^"\\])*"/g, '').replace(/'(?:\\.|[^'\\])*'/g, '');
  return /<style[\s>]/i.test(noStrings);
}
if (hasStyleTag(src))
  E("Найден тег <style>. По ТЗ разрешены только inline-стили (style=\"...\").");

// 2. Нет внешних CSS-файлов
if (/<link[^>]+rel=["']stylesheet["']/i.test(src))
  E("Найден <link rel=stylesheet>. Внешние CSS запрещены.");

// 3. Нет фреймворков (React, Vue, Angular, Alpine, Svelte, jQuery)
const frameworks = ["react", "vue", "angular", "alpine", "svelte", "jquery"];
frameworks.forEach(function(fw) {
  if (new RegExp('<script[^>]+' + fw, 'i').test(src))
    E("Найден импорт фреймворка: " + fw + ". По ТЗ только Vanilla JS.");
});

// 4. Нет CDN ссылок на фреймворки в script src
const cdnPatterns = [
  /cdn\.jsdelivr\.net/i,
  /unpkg\.com/i,
  /cdnjs\.cloudflare\.com/i,
  /ajax\.googleapis\.com/i
];
cdnPatterns.forEach(function(p) {
  if (p.test(src)) E("Найдена CDN ссылка на внешнюю библиотеку: " + p.source);
});

// ════════════════════════════════════════════════════════════════════════
// ОБЯЗАТЕЛЬНО
// ════════════════════════════════════════════════════════════════════════

// 5. charset UTF-8
if (!/<meta[^>]+charset=["']?UTF-8["']?/i.test(src))
  E("Отсутствует <meta charset=\"UTF-8\">.");

// 6. viewport meta
if (!/<meta[^>]+name=["']viewport["']/i.test(src))
  E("Отсутствует <meta name=\"viewport\">.");

// 7. lang на <html>
if (!/<html[^>]+lang=/i.test(src))
  E("Отсутствует атрибут lang на теге <html>.");

// 8. Оба объекта темы (light/dark) объявлены в THEMES
// Ищем const THEMES = { ... } до закрывающей скобки на уровне THEMES
const themesMatch = src.match(/const\s+THEMES\s*=\s*(\{[\s\S]*?\n\})/);
if (!themesMatch) {
  E("Объект THEMES не найден.");
} else {
  const themesBlock = themesMatch[1];
  if (!/^\s*light\s*:/m.test(themesBlock))
    E("В THEMES не найден ключ 'light'.");
  if (!/^\s*dark\s*:/m.test(themesBlock))
    E("В THEMES не найден ключ 'dark'.");

  // 9. Симметрия переменных темы: все --ac-* переменные должны быть в обоих объектах
  const lightVars = themesBlock.match(/"--ac-[\w-]+":/g) || [];
  if (lightVars.length === 0) {
    W("Не найдено ни одной --ac-переменной в THEMES.");
  } else if (lightVars.length % 2 !== 0) {
    W("Нечётное число объявлений --ac-переменных: возможно, light и dark не симметричны.");
  }
}

// 10. applyTheme вызывается при старте (в DOMContentLoaded)
if (!/applyTheme\s*\(/.test(src))
  E("Функция applyTheme не вызывается — переключение темы не работает.");

// 11. DOMContentLoaded — точка запуска
if (!/DOMContentLoaded/.test(src))
  E("Нет DOMContentLoaded — точки запуска приложения.");

// 12. min-width на body или корневом контейнере (>= 250px по ТЗ)
if (!/min-width\s*:\s*2[0-9]{2}/.test(src) && !/min-width\s*:\s*[3-9][0-9]{2}/.test(src))
  W("Не найден min-width >= 250px. По ТЗ минимальная ширина 250px должна быть защищена.");

// 13. Нет горизонтального scroll на body
if (/overflow-x\s*:\s*auto/.test(src) || /overflow-x\s*:\s*scroll/.test(src))
  W("overflow-x:auto/scroll на body может дать горизонтальный скролл.");

// 14. JS-синтаксис: быстрая проверка через new Function с точным номером строки
const scriptMatch = src.match(/<script>([\s\S]*?)<\/script>/i);
if (scriptMatch) {
  try {
    new Function(scriptMatch[1]);
    console.log("✓  JS-синтаксис: ОК");
  } catch(e) {
    // Определяем номер строки внутри <script>
    const scriptStartLine = src.slice(0, src.indexOf("<script>")).split("\n").length;
    const lineMatch = e.message.match(/line (\d+)/i);
    const jsLine = lineMatch ? parseInt(lineMatch[1]) : "?";
    const htmlLine = jsLine !== "?" ? scriptStartLine + jsLine : "?";
    E(`Ошибка синтаксиса JS (строка ~${htmlLine} в HTML): ${e.message}`);
  }
} else {
  W("Тег <script> не найден — JS не проверен.");
}

// 15. Проверка наличия ACBridge (мок/мост)
if (!/ACBridge/.test(src))
  W("Объект ACBridge не найден — проверьте наличие моста с Archicad.");

// ════════════════════════════════════════════════════════════════════════
// ИТОГ
// ════════════════════════════════════════════════════════════════════════

if (warn.length) {
  console.log("\n⚠  ПРЕДУПРЕЖДЕНИЯ (" + warn.length + "):");
  warn.forEach(w => console.log("   - " + w));
}
if (errors.length) {
  console.log("\n✖  ОШИБКИ (" + errors.length + "):");
  errors.forEach(e => console.log("   - " + e));
}

console.log("");

if (errors.length) {
  console.error("❌  Верификация провалена: " + errors.length + " ошибок.");
  process.exit(1);
} else {
  console.log("✅  Верификация прошла" + (warn.length ? " с предупреждениями." : " без замечаний."));
  process.exit(0);
}