module.exports = {
  env: {
    browser: true,
    es2021: true,
  },
  extends: [
    'plugin:react/recommended',
    'airbnb',
  ],
  parserOptions: {
    ecmaFeatures: {
      jsx: true,
    },
    ecmaVersion: 'latest',
    sourceType: 'module',
  },
  plugins: [
    'react',
  ],
  rules: {
    "react/jsx-props-no-spreading": "off",
    "react/self-closing-comp": "off",
    "react/destructuring-assignment": "off",
    "react/jsx-no-bind": "off",
    "react/prop-types": "off",
    "react/jsx-fragments": "off",
    'no-plusplus': 'off',
    "import/no-extraneous-dependencies": [
      "error",
      {
        "devDependencies": [
          "**/*.stories.*",
          "**/.storybook/**/*.*"
        ],
        "peerDependencies": true
      }
    ]
  },
  ignorePatterns: ['build/*.js', 'scripts/*.js', 'config/*.js'],

};
