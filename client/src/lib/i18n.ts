import i18n from 'i18next'
import { initReactI18next } from 'react-i18next'

export function createI18nInstance() {
  const instance = i18n.createInstance()
  instance.use(initReactI18next)
  return instance
}

i18n.use(initReactI18next).init({
  lng: 'en',
  fallbackLng: 'en',
  resources: {},
  interpolation: { escapeValue: false },
})

export default i18n
