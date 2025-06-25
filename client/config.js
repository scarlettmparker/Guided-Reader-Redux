/**
 * @fileoverview Configuration settings for the server, loaded from environment variables.
 * @module config
 */

import { config } from "dotenv";

/**
 * Checks if the current environment is production.
 * @type {boolean}
 */
export const isProduction = process.env.NODE_ENV === "production";

// Load environment variables based on the environment.
config({
  path: `.env${isProduction ? ".production" : ""}`,
});

/**
 * The port on which the server will listen. Defaults to 5173.
 * @type {number}
 */
export const port = parseInt(process.env.SERVER_PORT || "5173", 10);

/**
 * The base URL path for the server. Defaults to "/".
 * @type {string}
 */
export const base = process.env.SERVER_BASE || "/";

/**
 * The host of the backend API. Defaults to "0.0.0.0".
 * @type {string}
 */
export const backendHost = process.env.BACKEND_HOST || "0.0.0.0";

/**
 * The port of the backend API. Defaults to 443.
 * @type {number}
 */
export const backendPort = parseInt(process.env.BACKEND_PORT || "443", 10);

/**
 * The path to the client manifest file for production builds.
 * @type {string}
 */
export const manifestPath = "./dist/client/.vite/manifest.json";
