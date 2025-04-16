import { Component, createSignal, For } from "solid-js";
import { createResource } from "solid-js";
import { TitlesController } from "~/utils/api";
import { TextContext } from "~/contexts/text-context";

import ReaderModal from "~/components/reader-modal";
import TextList from "~/components/text-list";
import TextModal from "~/components/text-modal";
import styles from './reader.module.css';

import { LoadingState, ErrorState } from "~/components/state";
import { TextListContent } from "../text-list-modal";

const Reader: Component = () => {
  const [selectedTextId, setSelectedTextId] = createSignal<number | null>(null);
  const [titles] = createResource(() => TitlesController.getTitles());

  // Extract the message from the titles response
  const titleData = () => (
    titles() && titles()!.message
  );

  return (
    <TextContext.Provider value={{ setSelectedTextId }}>
      <div class={styles.reader}>
        <TextList>
          {titles.loading ? (
            <LoadingState>Loading...</LoadingState>
          ) : titles.error ? (
            <ErrorState>Error: {titles.error.message}</ErrorState>
          ) : titleData() ? (
            <TextListContent
              texts={titleData()!}
            />
          ) : null}
        </TextList>
        <ReaderModal>
          <TextModal
          selectedTextId={selectedTextId()}
        />
        </ReaderModal>
      </div>
    </TextContext.Provider>
  );
};

export default Reader;