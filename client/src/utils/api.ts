import { TextListItem as TextListItemType } from "~/types";

const MAX_RETRIES = 3;
const RETRY_DELAY = 1000;

interface FetchOptions extends RequestInit {
  retries?: number;
}

async function sleep(ms: number) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

/**
 * Helper function to fetch data from the server with retries
 *
 * @param url URL to fetch data from
 * @param options Fetch options
 * @return Fetched data
 */
export async function getFetch<T>(
  url: string,
  options: FetchOptions = {},
): Promise<T> {
  const { retries = MAX_RETRIES, ...fetchOptions } = options;
  let lastError: Error | null = null;

  // We want to retry the fetch request a number of times in case of failure
  // Useful for network errors or server errors that might be temporary
  for (let i = 0; i < retries; i++) {
    try {
      const response = await fetch(url, {
        ...fetchOptions,
        // Add default method if not provided
        headers: {
          "Content-Type": "application/json",
          ...fetchOptions.headers,
        },
      });

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      lastError = error as Error;
      if (i < retries - 1) {
        // Wait before retrying with exponential backoff
        await sleep(RETRY_DELAY * Math.pow(2, i));
        continue;
      }
    }
  }

  throw lastError || new Error("Failed to fetch");
}

// API controllers, this is where we define the API endpoints and their methods
export const TitlesController = {
  getTitles: (sort = 0, page = 0, pageSize = 336) =>
    getFetch<{ message: TextListItemType[] }>(
      `/api/titles?sort=${sort}&page=${page}&page_size=${pageSize}`,
    ),
};

export const TextController = {
  getText: (
    textObjectId: number,
    language: string = "GR",
    type: string = "all",
  ) =>
    getFetch<{ message: string }>(
      `/api/text?text_object_id=${textObjectId}&language=${language}&type=${type}`,
    ),
};
