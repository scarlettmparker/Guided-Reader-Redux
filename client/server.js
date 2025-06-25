/**
 * @fileoverview Main entry point for the Express server application.
 * Sets up middleware, Vite integration (for development), and routes, then starts the server.
 */

import express from "express";
import path from "path";
import cookieParser from "cookie-parser";
import { port, base, isProduction, backendHost, backendPort } from "./config.js";
import { setupRoutes } from "./routes/index.js";

const app = express();

// Serve static files from the 'public' directory
app.use(express.static(path.resolve("./public")));

// Parse cookies from the request headers
app.use(cookieParser());

let vite;

// Conditional setup for Vite development server or production static serving
if (!isProduction) {
  // In development, create and use the Vite dev server middleware
  const { createServer } = await import("vite");
  vite = await createServer({
    server: { middlewareMode: true },
    appType: "custom",
    base,
  });
  app.use(vite.middlewares);
} else {
  // In production, use compression and serve static files from the build output
  const compression = (await import("compression")).default;
  const sirv = (await import("sirv")).default;
  const { createProxyMiddleware } = await import("http-proxy-middleware");

  app.use(compression());
  app.use(base, sirv("./dist/client", { extensions: ["html", "js", "css"] }));
  app.use("/locales", express.static(path.resolve("./locales")));


  // Proxy API requests to the backend server
  app.use(
    "/api",
    createProxyMiddleware({
      target: `http://${backendHost}:${backendPort}`,
      changeOrigin: true,
      pathRewrite: { "^/api": "" },
      secure: false,
    })
  );
}

// Set up all defined application routes
setupRoutes(app, vite);

// Start the HTTP server
app.listen(port, () => {
  console.log(`Server started at http://localhost:${port}`);
});
