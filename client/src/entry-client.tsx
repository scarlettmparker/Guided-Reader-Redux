import { BrowserRouter } from "react-router-dom";
import { Router } from "./router";
import { initReactI18next } from "react-i18next";
import ReactDOM from "react-dom/client";
import i18n from "i18next";

import './styles/globals.css';  

declare global {
  interface Window {
    __locale__?: string;
    __translations__?: Record<string, any>;
  }
}

// Initialize i18n on the client with translations injected from the server
i18n
  .use(initReactI18next)
  .init({
    lng: window.__locale__ || "en",
    resources: {
      [window.__locale__ || "en"]: {
        login: window.__translations__ || {},
      },
    },
    interpolation: { escapeValue: false },
    react: { useSuspense: true },
  })
  .then(() => {
    // Only hydrate after i18n initialization is complete (just in case really)
    ReactDOM.hydrateRoot(
      document.getElementById("app") as HTMLElement,
      <BrowserRouter>
        <Router />
      </BrowserRouter>
    );
  })
  .catch((error) => {
    console.error("i18n initialization failed", error);
  });
