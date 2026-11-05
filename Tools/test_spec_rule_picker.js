const assert = require("assert");
const fs = require("fs");
const vm = require("vm");

const files = process.argv.slice(2);
if (!files.length) files.push("Sources/AddOnResources/RFIX/HTML/Interface_ru.html");
for (const file of files) {
  const html = fs.readFileSync(file, "utf8");
  const names = ["matchesSpecQuery", "selectSpecProperty", "renderSpecPropertyDropdown"];
  const functions = names.map(function (name) {
    const match = html.match(new RegExp("  function " + name + "\\([^]*?\\n  \\}"));
    assert(match, name + " not found in " + file);
    return match[0];
  });
  const properties = [
    { id: "wall", name: "Спецификация/Стены" },
    { id: "slab", name: "Спецификация/Перекрытия" },
    { id: "window", name: "Спецификация/Окна" },
    { id: "door", name: "Спецификация/Двери" }
  ];
  const context = {
    cfg: { query: "", ruleProperties: properties },
    searchInput: { value: "" },
    dropdown: { style: {}, children: [], appendChild: function (child) { this.children.push(child); } },
    clear: function (node) { node.children = []; },
    h: function (tag, options) { return options; },
    t: function (key) { return key; },
    rebuildSpecBlocks: function () {}
  };
  vm.createContext(context);
  vm.runInContext(functions.join("\n"), context);
  function displayed() {
    context.renderSpecPropertyDropdown();
    return context.dropdown.children.map(function (child) { return child.text; });
  }
  assert.deepStrictEqual(displayed(), properties.map(p => p.name));
  context.selectSpecProperty(properties[0]);
  assert.strictEqual(context.searchInput.value, properties[0].name);
  assert.deepStrictEqual(displayed(), properties.map(p => p.name));
  context.cfg.query = "ОКН";
  assert.deepStrictEqual(displayed(), [properties[2].name]);
  context.cfg.query = "нет такого правила";
  assert.deepStrictEqual(displayed(), ["common.empty.found"]);
  context.cfg.query = "";
  assert.deepStrictEqual(displayed(), properties.map(p => p.name));
  context.selectSpecProperty(properties[3]);
  assert.strictEqual(context.cfg.selectedId, "door");
  assert.deepStrictEqual(displayed(), properties.map(p => p.name));
  console.log("PASS " + file + ": initial list, auto-selection, search, no match, cleared search, reselection");
}
