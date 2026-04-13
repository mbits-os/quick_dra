import js from '@eslint/js';
import { defineConfig } from 'eslint/config';
import jest from 'eslint-plugin-jest';
import globals from 'globals';
import ts_eslint from 'typescript-eslint';
import { configs as litConfigs } from 'eslint-plugin-lit';
import prettierConfigRecommended from 'eslint-config-prettier/flat';

type MetaEx = ImportMeta & {
  dirname: string;
};

export default defineConfig([
  {
    ignores: ['**/node_modules', '**/dist', '**/coverage', '**/.yarn'],
  },
  js.configs.recommended,
  ...ts_eslint.configs.recommended,
  prettierConfigRecommended,
  {
    files: ['**/*.ts'],
    plugins: { js },
    extends: ['js/recommended'],
    languageOptions: { globals: globals.browser },
  },
  {
    files: ['**/web-dev-server.config.js'],
    rules: { 'no-undef': 'off' },
  },
  {
    files: ['**/*.spec.ts', '**/*.test.ts', '**/*.spec.tsx', '**/*.test.tsx'],
    ...jest.configs['flat/recommended'],
  },
  {
    files: ['**/*.spec.ts', '**/*.test.ts', '**/*.spec.tsx', '**/*.test.tsx'],
    rules: {
      'jest/expect-expect': [
        'warn',
        { assertFunctionNames: ['expect', 'expectObservable'] },
      ],
    },
  },
  {
    languageOptions: {
      globals: {
        ...globals.browser,
        ...globals.node,
        ...globals.jest,
      },

      parser: ts_eslint.parser,
      parserOptions: {
        projectService: {
          allowDefaultProject: ['../*.js', '*.js'],
        },
        tsconfigRootDir: (import.meta as MetaEx).dirname,
        // or, in CommonJS, __dirname
      },
    },

    rules: {
      '@typescript-eslint/no-unused-vars': [
        'error',
        { argsIgnorePattern: '^_' },
      ],
      'no-unused-vars': ['error', { argsIgnorePattern: '^_' }],
    },
  },

  {
    files: ['**/*.cjs'],
    rules: { '@typescript-eslint/no-require-imports': 'off' },
  },
  litConfigs['flat/recommended'],
]);
