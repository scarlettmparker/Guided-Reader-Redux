import { defineConfig } from "vite";
import solidPlugin from "vite-plugin-solid";

import path from "path";

export default defineConfig({
  plugins: [solidPlugin()],
  server: {
    port: 3000,
    proxy: {
      "/api": {
        target: "https://readerapi.scarlettparker.co.uk",
        changeOrigin: true,
        rewrite: (path) => path.replace(/^\/api/, ""),
      },
    },
  },
  build: {
    target: "esnext",
  },
  resolve: {
    alias: {
      "~": path.resolve(__dirname, "src"),
    },
  },
});
