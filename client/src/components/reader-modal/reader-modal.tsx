import { Component, JSX } from "solid-js";
import Header from "~/components/header";
import styles from "./reader-modal.module.css";

interface ReaderModalProps {
  children: JSX.Element;
}

const ReaderModal: Component<ReaderModalProps> = (props) => {
  const { children } = props;

  return (
    <div class={styles.reader_modal}>
      <Header />
      {children}
    </div>
  );
};

export default ReaderModal;
