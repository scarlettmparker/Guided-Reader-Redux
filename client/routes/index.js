/**
 * @fileoverview Defines and sets up all application routes.
 * @module routes
 */

import express from "express";
import { getUser, loginUser, logoutUser } from "../utils/auth.js";
import { renderApp } from "../utils/ssr.js";
import { base, isProduction } from "../config.js";

/**
 * Sets up all routes for the Express application.
 *
 * @param {express.Application} app - The Express application instance.
 * @param {object} vite - The Vite dev server instance (optional, only in development).
 */
export function setupRoutes(app, vite) {
  /**
   * Middleware for handling login attempts.
   * If a user is already logged in, they are redirected to the home page.
   * Otherwise, the request proceeds to the next handler.
   * @param {import("express").Request} req - Express request object.
   * @param {import("express").Response} res - Express response object.
   * @param {import("express").NextFunction} next - Express next middleware function.
   */
  app.get("/login", async (req, res, next) => {
    let user = await getUser(req);
    if (user) {
      // User is already logged in, redirect to home
      return res.redirect("/");
    }
    next();
  });

  /**
   * Handles user login requests.
   * Processes the username and password from the request body, attempts to log in,
   * and sends back a JSON response indicating success or failure.
   */
  app.post("/login", express.json(), async (req, res) => {
    await loginUser(req, res);
  });

  app.post("/logout", async (req, res) => {
    await logoutUser(req, res);
  });

  // Catch-all for unmatched /api routes (prevents infinite loops)
  app.all("/api/*", (_, res) => {
    res.status(404).json({ error: "API route not found" });
  });

  /**
   * Catch-all route for server-side rendering of pages.
   * This route handles all GET requests not otherwise handled by static file serving or specific API routes.
   * It fetches user data, loads translations, and renders the React application.
   * It also includes a basic check for file extensions to bypass SSR for static assets.
   *
   * @param {import("express").Request} req - Express request object.
   * @param {import("express").Response} res - Express response object.
   * @param {import("express").NextFunction} next - Express next middleware function.
   */
  app.get("*", async (req, res, next) => {
    // Skip SSR for requests with file extensions (e.g., .js, .css, .png)
    if (/\.[^\/]+$/.test(req.path)) {
      return next();
    }

    let url = req.originalUrl.replace(base, "");
    if (!url.startsWith("/")) url = "/" + url;

    // Localization
    const langHeader = req.headers["accept-language"] || "en";
    const locale = langHeader.split(",")[0] || "en";
    const urlPath = url.split("?")[0];
    const pageName = urlPath.split("/")[1] || "home";

    // List of known routes (update as needed)
    const routes = ["", "home", "login"];
    const isRoute = routes.includes(pageName);

    let user = null;
    if (isRoute) {
      user = await getUser(req);
    }

    try {
      await renderApp(
        {
          vite,
          isProduction,
          url,
          locale,
          pageName,
          user,
        },
        res
      );
    } catch (e) {
      console.error("Error during route handling:", e);
      res.status(500).end("Internal Server Error: " + e.message);
    }
  });
}
