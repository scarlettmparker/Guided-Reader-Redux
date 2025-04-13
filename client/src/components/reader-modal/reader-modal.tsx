import { Component } from "solid-js";
import TextModal from "~/components/text-modal";
import Header from "~/components/header";
import styles from "./reader-modal.module.css";

const ReaderModal: Component = () => {
  return (
    <div class={styles.reader_modal}>
      <Header />
      <TextModal />
    </div>
  );
};

export default ReaderModal;