import { BrowserRouter, useLocation } from "react-router-dom";
import { Router } from "./router";
import { initReactI18next } from "react-i18next";
import ReactDOM from "react-dom/client";
import i18n from "i18next";
import { useEffect } from "react";

import Layout from "./layout";
import "./styles/globals.css";

declare global {
  interface Window {
    __locale__?: string;
    __translations__?: Record<string, any>;
  }
}

// Helper to get page name from path
function getPageName(pathname: string) {
  const page = pathname.split("/")[1];
  return page || "home";
}

// Dynamically load translations for a given page/locale
async function loadTranslations(page: string, locale: string) {
  try {
    const res = await fetch(`/locales/${page}/${locale}.json`);
    if (!res.ok) throw new Error("Not found");
    return await res.json();
  } catch {
    // fallback to en
    const res = await fetch(`/locales/${page}/en.json`);
    return await res.json();
  }
}

// Wrapper to handle translation loading on route change
function AppWithI18n() {
  const location = useLocation();
  useEffect(() => {
    const locale = window.__locale__ || "en";
    const page = getPageName(location.pathname);
    loadTranslations(page, locale).then((translations) => {
      i18n.addResourceBundle(locale, page, translations, true, true);
      i18n.changeLanguage(locale);
    });
  }, [location.pathname]);
  return (
    <Layout>
      <Router />
    </Layout>
  );
}

// Initialize i18n on the client with translations injected from the server
i18n
  .use(initReactI18next)
  .init({
    lng: window.__locale__ || "en",
    resources: {
      [window.__locale__ || "en"]: {
        home: window.__translations__ || {},
        login: window.__translations__ || {},
      },
    },
    interpolation: { escapeValue: false },
    react: { useSuspense: true },
  })
  .then(() => {
    ReactDOM.hydrateRoot(
      document.getElementById("app") as HTMLElement,
      <BrowserRouter>
        <AppWithI18n />
      </BrowserRouter>
    );
  })
  .catch((error) => {
    console.error("i18n initialization failed", error);
  });
