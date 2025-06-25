import React from "react";
import { createI18nInstance } from "~/utils/i18n";
import { renderToPipeableStream } from "react-dom/server";
import { StaticRouter } from "react-router-dom/server";
import { Router } from "./router";
import Layout from "./layout";
import NotFound from "./routes/not-found/not-found";

type i18n = {
  translations: Record<string, string>;
  locale: string;
  pageName: string;
};

type RenderProps = {
  url: string;
  translations: i18n["translations"];
  locale: string;
  pageName: string;
  clientJs: string;
  clientCss: string;
  user?: any;
};

export async function render({
  url,
  locale,
  pageName,
  clientJs,
  clientCss,
  user,
}: RenderProps) {
  if (!clientJs || !clientCss) {
    throw new Error("Missing required clientJs or clientCss path");
  }

  const i18n = createI18nInstance();
  await i18n.init({
    lng: locale,
    fallbackLng: "en",
    resources: {},
    interpolation: { escapeValue: false },
  });
  const translations = i18n.getResourceBundle(locale, pageName) || {};

  // SSR 404 detection: if the route is not matched, render NotFound and set status 404
  let didMatch = false;
  const App = (
    <React.StrictMode>
      <StaticRouter location={url}>
        <Layout user={user}>
          <Router />
        </Layout>
      </StaticRouter>
    </React.StrictMode>
  );

  // Patch useRoutes to detect if a route matched
  // (react-router-dom v6+ sets a context property for this)
  // We'll use a workaround: if pageName is not a known route, treat as 404
  const knownRoutes = ["home", "login", ""]; // add more if needed
  if (!knownRoutes.includes(pageName)) {
    didMatch = false;
  } else {
    didMatch = true;
  }

  return new Promise((resolve) => {
    const stream = renderToPipeableStream(
      didMatch ? App : <NotFound />, // Render NotFound if no match
      {
        bootstrapModules: [clientJs],
        onShellReady() {
          resolve({
            statusCode: didMatch ? 200 : 404,
            headers: { "Content-Type": "text/html" },
            prelude: `<!DOCTYPE html>
              <html lang="en">
                <head>
                  <meta charset="UTF-8" />
                  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
                  <link rel="stylesheet" href="${clientCss}" />
                  <title>Guided Reader</title>
                </head>
                <script type="module">
                  import RefreshRuntime from 'http://${process.env.SERVER_BASE || "localhost"}:${process.env.SERVER_PORT || "5173"}/@react-refresh'
                  RefreshRuntime.injectIntoGlobalHook(window)
                  window.$RefreshReg$ = () => {}
                  window.$RefreshSig$ = () => (type) => type
                  window.__vite_plugin_react_preamble_installed__ = true
                </script>
                <script>
                  // Inject the translations and user into the client-side window object
                  window.__translations__ = ${JSON.stringify(translations)};
                  window.__locale__ = '${locale}';
                  window.__user__ = ${JSON.stringify(user)};
                </script>
                <body>
                  <div id="app">`,
            postlude: `</div>
                  <script type="module" src="${clientJs}"></script>
                </body>
              </html>`,
            stream,
          });
        },
      }
    );
  });
}
