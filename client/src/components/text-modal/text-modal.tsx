import { Component } from "solid-js";
import styles from './text-modal.module.css';

interface TextModalProps {
  selectedTextId: number | null;
}

const TextModal: Component<TextModalProps> = (props) => {
  return (
    <div class={styles.text_modal}>
      {props.selectedTextId !== null ? (
        <span>Selected Text ID: {props.selectedTextId}</span>
      ) : (
        <span>Select a text to begin</span>
      )}
    </div>
  )
};

export default TextModal;