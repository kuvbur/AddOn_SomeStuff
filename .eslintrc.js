// .eslintrc.js — конфиг ESLint для проверки JS внутри HTML
//
// plugins: ['html'] обязателен: без него ESLint читает .html как исходник JS и
// падает на первом же `<` — npm run validate:js не проходил никогда. Плагин
// установлен как devDependency (eslint-plugin-html).
//
// env.node нужен CommonJS-файлам репозитория (Tools/*.js): конфиг нацелен на
// browser-код палитры, и без node-окружения require/process/module считаются
// неопределёнными.
//
// no-new-func отключён для инструментов валидации: new Function там —
// единственный способ скомпилировать JS из HTML и выполнить его в песочнице.
// В самой палитре правило остаётся включённым.
module.exports = {
  env: { browser: true, es2020: true, node: true },
  parserOptions: { ecmaVersion: 2020, sourceType: 'script' },
  plugins: ['html'],
  overrides: [
    {
      files: ['Tools/**/*.js'],
      rules: { 'no-new-func': 'off' }
    }
  ],
  rules: {
    "no-undef": "error",
    "no-unused-vars": "warn",
    "no-eval": "error",
    "eqeqeq": "warn",
    "no-implied-eval": "error",
    "no-new-func": "error",
    "no-script-url": "error"
  }
};
