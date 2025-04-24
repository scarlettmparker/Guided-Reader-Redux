import React from "react";
import { createI18nInstance } from "~/utils/i18n";
import { renderToPipeableStream } from "react-dom/server";
import { StaticRouter } from "react-router-dom/server";
import { Router } from "./router";

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
};

export async function render({
  url,
  translations,
  locale,
  pageName,
  clientJs,
  clientCss,
}: RenderProps) {
  if (!clientJs || !clientCss) {
    throw new Error("Missing required clientJs or clientCss path");
  }

  const i18n = createI18nInstance();
  await i18n.init({
    lng: locale,
    fallbackLng: "en",
    resources: {
      [locale]: {
        [pageName]: translations,
      },
    },
    interpolation: { escapeValue: false },
  });

  return new Promise((resolve) => {
    const stream = renderToPipeableStream(
      <React.StrictMode>
        <StaticRouter location={url}>
          <Router />
        </StaticRouter>
      </React.StrictMode>,
      {
        bootstrapModules: [clientJs],
        onShellReady() {
          resolve({
            statusCode: 200,
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
                  // Inject the translations into the client-side window object
                  window.__translations__ = ${JSON.stringify(translations)};
                  window.__locale__ = '${locale}';
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
      },
    );
  });
}
