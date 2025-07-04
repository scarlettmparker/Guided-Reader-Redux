import { Annotation, Text as TextType } from "~/types";
import styles from "~/components/routes/index/text-modal/text-modal.module.css";

/**
 * Get the current window location (for SSR)
 */
function getLocation() {
  if (typeof window !== "undefined") {
    return window.location;
  }
  return { pathname: "", search: "" };
}

/**
 * Update the browser URL without reloading the page.
 *
 * @param url - New URL to set in the browser.
 */
function updateUrl(url: string) {
  if (typeof window !== "undefined") {
    window.history.pushState({}, "", url);
  }
}

/**
 * Get the annotation id from the query params
 */
export function getAnnotationIdFromQuery(): number | null {
  const location = getLocation();
  const params = new URLSearchParams(location.search);
  const annotationId = params.get("annotation_id");
  return annotationId ? Number(annotationId) : null;
}

/**
 * Get the text id from the query params
 */
export function getTextIdFromQuery(): number | null {
  const location = getLocation();
  const params = new URLSearchParams(location.search);
  const textParam = params.get("text");
  return textParam ? Number(textParam) : null;
}

/**
 * Update the query parameters in the URL based on the selected text and annotation.
 *
 *  @param selectedTextId - ID of the selected text, or null to clear it.
 *  @param selectedAnnotation - Selected annotation object, or null to clear it.
 */
export function updateQueryParams(
  selectedTextId: number | null,
  selectedAnnotation: Annotation | null,
) {
  const location = getLocation();
  const params = new URLSearchParams(location.search);
  if (selectedTextId !== null) {
    params.set("text", String(selectedTextId));
  } else {
    params.delete("text");
  }
  if (selectedAnnotation) {
    params.set("annotation_id", String(selectedAnnotation.id));
  }
  const newUrl = `${location.pathname}?${params.toString()}`;
  updateUrl(newUrl);
}

/**
 * Calculate the time ago from a given timestamp to human readable format.
 *
 * @param timestamp Timestamp to calculate time ago from.
 * @returns String representing the time ago.
 */
export function unixToDate(timestamp: number): string {
  const seconds = Math.floor(Date.now() / 1000 - timestamp);

  if (seconds >= 31536000) {
    const years = Math.floor(seconds / 31536000);
    return `${years} year${years > 1 ? "s" : ""} ago`;
  } else if (seconds >= 2592000) {
    const months = Math.floor(seconds / 2592000);
    return `${months} month${months > 1 ? "s" : ""} ago`;
  } else if (seconds >= 86400) {
    const days = Math.floor(seconds / 86400);
    return `${days} day${days > 1 ? "s" : ""} ago`;
  } else if (seconds >= 3600) {
    const hours = Math.floor(seconds / 3600);
    return `${hours} hour${hours > 1 ? "s" : ""} ago`;
  } else if (seconds >= 60) {
    const minutes = Math.floor(seconds / 60);
    return `${minutes} minute${minutes > 1 ? "s" : ""} ago`;
  }

  return `${seconds} second${seconds > 1 ? "s" : ""} ago` || "just now";
}

/**
 * Helper function for rendering HTML content.
 * Sanitizes text by removing HTML tags and replacing non-breaking spaces with regular spaces.
 *
 * @param raw_text The raw text to sanitize.
 * @param trim Whether to trim the text after sanitizing.
 * @return The sanitized text.
 */
function sanitizeText(rawText: string, trim: boolean = true): string {
  let sanitizedText = rawText
    .replace(/\u00A0/g, " ") // Replace non-breaking spaces with regular spaces
    .replace(/\s+/g, " ") // Replace all whitespace characters with a single space
    .trim();

  return trim ? sanitizedText.trim() : sanitizedText;
}
/**
 * Processes a text node and applies annotations to create HTML spans with appropriate styling.
 * Splits the text into annotated and unannotated segments, wrapping each in span elements
 * with unique IDs and classes for styling.
 *
 * @param last_index - The current position in the overall text being processed
 * @param parts - Array to collect HTML string fragments during processing
 * @param annotations - Array of Annotation objects defining ranges and styles to apply
 * @param text_node - The text content to process and annotate
 * @param start_offset - Starting character offset of this text node in the overall text
 * @return Ending position (lastIndex + length of processed text)
 *
 * @example
 * const parts = [];
 * const annotations = [{start: 5, end: 10}];
 * processTextNode(0, parts, annotations, "Hello world", 0);
 * // ... parts will contain: ["<span id="plain-text-0">Hello</span>", ...
 * // ...                    "<span id="annotated-text-0">, worl</span>" ...
 * // ...                    "<span id="plain-text-1">d</span>"] ...
 */
function processTextNode(
  lastIndex: number,
  parts: string[],
  annotations: Annotation[],
  textNode: string,
  startOffset: number,
) {
  let currentOffset = startOffset;
  let lastAnnotatedIndex = 0;
  let plainTextCounter = 0;

  annotations.forEach((annotation) => {
    // Check if the annotation overlaps with the current text node
    if (
      currentOffset < annotation.end &&
      currentOffset + textNode.length > annotation.start
    ) {
      const overlapStart = Math.max(annotation.start, currentOffset);
      const overlapEnd = Math.min(
        annotation.end,
        currentOffset + textNode.length,
      );
      const unannotatedText = textNode.slice(
        lastAnnotatedIndex,
        overlapStart - currentOffset,
      );

      if (unannotatedText) {
        parts.push(
          `<span id="plain-text-${plainTextCounter++}">${unannotatedText}</span>`,
        );
      }

      const annotatedText = textNode.slice(
        overlapStart - currentOffset,
        overlapEnd - currentOffset,
      );
      if (annotatedText) {
        parts.push(
          `<span id="annotated-text-${annotation.id}" class="${styles.annotated_text}">${annotatedText}</span>`,
        );
      }

      lastAnnotatedIndex = overlapEnd - currentOffset;
    }
  });

  // Add the remaining text as plain text
  if (lastAnnotatedIndex < textNode.length) {
    const remainingText = textNode.slice(lastAnnotatedIndex);
    if (remainingText) {
      parts.push(
        `<span id="plain-text-${plainTextCounter++}">${remainingText}</span>`,
      );
    }
  }
  return lastIndex + textNode.length;
}
/**
 * Helper function to get the attributes of an HTML element as a string.
 *
 * @param element HTML element to get the attributes of.
 * @return Attributes of the HTML element as a string.
 */
function getAttributes(element: HTMLElement): string {
  return Array.from(element.attributes).reduce(
    (attrs, attr) => `${attrs} ${attr.name}="${attr.value}"`,
    "",
  );
}

/**
 * Recursively processes DOM nodes to build an annotated HTML representation.
 * Handles both text nodes and element nodes, preserving the original HTML structure
 * while applying annotations where specified.
 *
 * @param lastIndex - Current position in the overall text being processed
 * @param parts - Array to collect HTML string fragments during processing
 * @param annotations - Array of Annotation objects defining ranges and styles to apply
 * @param node - DOM Node to process (either text node or element node)
 * @return Updated lastIndex after processing the node and its children
 */
function processNode(
  lastIndex: number,
  parts: string[],
  annotations: Annotation[],
  node: Node,
) {
  if (node.nodeType === Node.TEXT_NODE) {
    const textNode = node.textContent || "";
    return processTextNode(lastIndex, parts, annotations, textNode, lastIndex);
  } else if (node.nodeType === Node.ELEMENT_NODE) {
    const element = node as HTMLElement;
    parts.push(`<${element.tagName.toLowerCase()}${getAttributes(element)}>`);

    let newLastIndex = lastIndex;
    node.childNodes.forEach((child) => {
      newLastIndex = processNode(newLastIndex, parts, annotations, child);
    });

    parts.push(`</${element.tagName.toLowerCase()}>`);
    return newLastIndex;
  }
  return lastIndex;
}

/**
 * Renders annotated text with HTML markup.
 * This function is used to render text with annotations, where each annotation is a span element with a specific class.
 * It is also used to render highlights of the current VTT cue.
 *
 * @param text Text with annotations to render.
 * @returns Rendered text with annotations.
 */
export function renderAnnotatedText(text: TextType): string {
  const annotations = text.annotations || [];
  const parts: string[] = [];
  let lastIndex = 0;

  const textContent = sanitizeText(text.text, false);
  const tempDiv = document.createElement("div");

  tempDiv.id = "text_content";
  tempDiv.innerHTML = textContent;

  processNode(lastIndex, parts, annotations, tempDiv);
  return parts.join("").replace(/<br\s*\/?>/g, "");
}

/**
 * Handles a click event on the document. This function is used to detect clicks on annotated text.
 * @param event Click event.
 * @param set_current_annotation Function to set the current annotation.
 */
export const handleAnnotationClick = (
  event: MouseEvent,
  setSelectedAnnotation: (annotation: Annotation | null) => void,
  annotations: Annotation[],
) => {
  const target = event.target as HTMLElement;
  if (target.id.startsWith("annotated-text-")) {
    const annotationId = parseInt(target.id.split("-")[2], 10);
    const annotation = annotations.find((a) => a.id === annotationId);
    setSelectedAnnotation(annotation || null);
  }
};
