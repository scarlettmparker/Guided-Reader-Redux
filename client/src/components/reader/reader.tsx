import {
  Component,
  createEffect,
  createSignal,
  For,
  createResource,
  onMount,
} from "solid-js";

import {
  Text as TextType,
  Annotation as AnnotationType,
  AnnotationResponse,
} from "~/types";

import { TextContext } from "~/contexts/text-context";
import { AnnotationController, TitlesController } from "~/utils/api";
import { handleAnnotationClick } from "~/utils/annotation";
import { shouldFetchText, getFromCache, cacheText } from "~/utils/text";

import { LoadingState, ErrorState } from "~/components/state";
import ReaderModal from "~/components/reader-modal";
import TextList from "~/components/text-list";
import TextModal from "~/components/text-modal";
import TextListItem from "~/components/text-list-item";
import Annotation from "~/components/annotation";
import textListItemStyles from "~/components/text-list-item/text-list-item.module.css";
import AnnotationList from "~/components/annotation-list";

import styles from "./reader.module.css";

const Reader: Component = () => {
  const [selectedTextId, setSelectedTextId] = createSignal<number | null>(null);
  const [hoveredTextId, setHoveredTextId] = createSignal<number | null>(null);
  const [selectedTextData, setSelectedTextData] = createSignal<
    TextType | undefined
  >();

  const [annotations, setAnnotations] = createSignal<AnnotationResponse[]>([]);
  const [selectedAnnotation, setSelectedAnnotation] =
    createSignal<AnnotationType | null>(null);
  const [titles] = createResource(() => TitlesController.getTitles());

  // createResource is used only to trigger the fetch when hoveredTextId changes
  // and for other side effects that are useful (I don't want to explain just trust me)

  const [] = createResource(hoveredTextId, async (id) => {
    // Don't fetch hover text if it's the same as selected text
    if (id === selectedTextId()) {
      return undefined;
    }

    const cached = getFromCache(id);
    if (cached) {
      return { message: [cached] };
    }

    if (shouldFetchText(id)) {
      const text = await cacheText(id);
      return text ? { message: [text] } : undefined;
    }

    return undefined;
  });

  // Handle selection changes and fetch text if needed
  createEffect(() => {
    const id = selectedTextId();
    setSelectedAnnotation(null);

    if (id === null) {
      return setSelectedTextData(undefined);
    }

    const cached = getFromCache(id);
    if (cached) {
      setSelectedTextData(cached);
    } else if (shouldFetchText(id)) {
      cacheText(id).then((text) => {
        if (text && id === selectedTextId()) {
          setSelectedTextData(text);
        }
      });
    }
  });

  // Handle loading annotations upon click
  createEffect(() => {
    const annotation = selectedAnnotation();
    if (annotation) {
      AnnotationController.getAnnotations(
        annotation.text_id,
        annotation.start,
        annotation.end,
      ).then((data) => {
        setAnnotations(data?.message || []);
      });
    } else {
      setAnnotations([]);
    }
  });

  onMount(() => {
    const handleClick = (e: MouseEvent) => {
      handleAnnotationClick(
        e,
        setSelectedAnnotation,
        selectedTextData()?.annotations || [],
      );
    };
    document.addEventListener("click", handleClick);
    return () => {
      document.removeEventListener("click", handleClick);
    };
  });

  return (
    <TextContext.Provider value={{ setSelectedTextId }}>
      <div class={styles.reader}>
        <TextList>
          {titles.loading ? (
            <LoadingState>Loading...</LoadingState>
          ) : titles.error ? (
            <ErrorState>Error: {titles.error.message}</ErrorState>
          ) : (
            <For each={titles()?.message}>
              {(item) => (
                <TextListItem
                  class={() =>
                    item.id === selectedTextId()
                      ? textListItemStyles.selected
                      : ""
                  }
                  onClick={() => setSelectedTextId(item.id)}
                  onMouseOver={() => setHoveredTextId(item.id)}
                >
                  {item.title}
                </TextListItem>
              )}
            </For>
          )}
        </TextList>
        <ReaderModal>
          <TextModal
            selectedTextId={selectedTextId()}
            text={selectedTextData()}
          />
        </ReaderModal>
      </div>
      {selectedAnnotation() !== null && annotations().length > 0 && (
        <AnnotationList class={styles.annotation_list}>
          <For each={annotations()}>
            {(annotation) => <Annotation annotation={annotation} />}
          </For>
        </AnnotationList>
      )}
    </TextContext.Provider>
  );
};

export default Reader;
