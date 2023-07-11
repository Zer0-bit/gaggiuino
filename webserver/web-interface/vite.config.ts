import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import viteTsconfigPaths from 'vite-tsconfig-paths';
import svgrPlugin from 'vite-plugin-svgr';

const targetIp = '192.168.0.21';

// https://vitejs.dev/config/
export default defineConfig({
  server: {
    proxy: {
      '/api': {
        target: `http://${targetIp}`,
        changeOrigin: true,
        secure: false,
      },
      '/ws': {
        target: `ws://${targetIp}`,
        ws: true,
        changeOrigin: true,
        secure: false,
      },
    },
    host: '0.0.0.0',
    port: 3000,
  },
  plugins: [react(), viteTsconfigPaths(), svgrPlugin()],
  build: {
    outDir: '../data',
    emptyOutDir: true,
  },
});
