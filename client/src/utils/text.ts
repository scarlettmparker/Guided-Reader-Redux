type TextCache = Set<string>;

let fetchedText: TextCache = new Set();

/**
 * Check if a text has been fetched before and mark it as fetched if it hasn't
 * 
 * @param textObjectId ID of the text object to check
 * @param language language of the text object (defaults to "GR")
 * @returns true if the text needs to be fetched, false if it's already cached
 */
export function shouldFetchText(textObjectId: number | null, language: string = "GR"): boolean {
  if (!textObjectId) return false;
  
  const cacheKey = `${textObjectId}-${language}`;
  if (fetchedText.has(cacheKey)) {
    return false;
  }
  
  fetchedText.add(cacheKey);
  return true;
}