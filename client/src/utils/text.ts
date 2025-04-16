import { TextType } from "~/types";

type TextCache = Map<string, { data: TextType, lastAccessed: number }>;

const MAX_CACHE_SIZE = 100;
let textCache: TextCache = new Map();

function getCacheKey(textObjectId: number, language: string = "GR"): string {
  return `${textObjectId}-${language}`;
}

/**
 * Prune the cache to keep it within the maximum size by removing the oldest entries
 */
function pruneCache() {
  if (textCache.size <= MAX_CACHE_SIZE) return;
  
  // Convert to array to sort by last accessed item
  const entries = Array.from(textCache.entries());
  entries.sort((a, b) => a[1].lastAccessed - b[1].lastAccessed);
  
  // Remove oldest entries until back at max size
  while (entries.length > MAX_CACHE_SIZE) {
    const [key] = entries.shift()!;
    textCache.delete(key);
  }
}

/**
 * Get a text from the cache
 * 
 * @param textObjectId ID of the text object to fetch
 * @param language language of the text object (defaults to "GR")
 * @return text data if found, undefined otherwise
 */
export function getFromCache(textObjectId: number, language: string = "GR"): TextType | undefined {
  const key = getCacheKey(textObjectId, language);
  const entry = textCache.get(key);
  
  if (entry) {
    // Update last accessed time
    entry.lastAccessed = Date.now();
    return entry.data;
  }
  
  return undefined;
}

/**
 * Add a text to the cache
 * 
 * @param textObjectId ID of the text object to cache
 * @param data text data to cache
 * @param language language of the text object (defaults to "GR")
 */
export function addToCache(textObjectId: number, data: TextType, language: string = "GR"): void {
  const key = getCacheKey(textObjectId, language);
  textCache.set(key, {
    data,
    lastAccessed: Date.now()
  });
  
  pruneCache();
}

/**
 * Check if a text needs to be fetched
 * 
 * @param textObjectId ID of the text object to check
 * @param language language of the text object (defaults to "GR")
 * @returns true if the text needs to be fetched, false if it's already cached
 */
export function shouldFetchText(textObjectId: number | null, language: string = "GR"): boolean {
  if (!textObjectId) return false;
  return !getFromCache(textObjectId, language);
}