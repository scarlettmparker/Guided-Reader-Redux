/**
 * @fileoverview Authentication utilities for managing user sessions and roles.
 * @module utils/auth
 */

import { backendHost, backendPort } from "../config.js";

/**
 * Placeholder map for defining required roles for specific paths.
 * In a real application, this would likely be loaded from a configuration or database.
 * @type {Object.<string, string[]>}
 */
const requiredRolesMap = {};

/**
 * Fetches the user data from the backend using the session ID from cookies.
 * This function is designed for server-side use to protect routes and retrieve user roles.
 *
 * @param {import("express").Request} req - Express request object.
 * @returns {Promise<object|null>} A promise that resolves to the user object if authenticated, otherwise null.
 */
export async function getUser(req) {
  let user = null;
  const sessionId = req.cookies.sessionId;

  if (sessionId) {
    try {
      const userRes = await fetch(`http://${backendHost}:${backendPort}/user`, {
        method: "GET",
        headers: {
          Cookie: `sessionId=${sessionId}`,
        },
      });
      if (userRes.ok) {
        const userData = await userRes.json();
        user = userData.message || null;
      } else {
        console.warn(
          `Backend user fetch failed with status: ${userRes.status}`
        );
      }
    } catch (e) {
      console.error("Error fetching user data for SSR:", e);
    }
  }
  return user;
}

/**
 * Handles the user login process by forwarding credentials to the backend.
 * Sets the session cookie from the backend response.
 *
 * @param {import("express").Request} req - Express request object, expecting `username` and `password` in `req.body`.
 * @param {import("express").Response} res - Express response object.
 * @returns {Promise<void>}
 */
export async function loginUser(req, res) {
  const { username, password } = req.body;
  try {
    const backendRes = await fetch(
      `http://${backendHost}:${backendPort}/user`,
      {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password }),
      }
    );

    const data = await backendRes.json();
    if (!backendRes.ok || data.status !== "ok") {
      return res.status(401).json({ message: "Invalid credentials" });
    }

    // Get sessionId from Set-Cookie header and set it on the client
    const setCookieHeader = backendRes.headers.get("set-cookie");
    if (setCookieHeader) {
      // Parse individual cookies if multiple are present
      setCookieHeader.split(",").forEach((cookieString) => {
        const parts = cookieString.split(";");
        const [nameValue] = parts[0].split("=");
        if (nameValue === "sessionId") {
          // Look for sessionId specifically
          res.setHeader("Set-Cookie", cookieString);
        }
      });
    }

    // Fetch user data from backend with the new session (optional, but good for immediate client update)
    let user = null;
    const sessionIdMatch = setCookieHeader
      ? setCookieHeader.match(/sessionId=([^;]+)/)
      : null;
    if (sessionIdMatch) {
      req.cookies.sessionId = sessionIdMatch[1];
      user = await getUser(req);
    }

    res.json({ message: "Login successful", user });
  } catch (e) {
    console.error("Login error:", e);
    res.status(500).json({ message: "Internal server error" });
  }
}

export async function logoutUser(req, res) {
  try {
    const sessionId = req.cookies.sessionId;
    if (sessionId) {
      await fetch(`http://${backendHost}:${backendPort}/logout`, {
        method: "POST",
        headers: {
          Cookie: `sessionId=${sessionId}`,
        },
      });
    }
    res.clearCookie("sessionId");
    res.json({ message: "Logged out successfully" });
  } catch (e) {
    console.error("Logout error:", e);
    res.status(500).json({ message: "Internal server error" });
  }
}

/**
 * Retrieves the roles associated with a given authentication token.
 *
 * @param {string} authToken - User's authentication token.
 * @returns {Promise<string[]>} Promise that resolves to an array of user roles.
 */
export async function getUserRoles(authToken) {
  return [];
}

/**
 * Checks if the user has the required roles to access a specific path.
 *
 * @param {string} path - Path of the route being accessed.
 * @param {string[]} userRoles - An array of roles assigned to the user.
 * @returns {boolean} True if the user has at least one of the required roles or if no roles are required for the path, false otherwise.
 */
export function hasRequiredRoles(path, userRoles) {
  const required = requiredRolesMap[path];
  if (!required || required.length === 0) {
    return true; // No specific roles required for this path
  }
  return required.some((role) => userRoles.includes(role));
}
