import { Component } from "solid-js";

import ReaderModal from "~/components/reader-modal";
import TextList from "~/components/text-list";
import styles from './reader.module.css';

const Reader: Component = () => {
  return (
    <div class={styles.reader}>
      <TextList />
      <ReaderModal />
    </div>
  );
};

export default Reader;