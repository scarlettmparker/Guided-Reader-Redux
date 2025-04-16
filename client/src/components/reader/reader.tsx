import { Component, createEffect, createSignal, For, createResource } from "solid-js";
import { TitlesController, TextController } from "~/utils/api";
import { TextContext } from "~/contexts/text-context";
import { TextType } from "~/types";
import { shouldFetchText, getFromCache, addToCache } from "~/utils/text";

import { LoadingState, ErrorState } from "~/components/state";
import ReaderModal from "~/components/reader-modal";
import TextList from "~/components/text-list";
import TextModal from "~/components/text-modal";
import TextListItem from "~/components/text-list-item";

import styles from "./reader.module.css";
import textListItemStyles from "~/components/text-list-item/text-list-item.module.css";

const Reader: Component = () => {
  const [selectedTextId, setSelectedTextId] = createSignal<number | null>(null);
  const [hoveredTextId, setHoveredTextId] = createSignal<number | null>(null);
  const [selectedTextData, setSelectedTextData] = createSignal<TextType | undefined>();

  const [titles] = createResource(() => TitlesController.getTitles());

  const cacheText = async (id: number) => {
    const result = await TextController.getText(id, "GR");
    const textData = result?.message[0] as unknown as TextType;

    if (textData) {
      addToCache(id, textData);
    }
    return textData;
  };

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
                  class={() => item.id === selectedTextId() ? textListItemStyles.selected : ""}
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
          <TextModal selectedTextId={selectedTextId()} text={selectedTextData()} />
        </ReaderModal>
      </div>
    </TextContext.Provider>
  );
};

export default Reader;
