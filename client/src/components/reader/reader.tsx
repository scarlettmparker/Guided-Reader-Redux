import { Component, createEffect, createSignal, For } from "solid-js";
import { createResource } from "solid-js";
import { TitlesController, TextController } from "~/utils/api";
import { TextContext } from "~/contexts/text-context";
import { TextType } from "~/types";
import { shouldFetchText } from "~/utils/text";

import { LoadingState, ErrorState } from "~/components/state";
import ReaderModal from "~/components/reader-modal";
import TextList from "~/components/text-list";
import TextModal from "~/components/text-modal";
import TextListItem from "~/components/text-list-item";

import styles from './reader.module.css';

const Reader: Component = () => {
  const [selectedTextId, setSelectedTextId] = createSignal<number | null>(null);
  const [hoveredTextId, setHoveredTextId] = createSignal<number | null>(null);
  const [selectedTextData, setSelectedTextData] = createSignal<TextType | undefined>(undefined);
  
  const [titles] = createResource(() => TitlesController.getTitles());
  const [text] = createResource(hoveredTextId, (id) => 
    shouldFetchText(id) ? TextController.getText(id, "GR") : undefined
  );

  // Update selected text data when text resource changes and IDs match
  createEffect(() => {
    const currentText = text()?.message[0] as TextType | undefined;
    if (hoveredTextId() === selectedTextId() && currentText) {
      setSelectedTextData(currentText);
    }
  });

  const titleData = () => titles()?.message;

  return (
    <TextContext.Provider value={{ setSelectedTextId }}>
      <div class={styles.reader}>
        <TextList>
          {titles.loading ? (
            <LoadingState>Loading...</LoadingState>
          ) : titles.error ? (
            <ErrorState>Error: {titles.error.message}</ErrorState>
          ) : titleData() ? (
            <For each={titleData()}>
              {(textListItem) => (
                <TextListItem 
                  onClick={() => setSelectedTextId(textListItem.id)}
                  onMouseOver={() => setHoveredTextId(textListItem.id)}
                >
                  {textListItem.title}
                </TextListItem>
              )}
            </For>
          ) : null}
        </TextList>
        <ReaderModal>
          <TextModal
            selectedTextId={selectedTextId()}
            text={selectedTextData()}
          />
        </ReaderModal>
      </div>
    </TextContext.Provider>
  );
};

export default Reader;