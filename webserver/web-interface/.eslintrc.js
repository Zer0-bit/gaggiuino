const extensions = ['.js', '.jsx', '.ts', '.tsx'];

module.exports = {
  env: {
    browser: true,
    es2021: true,
  },
  extends: [
    'plugin:react/recommended',
    'airbnb',
    'react-app',
    'react-app/jest',
    'plugin:@typescript-eslint/recommended',
  ],
  parserOptions: {
    ecmaFeatures: {
      jsx: true,
    },
    ecmaVersion: 'latest',
    sourceType: 'module',
  },
  plugins: [
    '@typescript-eslint',
    'react',
  ],
  settings: {
    'import/resolver': {
      node: {
        extensions,
      },
    },
  },
  rules: {
    'max-len': [
      'error',
      {
        code: 120,
        tabWidth: 2,
        comments: 120,
        ignoreComments: true,
        ignoreTrailingComments: true,
        ignoreUrls: true,
        ignoreStrings: true,
        ignoreTemplateLiterals: true,
        ignoreRegExpLiterals: true,
      },
    ],
    'react/jsx-props-no-spreading': 'off',
    'react/self-closing-comp': 'off',
    'react/destructuring-assignment': 'off',
    'react/jsx-no-bind': 'off',
    'react/prop-types': 'off',
    'react/jsx-fragments': 'off',
    'no-plusplus': 'off',
    'import/no-extraneous-dependencies': [
      'error',
      {
        devDependencies: [
          '!./src/setupTests.ts',
        ],
        peerDependencies: true,
      },
    ],
    // allow JSX with any file extension
    'import/extensions': [2, { extensions }],
    'react/jsx-filename-extension': [2, { extensions }],
  },
  ignorePatterns: ['build/*.js', 'scripts/*.js', 'config/*.js'],
};
