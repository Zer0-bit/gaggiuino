// eslint-disable-next-line import/no-extraneous-dependencies
const { createProxyMiddleware } = require('http-proxy-middleware');

const target = 'http://192.168.0.14';

module.exports = function (app) {
  app.use(createProxyMiddleware('/api', { target, secure: false }));
  // The following doesn't seem to work correctly and couldn't be bothered
  // to find how to make this work yet. For now we also need to update the
  // websocket path in Home.jsx
  // app.use(createProxyMiddleware('/ws', { target, ws: true, logLevel: 'info' }));
};
