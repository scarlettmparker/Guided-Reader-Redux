import { Component, createSignal } from "solid-js";

import ReaderModal from "~/components/reader-modal";
import TextList from "~/components/text-list";
import TextModal from "~/components/text-modal";
import TextListItems from "~/components/text-list/text-list-items";
import styles from './reader.module.css';

const Reader: Component = () => {
  const [selectedTextId, setSelectedTextId] = createSignal<number | null>(null);

  return (
    <div class={styles.reader}>
      <TextList>
        {(mockData) => (
          <TextListItems 
            mockData={mockData} 
            onSelect={setSelectedTextId} 
          />
        )}
      </TextList>
      <ReaderModal>
        <TextModal selectedTextId={selectedTextId()} />
      </ReaderModal>
    </div>
  );
};

export default Reader;