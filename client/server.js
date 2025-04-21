import express from 'express'
import fs from 'fs/promises'
import path from 'path'
import { Writable } from 'stream'
import { config } from 'dotenv'

const isProduction = process.env.NODE_ENV === 'production'
config({
  path: `.env${isProduction ? '.production' : ''}`,
})

const port = process.env.SERVER_PORT || 5173
const base = process.env.SERVER_BASE || '/'

const app = express()
app.use(express.static(path.resolve('./public')))

async function preloadTranslations(locale, page) {
  const basePath = path.resolve(`./locales/${page}`)
  const localeFile = path.join(basePath, `${locale}.json`)
  const fallbackFile = path.join(basePath, 'en.json')

  try {
    const file = await fs.readFile(localeFile, 'utf-8')
    return JSON.parse(file)
  } catch {
    console.warn(
      `Missing locale for ${locale} at ${localeFile}, falling back to en.json`,
    )
    const fallback = await fs.readFile(fallbackFile, 'utf-8')
    return JSON.parse(fallback)
  }
}

// Load manifest for production
async function loadManifest() {
  const manifestPath = path.resolve('./dist/client/.vite/manifest.json');
  const manifest = JSON.parse(await fs.readFile(manifestPath, 'utf-8'));
  return manifest;
}

// Add Vite or respective production middlewares
let vite
if (!isProduction) {
  const { createServer } = await import('vite')
  vite = await createServer({
    server: { middlewareMode: true },
    appType: 'custom',
    base,
  })
  app.use(vite.middlewares)
} else {
  const compression = (await import('compression')).default
  const sirv = (await import('sirv')).default
  app.use(compression())
  app.use(base, sirv('./dist/client', { extensions: ['html', 'js', 'css'] }))
}

app.get('*', async (req, res, next) => {
  if (/\.[^\/]+$/.test(req.path)) {
    return next()
  }

  try {
    let manifest;
    
    if (isProduction) {
      manifest = await loadManifest()
    }

    let url = req.originalUrl.replace(base, '')
    if (!url.startsWith('/')) url = '/' + url

    // Localization
    const langHeader = req.headers['accept-language'] || 'en'
    const locale = langHeader.split(',')[0] || 'en'
    const pageName = url.split('/')[1] || 'home'
    const translations = await preloadTranslations(locale, pageName)

    let render
    if (!isProduction) {
      render = (await vite.ssrLoadModule('/src/entry-server.tsx')).render
    } else {
      render = (await import('./dist/server/entry-server.js')).render
    }

    // Hashed JS/CSS for production
    let clientJs, clientCss
    
    if (isProduction) {
      clientJs = manifest['src/entry-client.tsx'].file
      clientCss = manifest['src/entry-client.tsx'].css[0]
    } else {
      clientJs = '/src/entry-client.tsx'
      clientCss = '/src/styles/globals.css'
    }

    const rendered = await render({
      url,
      translations,
      locale,
      pageName,
      clientJs,
      clientCss
    })

    res.statusCode = rendered.statusCode
    for (const [key, value] of Object.entries(rendered.headers)) {
      res.setHeader(key, value)
    }

    res.write(rendered.prelude)

    const writable = new Writable({
      write(chunk, _encoding, callback) {
        res.write(chunk)
        callback()
      },
      final(callback) {
        res.write(rendered.postlude)
        res.end()
        callback()
      },
    })

    rendered.stream.pipe(writable)
  } catch (e) {
    vite?.ssrFixStacktrace(e)
    console.error(e.stack)
    res.status(500).end(e.stack)
  }
})

// Start http server
app.listen(port, () => {
  console.log(`Server started at http://localhost:${port}`)
})
