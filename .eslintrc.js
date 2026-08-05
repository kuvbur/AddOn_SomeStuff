// .eslintrc.js — конфиг ESLint для проверки JS внутри HTML
module.exports = {
  env: { browser: true, es2020: true },
  parserOptions: { ecmaVersion: 2020, sourceType: 'script' },
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