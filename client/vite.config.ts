import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import path from "path";

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  base: process.env.NODE_ENV === "production" ? "/pynkart/" : "/",
  resolve: {
    alias: {
      "~": path.resolve(__dirname, "./src"),
    },
  },
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
  assetsInclude: ["**/*.json"],
  json: {
    stringify: true,
  },
  build: {
    manifest: true,
    rollupOptions: {
      input: {
        client: "/src/entry-client.tsx",
      },
    },
    outDir: "dist/client",
    cssCodeSplit: true,
  },
  ssr: {
    noExternal: ["react-router-dom"],
  },
});
